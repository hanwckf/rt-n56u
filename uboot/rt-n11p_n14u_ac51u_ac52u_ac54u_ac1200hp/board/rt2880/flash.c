/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <rt_mmap.h>

#undef DEBUG
flash_info_t	flash_info[CFG_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

extern ulong rt2880_flash_start_t;

/* NOTE - CONFIG_FLASH_16BIT means the CPU interface is 16-bit, it
 *        has nothing to do with the flash chip being 8-bit or 16-bit.
 */
//#define CONFIG_FLASH_16BIT 1  (defined in include/configs/rt2880.h)
#ifdef CONFIG_FLASH_16BIT
typedef unsigned short FLASH_PORT_WIDTH;
typedef volatile unsigned short FLASH_PORT_WIDTHV;
#define	FLASH_ID_MASK	0xFFFF
#else
typedef unsigned long FLASH_PORT_WIDTH;
typedef volatile unsigned long FLASH_PORT_WIDTHV;
#define	FLASH_ID_MASK	0xFFFFFFFF
#endif

#define FPW	FLASH_PORT_WIDTH
#define FPWV	FLASH_PORT_WIDTHV

#define ORMASK(size) ((-size) & OR_AM_MSK)


#if 0


#define FLASH_CYCLE1	0x05555
#define FLASH_CYCLE2	0x02aaa

#else

// #define FLASH_CYCLE1	0x0554
// #define FLASH_CYCLE2	0x02ab
#endif



 static unsigned int FLASH_CYCLE1 = 0x05555;
	
 static unsigned int FLASH_CYCLE2 = 0x02aaa;

 static int flash_use_SSI_standard = 0;


/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size(FPWV *addr, flash_info_t *info);
static void flash_reset(flash_info_t *info);
static int write_word_intel(flash_info_t *info, FPWV *dest, FPW data);
static int write_word_amd(flash_info_t *info, FPWV *dest, FPW data);
static void flash_get_offsets(ulong base, flash_info_t *info);
static flash_info_t *flash_get_info(ulong base);

/*-----------------------------------------------------------------------
 * flash_init()
 *
 * sets up flash_info and returns size of FLASH (bytes)
 */
unsigned long flash_init (void)
{
	unsigned long size = 0;
	int i;
#if 0
	u32 regvalue,kk0 = 0xF,kk1 = 0xF ;
	char *s;

	s = getenv("twe0");
	//printf("twe0 set to %s\n",s);

	if(s)
		kk0 = simple_strtoul (s, NULL, 16);

	s = getenv("toe0");
	//printf("toe0 set to %s\n",s);
	
	if(s)
		kk1 = simple_strtoul (s, NULL, 16);
#if defined(RT3883_ASIC_BOARD) || defined(RT3883_FPGA_BOARD)
	regvalue = *(volatile u_long *)(RALINK_SYSCTL_BASE + 0x0700);
#else
	regvalue = *(volatile u_long *)(RALINK_SYSCTL_BASE + 0x0308);
#endif
#ifdef DEBUG
	printf("\n Default FLASH_CFG0 = %08X \n",regvalue);
#endif
	regvalue &= ~(0x3 << 26);
	regvalue |= (0x1 << 26);

	regvalue |= (0x1 << 24);

	regvalue &= ~(0x3 << 20);
	regvalue |= (0x1 << 20);

	regvalue &= ~(0x3 << 16);
	regvalue |= (0x1 << 16);

	regvalue &= ~(0xF << 12);
	regvalue |= (kk0 << 12);

	regvalue &= ~(0xF << 8);
	regvalue |= (kk1 << 8);
#ifdef DEBUG
	printf("\n Ready Set Value = 0x%08X\n",regvalue);
#endif	

#if defined(RT3883_ASIC_BOARD) || defined(RT3883_FPGA_BOARD)
	*(volatile u_long *)(RALINK_SYSCTL_BASE + 0x0700) = regvalue;
#else
	*(volatile u_long *)(RALINK_SYSCTL_BASE + 0x0308) = regvalue;
#endif

#if defined(RT3883_ASIC_BOARD) || defined(RT3883_FPGA_BOARD)
        regvalue = *(volatile u_long *)(RALINK_SYSCTL_BASE + 0x0700);
#else
        regvalue = *(volatile u_long *)(RALINK_SYSCTL_BASE + 0x0308);
#endif
#ifdef DEBUG
	printf("\n Setup FLASH_CFG0 = %08X \n",regvalue);
#endif
#endif

	/* Init: no FLASHes known */
	for (i=0; i < CFG_MAX_FLASH_BANKS; ++i) {
		ulong flashbase = PHYS_FLASH_START;
#if defined (RT3052_MP2) && defined (ON_BOARD_32M_FLASH_COMPONENT)
		if (i == 1)
			ulong flashbase = PHYS_FLASH2_START;
#endif
#if 1
		memset(&flash_info[i], 0, sizeof(flash_info_t));
#endif

		flash_info[i].size =
			flash_get_size((FPW *)flashbase, &flash_info[i]);

		if (flash_info[i].flash_id == FLASH_UNKNOWN) {
			printf("## Unknown FLASH on Bank %d - Size = 0x%08lx\n",
				i, flash_info[i].size);
		}

		size += flash_info[i].size;
	}
#if 1
#if CFG_MONITOR_BASE >= CFG_FLASH_BASE
	/* monitor protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		PHYS_FLASH_1,
		PHYS_FLASH_1+monitor_flash_len-1,
		flash_get_info(PHYS_FLASH_1));
#ifdef DEBUG		
  printf("\n  monitor protection ON by default,monitor_flash_len = %d \n",monitor_flash_len);
#endif  
#endif
#endif

#ifndef	CFG_RUN_CODE_IN_RAM           //CFG_ENV_IS_IN_FLASH
	/* ENV protection ON by default */
#ifdef DEBUG	
   printf("\n ENV protection ON by default !! \n");
#endif   
	flash_protect(FLAG_PROTECT_SET,
		CFG_ENV_ADDR,
		CFG_ENV_ADDR+CFG_ENV_SIZE-1,
		flash_get_info(CFG_ENV_ADDR));
#endif


	return size;
}


/*-----------------------------------------------------------------------
 */
static void flash_reset(flash_info_t *info)
{
	FPWV *base = (FPWV *)(info->start[0]);

	/* Put FLASH back in read mode */
	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL)
		*base = (FPW)0x00FF00FF;	/* Intel Read Mode */
	else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_AMD)
	{
		*base = (FPW)0x00F000F0;	/* AMD Read Mode */
   #ifdef DEBUG
	   printf("\nAMD Read Mode, flash reset,triger at %08X\n",base);
   #endif
	}	 
	else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_MX)
	{
		*base = (FPW)0x00F000F0;	/* MX Read Mode */
		#ifdef DEBUG
		printf("\n  MX Read Mode ,flash reset, triger at %08X\n",base);
		#endif
	}
	else
	{
		*base = (FPW)0x00F000F0;	/* MX Read Mode */
		#ifdef DEBUG
		printf("\n EON Flash series !! \n");
		#endif
	}
}

/*-----------------------------------------------------------------------
 */
static void flash_get_offsets(ulong base, flash_info_t *info)
{
	int i;
#ifdef kaiker
	/* set up sector start address table */
	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL
		&& (info->flash_id & FLASH_BTYPE)) {
		int bootsect_size;	/* number of bytes/boot sector	*/
		int sect_size;		/* number of bytes/regular sector */

		bootsect_size = 0x00002000 * (sizeof(FPW)/2);
		sect_size =     0x00010000 * (sizeof(FPW)/2);

		/* set sector offsets for bottom boot block type	*/
		for (i = 0; i < 8; ++i) {
			info->start[i] = base + (i * bootsect_size);
		}
		for (i = 8; i < info->sector_count; i++) {
			info->start[i] = base + ((i - 7) * sect_size);
		}
	} else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_AMD
		&& (info->flash_id & FLASH_TYPEMASK) == FLASH_AM640U) {

		int sect_size;		/* number of bytes/sector */

		sect_size = 0x00010000 * (sizeof(FPW)/2);

		/* set up sector start address table (uniform sector type) */
		for ( i = 0; i < info->sector_count; i++ )
			info->start[i] = base + (i * sect_size);
	}
	//kaiker
	else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_MX
		&& (info->flash_id & FLASH_TYPEMASK) == MX_ID_LV320BT) {
#else	
	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_MX
		&& ((info->flash_id & FLASH_TYPEMASK) == MX_ID_LV320BT ||
		    (info->flash_id & FLASH_TYPEMASK) == MX_ID_29LV640DB ||
		    (info->flash_id & FLASH_TYPEMASK) == MX_ID_29LV128DB)) {
#endif
		int bootsect_size;	/* number of bytes/boot sector	*/
		int sect_size;		/* number of bytes/sector */

		bootsect_size = 0x00002000 * (sizeof(FPW)/2);
		sect_size = 0x00010000 * (sizeof(FPW)/2);

		/* set up sector start address table (uniform sector type) */
		for (i = 0; i < 8; ++i) {
			info->start[i] = base + (i * bootsect_size);
		}
		for (i = 8; i < info->sector_count; i++) {
			info->start[i] = base + ((i - 7) * sect_size);
		}
	}
	else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_MX
		&& ((info->flash_id & FLASH_TYPEMASK) == MX_ID_29GL128EHT)) {
		int sect_size;		/* number of bytes/sector */

		sect_size = 0x00020000 * (sizeof(FPW)/2); /* 128K */
		for (i = 0; i < info->sector_count; i++) {
		    info->start[i] = base + ( i * sect_size);
		}
	}
	else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_MX
		&& ((info->flash_id & FLASH_TYPEMASK) == MX_ID_LV160B)) {
			info->start[0] = base; //16K
			info->start[1] = info->start[0] + 0x4000; //16K
			info->start[2] = info->start[1] + 0x2000; //8K
			info->start[3] = info->start[2] + 0x2000; //8K
			info->start[4] = info->start[3] + 0x8000; //32K
			for (i = 5; i < info->sector_count; i++) {
				info->start[i] = info->start[4] + ( (i-4) * 0x00010000);//64K
			}
	}
	else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_MX
		&& ((info->flash_id & FLASH_TYPEMASK) == MX_ID_LV160T)) {
			for (i = 0; i < info->sector_count - 4; i++) {
				info->start[i] = base + (i * 0x10000);//64K
			}
			info->start[31] = info->start[30] + 0x10000; //32K
			info->start[32] = info->start[31] + 0x8000; //8K
			info->start[33] = info->start[32] + 0x2000; //8K
			info->start[34] = info->start[33] + 0x2000; //16K
	}
	else /*  if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_AMD
		&& (info->flash_id & FLASH_TYPEMASK) == EN_ID_29LV641L) */
   	{
		int sect_size;		/* number of bytes/sector */

		sect_size = 0x00010000 * (sizeof(FPW)/2);

		/* set up sector start address table (uniform sector type) */
		for ( i = 0; i < info->sector_count; i++ )
			info->start[i] = base + (i * sect_size);
		
		printf("\n Set info->start[0]=%08X\n",info->start[0]);
	}
}

/*-----------------------------------------------------------------------
 */

static flash_info_t *flash_get_info(ulong base)
{
	int i;
	flash_info_t * info;

	for (i = 0; i < CFG_MAX_FLASH_BANKS; i ++) {
		info = & flash_info[i];
		if (info->start[0] <= base && base < info->start[0] + info->size)
			break;
	}

	return i == CFG_MAX_FLASH_BANKS ? 0 : info;
}

/*-----------------------------------------------------------------------
 */

void flash_print_info(flash_info_t *info)
{
#ifdef kaiker
	int i;
	uchar *boottype;
	uchar *bootletter;
	uchar *fmt;
	uchar botbootletter[] = "B";
	uchar topbootletter[] = "T";
	uchar botboottype[] = "bottom boot sector";
	uchar topboottype[] = "top boot sector";

	if (info->flash_id == FLASH_UNKNOWN) {
		printf("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_AMD:	printf("AMD ");
		break;
	case FLASH_MAN_BM:	printf("BRIGHT MICRO ");
		break;
	case FLASH_MAN_FUJ:	printf("FUJITSU ");
		break;
	case FLASH_MAN_SST:	printf("SST ");
		break;
	case FLASH_MAN_STM:	printf("STM ");
		break;
	case FLASH_MAN_INTEL:	printf("INTEL ");
		break;
	default:		printf("Unknown Vendor ");
		break;
	}

	/* check for top or bottom boot, if it applies */
	if (info->flash_id & FLASH_BTYPE) {
		boottype = botboottype;
		bootletter = botbootletter;
	} else {
		boottype = topboottype;
		bootletter = topbootletter;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AM640U:
		fmt = "29LV641D (64 Mbit, uniform sectors)\n";
		break;
	case FLASH_28F800C3B:
	case FLASH_28F800C3T:
		fmt = "28F800C3%s (8 Mbit, %s)\n";
		break;
	case FLASH_INTEL800B:
	case FLASH_INTEL800T:
		fmt = "28F800B3%s (8 Mbit, %s)\n";
		break;
	case FLASH_28F160C3B:
	case FLASH_28F160C3T:
		fmt = "28F160C3%s (16 Mbit, %s)\n";
		break;
	case FLASH_INTEL160B:
	case FLASH_INTEL160T:
		fmt = "28F160B3%s (16 Mbit, %s)\n";
		break;
	case FLASH_28F320C3B:
	case FLASH_28F320C3T:
		fmt = "28F320C3%s (32 Mbit, %s)\n";
		break;
	case FLASH_INTEL320B:
	case FLASH_INTEL320T:
		fmt = "28F320B3%s (32 Mbit, %s)\n";
		break;
	case FLASH_28F640C3B:
	case FLASH_28F640C3T:
		fmt = "28F640C3%s (64 Mbit, %s)\n";
		break;
	case FLASH_INTEL640B:
	case FLASH_INTEL640T:
		fmt = "28F640B3%s (64 Mbit, %s)\n";
		break;
	default:
		fmt = "Unknown Chip Type\n";
		break;
	}

	printf(fmt, bootletter, boottype);

	printf("  Size: %ld MB in %d Sectors\n",
		info->size >> 20,
		info->sector_count);

	printf("  Sector Start Addresses:");

	for (i=0; i<info->sector_count; ++i) {
		if ((i % 5) == 0) {
			printf("\n   ");
		}

		printf(" %08lX%s", info->start[i],
			info->protect[i] ? " (RO)" : "     ");
	}

	printf("\n");
#endif
}

/*-----------------------------------------------------------------------
 */

/*
 * The following code cannot be run from FLASH!
 */

ulong flash_get_size(FPWV *addr, flash_info_t *info)
{
	/* Write auto select command: read Manufacturer ID */

	

	/* Write auto select command sequence and test FLASH answer */
	addr[FLASH_CYCLE1] = (FPW)0x00AA00AA;	/* for AMD, Intel ignores this */
	addr[FLASH_CYCLE2] = (FPW)0x00550055;	/* for AMD, Intel ignores this */
	addr[FLASH_CYCLE1] = (FPW)0x00900090;	/* selects Intel or AMD */

	/* The manufacturer codes are only 1 byte, so just use 1 byte.
	 * This works for any bus width and any FLASH device width.
	 */
#if 1 	 
	 if(addr[0] != (FPW)SST_MANUFACT)
	 {
		FLASH_CYCLE1 = 0x0555; 
		FLASH_CYCLE2 = 0x02aa; 
		
		addr[FLASH_CYCLE1] = (FPW)0x00AA00AA;	/* for AMD, Intel ignores this */
		addr[FLASH_CYCLE2] = (FPW)0x00550055;	/* for AMD, Intel ignores this */
		addr[FLASH_CYCLE1] = (FPW)0x00900090;	/* selects Intel or AMD */

		
	 }
	 else
	 {
		printf("\n The Flash follow SSI standard \n");
		flash_use_SSI_standard = 1;
		
	 }
	
#endif  
  //   printf("\n The Flash ID =%08X \n",addr[1]);
	
	switch (addr[0] & 0xff) {

	case (uchar)AMD_MANUFACT:
		info->flash_id = FLASH_MAN_AMD;
	//	printf("\nFLASH_MAN_AMD\n");
		break;

	case (uchar)INTEL_MANUFACT:
		info->flash_id = FLASH_MAN_INTEL;
	//	printf("\nFLASH_MAN_INTEL\n");
		break;

	case (uchar)MX_MANUFACT:
		info->flash_id = FLASH_MAN_MX;
	//	printf("\n vender : MX_MANUFACT  \n");
		break; 

	case (uchar)AMD_MANUFACT_EON:
		info->flash_id = FLASH_MAN_AMD;
	//	printf("\n vender : AMD_MANUFACT_EON  \n");
		break;

	default:
		info->flash_id = FLASH_MAN_AMD;
	//	printf("\n vender : AMD_MANUFACT_EON  \n");
		break;
		/*
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		printf("\n FLASH_UNKNOWN \n");
		*/
		break;
	}

	/* Check 16 bits or 32 bits of ID so work on 32 or 16 bit bus. */
	if (info->flash_id != FLASH_UNKNOWN)

		switch (addr[1]) {

		case (FPW)AMD_ID_LV640U:	/* 29LV640 and 29LV641 have same ID */
			info->flash_id += FLASH_AM640U;
			info->sector_count = 128;
			info->size = 0x00800000 * (sizeof(FPW)/2);
			break;				/* => 8 or 16 MB	*/

		case (FPW)INTEL_ID_28F800C3B:
			info->flash_id += FLASH_28F800C3B;
			info->sector_count = 23;
			info->size = 0x00100000 * (sizeof(FPW)/2);
			break;				/* => 1 or 2 MB		*/

		case (FPW)INTEL_ID_28F800B3B:
			info->flash_id += FLASH_INTEL800B;
			info->sector_count = 23;
			info->size = 0x00100000 * (sizeof(FPW)/2);
			break;			  	/* => 1 or 2 MB		*/
   
		case (FPW)INTEL_ID_28F160C3B:
			info->flash_id += FLASH_28F160C3B;
			info->sector_count = 39;
			info->size = 0x00200000 * (sizeof(FPW)/2);
			break;				/* => 2 or 4 MB		*/

		case (FPW)INTEL_ID_28F160B3B:
			info->flash_id += FLASH_INTEL160B;
			info->sector_count = 39;
			info->size = 0x00200000 * (sizeof(FPW)/2);
			break;				/* => 2 or 4 MB		*/

		case (FPW)INTEL_ID_28F320C3B:
			info->flash_id += FLASH_28F320C3B;
			info->sector_count = 71;
			info->size = 0x00400000 * (sizeof(FPW)/2);
			break;				/* => 4 or 8 MB		*/

		case (FPW)INTEL_ID_28F320B3B:
			info->flash_id += FLASH_INTEL320B;
			info->sector_count = 71;
			info->size = 0x00400000 * (sizeof(FPW)/2);
			break;				/* => 4 or 8 MB		*/

		case (FPW)INTEL_ID_28F640C3B:
			info->flash_id += FLASH_28F640C3B;
			info->sector_count = 135;
			info->size = 0x00800000 * (sizeof(FPW)/2);
			break;				/* => 8 or 16 MB	*/

		case (FPW)INTEL_ID_28F640B3B:
			info->flash_id += FLASH_INTEL640B;
			info->sector_count = 135;
			info->size = 0x00800000 * (sizeof(FPW)/2);
			break;				/* => 8 or 16 MB	*/

		case (FPW)AMD_ID_LV320B:
			info->flash_id += AMD_ID_LV320B;
			info->sector_count = 71;
			info->size = 0x00400000 * (sizeof(FPW)/2);
		//	printf("\n AMD_ID_LV320B, Size = %08x bytes\n",info->size);
			break;				/* => 4 or 8 MB		*/
		 
		case (FPW)MX_ID_LV320BT:
			info->flash_id += MX_ID_LV320BT;
			info->sector_count = 71;
			info->size = 0x00400000 * (sizeof(FPW)/2);
		//	printf("\n MX_ID_LV320BT, Size = %08x bytes\n",info->size);
			break;				/* => 4 or 8 MB		*/

		case (FPW)MX_ID_LV160B:
		case (FPW)MX_ID_LV160T:
			info->flash_id += addr[1];
			info->sector_count = 35;
			info->size = 0x00200000 * (sizeof(FPW)/2);
		//	printf("\n MX_ID_LV160B, Size = %08x bytes\n",info->size);
			break;				

		case (FPW)MX_ID_29LV640DB:
			info->flash_id += MX_ID_29LV640DB;
			info->sector_count = 135;
			info->size = 0x00800000 * (sizeof(FPW)/2);
			break;

		case (FPW)MX_ID_29LV128DB:
			info->flash_id += MX_ID_29LV128DB;
			info->sector_count = 263;
			info->size = 0x01000000 * (sizeof(FPW)/2);
			break;
		
		case (FPW) MX_ID_29GL128EHT:
			info->flash_id +=  MX_ID_29GL128EHT;
			info->sector_count = 128;
			info->size = 0x01000000 * (sizeof(FPW)/2);
			break;

#if defined (RT3052_MP2) && defined (ON_BOARD_32M_FLASH_COMPONENT)
		case (FPW)FLASH_S29GL256N:
			/* winfred:
			 *  To support 32MB*1 (Spansion S29GL256N) flash on
			 *  RT3052 MP2, we separate it into 2 16MB bank:
			 *  bank 0: 0xBF000000 ~ 0xBFFFFFFF
			 *  bank 1: 0xBB000000 ~ 0xBBFFFFFF
			 *  So, we need to modify the flash info.
			 *  (We currently use first bank only, which makes
			 *   the sector_count to be 256 instead of 512.
			 *   Besides, sectorsize is 0x10000(wrong) instead of
			 *   0x20000, and size is 0x1000000(wront) instead of
			 *   0x2000000.)
			 */
			info->flash_id += FLASH_S29GL256N;
			info->sector_count = 256;
			info->size = 0x01000000 * (sizeof(FPW)/2);
			break;
#endif

		default:
			info->flash_id += EN_ID_29LV640H;
			info->sector_count = 128;
			info->size = 0x00800000 * (sizeof(FPW)/2);
		//	printf("\n EN_ID_29LV640H, Size = %08x bytes\n",info->size);
			break;
			#if 0
			info->flash_id = FLASH_UNKNOWN;
			info->sector_count = 0;
			info->size = 0;
			return (0);			/* => no or unknown flash */
			#endif
			break;
		}

	flash_get_offsets((ulong)addr, info);

	/* Put FLASH back in read mode */
	flash_reset(info);

	return (info->size);
}

/*-----------------------------------------------------------------------
 */
int erase_all_chip(flash_info_t *info, int s_first, int s_last)
{
	int rcode = 0;
 	FPWV *addr;
	int sect = 0;
	int intel = (info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL;
	ulong start, now, last;
	
	int poll;

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf("- missing\n");
		} else {
			printf("- no sectors to erase\n");
		}
		return 1;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_INTEL800B:
	case FLASH_INTEL160B:
	case FLASH_INTEL320B:
	case FLASH_INTEL640B:
	case FLASH_28F800C3B:
	case FLASH_28F160C3B:
	case FLASH_28F320C3B:
	case FLASH_28F640C3B:
	case FLASH_AM640U:
	case FLASH_MXLV320BT:
	case FLASH_MXLV160B:
	case FLASH_MXLV160T:
	case AMD_ID_LV320B:	
	case EN_ID_29LV640H:	
	case MX_ID_29GL128EHT:	
		break;
	case FLASH_UNKNOWN:
	default:
		printf("Can't erase unknown flash type %08lx - aborted\n",
			info->flash_id);
		return 1;
	}

	last  = get_timer(0);

	/* Start erase on unprotected sectors */
	printf("\n Erase All \n");
//	for (sect = s_first; sect<=s_last && rcode == 0; sect++) 
	{
		/* Disable interrupts which might cause a timeout here */
		//flag = disable_interrupts();

		addr = (FPWV *)(info->start[sect]);
		{
			/* must be AMD style if not Intel */
			FPWV *base;		/* first address in bank */

			base = (FPWV *)(info->start[0]);
			base[FLASH_CYCLE1] = (FPW)0x00AA00AA;	/* unlock */
			base[FLASH_CYCLE2] = (FPW)0x00550055;	/* unlock */
			base[FLASH_CYCLE1] = (FPW)0x00800080;	/* erase mode */
			base[FLASH_CYCLE1] = (FPW)0x00AA00AA;	/* unlock */
			base[FLASH_CYCLE2] = (FPW)0x00550055;	/* unlock */
			base[FLASH_CYCLE1] = (FPW)0x10101010;   /* erase all */
			
		}

		/* re-enable interrupts if necessary */
		/*
		if (flag)
			enable_interrupts();
			*/

		start = get_timer(0);


		/* wait at least 50us for AMD, 80us for Intel.
		 * Let's wait 1 ms.
		 */
		udelay(1000);

		poll = 0;
		while ((*addr & (FPW)0x00800080) != (FPW)0x00800080) {
			if ((now = get_timer(start)) > CFG_FLASH_ERASE_TOUT) {
				printf("Timeout\n");

				if (intel) {
					/* suspend erase	*/
					*addr = (FPW)0x00B000B0;
				}

				flash_reset(info);	/* reset to read mode */
				rcode = 1;		/* failed */
				break;
			}

			poll++;
			/* show that we're waiting */
			if ((get_timer(last)) > CFG_HZ) {/* every second */
				putc('*');
				last = get_timer(0);
			}
		}
		printf(" Erase all OK!! \n");

		for (sect=s_first; sect<=s_last; ++sect) {
		if (info->protect[sect]) {
			//kaiker,debug
			info->protect[sect]=0;
			
		}
		printf("protect off all  !\n");
	}

		

		flash_reset(info);	/* reset to read mode	*/
	}

	printf(" done\n");

	return rcode;
}



int	flash_erase(flash_info_t *info, int s_first, int s_last)
{
	FPWV *addr;
	int prot, sect;
	int intel = (info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL;
	ulong start, now, last;
	int rcode = 0;
	int poll,kk,s_kk = 8;
	u32 s_sector_size = 0x00001000 ;
	FPWV *base;		/* first address in bank */
	

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf("- missing\n");
		} else {
			printf("- no sectors to erase\n");
		}
		return 1;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_INTEL800B:
	case FLASH_INTEL160B:
	case FLASH_INTEL320B:
	case FLASH_INTEL640B:
	case FLASH_28F800C3B:
	case FLASH_28F160C3B:
	case FLASH_28F320C3B:
	case FLASH_28F640C3B:
	case FLASH_AM640U:
	case FLASH_MXLV320BT:
	case FLASH_MXLV160B:
	case FLASH_MXLV160T:
	case AMD_ID_LV320B:
	case EN_ID_29LV640H:
	case MX_ID_29LV640DB:
	case MX_ID_29LV128DB:
#if defined (RT3052_MP2) && defined (ON_BOARD_32M_FLASH_COMPONENT)
	case FLASH_S29GL256N:
#endif
	case MX_ID_29GL128EHT:	
		break;
	case FLASH_UNKNOWN:
	default:
		printf("Can't erase unknown flash type %08lx - aborted\n",
			info->flash_id);
		return 1;
	}

	prot = 0;
	for (sect=s_first; sect<=s_last; ++sect) {
		if (info->protect[sect]) {
			//kaiker,debug
			//info->protect[sect]=0;
			printf("\n protect sect[%d]\n",sect);
			
			prot++;
		}
	}

	if (prot) {
		printf("- Warning: %d protected sectors will not be erased!\n",
			prot);
	} else {
		printf("\n");
	}

	last  = get_timer(0);

	/* Start erase on unprotected sectors */

	if(flash_use_SSI_standard)
	{
		s_kk = 16;
		s_sector_size = 0x00000800;
		printf("\n SSI Manufacture so its small sector  \n");
	}
	
	for (sect = s_first; sect<=s_last && rcode == 0; sect++) {

		if (info->protect[sect] != 0)	/* protected, skip it */
		{
			printf("\n sect[%d] is protected,skip \n",sect);
			continue;
		}	
		printf("\n erase sector  = %d \n",sect);

		/* Disable interrupts which might cause a timeout here */
		//flag = disable_interrupts();

		addr = (FPWV *)(info->start[sect]);


		//add by kaiker
		if( sect == 0 || flash_use_SSI_standard == 1)
		{
			base = (FPWV *)(info->start[0]);

			if(flash_use_SSI_standard)
			{
				printf("\n Erase to sector address[%08X]\n",addr);
			}
			else
			{
				printf("\n Elase Sector 0 with 8K SIZE,The sector 0 is Total 64K\n");
			}
						
			for(kk =0 ; kk < s_kk ;kk++)
			{
				/* must be AMD style if not Intel */
			
				base[FLASH_CYCLE1] = (FPW)0x00AA00AA;	/* unlock */
				base[FLASH_CYCLE2] = (FPW)0x00550055;	/* unlock */
				base[FLASH_CYCLE1] = (FPW)0x00800080;	/* erase mode */
				base[FLASH_CYCLE1] = (FPW)0x00AA00AA;	/* unlock */
				base[FLASH_CYCLE2] = (FPW)0x00550055;	/* unlock */
				*addr = (FPW)0x00300030;	/* erase sector */
		
				start = get_timer(0);

				/* wait at least 50us for AMD, 80us for Intel.
					* Let's wait 1 ms.
				*/
				udelay(1000);

				poll = 0;
				
				while ((*addr & (FPW)0x00800080) != (FPW)0x00800080) 
				{
					if ((now = get_timer(start)) > (ulong)CFG_FLASH_ERASE_TOUT) 
					{
						printf("Timeout\n");
						flash_reset(info);	/* reset to read mode */
						rcode = 1;		/* failed */
						break;
					}

					poll++;
					/* show that we're waiting */
					if ((get_timer(last)) > CFG_HZ) 
					{/* every second */
						putc('*');
						last = get_timer(0);
					}
				}
			//	printf("sect = %d,s_last = %d,erase poll = %d\n",sect,s_last,poll);

				/* show that we're waiting */
				if ((get_timer(last)) > CFG_HZ) {	/* every second */
					putc('.');
					last = get_timer(0);
				}

				flash_reset(info);	/* reset to read mode	*/

			//	printf("\n Pre addr = %08X \n",addr);
	
				addr = addr + s_sector_size;
			}
			
			if(!flash_use_SSI_standard)
			{
				printf("\n Exit Sector 0 erase  !! \n");
			}
			continue;
			sect = 7;
		}
		
		if (intel) {
			*addr = (FPW)0x00500050; /* clear status register */
			*addr = (FPW)0x00200020; /* erase setup */
			*addr = (FPW)0x00D000D0; /* erase confirm */
		} else {
			/* must be AMD style if not Intel */
		//	FPWV *base;		/* first address in bank */

			base = (FPWV *)(info->start[0]);
			base[FLASH_CYCLE1] = (FPW)0x00AA00AA;	/* unlock */
			base[FLASH_CYCLE2] = (FPW)0x00550055;	/* unlock */
			base[FLASH_CYCLE1] = (FPW)0x00800080;	/* erase mode */
			base[FLASH_CYCLE1] = (FPW)0x00AA00AA;	/* unlock */
			base[FLASH_CYCLE2] = (FPW)0x00550055;	/* unlock */
			*addr = (FPW)0x00300030;	/* erase sector */
		}

		/* re-enable interrupts if necessary */
		/*
		if (flag)
			enable_interrupts();
			*/

		start = get_timer(0);

#if 1
		/* wait at least 50us for AMD, 80us for Intel.
		 * Let's wait 1 ms.
		 */
		udelay(1000);

		poll = 0;
		while ((*addr & (FPW)0x00800080) != (FPW)0x00800080) {
			if ((now = get_timer(start)) > (ulong)CFG_FLASH_ERASE_TOUT) {
				printf("Timeout\n");

				if (intel) {
					/* suspend erase	*/
					*addr = (FPW)0x00B000B0;
				}

				flash_reset(info);	/* reset to read mode */
				rcode = 1;		/* failed */
				break;
			}

			poll++;
			/* show that we're waiting */
			if ((get_timer(last)) > CFG_HZ) {/* every second */
				putc('*');
				last = get_timer(0);
			}
		}
		printf("sect = %d,s_last = %d,erase poll = %d\n",sect,s_last,poll);
#else

#endif


		/* show that we're waiting */
		if ((get_timer(last)) > CFG_HZ) {	/* every second */
			putc('.');
			last = get_timer(0);
		}

		flash_reset(info);	/* reset to read mode	*/
	}

	printf(" done\n");
	return rcode;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
int write_buff(flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	FPW data = 0; /* 16 or 32 bit word, matches flash bus width on MPC8XX */
	int bytes;	  /* number of bytes to program in current word		*/
	int left;	  /* number of bytes left to program			*/
	int i, res;

	for (left = cnt, res = 0;
		left > 0 && res == 0;
		addr += sizeof(data), left -= sizeof(data) - bytes) {

		bytes = addr & (sizeof(data) - 1);
		addr &= ~(sizeof(data) - 1);

		/* combine source and destination data so can program
		 * an entire word of 16 or 32 bits
		 */
		for (i = 0; i < sizeof(data); i++) {
			if (i < bytes || i - bytes >= left )
				*((uchar *)&data + i) = *((uchar *)addr + i);
			else	 
				*((uchar *)&data + i)= *src++;						
		}	
		if(get_timer(rt2880_flash_start_t) > CFG_FLASH_STATE_DISPLAY_TOUT)
		{
			printf("\n addr = 0x%08X ,cnt=%d ",addr,left);
			rt2880_flash_start_t = get_timer(0);
		}

		/* write one word to the flash */
		switch (info->flash_id & FLASH_VENDMASK) {
		case FLASH_MAN_AMD:
			res = write_word_amd(info, (FPWV *)addr, data);
			break;
		case FLASH_MAN_INTEL:
			res = write_word_intel(info, (FPWV *)addr, data);
			break;
		case FLASH_MAN_MX:	
			
			res = write_word_amd(info, (FPWV *)addr, data);
			break;
		default:
			/* unknown flash type, error! */
			printf("missing or unknown FLASH type\n");
			res = 1;	/* not really a timeout, but gives error */
			break;
		}
	}

	return (res);
}

/*-----------------------------------------------------------------------
 * Write a word to Flash for AMD FLASH
 * A word is 16 or 32 bits, whichever the bus width of the flash bank
 * (not an individual chip) is.
 *
 * returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_word_amd(flash_info_t *info, FPWV *dest, FPW data)
{
	ulong start;
	
	int res = 0;	/* result, assume success	*/
	FPWV *base;		/* first address in flash bank	*/

	start = get_timer(0);

	while(*dest != 0xFFFF)  {
#ifndef RT3052_FPGA_BOARD
		if (get_timer(start) > CFG_FLASH_STATE_DISPLAY_TOUT) 
		{
			start = get_timer(0);
			printf("\n dest[0x%08X]=%04X\n",dest,*dest);
		}
#endif
		
#if 1 //defined (RT2880_ASIC_BOARD) || defined (RT3052_ASIC_BOARD)
#else
		printf("Not Erase: address 0x%08x(value=0x%08x)\n", (ulong) dest, *dest);
		return (2);
#endif
	}
	
	base = (FPWV *)(info->start[0]);

	/* Disable interrupts which might cause a timeout here */
	//flag = disable_interrupts();

	base[FLASH_CYCLE1] = (FPW)0x00AA00AA;	/* unlock */
	base[FLASH_CYCLE2] = (FPW)0x00550055;	/* unlock */
	base[FLASH_CYCLE1] = (FPW)0x00A000A0;	/* selects program mode */

	*dest = data;		/* start programming the data	*/

	/* re-enable interrupts if necessary */
	/*
	if (flag)
		enable_interrupts();
		*/

	start = get_timer(0);
    
#if 0
	/* data polling for D7 */
	while (res == 0 && (*dest & (FPW)0x00800080) != (data & (FPW)0x00800080)) {
		if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
			*dest = (FPW)0x00F000F0;	/* reset bank */	 
			res = 1;
		}
	}
#else
{

	unsigned int oldstatus, status, poll=0;
	unsigned int dq6, dq5, dq2;	

	udelay(10);
	oldstatus = *dest;
	status = *dest;
	
//	printf("[%d] Toggle %d, %d\n", poll, (status  ^ oldstatus) & dq6, (status ^ oldstatus) & dq2);

	dq6 = (1<<6) & 0x000000FF;
	dq5 = (1<<5) & 0x000000FF;
	dq2 = (1<<2) & 0x000000FF;

	while( res ==0 && (status & dq6) != (oldstatus & dq6) && 
	       (status & dq5) != dq5 ) {
	
		if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
			*dest = (FPW)0x00F000F0;	/* reset bank */
			printf("*reset bank\n");	 
			res = 1;			
		}

		poll++;
		oldstatus = *dest;
//		udelay(20);
		status = *dest;

		//printf("[%d] Toggle %d\n", poll, (status  ^ oldstatus) & dq6);

	}
	//if (poll != 0) printf("poll %d\n", poll);

}
#endif

	return (res);
}

/*-----------------------------------------------------------------------
 * Write a word to Flash for Intel FLASH
 * A word is 16 or 32 bits, whichever the bus width of the flash bank
 * (not an individual chip) is.
 *
 * returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_word_intel(flash_info_t *info, FPWV *dest, FPW data)
{
	int res = 0;	/* result, assume success	*/
#ifdef kaiker
	ulong start;

	/* Check if Flash is (sufficiently) erased */
	if ((*dest & data) != data) {
		return (2);
	}

	/* Disable interrupts which might cause a timeout here */
	//flag = disable_interrupts();

	*dest = (FPW)0x00500050;	/* clear status register	*/
	*dest = (FPW)0x00FF00FF;	/* make sure in read mode	*/
	*dest = (FPW)0x00400040;	/* program setup		*/

	*dest = data;		/* start programming the data	*/

	/* re-enable interrupts if necessary */
	/*
	if (flag)
		enable_interrupts();
		*/

	start = get_timer(0);

	while (res == 0 && (*dest & (FPW)0x00800080) != (FPW)0x00800080) {
		if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
			*dest = (FPW)0x00B000B0;	/* Suspend program	*/
			res = 1;
		}
	}

	if (res == 0 && (*dest & (FPW)0x00100010))
		res = 1;	/* write failed, time out error is close enough	*/

	*dest = (FPW)0x00500050;	/* clear status register	*/
	*dest = (FPW)0x00FF00FF;	/* make sure in read mode	*/
#endif
	return (res);
}
