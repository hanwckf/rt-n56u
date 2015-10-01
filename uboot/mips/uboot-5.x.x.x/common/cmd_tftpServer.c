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

uchar asuslink[] = "ASUSSPACELINK";
uchar maclink[] = "snxxxxxxxxxxx";

unsigned char *ptr;
uint16_t RescueAckFlag = 0;
uint32_t copysize = 0;
uint32_t offset = 0;
int rc = 0;
int MAC_FLAG = 0;
int env_loc = 0;

static void _tftpd_open(void);
static void TftpHandler(uchar * pkt, unsigned dest, unsigned src, unsigned len);
static void SolveImage(void);
extern image_header_t header;
extern int do_bootm(cmd_tbl_t *, int, int, char *[]);
extern int do_reset(cmd_tbl_t *, int, int, char *[]);
extern int verify_kernel_image(int, char *[], ulong *, ulong *, ulong *);
extern int reset_to_default(void);

extern IPaddr_t TempServerIP;

#if (CONFIG_COMMANDS & CFG_CMD_TFTPSERVER)
int check_image(int argc, char *argv[])
{
	memset(&header, 0, sizeof(header));
	return verify_kernel_image(argc, argv, NULL, NULL, NULL);
}

static void TftpdSend(void)
{
	volatile uchar *pkt;
	volatile uchar *xp;
	volatile ushort *s;
	int	len = 0;
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

U_BOOT_CMD(
	tftpd, 1, 1, do_tftpd,
	"tftpd\t -load the data by tftp protocol\n",
	NULL
);

int do_tftpd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	const int press_times = 1;
	int i = 0;

	if (DETECT_BTN_RESET())		/* Reset to default button */
	{
		printf(" \n## Enter Rescue Mode ##\n");
		setenv("autostart", "no");
		
		LED_ALERT_ON();
		
		/* Wait forever for an image */
		if (NetLoop(TFTPD) < 0) 
			return 1;
	}
	else if (DETECT_BTN_WPS())	/* WPS button */
	{
		/* Make sure WPS button is pressed at least press_times * 0.01s. */
		while (DETECT_BTN_WPS() && i++ < press_times)
		{
			udelay(10000);
		}
		
		if (i >= press_times) {
			while (DETECT_BTN_WPS())
			{
				LED_ALERT_BLINK();
				udelay(90000);
			}
			LED_ALERT_OFF();
			reset_to_default();
			do_reset (NULL, 0, 0, NULL);
		}
	}
	else
	{
		if (check_image(argc, argv) <= 0)
		{
			printf(" \n## Enter Recuse Mode (image error) ##\n");
			if (NetLoop(TFTPD) < 0)
				return 1;
		}
		
		LED_POWER_ON();
		do_bootm(cmdtp, 0, argc, argv);
	}

	return 0;
}

void TftpdStart(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	ulong addr = CFG_LOAD_ADDR;
	ptr = (unsigned char*)addr;

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

	memset(ptr,0,sizeof(ptr));
	_tftpd_open();
}

static void _tftpd_open()
{
	NetSetTimeout(TIMEOUT * CFG_HZ * 2, TftpdTimeout);
	NetSetHandler(TftpHandler);
	
	TftpOurPort = PORT_TFTP;
	TftpTimeoutCount = 0;
	TftpState = STATE_RRQ;
	TftpBlock = 0;
	
	/* zero out server ether in case the server ip has changed */
	memset(NetServerEther, 0, 6);
}

static void TftpHandler(uchar * pkt, unsigned dest, unsigned src, unsigned len)
{
	ushort proto;
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
		copysize = offset + len;/* the total length of the data */

		(void)memcpy((void *)(ptr+ offset), pkt+2, len);/* store the data part to RAM */
		
		/*
		*	Acknowledge the block just received, which will prompt
		*	the Server for the next one.
		*/
		TftpdSend();
		
		if (len < TFTP_BLOCK_SIZE)
		{
		/*
		*	We received the whole thing.  Try to run it.
		*/
			puts("\ndone\n");
			TftpState = STATE_FINISHACK;
			NetState = NETLOOP_SUCCESS;
			SolveImage();
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

static void SolveImage(void)
{
	int  i = 0;
	int rrc = 0;
	ulong count = 0;
	ulong e_end = 0;

	printf("Check image from 0x%x and write it to FLASH \n", ptr);

	load_addr = (unsigned long)ptr;
	if (check_image(0, NULL) <= 0)
	{
		printf("Check image error! SYSTEM RESET!!!\n\n");
		udelay(500);
		do_reset (NULL, 0, 0, NULL);
	}
	else
	{
		count = copysize;

		if (count == 0)
		{
			puts ("Zero length ???\n");
			return;
		}

		e_end = CFG_KERN_ADDR + count - 1;

#if defined (CFG_ENV_IS_IN_NAND)
		printf ("\n Copy %d bytes to Flash... \n", count);
		rrc = ranand_erase_write((uchar *)ptr, CFG_KERN_ADDR-CFG_FLASH_BASE, count);
#elif defined (CFG_ENV_IS_IN_SPI)
		printf ("\n Copy %d bytes to Flash... \n", count);
		rrc = raspi_erase_write((uchar *)ptr,  CFG_KERN_ADDR-CFG_FLASH_BASE, count);
#else /* NOR */
		puts ("Copy to Flash... ");
		if (get_addr_boundary(&e_end) == 0) {
			printf("\n Erase block from 0x%X to 0x%X\n", CFG_KERN_ADDR, e_end);
			flash_sect_protect(0, CFG_KERN_ADDR, e_end);
			flash_sect_erase(CFG_KERN_ADDR, e_end);
			printf ("\n Copy %d bytes to Flash... \n", count);
			rrc = flash_write((uchar *)ptr, CFG_KERN_ADDR, count);
			flash_sect_protect(1, CFG_KERN_ADDR, e_end);
		} else {
			rrc = ERR_ALIGN;
		}
#endif

		if (rrc)
		{
			printf("rescue FAILED!\n");
#if defined (CFG_ENV_IS_IN_NAND) || defined (CFG_ENV_IS_IN_SPI)
			printf("\n Error code: %d\n", rrc);
#else
			flash_perror(rrc);
#endif
			NetState = NETLOOP_FAIL;
			TftpState = STATE_FINISHACK;
			RescueAckFlag = 0x0000;
			for (i=0; i<6; i++)
				TftpdSend();
			return;
		}
		else
		{
			printf("done. %d bytes written\n", count);
			TftpState = STATE_FINISHACK;
			RescueAckFlag = 0x0001;
			for (i=0; i<6; i++)
				TftpdSend();
			printf("\nSYSTEM RESET!!!\n\n");
			udelay(500);
			do_reset(NULL, 0, 0, NULL);
			return;
		}
	}
}

#endif
