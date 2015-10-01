/*
*	TFTP server     File: cmd_tftpServer.c
*/

#include <cmd_tftpServer.h>
#include <configs/rt2880.h>
#include <spi_api.h>
#include <nand_api.h>
#include <malloc.h>
#include "../autoconf.h"

#ifdef CFG_DIRECT_FLASH_TFTP
extern flash_info_t flash_info[CFG_MAX_FLASH_BANKS];/* info for FLASH chips   */
#endif

static int	TftpServerPort;		/* The UDP port at their end		*/
static int	TftpOurPort;		/* The UDP port at our end		*/
static int	TftpTimeoutCount;
static ulong	TftpBlock;		/* packet sequence number		*/
static ulong	TftpLastBlock;		/* last packet sequence number received */
static ulong	TftpBlockWrapOffset;	/* memory offset due to wrapping	*/
static int	TftpState;

static uchar asuslink[] = "ASUSSPACELINK";
static uchar maclink[] = "snxxxxxxxxxxx";

static unsigned char *image_ptr;
static uint32_t image_len;
static uint16_t RescueAckFlag;

extern IPaddr_t TempServerIP;
extern image_header_t header;
extern int do_bootm(cmd_tbl_t *, int, int, char *[]);
extern int do_reset(cmd_tbl_t *, int, int, char *[]);
extern int verify_kernel_image(ulong, ulong *, ulong *, ulong *);
extern int flash_kernel_image(ulong image_ptr, ulong image_size);
extern int flash_kernel_image_from_usb(cmd_tbl_t *cmdtp);
extern int reset_to_default(void);
extern void perform_system_reset(void);

#if (CONFIG_COMMANDS & CFG_CMD_TFTPSERVER)

int do_tftpd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	const int press_times = 1;
	int i = 0;

	if (DETECT_BTN_RESET())		/* RESET button */
	{
		printf(" \n## Enter to Rescue Mode (%s) ##\n", "manual");
		setenv("autostart", "no");
		
		LED_ALERT_ON();
		
#if defined (RALINK_USB) || defined (MTK_USB)
		if (flash_kernel_image_from_usb(cmdtp) == 0) {
			perform_system_reset();
			return 0;
		}
#endif
		/* Wait forever for an image */
		NetLoop(TFTPD);
		perform_system_reset();
	}
	else if (DETECT_BTN_WPS())	/* WPS button */
	{
		/* Make sure WPS button is pressed at least press_times * 0.01s. */
		while (DETECT_BTN_WPS() && i++ < press_times) {
			udelay(10000);
		}
		
		if (i >= press_times) {
			while (DETECT_BTN_WPS()) {
				LED_ALERT_BLINK();
				udelay(90000);
			}
			LED_ALERT_OFF();
			reset_to_default();
			perform_system_reset();
		}
	}
	else
	{
		ulong addr_src;
		
		if (argc < 2) {
			addr_src = load_addr;
		} else {
			addr_src = simple_strtoul(argv[1], NULL, 16);
		}
		
		memset(&header, 0, sizeof(header));
		if (verify_kernel_image(addr_src, NULL, NULL, NULL) <= 0) {
			printf(" \n## Enter to Rescue Mode (%s) ##\n", "image error");
			
			LED_ALERT_ON();
			
			/* Wait forever for an image */
			NetLoop(TFTPD);
			perform_system_reset();
		}
		
		LED_POWER_ON();
		gpio_init_usb(0);
		do_bootm(cmdtp, 0, argc, argv);
	}

	return 0;
}

U_BOOT_CMD(
	tftpd, 1, 1, do_tftpd,
	"tftpd\t -load the data by tftp protocol\n",
	NULL
);

static void TftpdSend(void)
{
	volatile uchar *pkt;
	volatile uchar *xp;
	volatile ushort *s;
	int	len = 0;
	uint32_t offset = 0;

	/*
	*	We will always be sending some sort of packet, so
	*	cobble together the packet headers now.
	*/
	pkt = NetTxPacket + NetEthHdrSize() + IP_HDR_SIZE;
	
	switch (TftpState) 
	{
	case STATE_RRQ:
		xp = pkt;
		s = (ushort*)pkt;
		*s++ = htons(TFTP_DATA);
		*s++ = htons(TftpBlock);/*fullfill the data part*/
		pkt = (uchar*)s;
		len = pkt - xp;
		break;
		
	case STATE_WRQ:
		xp = pkt;
		s = (ushort*)pkt;
		*s++ = htons(TFTP_ACK);
		*s++ = htons(TftpBlock);
		pkt = (uchar*)s;
		len = pkt - xp;
		break;
		
#ifdef ET_DEBUG
		printf("send option \"timeout %s\"\n", (char *)pkt);
#endif
		pkt += strlen((char *)pkt) + 1;
		len = pkt - xp;
		break;
		
	case STATE_DATA:
		xp = pkt;
		s = (ushort*)pkt;
		*s++ = htons(TFTP_ACK);
		*s++ = htons(TftpBlock);
		pkt = (uchar*)s;
		len = pkt - xp;
		break;
		
	case STATE_FINISHACK:
		xp = pkt;
		s = (ushort*)pkt;
		*s++ = htons(TFTP_FINISHACK);
		*s++ = htons(RescueAckFlag);
		pkt = (uchar*)s;
		len = pkt - xp;
		break;
		
	case STATE_TOO_LARGE:
		xp = pkt;
		s = (ushort*)pkt;
		*s++ = htons(TFTP_ERROR);
		*s++ = htons(3);
		pkt = (uchar*)s;
		strcpy((char *)pkt, "File too large");
		pkt += 14 /*strlen("File too large")*/ + 1;
		len = pkt - xp;
		break;
		
	case STATE_BAD_MAGIC:
		xp = pkt;
		s = (ushort*)pkt;
		*s++ = htons(TFTP_ERROR);
		*s++ = htons(2);
		pkt = (uchar*)s;
		strcpy((char *)pkt, "File has bad magic");
		pkt += 18 /*strlen("File has bad magic")*/ + 1;
		len = pkt - xp;
		break;
	}
	
	NetSendUDPPacket(NetServerEther, NetServerIP, TftpServerPort, TftpOurPort, len);
}

static void TftpTimeout(void)
{
	puts("T ");
	NetSetTimeout(TIMEOUT * CFG_HZ, TftpTimeout);
	TftpdSend();
}

static void TftpdTimeout(void)
{
	puts("D ");
	NetSetTimeout(TIMEOUT * CFG_HZ, TftpdTimeout);
}

static void TftpdFinish(void)
{
	int i;

	for (i=0; i<6; i++)
		TftpdSend();
}

static void TftpHandler(uchar * pkt, unsigned dest, unsigned src, unsigned len)
{
	ushort proto;
	uint32_t offset;
	volatile ushort *s;
	int i;

	if (dest != TftpOurPort) 
	{
		return;
	}

	/* don't care the packets that donot send to TFTP port */
	if (TftpState != STATE_RRQ && src != TftpServerPort)
	{
		return;
	}

	if (len < 2)
	{
		return;
	}

	len -= 2;
	/* warning: don't use increment (++) in ntohs() macros!! */
	s = (ushort*)pkt;
	proto = *s++;
	pkt = (uchar*)s;

	switch (ntohs(proto))
	{
	case TFTP_RRQ:

		printf("\n Get read request from:(");
		print_IPaddr(TempServerIP);
		printf(")\n");
		NetCopyIP(&NetServerIP,&TempServerIP);

		TftpServerPort = src;
		TftpBlock = 1;
		TftpBlockWrapOffset = 0;
		TftpState = STATE_RRQ;

		for (i=0; i<13; i++) 
		{
			if (*pkt++ != asuslink[i])
				break;
		}
		if (i==13)
		{ /* it's the firmware transmitting situation */
			/* here get the IP address from the first packet. */
			NetOurIP = (*pkt++) & 0x000000ff;
			NetOurIP<<=8;
			NetOurIP|= (*pkt++) & 0x000000ff;
			NetOurIP<<=8;
			NetOurIP|= (*pkt++) & 0x000000ff;
			NetOurIP<<=8;
			NetOurIP|= (*pkt++) & 0x000000ff;
		}
		else
		{
			for (i=0; i<13; i++)
			{
				if (*pkt++ != maclink[i])
					break;
			}
			if(i==13)
			{
				/* here get the IP address from the first packet. */
				NetOurIP = (*pkt++) & 0x000000ff;
				NetOurIP<<=8;
				NetOurIP|= (*pkt++) & 0x000000ff;
				NetOurIP<<=8;
				NetOurIP|= (*pkt++) & 0x000000ff;
				NetOurIP<<=8;
				NetOurIP|= (*pkt++) & 0x000000ff;
			}
		}

		TftpdSend();//send a vacant Data packet as a ACK
		break;
		
	case TFTP_WRQ:
		TftpServerPort = src;
		TftpBlock = 0;
		TftpState = STATE_WRQ;
		TftpdSend();
		break;
		
	case TFTP_DATA:
		if (len < 2)
			return;
		len -= 2;
		TftpBlock = ntohs(*(ushort *)pkt);
		/*
		* RFC1350 specifies that the first data packet will
		* have sequence number 1. If we receive a sequence
		* number of 0 this means that there was a wrap
		* around of the (16 bit) counter.
		*/
		if (TftpBlock == 0)
		{
			printf("\n\t %lu MB received\n\t ", TftpBlockWrapOffset>>20);
		} 
		else 
		{
			if (((TftpBlock - 1) % 10) == 0) 
			{/* print out progress bar */
				puts("#");
			} 
			else
				if ((TftpBlock % (10 * HASHES_PER_LINE)) == 0)
				{
					puts("\n");
				}
		}
		if (TftpState == STATE_WRQ)
		{		
			/* first block received */
			TftpState = STATE_DATA;
			TftpServerPort = src;
			TftpLastBlock = 0;
			TftpBlockWrapOffset = 0;
			
			if (TftpBlock != 1)
			{	/* Assertion */
				printf("\nTFTP error: "
					"First block is not block 1 (%ld)\n"
					"Starting again\n\n",
					TftpBlock);
				NetStartAgain();
				break;
			}
		}
		
		if (TftpBlock == TftpLastBlock)
		{	/* Same block again; ignore it. */
			printf("\n Same block again; ignore it \n"); 
			break;
		}
		TftpLastBlock = TftpBlock;
		NetSetTimeout(TIMEOUT * CFG_HZ, TftpTimeout);
		
		offset = (TftpBlock - 1) * TFTP_BLOCK_SIZE;
		image_len = offset + len;/* the total length of the data */

		(void)memcpy((void *)(image_ptr + offset), pkt+2, len);/* store the data part to RAM */
		
		/*
		*	Acknowledge the block just received, which will prompt
		*	the Server for the next one.
		*/
		TftpdSend();
		
		if (len < TFTP_BLOCK_SIZE)
		{
			int rrc_code;
		/*
		*	We received the whole thing.  Try to run it.
		*/
			puts("\ndone\n");
			flush_cache((ulong)image_ptr, image_len);
			rrc_code = flash_kernel_image((ulong)image_ptr, image_len);
			RescueAckFlag = (rrc_code) ? 0x0000 : 0x0001;
			TftpState = STATE_FINISHACK;
			TftpdFinish();
			NetState = NETLOOP_SUCCESS;
		}
		break;
		
	case TFTP_ERROR:
		printf("\nTFTP error: '%s' (%d)\n",
			pkt + 2, ntohs(*(ushort *)pkt));
		puts("Starting again\n\n");
		NetStartAgain();
		break;
		
	default:
		break;
	}
}

void TftpdStart(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	ulong addr = CFG_LOAD_ADDR;
	image_ptr = (unsigned char*)addr;
	image_len = 0;

	RescueAckFlag = 0x0000;

	/* clear image magic */
	memset(image_ptr, 0, sizeof(image_header_t));

#if defined(CONFIG_NET_MULTI)
	printf("Using %s device\n", eth_get_name());
#endif
	puts("\nOur IP address is:(");	
	print_IPaddr(NetOurIP);
	puts(")\nWait for TFTP request...\n");
	/* Check if we need to send across this subnet */
	if (NetOurGatewayIP && NetOurSubnetMask) 
	{
		IPaddr_t OurNet 	= NetOurIP    & NetOurSubnetMask;
		IPaddr_t ServerNet 	= NetServerIP & NetOurSubnetMask;
		
		if (OurNet != ServerNet)
		{
			puts("; sending through gateway ");
			print_IPaddr(NetOurGatewayIP) ;
		}
	}

	NetSetTimeout(TIMEOUT * CFG_HZ * 2, TftpdTimeout);
	NetSetHandler(TftpHandler);

	TftpOurPort = PORT_TFTP;
	TftpTimeoutCount = 0;
	TftpState = STATE_RRQ;
	TftpBlock = 0;

	/* zero out server ether in case the server ip has changed */
	memset(NetServerEther, 0, 6);
}

#endif
