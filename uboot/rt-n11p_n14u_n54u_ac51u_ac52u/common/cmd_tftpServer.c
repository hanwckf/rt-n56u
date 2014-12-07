/*
*	TFTP server     File: cmd_tftpServer.c
*/

#include <cmd_tftpServer.h>
#include <configs/rt2880.h>
#include <spi_api.h>
#include <nand_api.h>
#include <malloc.h>
#include "../autoconf.h"

#if defined(UBI_SUPPORT)
#include "../drivers/mtd/ubi/ubi-media.h"
#endif

#ifdef CFG_DIRECT_FLASH_TFTP
extern flash_info_t flash_info[CFG_MAX_FLASH_BANKS];/* info for FLASH chips   */
#endif

#if defined(UBOOT_STAGE1)
struct stage2_loc g_s2_loc;
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
#define PTR_SIZE	0x800000	// 0x390000
#define BOOTBUF_SIZE	0x30000
uchar MAC0[13], RDOM[7], ASUS[24], nvramcmd[60];
unsigned char *ptr;
uint16_t RescueAckFlag = 0;
uint32_t copysize = 0;
uint32_t offset = 0;
int rc = 0;
int MAC_FLAG = 0;
int env_loc = 0;

static void _tftpd_open(void);
static void TftpHandler(uchar * pkt, unsigned dest, unsigned src, unsigned len);
static void RAMtoFlash(void);
#if !defined(UBOOT_STAGE1)
static void SolveTRX(void);
#endif
extern image_header_t header;
extern int do_bootm(cmd_tbl_t *, int, int, char *[]);
extern int do_reset(cmd_tbl_t *, int, int, char *[]);

extern IPaddr_t TempServerIP;

#if (CONFIG_COMMANDS & CFG_CMD_TFTPSERVER)
int check_trx(int argc, char *argv[])
{
	extern int verify_kernel_image(int, char *[], ulong *, ulong *, ulong *);
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
#if ! defined(UBOOT_STAGE1)
	const int press_times = 1;
	int i = 0;

	if(DETECT())		/* Reset to default button */
	{
		printf(" \n## Enter Rescue Mode ##\n");		
		setenv("autostart", "no");
		/* Wait forever for an image */
		if (NetLoop(TFTPD) < 0) 
			return 1;
	}
	else if (DETECT_WPS())	/* WPS button */
	{
		/* Make sure WPS button is pressed at least press_times * 0.01s. */
		while (DETECT_WPS() && i++ < press_times) {
			udelay(10000);
		}

		if (i >= press_times) {
			while (DETECT_WPS()) {
				udelay(90000);
				i++;
				if (i & 1)
					LEDON();
				else
					LEDOFF();
			}
			LEDOFF();

			reset_to_default();
			do_reset (NULL, 0, 0, NULL);
		}
	}
	else
#endif /* ! UBOOT_STAGE1 */
	{
#if defined(UBOOT_STAGE1)
		int i, ret, space, nr_good = 0;
		char addr1[] = "ffffffffXXX";
		char *boot_argv[] = { NULL, addr1 };
		struct stage2_loc *s2 = &g_s2_loc;
		struct stage2_desc *desc;

		if ((ret = ranand_check_and_fix_stage1())) {
			char buf[16], *p;

			printf("check & fix stage1 fail!!! (ret %d)\n", ret);
			p = getenv("reprog_stage1_status");
			if (!p || simple_strtol(p, NULL, 10) != ret) {
				sprintf(buf, "%d", ret);
				setenv("reprog_stage1_status", buf);
				saveenv();
			}
		}

		ranand_locate_stage2(s2);
		/* check and fix every stage2 if necessary/possible */
		for (i = 0, desc = &s2->desc[0];
		     i < s2->count && s2->good;
		     ++i, ++desc)
		{
			int ret, wlen = s2->good->blk_len;
			unsigned char *ptr = s2->good->code;
			image_header_t *hdr;

			if (!desc->crc_error) {
				nr_good++;
				if (!desc->corrected && !desc->failed)
					continue;

				ptr = desc->code;
				wlen = desc->blk_len;
			}

			hdr = (image_header_t*) ptr;
			if (!ranand_check_space(desc->offset, wlen, desc->boundary)) {
				printf("No space to program %x bytes stage2 code to %s@%x-%x\n",
					wlen, desc->name, desc->offset, desc->boundary);
				continue;
			}
			ranand_set_sbb_max_addr(desc->boundary);
			printf("Reprogram %s@%x-%x (source %p, len %x): ",
				desc->name, desc->offset, desc->boundary, ptr, wlen);
			ret = ranand_erase_write(ptr, desc->offset, wlen);
			if (ret != wlen) {
				printf("fail. (ret %d)\n", ret);
				continue;
			}
			nr_good++;
		}
		/* if it is possible to obtain more copy of stage2 code, reprogram whole stage2 area */
		if (s2->good) {
			space = ranand_check_space(CFG_BOOTSTAGE2_OFFSET, s2->good->blk_len, CFG_BOOTLOADER_SIZE);
			if (nr_good < (space / s2->good->blk_len)) {
				debug("Reprogram stage2 area to obtain more copy of stage2 code.\n");
				ranand_write_stage2((unsigned int) s2->good->code, s2->good->len);
				ranand_locate_stage2(s2);
			}
		}
		ranand_set_sbb_max_addr(0);

		if (!s2->good) {
			printf(" \nHello!! Enter Recuse Mode: (Check error)\n\n");
			if (NetLoop(TFTPD) < 0)
				return 1;
		}

		sprintf(addr1, "%x", s2->good->code);
		do_bootm(cmdtp, 0, ARRAY_SIZE(boot_argv), boot_argv);
#elif defined(DUAL_TRX)
		int r;
		int trx1_len = -1, trx2_len = -1;
		unsigned int src = 0, dest = 0, len = 0;
		/* In most cases, we use CFG_LOAD_ADDR to load firmware without heading image header.
		 * If we want to use same area to read good firmware and overwrite bad one, we should subtract
		 * heading image header size from address.  If not, you have to read firmware from flash again.
		 */
		unsigned char *buf_addr = (unsigned char*)(CFG_LOAD_ADDR - sizeof(image_header_t));
		char addr1[] = "ffffffffXXX", addr2[] = "ffffffffXXX";
		char *load_trx1[] = { NULL, addr1 };
		char *load_trx2[] = { NULL, addr2 };
		char *boot_argv[] = { NULL, addr1 };

		sprintf(addr1, "%x", CFG_KERN_ADDR);
		sprintf(addr2, "%x", CFG_KERN2_ADDR);
		trx2_len = check_trx(ARRAY_SIZE(load_trx2), load_trx2);
		trx1_len = check_trx(ARRAY_SIZE(load_trx1), load_trx1);

		if (trx1_len <= CFG_BOOTLOADER_SIZE)
			trx1_len = 0;
		if (trx2_len <= CFG_BOOTLOADER_SIZE)
			trx2_len = 0;

		if (trx1_len > 0 && trx2_len <= 0) {
			/* 1st fw good; 2nd fw damaged. Fix 2nd fw and boot from 1st fw. */
			boot_argv[1] = addr1;
			src  = CFG_KERN_ADDR;
			dest = CFG_KERN2_ADDR;
			len  = trx1_len;
		}
		else if (trx1_len <= 0 && trx2_len > 0) {
			/* 1st fw damaged; 2nd fw good. Fix 1st fw and boot from 1st fw. */
			boot_argv[1] = addr1;
			src  = CFG_KERN2_ADDR;
			dest = CFG_KERN_ADDR;
			len  = trx2_len;
		}
		else if (trx1_len <= 0 && trx2_len <= 0) {
			/* Both firmware damaged. enter rescue mode */
			printf(" \nHello!! Enter Recuse Mode: (Check error)\n\n");
			if (NetLoop(TFTPD) < 0)
				return 1;
		}
		else {
			/* Both firmware are good. Boot from 1st fw. */
			boot_argv[1] = addr1;
		}

		if (src && dest && len) {
			printf("Copy firmware from %x to %x, length %x\n", src, dest, len);
			r = ra_flash_read(buf_addr, src, len);
			if (r) {
				printf("Read firmware from %x, length %x fail. (len %x, r = %d)\n", src, len, r);
			} else {
				r = ra_flash_erase_write(buf_addr, dest, len, 0);
				if (r)
					printf("Write firmware to %x, length %x fail. (r = %d)\n", dest, len, r);
			}

			/* If we cannot recover 1st firmware for some reason,
			 * boot from 2nd firmware instead.
			 */
			if (r && dest == CFG_KERN_ADDR) {
				printf("Switch to 2nd firmware!\n");
				boot_argv[1] = addr2;
			}
		}

		do_bootm(cmdtp, 0, ARRAY_SIZE(boot_argv), boot_argv);
#else
		if (check_trx(argc, argv) <= 0)
		{
			printf(" \nHello!! Enter Recuse Mode: (Check error)\n\n");
			if (NetLoop(TFTPD) < 0) 
				return 1;
		}
		
		do_bootm(cmdtp, 0, argc, argv);
#endif	/* DUAL_TRX */
	}
	
	return 0;
}
	
	
	
void TftpdStart(void)
{
	ptr = (unsigned char*) CFG_LOAD_ADDR;

#if defined(CONFIG_NET_MULTI)
	printf("Using %s device\n", eth_get_name());
#endif
	//puts(" \nTFTP from server ");	print_IPaddr(NetServerIP);
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

	printf("tftpd open\n");	// tmp test
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
		//printf("case TFTPDATA\n");	// tmp test
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
			printf("\n\t %lu MB reveived\n\t ", TftpBlockWrapOffset>>20);
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
			printf("\n First block received  \n");
			//printf("Load Addr is %x\n", ptr);	// tmp test
			//ptr = 0x80100000;
			
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
			RAMtoFlash();
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


void flash_perrormsg (int err)
{
	switch (err) {
	case ERR_OK:
		break;
	case ERR_TIMOUT:
		puts ("Timeout writing to Flash\n");
		break;
	case ERR_NOT_ERASED:
		puts ("Flash not Erased\n");
		break;
	case ERR_PROTECTED:
		puts ("Can't write to protected Flash sectors\n");
		break;
	case ERR_INVAL:
		puts ("Outside available Flash\n");
		break;
	case ERR_ALIGN:
		puts ("Start and/or end address not on sector boundary\n");
		break;
	case ERR_UNKNOWN_FLASH_VENDOR:
		puts ("Unknown Vendor of Flash\n");
		break;
	case ERR_UNKNOWN_FLASH_TYPE:
		puts ("Unknown Type of Flash\n");
		break;
	case ERR_PROG_ERROR:
		puts ("General Flash Programming Error\n");
		break;
	default:
		printf ("%s[%d] FIXME: rc=%d\n", __FILE__, __LINE__, err);
		break;
	}
}

#if defined(UBI_SUPPORT)
/* Program UBI image with or without OOB information to UBI_DEV.  */
int SolveUBI(void)
{
	printf("Solve UBI, ptr=0x%x\n", ptr);
	return __SolveUBI(ptr, CFG_UBI_DEV_OFFSET, copysize);
}
#endif

static void RAMtoFlash(void)
{
	int i=0, parseflag=0;
#if !defined(UBOOT_STAGE1)
	int j=0;
	uchar mac_temp[7],secretcode[9];
	uint8_t SCODE[5] = {0x53, 0x43, 0x4F, 0x44, 0x45};
	char *end ,*tmp;
#endif

	printf("RAMtoFLASH\n");	// tmp test
    /* Check for the TRX magic. If it's a TRX image, then proceed to flash it. */
	if ((*(unsigned long *) (ptr) == TRX_MAGIC) && (copysize > CFG_MAX_BOOTLOADER_BINARY_SIZE))
	{
#if defined(UBOOT_STAGE1)
		printf("not support Firmware in Uboot Stage1 !!!\n");
		rc = 1;	/* send fail response to firmware restoration utility */
#else
		printf("Chk trx magic\n");	// tmp test
		printf("Download of 0x%x bytes completed\n", copysize);
		printf("Check TRX and write it to FLASH \n");
		SolveTRX();
#endif /* UBOOT_STAGE1 */
	}
#if defined(UBI_SUPPORT)
	else if (be32_to_cpu(*(uint32_t *)ptr) == UBI_EC_HDR_MAGIC) {
		int r;

		printf("Check UBI magic\n");
		printf("Download of 0x%x bytes completed\n", copysize);
		if ((r = SolveUBI()) != 0) {
			printf("rescue UBI image failed! (r = %d)\n", r);
			NetState = NETLOOP_FAIL;
			TftpState = STATE_FINISHACK;
			RescueAckFlag = 0x0000;
			for (i = 0; i < 6; i++)
				TftpdSend();
			return;
		} else {
			printf("done. %d bytes written\n", copysize);
			TftpState = STATE_FINISHACK;
			RescueAckFlag = 0x0001;
			for (i = 0; i < 6; i++)
				TftpdSend();
		}

		printf("SYSTEM RESET!!!\n\n");
		do_reset (NULL, 0, 0, NULL);
	}
#endif
	else
	{
		const char __attribute__((aligned(4))) nvram_magic[4];
		const char *p = &nvram_magic[0], *q;
		unsigned int magic_offset = 0x2FFF0;

		if (CFG_MAX_BOOTLOADER_BINARY_SIZE > 0x30000)
			magic_offset = CFG_MAX_BOOTLOADER_BINARY_SIZE - 7;

		*((unsigned long*)p) = NVRAM_MAGIC;	/* little-endian only */
		q = ptr + magic_offset;
		printf("upgrade boot code\n");	// tmp test

#if defined(UBOOT_STAGE1) || defined(UBOOT_STAGE2)
		if (copysize > 0 && copysize <= CFG_BOOTSTAGE1_SIZE && *(unsigned long *) (ptr) == TRX_MAGIC)	// uboot Stage1
		{
			ulong addr = (ulong)ptr, len = copysize;

			printf(" Download of 0x%x bytes completed\n", copysize);
			printf("Write boot Stage1 binary to FLASH \n");
			ranand_set_sbb_max_addr(0);
			rc = ra_flash_erase_write((uchar*) addr, CFG_FLASH_BASE, len, 1);
			parseflag = 5;	//avoid to write
		}
		else
#endif
		if (copysize > 0 && copysize <= CFG_MAX_BOOTLOADER_BINARY_SIZE)		// uboot
		{
			/* ptr + magic_offset may not aligned to 4-bytes boundary.
			 * use char pointer to access it instead.
			 */
			if (copysize >= (magic_offset+4) && (p[0] == q[0] && p[1] == q[1] && p[2] == q[2] && p[3] == q[3])
				&& (*(unsigned long *) (ptr) != NVRAM_MAGIC_MAC0)
				&& (*(unsigned long *) (ptr) != NVRAM_MAGIC_MAC1)
				&& (*(unsigned long *) (ptr) != NVRAM_MAGIC_RDOM )
				&& (*(unsigned long *) (ptr) != NVRAM_MAGIC_ASUS ))
			{
				printf(" Download of 0x%x bytes completed\n", copysize);
				printf("Write bootloader binary to FLASH \n");
				parseflag = 1;
			}
#if ! defined(UBOOT_STAGE1)
			else if( *(unsigned long *) (ptr) == NVRAM_MAGIC_MAC0 )
			{
				printf("  Download of 0x%x bytes completed\n", copysize);
				for (i=0; i<17; i++)
					MAC0[i] = ptr[4+i];
				MAC0[i]='\0';
				
				i = i + 4;
				/* Debug message */
				if ((ptr[i]==SCODE[0]) 
					&&(ptr[i+1]== SCODE[1]) 
					&&(ptr[i+2]== SCODE[2]) 
					&&(ptr[i+3]== SCODE[3]) 
					&&(ptr[i+4]== SCODE[4]))
				{
					for (i=26,j=0; i<34; i++) 
					{
						secretcode[j] = ptr[i];
						printf("secretcode[%d]=%x, ptr[%d]=%x\n", j, secretcode[j], i, ptr[i]);
						j++;
					}
					secretcode[j]='\0';
					printf("Write secret code = %s to FLASH\n", secretcode);
					rc = replace(0x100, secretcode, 9);

				}
				tmp=MAC0;
				for (i=0; i<6; i++)
				{
					mac_temp[i] = tmp ? simple_strtoul(tmp, &end, 16) : 0;
					if (tmp)
						tmp= (*end) ? end+1 : end;
				}
				mac_temp[i] = '\0';
				
				printf("Write MAC0 = %s  to FLASH \n", MAC0);
				MAC_FLAG=1;
				rc = replace(0x4, mac_temp, 7);
				sprintf(nvramcmd, "nvram set et0macaddr=%s", MAC0);
				printf("%s\n", nvramcmd);
				
				parseflag = 2;
			}
			else if ( *(unsigned long *) (ptr) == NVRAM_MAGIC_RDOM ) 
			{
				for (i=0; i<6; i++)
					RDOM[i] = ptr[8+i];
				RDOM[i] = '\0';
				rc = replace(0x4e, RDOM, 7);
				printf("Write RDOM = %s  to FLASH \n", RDOM);
                		sprintf(nvramcmd, "nvram set regulation_domain=%s", RDOM);
				printf("%s\n", nvramcmd);
				parseflag = 3;
			}
			else if (*(unsigned long *)(ptr) == NVRAM_MAGIC_ASUS) 
			{
				for (i=0; i<23; i++)
					ASUS[i] = ptr[4+i];
				ASUS[i] = '\0';
				for (i=0; i<17; i++)
					MAC0[i] = ASUS[i];
				MAC0[i]  = '\0';
				rc = replace(0x4, mac_temp, 7);
				for (i=0; i<6; i++)
					RDOM[i] = ASUS[17+i];
				RDOM[i] = '\0';
				printf("Write MAC0 = %s  to FLASH \n", MAC0);
				sprintf(nvramcmd, "nvram set et0macaddr=%s", MAC0);
				printf("%s\n", nvramcmd);
				rc = replace(0x4e, RDOM, 7);
				printf("Write RDOM = %s  to FLASH \n", RDOM);
				sprintf(nvramcmd, "nvram set regulation_domain=%s", RDOM);
				printf("%s\n", nvramcmd);
				parseflag = 4;
			}
#endif	/* ! UBOOT_STAGE1 */
			else 
			{
				parseflag = -1;
				printf("Download of 0x%x bytes completed\n", copysize);		
				printf("Warning, Bad Magic word!!\n");
				NetState=STATE_BAD_MAGIC;
				TftpdSend();
				copysize = 0;
			}
			
		}
		else if (copysize > CFG_MAX_BOOTLOADER_BINARY_SIZE)
		{
			parseflag = 0;
			printf("    Download of 0x%x bytes completed\n", copysize);
			printf("Write kernel and filesystem binary to FLASH (0xbfc30000)\n");
		}
		else 
		{
			parseflag = -1;
			copysize = 0;
			printf("Downloading image time out\n");
		}
		
		
		if (copysize == 0) 
			return;    /* 0 bytes, don't flash */
		
		if (parseflag != 0)
			copysize = CFG_MAX_BOOTLOADER_BINARY_SIZE;
		
		if (parseflag == 1)
		{
			char *argv[2];
			char buf[16];

			sprintf(buf, "0x%08x", ptr);
			argv[0] = NULL;
			argv[1] = buf;
			if((rc = check_trx(2, argv)) <= 0)
			{
				printf("# image checksum error #\n");
				parseflag = -1;
			}
		}

		printf("  BootLoader Programming...\n");
		
		if (parseflag == 1)
		{
			ulong addr = (ulong)ptr, len = copysize;

#if defined(CFG_ENV_IS_IN_NAND)
			/* NAND boot use UIMAGE. Don't skip UIMAGE header. */
#elif defined(CFG_ENV_IS_IN_SPI)
			addr += 0x40;
			len -= 0x40;
#else	/* CFG_ENV_IS_IN_FLASH */
#ifdef ASUS_EAN66
			addr += 0x40;
			len -= 0x40;
#endif
#endif

		        printf("copy imge to sdram, addr = 0x%x len = 0x%x\n", ptr, len);
#if defined(CFG_ENV_IS_IN_NAND) || defined(CFG_ENV_IS_IN_SPI)
#if defined(UBOOT_STAGE1) || defined(UBOOT_STAGE2)
			ranand_write_stage2(addr, len);
			rc = 0;
#else
			ranand_set_sbb_max_addr(CFG_BOOTLOADER_SIZE);
			rc = ra_flash_erase_write((uchar*) addr, CFG_FLASH_BASE, len, 1);
			ranand_set_sbb_max_addr(0);
#endif /* UBOOT_STAGE1 */
#else
#ifdef ASUS_EAN66
			printf("\n Erase Uboot block !!\n");
			rc = ra_flash_erase_write((uchar*) addr, CFG_FLASH_BASE, len, 1);
			printf("@ done @ \n");
#else			
#warning		FIXME: rewrite below code to new flash wrapper function.
			flash_sect_protect(0, 0xBF000000, 0xBF02FFFF);	// disable for tmp
			printf("\n Erase Uboot block !!\n From 0xBF000000 To 0xBF02FFFF\n");
			flash_sect_erase(0xBF000000, 0xBF02FFFF);		// disable for tmp
			rc = flash_write((uchar *)(ptr), (ulong)0xBF000000, copysize);
			flash_sect_protect(1, 0xBF000000, 0xBF02FFFF);	// disable for tmp
#endif
#endif
		}
		
		if (rc)
		{
			printf("rescue failed!\n");
			flash_perrormsg(rc);
			NetState = NETLOOP_FAIL; 
			/* Network loop state */
			TftpState = STATE_FINISHACK;
			RescueAckFlag= 0x0000;
			for (i=0; i<6; i++)
				TftpdSend();
			return;			
		}
		else
		{
			printf("done. %d bytes written\n",copysize);
			TftpState = STATE_FINISHACK;
			RescueAckFlag= 0x0001;
			for (i=0; i<6; i++)
				TftpdSend();
			if (parseflag != 0)
			{
				printf("SYSTEM RESET \n\n");
				//NetCopyIP(&NetOurIP,&((IPaddr_t)(0x0101A8C0)));
				NetOurIP=(IPaddr_t)0x0101A8C0;
				//printf("\nTempOurIP=%x",(ulong)NetOurIP);
				udelay(500);
				do_reset(NULL, 0, 0, NULL);
			}
			return;
		}
	}
}

#if ! defined(UBOOT_STAGE1)
static void SolveTRX(void)
{
	int  i = 0;
	int rrc = 0;
	ulong count = 0;
	int e_end = 0;

	printf("Solve TRX, ptr=0x%x\n", ptr);
	load_addr = (unsigned long)ptr;
	if (check_trx(0, NULL) <= 0)
	{
		printf("Check trx error! SYSTEM RESET!!!\n\n");
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
		
		//puts ("Copy to Flash... ");
		// erase linux
		e_end = CFG_KERN_ADDR + count;
		printf("first e_end is %x\n", e_end);	// tmp test
		#if 0	// disable for tmp
                if (0 != get_addr_boundary(&e_end))
		{
		    printf("get addr boundary err!\n");
                    return;
		}
		#endif

#if defined(CFG_ENV_IS_IN_NAND) || defined(CFG_ENV_IS_IN_SPI)
		printf("\n Erase kernel block !!\n From %x To %x (%d/h:%x)\n",
			CFG_KERN_ADDR, e_end, count, count);
		rc = ra_flash_erase_write(ptr, CFG_KERN_ADDR, count, 0);
#if defined(DUAL_TRX)
		if (rc)
			printf("Write 1st firmware fail. (r = %d)\n");
		rc = ra_flash_erase_write(ptr, CFG_KERN2_ADDR, count, 0);
		if (rc)
			printf("Write 2nd firmware fail. (r = %d)\n");
#else
		if (rc)
			printf("Write firmware fail. (r = %d)\n");
#endif
#else
#ifdef ASUS_EAN66
		printf("Erase linux kernel block !! From 0x%X To 0x%X\n", CFG_KERN_ADDR, e_end);
		rc = ra_flash_erase_write(ptr, CFG_KERN_ADDR, count, 0);
#else		
#warning	FIXME: rewrite below code to new flash wrapper function.
		flash_sect_protect(0, 0xbc450000, 0xbc7fffff);	// disable for tmp

		flash_sect_erase(CFG_KERN_ADDR, e_end);	// disable for tmp
		printf ("\n Copy %d bytes to Flash... ", count);		
		rrc = flash_write((uchar *)ptr, CFG_KERN_ADDR, count);	// disable for tmp

		flash_sect_protect(1, 0xbc450000, 0xbc7fffff);	// disable for tmp
#endif
#endif
		
		if (rrc) 
		{
			printf("rescue failed!\n");
			flash_perrormsg(rrc);
			NetState = NETLOOP_FAIL;
			TftpState = STATE_FINISHACK;
			RescueAckFlag= 0x0000;
			for (i=0; i<6; i++)
				TftpdSend();
			return;
		}
		else
		{
			printf("done. %d bytes written\n", count);
			TftpState = STATE_FINISHACK;
			RescueAckFlag= 0x0001;
			for (i=0; i<6; i++)
				TftpdSend();
			printf("\nRT2880 SYSTEM RESET!!!\n\n");
            udelay(500);
			do_reset(NULL, 0, 0, NULL);
			return;
		}
	}
}
#endif /* ! UBOOT_STAGE1 */

#endif
