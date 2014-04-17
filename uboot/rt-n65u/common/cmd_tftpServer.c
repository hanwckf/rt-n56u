/*
*	TFTP server     File: cmd_tftpServer.c
*/

#include <cmd_tftpServer.h>
#include <replace.h>



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

uint8_t asuslink[] = "ASUSSPACELINK";
uint8_t maclink[] = "snxxxxxxxxxxx";
#define PTR_SIZE	0x800000
#define BOOTBUF_SIZE	0x30000
//uint8_t ptr[PTR_SIZE], bootbuf[BOOTBUF_SIZE], MAC0[13], RDOM[7], ASUS[24], nvramcmd[60];
uint8_t /*ptr[PTR_SIZE], bootbuf[BOOTBUF_SIZE], */MAC0[13], RDOM[7], ASUS[24], nvramcmd[60];
uint8_t *ptr;
uint16_t RescueAckFlag = 0;
uint32_t copysize = 0;
uint32_t offset = 0;
int rc = 0;
int MAC_FLAG = 0;
static char default_filename[DEFAULT_NAME_LEN];
int env_loc = 0;

static void _tftpd_open(void);
static void TftpHandler(uchar * pkt, unsigned dest, unsigned src, unsigned len);
static void RAMtoFlash(void);
static void SolveTRX(void);
extern image_header_t header;
extern int do_bootm(cmd_tbl_t *, int, int, char *[]);
extern int do_reset(cmd_tbl_t *, int, int, char *[]);

extern IPaddr_t TempServerIP;

#if (CONFIG_COMMANDS & CFG_CMD_TFTPSERVER)
#if 1
int check_trx(int argc, char *argv[])
{
	extern int verify_kernel_image(int, char *[], ulong *, ulong *, ulong *);
	memset(&header, 0, sizeof(header));
	return verify_kernel_image(argc, argv, NULL, NULL, NULL);
}
#else
int check_trx(int mode)
{
	ulong addr;
	ulong data, len, checksum;
    int verify;
    char *s;
	image_header_t *hdr = &header;
	
	s = getenv("verify");
	verify = (s && (*s == 'n')) ? 0 : 1;
	
	if (mode == 0)
		addr = CFG_FLASH_BASE + 0x50000;
	else
		addr = load_addr;
	
//	printf("## Booting image at %08lx ...\n", addr);
	printf("## Checking image at %08lx ...\n", addr);
	
	/* Copy header so we can blank CRC field for re-calculation */
#ifdef CONFIG_HAS_DATAFLASH
	if (addr_dataflash(addr))
	{
		read_dataflash(addr, sizeof(image_header_t), (char *)&header);
	} 
#endif
#if defined (CFG_ENV_IS_IN_NAND)
	if (addr >= CFG_FLASH_BASE)
		ranand_read(&header, (char *)(addr - CFG_FLASH_BASE), sizeof(image_header_t));
	else
		memmove (&header, (char *)addr, sizeof(image_header_t));
#elif defined (CFG_ENV_IS_IN_SPI)
	if (addr >= CFG_FLASH_BASE)
		raspi_read(&header, (char *)(addr - CFG_FLASH_BASE), sizeof(image_header_t));
	else
		memmove (&header, (char *)addr, sizeof(image_header_t));
#else //CFG_ENV_IS_IN_FLASH
	memmove (&header, (char *)addr, sizeof(image_header_t));
#endif //CFG_ENV_IS_IN_FLASH
	
	if (ntohl(hdr->ih_magic) != IH_MAGIC)
	{
		printf("Bad Magic Number,%08X \n", ntohl(hdr->ih_magic));
		return 1;
	}
//	else
//		printf("Magic Number: %08X, OK\n", ntohl(hdr->ih_magic));
	
	data = (ulong)&header;
	len  = sizeof(image_header_t);
	
	checksum = ntohl(hdr->ih_hcrc);
	hdr->ih_hcrc = 0;
	
	if (crc32(0, (char *)data, len) != checksum)
	{
		puts("Bad Header Checksum\n");
		return 1; 
	}
//	else
//		puts("Header Checksum OK\n");
	
	/* for multi-file images we need the data part, too */
	print_image_hdr((image_header_t *)hdr);
	
	data = addr + sizeof(image_header_t);
	len  = ntohl(hdr->ih_size);
	
#ifdef CONFIG_HAS_DATAFLASH
	if (addr_dataflash(addr))
	{
		read_dataflash(data, len, (char *)CFG_LOAD_ADDR);
		data = CFG_LOAD_ADDR;
	}
#endif
	
#if defined (CFG_ENV_IS_IN_NAND)
	if (addr >= CFG_FLASH_BASE) {
		ulong load_addr = CFG_SPINAND_LOAD_ADDR;
		ranand_read(load_addr, data - CFG_FLASH_BASE, len);
		data = load_addr;
	}
#elif defined (CFG_ENV_IS_IN_SPI)
	if (addr >= CFG_FLASH_BASE) {
		ulong load_addr = CFG_SPINAND_LOAD_ADDR;
		raspi_read(load_addr, data - CFG_FLASH_BASE, len);
		data = load_addr;
	}
#else //CFG_ENV_IS_IN_FLASH
#endif

	if (verify) 
	{
		puts("   Verifying Checksum ... ");
		if (crc32(0, (char *)data, len) != ntohl(hdr->ih_dcrc)) 
		{
			printf("Bad Data CRC\n");
			return 1;
		}
//		puts("Data CRC OK\n");
	}
	return 0;			
}
#endif // !0

static void TftpdSend(void)
{
	volatile uchar *pkt;
	volatile uchar *xp;
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
		*((ushort *)pkt)++ = htons(TFTP_DATA);
		*((ushort *)pkt)++ = htons(TftpBlock);/*fullfill the data part*/
		len = pkt - xp;
		break;
		
	case STATE_WRQ:
		xp = pkt;
		*((ushort *)pkt)++ = htons(TFTP_ACK);
		*((ushort *)pkt)++ = htons(TftpBlock);
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
		*((ushort *)pkt)++ = htons(TFTP_ACK);
		*((ushort *)pkt)++ = htons(TftpBlock);
		len = pkt - xp;
		break;
		
	case STATE_FINISHACK:
		xp = pkt;
		*((ushort *)pkt)++ = htons(TFTP_FINISHACK);
		*((ushort *)pkt)++ = htons(RescueAckFlag);
		len = pkt - xp;
		break;
		
	case STATE_TOO_LARGE:
		xp = pkt;
		*((ushort *)pkt)++ = htons(TFTP_ERROR);
		*((ushort *)pkt)++ = htons(3);
		strcpy((char *)pkt, "File too large");
		pkt += 14 /*strlen("File too large")*/ + 1;
		len = pkt - xp;
		break;
		
	case STATE_BAD_MAGIC:
		xp = pkt;
		*((ushort *)pkt)++ = htons(TFTP_ERROR);
		*((ushort *)pkt)++ = htons(2);
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

	asus_gpio_init();
	if (DETECT())		/* Reset button */
	{
		printf(" \n## Enter Rescue Mode ##\n");
		printf("   \n3: System Boot system code via TFTP.\n");
		setenv("autostart", "no");
		/* Wait forever for an image */
		if (NetLoop(TFTPD) < 0) 
			return 1;
	}
	else if (DETECT_WPS())	/* WPS button */
	{
		/* Make sure WPS button is pressed at least press_times * 0.01s. */
		while (DETECT_WPS() && i++ < press_times)
			udelay(10000);
		
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
	{
		if(check_trx(argc, argv))
		{
			printf(" \nEnter Recuse Mode for trx error\n");
			printf("   \n3: System Boot system code via TFTP.\n");
			if (NetLoop(TFTPD) < 0) 
				return 1;
		}
		
		printf("   \n3: System Boot system code via Flash.\n");
		do_bootm(cmdtp, 0, argc, argv);
	}
	
	return 0;
}
	
	
	
void TftpdStart(void)
{
	 ptr=0x80201000;  //only for RT6855/RT6856/RT63365   
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
	int i;
	static int ledcount=0,ledstate=1;
	
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
	

	#if 0
	if (0==ledcount%64)/*the led flicker when packet received*/
		{
			ledstate+=1;
			ledstate%=2;
		}
	if(0==ledstate%2)
		LEDON(11);
	else
		LEDOFF(11);
	++ledcount;
	if (0xffffff==i)
		i=0;
	#endif
	len -= 2;
	/* warning: don't use increment (++) in ntohs() macros!! */
	proto = *((ushort *)pkt)++;
	
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
			if (*((uint8_t *)pkt)++ != asuslink[i])
				break;
		}
		if (i==13) 
		{ /* it's the firmware transmitting situation */
			/* here get the IP address from the first packet. */
			NetOurIP = (*((uint8_t *)pkt)++) & 0x000000ff;
			NetOurIP<<=8;
			NetOurIP|= (*((uint8_t *)pkt)++) & 0x000000ff;
			NetOurIP<<=8;
			NetOurIP|= (*((uint8_t *)pkt)++) & 0x000000ff;
			NetOurIP<<=8;
			NetOurIP|= (*((uint8_t *)pkt)++) & 0x000000ff;
		}
		else
		{
			for (i=0; i<13; i++)
			{
				if (*((uint8_t *)pkt)++ != maclink[i])
					break;
			}
			if(i==13)
			{
				/* here get the IP address from the first packet. */
				NetOurIP = (*((uint8_t *)pkt)++)& 0x000000ff;
				NetOurIP<<=8;
				NetOurIP|=(*((uint8_t *)pkt)++)& 0x000000ff;
				NetOurIP<<=8;
				NetOurIP|=(*((uint8_t *)pkt)++)& 0x000000ff;
				NetOurIP<<=8;
				NetOurIP|=(*((uint8_t *)pkt)++)& 0x000000ff;
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
// Jiahao++
//			printf("\n First block received  \n");
			
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


static void RAMtoFlash(void)
{
	int i=0, parseflag=0, j=0;
	uchar mac_temp[7],secretcode[9];
	uint8_t SCODE[5] = {0x53, 0x43, 0x4F, 0x44, 0x45};
	char *end ,*tmp;
	
    /* Check for the TRX magic. If it's a TRX image, then proceed to flash it. */
	if ((*(unsigned long *) (ptr) == TRX_MAGIC)&& (copysize > 0x30000))
	{
		printf("Download of 0x%x bytes completed\n", copysize);
		printf("Check TRX and write it to FLASH \n");
		SolveTRX();
	}
	else
	{
		if (copysize>0 && copysize<=0x30000)
		{
			
			if ( (*(unsigned long *) (ptr+0x2fff0) == NVRAM_MAGIC) 
				&& (*(unsigned long *) (ptr) != NVRAM_MAGIC_MAC0)
				&& (*(unsigned long *) (ptr) != NVRAM_MAGIC_MAC1)
				&& (*(unsigned long *) (ptr) != NVRAM_MAGIC_RDOM )
				&& (*(unsigned long *) (ptr) != NVRAM_MAGIC_ASUS ))
			{
				printf("Download of 0x%x bytes completed\n", copysize);
				printf("Write bootloader binary to FLASH (0xbc400000)\n");
				parseflag = 1;
			}
			else if( *(unsigned long *) (ptr) == NVRAM_MAGIC_MAC0 )
			{
				printf("Download of 0x%x bytes completed\n", copysize);
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
					rc=replace(0x180, secretcode, 8);
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
				rc=replace(0x4, mac_temp, 6);
#if 0
				sprintf(nvramcmd, "nvram set et0macaddr=%s", MAC0);
				printf("%s\n", nvramcmd);
#endif
				parseflag = 2;
			}
			else if ( *(unsigned long *) (ptr) == NVRAM_MAGIC_RDOM ) 
			{
				for (i=0; i<6; i++)
					RDOM[i] = ptr[8+i];
				RDOM[i] = '\0';
				
				rc=replace(0x188, RDOM+4, 2);
				printf("Write RDOM = %s  to FLASH \n", RDOM);
#if 0
                		sprintf(nvramcmd, "nvram set regulation_domain=%s", RDOM);
				printf("%s\n", nvramcmd);
#endif
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
				rc=replace(0x4, mac_temp, 6);
				for (i=0; i<6; i++)
					RDOM[i] = ASUS[17+i];
				RDOM[i] = '\0';

				printf("Write MAC0 = %s  to FLASH \n", MAC0);
#if 0
				sprintf(nvramcmd, "nvram set et0macaddr=%s", MAC0);
				printf("%s\n", nvramcmd);
#endif
				
//				rc=replace(0x188, RDOM, 6);
				rc=replace(0x188, RDOM+4, 2);				
				printf("Write RDOM = %s  to FLASH \n", RDOM);
#if 0
				sprintf(nvramcmd, "nvram set regulation_domain=%s", RDOM);
				printf("%s\n", nvramcmd);
#endif
				parseflag = 4;
			}
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
		else if (copysize > 0x30000)
		{
			parseflag = 0;
			printf("Download of 0x%x bytes completed\n", copysize);
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
			copysize = 0x30000;
		
		printf("BootLoader Programming...\n");
		
		if (parseflag == 1)
		{
#if defined (CFG_ENV_IS_IN_NAND)
			rc = ranand_erase_write((u8 *)(ptr), 0, copysize);
#elif defined (CFG_ENV_IS_IN_SPI)
			rc = raspi_erase_write((u8 *)(ptr), 0, copysize);
#else
			flash_sect_protect(0, CFG_FLASH_BASE + 0x00000, CFG_FLASH_BASE + 0x2ffff);
			printf("\n Erase File System block !!\n From %x To %x\n", CFG_FLASH_BASE + 0x00000, CFG_FLASH_BASE + 0x2ffff);
			flash_sect_erase(CFG_FLASH_BASE + 0x00000, CFG_FLASH_BASE + 0x2ffff);
			rc = flash_write((uchar *)ptr, (ulong)(CFG_FLASH_BASE + 0x00000), copysize);
			flash_sect_protect(1, CFG_FLASH_BASE + 0x00000, CFG_FLASH_BASE + 0x2ffff);
#endif
		}
		
		if (rc)
		{
			printf("rescue failed!\n");
#if defined (CFG_ENV_IS_IN_NAND) || defined (CFG_ENV_IS_IN_SPI)
			printf("\n rc(%d)\n", rc);
#else
			flash_perror(rc);
#endif
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
				printf("\nRT2880 SYSTEM RESET!!!\n\n");
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

static void SolveTRX(void)
{
	int  i = 0;
	int rrc = 0;
	ulong	dest = 0, count = 0;
	int	size;

    	load_addr = ptr;
	if (check_trx(0, NULL))
	{
		printf("Hello!! Check trx error! RT2880 SYSTEM RESET!!!\n\n");
		udelay(500);
		do_reset (NULL, 0, 0, NULL);
	}
	else
	{
		printf("\n Copy linux image[%d byte] to Flash[0x%08X].... \n", copysize, dest);
		count = copysize;
		size = 1;
		
		if (count == 0) 
		{
			puts ("Zero length ???\n");
			return;
		}
		
		puts ("Copy to Flash... ");
#if defined (CFG_ENV_IS_IN_NAND)
		rrc = ranand_erase_write((u8 *)ptr, 0x50000, count*size);
#elif defined (CFG_ENV_IS_IN_SPI)
		rrc = raspi_erase_write((u8 *)ptr, 0x50000, count*size);
#else
		printf("\n Erase File System block !!\n From 0xBC450000 To 0xBC7FFFFF\n");
		flash_sect_protect(0, CFG_FLASH_BASE + 0x50000, CFG_FLASH_BASE + 0x7fffff);
		flash_sect_erase(CFG_FLASH_BASE + 0x50000, CFG_FLASH_BASE + 0x7FFFFF);
		printf ("\n Copy %d byte to Flash... ", count*size);		
		rrc = flash_write((uchar *)ptr, (ulong)(CFG_FLASH_BASE + 0x50000), count*size);
		flash_sect_protect(1, CFG_FLASH_BASE + 0x50000, CFG_FLASH_BASE + 0x7fffff);
#endif
		
		if (rrc) 
		{
			printf("rescue failed!\n");
#if defined (CFG_ENV_IS_IN_NAND) || defined (CFG_ENV_IS_IN_SPI)
			printf("\n rrc(%d)\n", rrc);
#else
			flash_perror(rrc);
#endif
			NetState = NETLOOP_FAIL;
			TftpState = STATE_FINISHACK;
			RescueAckFlag= 0x0000;
			for (i=0; i<6; i++)
				TftpdSend();
			return;
		}
		else
		{
			printf("done. %d bytes written\n", count*size);
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

#endif
