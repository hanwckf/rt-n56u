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
#include <command.h>
#include <malloc.h>
#include <devices.h>
#include <version.h>
#include <net.h>
#include <environment.h>
#include <asm/mipsregs.h>
#include <rt_mmap.h>
#include <spi_api.h>
#include <nand_api.h>
#include "../autoconf.h"

#ifdef ASUS_PRODUCT
#include <cmd_tftpServer.h>
#include <gpio.h>
#include <replace.h>
#endif

DECLARE_GLOBAL_DATA_PTR;
#undef DEBUG

#ifdef crc32
#undef crc32
#endif

#define SDRAM_CFG1_REG RALINK_SYSCTL_BASE + 0x0304

#if defined(CFG_ENV_IS_IN_NAND)
#if defined(UBOOT_STAGE1)
#define BOOTFILENAME	"uboot_stage1.img"
#elif defined(UBOOT_STAGE2)
#define BOOTFILENAME	"uboot_stage2.img"
#else
#define BOOTFILENAME	"uboot.img"
#endif
#else
#define BOOTFILENAME	"uboot.bin"
#endif

#if defined(UBOOT_STAGE1)
extern struct stage2_loc g_s2_loc;

char *boot_image_name = "Bootloader stage1 code";
char *sys_image_name = "Bootloader stage2 code";
#elif defined(UBOOT_STAGE2)
char *boot_image_name = "Bootloader stage1/2 code";
char *sys_image_name = "System code";
#else
char *boot_image_name = "Boot Loader code";
char *sys_image_name = "System code";
#endif

int modifies= 0;

#ifdef DEBUG
   #define DATE      "05/25/2006"
   #define VERSION   "v0.00e04"
#endif
#if ( ((CFG_ENV_ADDR+CFG_ENV_SIZE) < CFG_MONITOR_BASE) || \
      (CFG_ENV_ADDR >= (CFG_MONITOR_BASE + CFG_MONITOR_LEN)) ) || \
    defined(CFG_ENV_IS_IN_NVRAM)
#define	TOTAL_MALLOC_LEN	(CFG_MALLOC_LEN + CFG_ENV_SIZE)
#else
#define	TOTAL_MALLOC_LEN	CFG_MALLOC_LEN
#endif
#define ARGV_LEN  128

#if defined (RT6855A_ASIC_BOARD) || defined(RT6855A_FPGA_BOARD)	
static int watchdog_reset();
#endif

extern int timer_init(void);

extern void  rt2880_eth_halt(struct eth_device* dev);

extern int rw_rf_reg(int write, int reg, int *data);
extern int do_load_serial_bin (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#ifdef ASUS_PRODUCT
extern int do_tftpd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
#endif
extern int incaip_set_cpuclk(void);
extern int do_bootm (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
extern int do_tftpb (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
extern int do_mem_cp ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
extern int flash_sect_protect (int p, ulong addr_first, ulong addr_last);
int flash_sect_erase (ulong addr_first, ulong addr_last);
int get_addr_boundary (ulong *addr);
extern int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
extern void input_value(u8 *str);
#if defined (RT6855_ASIC_BOARD) || defined (RT6855_FPGA_BOARD) || \
    defined (MT7620_ASIC_BOARD) || defined (MT7620_FPGA_BOARD)
extern void rt_gsw_init(void);
#elif defined (RT6855A_ASIC_BOARD) || defined (RT6855A_FPGA_BOARD) 
extern void rt6855A_gsw_init(void);
#else
extern void rt305x_esw_init(void);
#endif
extern void LANWANPartition(void);

extern struct eth_device* 	rt2880_pdev;

extern ulong uboot_end_data;
extern ulong uboot_end;

#ifdef RALINK_USB
extern int usb_stor_curr_dev;
#endif

ulong monitor_flash_len;

const char version_string[] =
	U_BOOT_VERSION" (" __DATE__ " - " __TIME__ ")";

extern ulong load_addr; /* Default Load Address */


unsigned long mips_cpu_feq;
unsigned long mips_bus_feq;

/*
 * Begin and End of memory area for malloc(), and current "brk"
 */
static ulong mem_malloc_start;
static ulong mem_malloc_end;
static ulong mem_malloc_brk;

static char  file_name_space[ARGV_LEN];

#define read_32bit_cp0_register_with_select1(source)            \
({ int __res;                                                   \
        __asm__ __volatile__(                                   \
        ".set\tpush\n\t"                                        \
        ".set\treorder\n\t"                                     \
        "mfc0\t%0,"STR(source)",1\n\t"                          \
        ".set\tpop"                                             \
        : "=r" (__res));                                        \
        __res;})


#if defined(ASUS_PRODUCT) && ! defined(UBOOT_STAGE1)
extern const char *blver;

void set_ver(void)
{
	int rc;

	rc = replace(0x18a, (unsigned char*)blver, 4);
	if (rc)
		printf("\n### [set boot ver] flash write fail\n");
}

void init_mac(void)
{
	uint8_t ptr[8];
	int rc;

	printf("\ninit mac\n");
	memset(ptr, 0, sizeof(ptr));
	ptr[0] = 0x00;
	ptr[1] = 0x11;
	ptr[2] = 0x11;
	ptr[3] = 0x11;
	ptr[4] = 0x11;
	ptr[5] = 0x11;

	rc = replace(0x04, ptr, 6);

	if (rc)
		printf("\n### [init mac] flash write fail\n");
	else
		printf("\n### [init mac] flash write ok\n");

#if defined(DUAL_BAND)
	ptr[0] = 0x00;
	ptr[1] = 0x22;
	ptr[2] = 0x22;
	ptr[3] = 0x22;
	ptr[4] = 0x22;
	ptr[5] = 0x22;

	rc = replace(0x8004, ptr, 6);

	if (rc)
		printf("\n### [init mac2] flash write fail\n");
	else
		printf("\n### [init mac2] flash write ok\n");
#endif

	ptr[0] = 0x44;
	ptr[1] = 0x42;

	rc = replace(0x188, ptr, 2);

	if (rc)
		printf("\n### [init countrycode] flash write fail\n");
	else
		printf("\n### [init countrycode] flash write ok\n");

	ptr[0] = 0x31;
	ptr[1] = 0x32;
	ptr[2] = 0x33;
	ptr[3] = 0x34;
	ptr[4] = 0x35;
	ptr[5] = 0x36;
	ptr[6] = 0x37;
	ptr[7] = 0x30;

	rc = replace(0x180, ptr, 8);

	if (rc)
		printf("\n### [init pincode] flash write fail\n");
	else
		printf("\n### [init pincode] flash write ok\n");
}
#endif /* ASUS_PRODUCE && ! UBOOT_STAGE1 */

static void Init_System_Mode(void)
{
	u32 reg;
#if 	defined(RT2880_ASIC_BOARD) || \
	defined(RT2883_ASIC_BOARD) || \
	defined(RT3052_ASIC_BOARD) || \
	defined(RT3352_ASIC_BOARD) || \
	defined(RT3883_ASIC_BOARD) || \
	defined(RT5350_ASIC_BOARD) || \
	defined(RT6855_ASIC_BOARD)
	u8	clk_sel;
#endif
#if defined(RT5350_ASIC_BOARD)
	u8	clk_sel2;
#endif

	reg = RALINK_REG(RT2880_SYSCFG_REG);
		
	/* 
	 * CPU_CLK_SEL (bit 21:20)
	 */
#ifdef RT2880_FPGA_BOARD
	mips_cpu_feq = 25 * 1000 *1000;
	mips_bus_feq = mips_cpu_feq/2;
#elif defined (RT2883_FPGA_BOARD) || defined (RT3052_FPGA_BOARD) || defined (RT3352_FPGA_BOARD) || defined (RT5350_FPGA_BOARD)
	mips_cpu_feq = 40 * 1000 *1000;
	mips_bus_feq = mips_cpu_feq/3;
#elif defined (RT6855A_FPGA_BOARD)
	mips_cpu_feq = 50 * 1000 *1000;
	mips_bus_feq = mips_cpu_feq/2;
#elif defined (RT3883_FPGA_BOARD)
	mips_cpu_feq = 40 * 1000 *1000;
	mips_bus_feq = mips_cpu_feq;
#elif defined (RT6855_FPGA_BOARD) || defined (MT7620_FPGA_BOARD)
	/* FIXME */
	mips_cpu_feq = 50 * 1000 *1000;
	mips_bus_feq = mips_cpu_feq/4;
#elif defined (MT7621_FPGA_BOARD)
	mips_cpu_feq = 35 * 1000 *1000;
	mips_bus_feq = mips_cpu_feq;
#elif defined (RT2883_ASIC_BOARD) 
	clk_sel = (reg>>20) & 0x03;
	switch(clk_sel) {
		case 0:
			mips_cpu_feq = (380*1000*1000);
			break;
		case 1:
			mips_cpu_feq = (400*1000*1000);
			break;
		case 2:
			mips_cpu_feq = (420*1000*1000);
			break;
		case 3:
			mips_cpu_feq = (430*1000*1000);
			break;
	}
	mips_bus_feq = mips_cpu_feq/2;
#elif defined(RT3052_ASIC_BOARD)
#if defined(RT3350_ASIC_BOARD) 
	//MA10 is floating
	mips_cpu_feq = (320*1000*1000);
#else
        clk_sel = (reg>>18) & 0x01;
        switch(clk_sel) {
                case 0:
                        mips_cpu_feq = (320*1000*1000);
                        break;
                case 1:
                        mips_cpu_feq = (384*1000*1000);
                        break;
        }
#endif
        mips_bus_feq = mips_cpu_feq / 3;
#elif defined(RT3352_ASIC_BOARD)
	clk_sel = (reg>>8) & 0x01;
	switch(clk_sel) {
		case 0:
			mips_cpu_feq = (384*1000*1000);
			break;
		case 1:
			mips_cpu_feq = (400*1000*1000);
			break;
	}
	mips_bus_feq = (133*1000*1000);
#elif defined(RT5350_ASIC_BOARD)
	clk_sel2 = (reg>>10) & 0x01;
	clk_sel = ((reg>>8) & 0x01) + (clk_sel2 * 2);
	switch(clk_sel) {
		case 0:
			mips_cpu_feq = (360*1000*1000);
			mips_bus_feq = (120*1000*1000);
			break;
		case 1:
			//reserved
			break;
		case 2:
			mips_cpu_feq = (320*1000*1000);
			mips_bus_feq = (80*1000*1000);
			break;
		case 3:
			mips_cpu_feq = (300*1000*1000);
			mips_bus_feq = (100*1000*1000);
			break;
	}
#elif defined(RT6855_ASIC_BOARD)
	mips_cpu_feq = (400*1000*1000);
	mips_bus_feq = (133*1000*1000);
#elif defined (RT6855A_ASIC_BOARD)
	/* FPGA is 25/32Mhz
	 * ASIC RT6856/RT63368: DDR(0): 233.33, DDR(1): 175, SDR: 140
	 *      RT6855/RT6855A: DDR(0): 166.67, DDR(1): 125, SDR: 140 */
	reg = RALINK_REG(RT2880_SYSCFG_REG);
	if ((reg & (1 << 25)) == 0) { /* SDR */
		if ((reg & (1 << 9)) != 0)
			mips_cpu_feq = (560*1000*1000);
		else {
			if ((reg & (1 << 26)) != 0)	
				mips_cpu_feq = (560*1000*1000);
			else
				mips_cpu_feq = (420*1000*1000);
		}	
		mips_bus_feq = (140*1000*1000);
	} else { /* DDR */
		if ((reg & (1 << 9)) != 0) {
			mips_cpu_feq = (700*1000*1000);
			if ((reg & (1 << 26)) != 0)
				mips_bus_feq = (175*1000*1000);
			else
				mips_bus_feq = 233333333;
		} else {
			mips_cpu_feq = (500*1000*1000);
			if ((reg & (1 << 26)) != 0)
				mips_bus_feq = (125*1000*1000);
			else
				mips_bus_feq = 166666667;
		}
	}
#elif defined(MT7620_ASIC_BOARD)
	reg = RALINK_REG(RALINK_CPLLCFG1_REG);
	if( reg & ((0x1UL) << 24) ){
		mips_cpu_feq = (480*1000*1000);	/* from BBP PLL */
	}else{
		reg = RALINK_REG(RALINK_CPLLCFG0_REG);
		if(!(reg & CPLL_SW_CONFIG)){
			mips_cpu_feq = (600*1000*1000); /* from CPU PLL */
		}else{
			/* read CPLL_CFG0 to determine real CPU clock */
			int mult_ratio = (reg & CPLL_MULT_RATIO) >> CPLL_MULT_RATIO_SHIFT;
			int div_ratio = (reg & CPLL_DIV_RATIO) >> CPLL_DIV_RATIO_SHIFT;
			mult_ratio += 24;       /* begin from 24 */
			if(div_ratio == 0)      /* define from datasheet */
				div_ratio = 2;
			else if(div_ratio == 1)
				div_ratio = 3;
			else if(div_ratio == 2)
				div_ratio = 4;
			else if(div_ratio == 3)
				div_ratio = 8;
			mips_cpu_feq = ((BASE_CLOCK * mult_ratio ) / div_ratio) * 1000 * 1000;
		}
	}
	reg = (RALINK_REG(RT2880_SYSCFG_REG)) >> 4 & 0x3;
	if(reg == 0x0){				/* SDR (MT7620 E1) */
		mips_bus_feq = mips_cpu_feq/4;
	}else if(reg == 0x1 || reg == 0x2 ){	/* DDR1 & DDR2 */
		mips_bus_feq = mips_cpu_feq/3;
	}else{					/* SDR (MT7620 E2) */
		mips_bus_feq = mips_cpu_feq/5;
	}
#elif defined(MT7621_ASIC_BOARD)
	reg = RALINK_REG(RALINK_CPLLCFG1_REG);
	if( reg & ((0x1UL) << 24) ){
		mips_cpu_feq = (480*1000*1000);	/* from BBP PLL */
	}else{
		reg = RALINK_REG(RALINK_CPLLCFG0_REG);
		if(!(reg & CPLL_SW_CONFIG)){
			mips_cpu_feq = (600*1000*1000); /* from CPU PLL */
		}else{
			/* read CPLL_CFG0 to determine real CPU clock */
			int mult_ratio = (reg & CPLL_MULT_RATIO) >> CPLL_MULT_RATIO_SHIFT;
			int div_ratio = (reg & CPLL_DIV_RATIO) >> CPLL_DIV_RATIO_SHIFT;
			mult_ratio += 24;       /* begin from 24 */
			if(div_ratio == 0)      /* define from datasheet */
				div_ratio = 2;
			else if(div_ratio == 1)
				div_ratio = 3;
			else if(div_ratio == 2)
				div_ratio = 4;
			else if(div_ratio == 3)
				div_ratio = 8;
			mips_cpu_feq = ((BASE_CLOCK * mult_ratio ) / div_ratio) * 1000 * 1000;
		}
	}
	reg = (RALINK_REG(RT2880_SYSCFG_REG)) >> 4 & 0x3;
	if(!reg){	/* SDR  */
		mips_bus_feq = mips_cpu_feq/4;
	}else{		/* DDR1 & DDR2 */
		mips_bus_feq = mips_cpu_feq/3;
	}
#elif defined (RT3883_ASIC_BOARD) 
	clk_sel = (reg>>8) & 0x03;
	switch(clk_sel) {
		case 0:
			mips_cpu_feq = (250*1000*1000);
			break;
		case 1:
			mips_cpu_feq = (384*1000*1000);
			break;
		case 2:
			mips_cpu_feq = (480*1000*1000);
			break;
		case 3:
			mips_cpu_feq = (500*1000*1000);
			break;
	}
#if defined (CFG_ENV_IS_IN_SPI)
	if ((reg>>17) & 0x1) { //DDR2
		switch(clk_sel) {
			case 0:
				mips_bus_feq = (125*1000*1000);
				break;
			case 1:
				mips_bus_feq = (128*1000*1000);
				break;
			case 2:
				mips_bus_feq = (160*1000*1000);
				break;
			case 3:
				mips_bus_feq = (166*1000*1000);
				break;
		}
	}
	else {
		switch(clk_sel) {
			case 0:
				mips_bus_feq = (83*1000*1000);
				break;
			case 1:
				mips_bus_feq = (96*1000*1000);
				break;
			case 2:
				mips_bus_feq = (120*1000*1000);
				break;
			case 3:
				mips_bus_feq = (125*1000*1000);
				break;
		}
	}
#elif defined ON_BOARD_SDR
	switch(clk_sel) {
		case 0:
			mips_bus_feq = (83*1000*1000);
			break;
		case 1:
			mips_bus_feq = (96*1000*1000);
			break;
		case 2:
			mips_bus_feq = (120*1000*1000);
			break;
		case 3:
			mips_bus_feq = (125*1000*1000);
			break;
	}
#elif defined ON_BOARD_DDR2
	switch(clk_sel) {
		case 0:
			mips_bus_feq = (125*1000*1000);
			break;
		case 1:
			mips_bus_feq = (128*1000*1000);
			break;
		case 2:
			mips_bus_feq = (160*1000*1000);
			break;
		case 3:
			mips_bus_feq = (166*1000*1000);
			break;
	}
#else
#error undef SDR or DDR
#endif
#else /* RT2880 ASIC version */
	clk_sel = (reg>>20) & 0x03;
	switch(clk_sel) {
		#ifdef RT2880_MP
			case 0:
			mips_cpu_feq = (250*1000*1000);
			break;
		case 1:
			mips_cpu_feq = (266*1000*1000);
			break;
		case 2:
			mips_cpu_feq = (280*1000*1000);
			break;
		case 3:
			mips_cpu_feq = (300*1000*1000);
			break;
		#else
			case 0:
			mips_cpu_feq = (233*1000*1000);
			break;
		case 1:
			mips_cpu_feq = (250*1000*1000);
			break;
		case 2:
			mips_cpu_feq = (266*1000*1000);
			break;
		case 3:
			mips_cpu_feq = (280*1000*1000);
			break;
		
		#endif
	}
	mips_bus_feq = mips_cpu_feq/2;
#endif

   	//RALINK_REG(RT2880_SYSCFG_REG) = reg;

	/* in general, the spec define 8192 refresh cycles/64ms
	 * 64ms/8192 = 7.8us
	 * 7.8us * 106.7Mhz(SDRAM clock) = 832
	 * the value of refresh cycle shall smaller than 832. 
	 * so we config it at 0x300 (suggested by ASIC)
	 */
#if defined(ON_BOARD_SDR) && defined(ON_BOARD_256M_DRAM_COMPONENT) && (!defined(MT7620_ASIC_BOARD))
	{
	u32 tREF;
	tREF = RALINK_REG(SDRAM_CFG1_REG);
	tREF &= 0xffff0000;
#if defined(ASIC_BOARD)
	tREF |= 0x00000300;
#elif defined(FPGA_BOARD) 
	tREF |= 0x000004B;
#else
#error "not exist"
#endif
	RALINK_REG(SDRAM_CFG1_REG) = tREF;
	}
#endif

}


/*
 * The Malloc area is immediately below the monitor copy in DRAM
 */
static void mem_malloc_init (void)
{

	ulong dest_addr = CFG_MONITOR_BASE + gd->reloc_off;

	mem_malloc_end = dest_addr;
	mem_malloc_start = dest_addr - TOTAL_MALLOC_LEN;
	mem_malloc_brk = mem_malloc_start;

	memset ((void *) mem_malloc_start,
		0,
		mem_malloc_end - mem_malloc_start);
}

void *sbrk (ptrdiff_t increment)
{
	ulong old = mem_malloc_brk;
	ulong new = old + increment;

	/*
	 * if we are giving memory back make sure we clear it out since
	 * we set MORECORE_CLEARS to 1
	 */
	if (increment < 0)
		memset((void *)new, 0, -increment);

	if ((new < mem_malloc_start) || (new > mem_malloc_end)) {
		return (void*)MORECORE_FAILURE;
	}
	mem_malloc_brk = new;
	return ((void *) old);
}

static int init_func_ram (void)
{

#ifdef	CONFIG_BOARD_TYPES
	int board_type = gd->board_type;
#else
	int board_type = 0;	/* use dummy arg */
#endif
	puts ("DRAM:  ");

/*init dram config*/
#ifdef RALINK_DDR_OPTIMIZATION
#ifdef ON_BOARD_DDR2
/*optimize ddr parameter*/
{	
	u32 tDDR;
	tDDR = RALINK_REG(DDR_CFG0_REG);

        tDDR &= 0xf0780000; 
	tDDR |=  RAS_VALUE << RAS_OFFSET;
	tDDR |=  TRFC_VALUE << TRFC_OFFSET;
	tDDR |=  TRFI_VALUE << TRFI_OFFSET;
	RALINK_REG(DDR_CFG0_REG) = tDDR;
}
#endif
#endif


	if ((gd->ram_size = initdram (board_type)) > 0) {
		print_size (gd->ram_size, "\n");
		return (0);  
	}
	puts ("*** failed ***\n");

	return (1);
}

static int display_banner(void)
{
	char *line="";

#if defined(UBOOT_STAGE1)
	line = "========= STAGE 1 =========";
#elif defined(UBOOT_STAGE2)
	line = "========= STAGE 2 =========";
#endif
	printf ("\n\n%s\n%s\n\n", line, version_string);
	return (0);
}

/*
static void display_flash_config(ulong size)
{
	puts ("Flash: ");
	print_size (size, "\n");
}
*/

static int init_baudrate (void)
{
	//uchar tmp[64]; /* long enough for environment variables */
	//int i = getenv_r ("baudrate", tmp, sizeof (tmp));
	//kaiker 
	gd->baudrate = CONFIG_BAUDRATE;
/*
	gd->baudrate = (i > 0)
			? (int) simple_strtoul (tmp, NULL, 10)
			: CONFIG_BAUDRATE;
*/
	return (0);
}


/*
 * Breath some life into the board...
 *
 * The first part of initialization is running from Flash memory;
 * its main purpose is to initialize the RAM so that we
 * can relocate the monitor code to RAM.
 */

/*
 * All attempts to come up with a "common" initialization sequence
 * that works for all boards and architectures failed: some of the
 * requirements are just _too_ different. To get rid of the resulting
 * mess of board dependend #ifdef'ed code we now make the whole
 * initialization sequence configurable to the user.
 *
 * The requirements for any new initalization function is simple: it
 * receives a pointer to the "global data" structure as it's only
 * argument, and returns an integer return code, where 0 means
 * "continue" and != 0 means "fatal error, hang the system".
 */
#if 0
typedef int (init_fnc_t) (void);

init_fnc_t *init_sequence[] = {
	timer_init,
	env_init,		/* initialize environment */
	init_baudrate,		/* initialze baudrate settings */
	serial_init,		/* serial communications setup */
	console_init_f,
	display_banner,		/* say that we are here */
	checkboard,
	init_func_ram,
	NULL,
};
#endif

//  
void board_init_f(ulong bootflag)
{
	gd_t gd_data, *id;
	bd_t *bd;  
	//init_fnc_t **init_fnc_ptr;
	ulong addr, addr_sp, len = (ulong)&uboot_end - CFG_MONITOR_BASE;
	ulong *s;
	u32 value;
    u32 fdiv = 0, step = 0;

#if defined(MT7620_FPGA_BOARD) || defined(MT7620_ASIC_BOARD)
/* Adjust CPU Freq from 60Mhz to 600Mhz(or CPLL freq stored from EE) */
	value = RALINK_REG(RT2880_SYSCLKCFG_REG);
	fdiv = ((value>>8)&0x1F);
	step = (unsigned long)(value&0x1F);
	while(step < fdiv) {
		value = RALINK_REG(RT2880_SYSCLKCFG_REG);
		step = (unsigned long)(value&0x1F) + 1;
		value &= ~(0x1F);
		value |= (step&0x1F);
		RALINK_REG(RT2880_SYSCLKCFG_REG) = value;
		udelay(10);
	};	

#endif	
#ifdef CONFIG_PURPLE
	void copy_code (ulong); 
#endif
	//*pio_mode = 0xFFFF;

	/* Pointer is writable since we allocated a register for it.
	 */
	gd = &gd_data;
	/* compiler optimization barrier needed for GCC >= 3.4 */
	__asm__ __volatile__("": : :"memory");
	
		
	memset ((void *)gd, 0, sizeof (gd_t));

#if defined (RT6855A_ASIC_BOARD) || defined(RT6855A_FPGA_BOARD)	
	watchdog_reset();
#endif
	timer_init();
	env_init();		/* initialize environment */
	init_baudrate();		/* initialze baudrate settings */
	serial_init();		/* serial communications setup */
	console_init_f();
	display_banner();		/* say that we are here */
	checkboard();

	init_func_ram(); 

#ifdef ASUS_PRODUCT
	led_init(); // turn_on_led
	gpio_init();
#if !defined(ASUS_RTN14U) && !defined(ASUS_RTAC52U) && !defined(ASUS_RTAC51U) && !defined(ASUS_RTN11P) && !defined(ASUS_RTN54U)
	LEDOFF();
#endif
#endif	

	/* reset Frame engine */
	value = le32_to_cpu(*(volatile u_long *)(RALINK_SYSCTL_BASE + 0x0034));
	udelay(100);    
#if defined (RT2880_FPGA_BOARD) || defined (RT2880_ASIC_BOARD)
	value |= (1 << 18);
#else
	//2880 -> 3052 reset Frame Engine from 18 to 21
	value |= (1 << 21);
#endif
	*(volatile u_long *)(RALINK_SYSCTL_BASE + 0x0034) = cpu_to_le32(value);	
#if defined (RT2880_FPGA_BOARD) || defined (RT2880_ASIC_BOARD)
	value &= ~(1 << 18);
#else
	value &= ~(1 << 21);
#endif
	*(volatile u_long *)(RALINK_SYSCTL_BASE + 0x0034) = cpu_to_le32(value);	
	udelay(200);      

#if 0	
	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
	
		if ((*init_fnc_ptr)() != 0) {
			hang ();
		}
	}
#endif
#ifdef DEBUG	
	debug("rt2880 uboot %s %s\n", VERSION, DATE);
#endif

	/*
	 * Now that we have DRAM mapped and working, we can
	 * relocate the code and continue running from DRAM.
	 */
	addr = CFG_SDRAM_BASE + gd->ram_size;

	/* We can reserve some RAM "on top" here.
	 */
#ifdef DEBUG	    
	debug ("SERIAL_CLOCK_DIVISOR =%d \n", SERIAL_CLOCK_DIVISOR);
	debug ("kaiker,,CONFIG_BAUDRATE =%d \n", CONFIG_BAUDRATE); 
	debug ("SDRAM SIZE:%08X\n",gd->ram_size);
#endif

	/* round down to next 4 kB limit.
	 */
	addr &= ~(4096 - 1);
#ifdef DEBUG
	debug ("Top of RAM usable for U-Boot at: %08lx\n", addr);
#endif	 
   
	/* Reserve memory for U-Boot code, data & bss
	 * round down to next 16 kB limit
	 */
	addr -= len;
	addr &= ~(16 * 1024 - 1);
#ifdef DEBUG
	debug ("Reserving %ldk for U-Boot at: %08lx\n", len >> 10, addr);
#endif
	 /* Reserve memory for malloc() arena.
	 */
	addr_sp = addr - TOTAL_MALLOC_LEN;
#ifdef DEBUG
	debug ("Reserving %dk for malloc() at: %08lx\n",
			TOTAL_MALLOC_LEN >> 10, addr_sp);
#endif
	/*
	 * (permanently) allocate a Board Info struct
	 * and a permanent copy of the "global" data
	 */
	addr_sp -= sizeof(bd_t);
	bd = (bd_t *)addr_sp;
	gd->bd = bd;
#ifdef DEBUG
	debug ("Reserving %d Bytes for Board Info at: %08lx\n",
			sizeof(bd_t), addr_sp);
#endif
	addr_sp -= sizeof(gd_t);
	id = (gd_t *)addr_sp;
#ifdef DEBUG
	debug ("Reserving %d Bytes for Global Data at: %08lx\n",
			sizeof (gd_t), addr_sp);
#endif
 	/* Reserve memory for boot params.
	 */
	addr_sp -= CFG_BOOTPARAMS_LEN;
	bd->bi_boot_params = addr_sp;
#ifdef DEBUG
	debug ("Reserving %dk for boot params() at: %08lx\n",
			CFG_BOOTPARAMS_LEN >> 10, addr_sp);
#endif
	/*
	 * Finally, we set up a new (bigger) stack.
	 *
	 * Leave some safety gap for SP, force alignment on 16 byte boundary
	 * Clear initial stack frame
	 */
	addr_sp -= 16;
	addr_sp &= ~0xF;
	s = (ulong *)addr_sp;
	*s-- = 0;
	*s-- = 0;
	addr_sp = (ulong)s;
#ifdef DEBUG
	debug ("Stack Pointer at: %08lx\n", addr_sp);
#endif

	/*
	 * Save local variables to board info struct
	 */
	bd->bi_memstart	= CFG_SDRAM_BASE;	/* start of  DRAM memory */
	bd->bi_memsize	= gd->ram_size;		/* size  of  DRAM memory in bytes */
	bd->bi_baudrate	= gd->baudrate;		/* Console Baudrate */

	memcpy (id, (void *)gd, sizeof (gd_t));

	/* On the purple board we copy the code in a special way
	 * in order to solve flash problems
	 */
#ifdef CONFIG_PURPLE
	copy_code(addr);
#endif

#if defined RT6855_FPGA_BOARD || defined MT7620_FPGA_BOARD  || MT7621_FPGA_BOARD 
	value = le32_to_cpu(*(volatile u_long *)(RALINK_SYSCTL_BASE + 0x0B10));
	value &= ~(0x7);
	*(volatile u_long *)(RALINK_SYSCTL_BASE + 0x0B10) = cpu_to_le32(value);	
#elif defined MT7620_ASIC_BOARD
	value = le32_to_cpu(*(volatile u_long *)(RALINK_SYSCTL_BASE + 0x0B10));
	value &= ~(0x7);
	value |= 0x2;
	*(volatile u_long *)(RALINK_SYSCTL_BASE + 0x0B10) = cpu_to_le32(value);	
#endif

#if defined(CFG_RUN_CODE_IN_RAM)
	/* 
	 * tricky: relocate code to original TEXT_BASE
	 * for ICE souce level debuggind mode 
	 */	
	debug ("relocate_code Pointer at: %08lx\n", addr);
	relocate_code (addr_sp, id, /*TEXT_BASE*/ addr);	
#else
	debug ("relocate_code Pointer at: %08lx\n", addr);
	relocate_code (addr_sp, id, addr);
#endif

	/* NOTREACHED - relocate_code() does not return */
}

#define SEL_LOAD_LINUX_WRITE_FLASH_BY_SERIAL 0
#define SEL_LOAD_LINUX_SDRAM            1
#define SEL_LOAD_LINUX_WRITE_FLASH      2
#define SEL_BOOT_FLASH                  3
#define SEL_ENTER_CLI                   4

#if defined(ASUS_RTN14U) || defined(ASUS_RTAC52U) || defined(ASUS_RTAC51U) || defined(ASUS_RTN11P) || defined(ASUS_RTN54U) 
#define SEL_LOAD_BOOT_SDRAM_VIA_SERIAL  5
#endif

#define SEL_LOAD_BOOT_WRITE_FLASH_BY_SERIAL 7
#define SEL_LOAD_BOOT_SDRAM             8
#define SEL_LOAD_BOOT_WRITE_FLASH       9


void OperationSelect(void)
{
	printf("\nPlease choose the operation: \n");

	printf("   %d: Load %s then write to Flash via Serial. \n", SEL_LOAD_LINUX_WRITE_FLASH_BY_SERIAL, sys_image_name);
#if !defined(UBOOT_STAGE1)
	printf("   %d: Load %s to SDRAM via TFTP. \n", SEL_LOAD_LINUX_SDRAM, sys_image_name);
#endif
	printf("   %d: Load %s then write to Flash via TFTP. \n", SEL_LOAD_LINUX_WRITE_FLASH, sys_image_name);

	printf("   %d: Boot %s via Flash (default).\n", SEL_BOOT_FLASH, sys_image_name);

#ifdef RALINK_CMDLINE
	printf("   %d: Entr boot command line interface.\n", SEL_ENTER_CLI);
#endif // RALINK_CMDLINE //

#if defined(ASUS_RTN14U) || defined(ASUS_RTAC52U) || defined(ASUS_RTAC51U) || defined(ASUS_RTN11P)|| defined(ASUS_RTN54U)
#if !defined(UBOOT_STAGE1)
	printf("   %d: Load %s to SDRAM via Serial. \n", SEL_LOAD_BOOT_SDRAM_VIA_SERIAL, boot_image_name);
#endif
#endif	
#ifdef RALINK_UPGRADE_BY_SERIAL
	printf("   %d: Load %s then write to Flash via Serial. \n", SEL_LOAD_BOOT_WRITE_FLASH_BY_SERIAL, boot_image_name);
#endif // RALINK_UPGRADE_BY_SERIAL //

#if defined(ASUS_RTN14U) || defined(ASUS_RTAC52U) || defined(ASUS_RTAC51U) || defined(ASUS_RTN11P)  || defined(ASUS_RTN54U)
#if !defined(UBOOT_STAGE1)
	printf("   %d: Load %s to SDRAM via TFTP. \n", SEL_LOAD_BOOT_SDRAM, boot_image_name);
#endif
#endif
	printf("   %d: Load %s then write to Flash via TFTP. \n", SEL_LOAD_BOOT_WRITE_FLASH, boot_image_name);
}

#if defined(ASUS_RTN14U) || defined(ASUS_RTAC52U) || defined(ASUS_RTAC51U) || defined(ASUS_RTN11P) || defined(ASUS_RTN54U)
void filename_copy (uchar *dst, uchar *src, int size)
{
	*dst = '"';
	dst++;
	while ((size > 0) && *src && (*src != '"')) {
		*dst++ = *src++;
		size--;
	}
	*dst++ = '"';
	*dst = '\0';
}
#endif
int tftp_config(int type, char *argv[])
{
	char *s;
	char default_file[ARGV_LEN], file[ARGV_LEN], devip[ARGV_LEN], srvip[ARGV_LEN], default_ip[ARGV_LEN];
	static char buf_addr[] = "0x80100000XXX";

	printf(" Please Input new ones /or Ctrl-C to discard\n");

	memset(default_file, 0, ARGV_LEN);
	memset(file, 0, ARGV_LEN);
	memset(devip, 0, ARGV_LEN);
	memset(srvip, 0, ARGV_LEN);
	memset(default_ip, 0, ARGV_LEN);

	printf("\tInput device IP ");
	s = getenv("ipaddr");
	memcpy(devip, s, strlen(s));
	memcpy(default_ip, s, strlen(s));

	printf("(%s) ", devip);
	input_value(devip);
	setenv("ipaddr", devip);
	if (strcmp(default_ip, devip) != 0)
		modifies++;

	printf("\tInput server IP ");
	s = getenv("serverip");
	memcpy(srvip, s, strlen(s));
	memset(default_ip, 0, ARGV_LEN);
	memcpy(default_ip, s, strlen(s));

	printf("(%s) ", srvip);
	input_value(srvip);
	setenv("serverip", srvip);
	if (strcmp(default_ip, srvip) != 0)
		modifies++;

	if(type == SEL_LOAD_BOOT_SDRAM 
			|| type == SEL_LOAD_BOOT_WRITE_FLASH 
#ifdef RALINK_UPGRADE_BY_SERIAL
			|| type == SEL_LOAD_BOOT_WRITE_FLASH_BY_SERIAL
#endif
			) {
		if(type == SEL_LOAD_BOOT_SDRAM) {
#if defined (RT2880_ASIC_BOARD) || defined (RT2880_FPGA_BOARD)
			argv[1] = "0x8a200000";
#else
		sprintf(buf_addr, "0x%x", CFG_LOAD_ADDR);
		argv[1] = buf_addr;
#endif
		} else {
#if defined (RT2880_ASIC_BOARD) || defined (RT2880_FPGA_BOARD)
			argv[1] = "0x8a100000";
#else
		sprintf(buf_addr, "0x%x", CFG_LOAD_ADDR);
		argv[1] = buf_addr;
#endif
		}
		printf("\tInput Uboot filename ");
		//argv[2] = "uboot.bin";
		strncpy(argv[2], "uboot.bin", ARGV_LEN);
	}
	else if (type == SEL_LOAD_LINUX_WRITE_FLASH) {
#if defined (RT2880_ASIC_BOARD) || defined (RT2880_FPGA_BOARD)
		argv[1] = "0x8a100000";
#else
		sprintf(buf_addr, "0x%x", CFG_LOAD_ADDR);
		argv[1] = buf_addr;
#endif
		printf("\tInput Linux Kernel filename ");
		//argv[2] = "uImage"; winfred: use strncpy instead to prevent the buffer overflow at copy_filename later
		strncpy(argv[2], "uImage", ARGV_LEN);
	}
	else if (type == SEL_LOAD_LINUX_SDRAM ) {
		/* bruce to support ramdisk */
#if defined (RT2880_ASIC_BOARD) || defined (RT2880_FPGA_BOARD)
		argv[1] = "0x8a800000";
#else
#ifdef ASUS_PRODUCT
		sprintf(buf_addr, "0x%x", CFG_LOAD_ADDR);
		argv[1] = buf_addr;
#else          
               argv[1] = "0x80A00000";
#endif   
#endif
		printf("\tInput Linux Kernel filename ");
		//argv[2] = "uImage";
		strncpy(argv[2], "uImage", ARGV_LEN);
	}

	s = getenv("bootfile");
	if (s != NULL) {
		memcpy(file, s, strlen(s));
		memcpy(default_file, s, strlen(s));
	}
	printf("(%s) ", file);
	input_value(file);
	if (file == NULL)
		return 1;
	copy_filename (argv[2], file, sizeof(file));
	setenv("bootfile", file);
	if (strcmp(default_file, file) != 0)
		modifies++;

	return 0;
}

void trigger_hw_reset(void)
{
#ifdef GPIO14_RESET_MODE
        //set GPIO14 as output to trigger hw reset circuit
        RALINK_REG(RT2880_REG_PIODIR)|=1<<14; //output mode

        RALINK_REG(RT2880_REG_PIODATA)|=1<<14; //pull high
	udelay(100);
        RALINK_REG(RT2880_REG_PIODATA)&=~(1<<14); //pull low
#endif
}

#ifdef DUAL_IMAGE_SUPPORT

/* 
 * dir=1: Image1 to Image2
 * dir=2: Image2 to Image1
 */
int copy_image(int dir, unsigned long image_size) 
{
	int ret = 0;
#ifdef CFG_ENV_IS_IN_FLASH
	unsigned long e_end, len;
#endif

	if (dir == 1) {
		printf("\nCopy Image:\nImage1(0x%X) to Image2(0x%X), size=0x%X\n",
				CFG_KERN_ADDR - CFG_FLASH_BASE,
				CFG_KERN2_ADDR - CFG_FLASH_BASE, image_size);
		ra_flash_read(CFG_SPINAND_LOAD_ADDR, CFG_KERN_ADDR, image_size);
		ret = ra_flash_erase_write((uchar*) CFG_SPINAND_LOAD_ADDR, CFG_KERN2_ADDR, image_size, 0);
	}
	else if (dir == 2) {
		printf("\nCopy Image:\nImage2(0x%X) to Image1(0x%X), size=0x%X\n",
				CFG_KERN2_ADDR - CFG_FLASH_BASE,
				CFG_KERN_ADDR - CFG_FLASH_BASE, image_size);
		ra_flash_read(CFG_SPINAND_LOAD_ADDR, CFG_KERN2_ADDR, image_size);
		ret = ra_flash_erase_write((uchar*) CFG_SPINAND_LOAD_ADDR, CFG_KERN_ADDR, image_size, 0);
#if defined(CFG_ENV_IS_IN_FLASH)
#warning	FIXME: rewrite below code to new flash wrapper function.
		printf("\nCopy Image:\nImage2(0x%X) to Image1(0x%X), size=0x%X\n", CFG_KERN2_ADDR, CFG_KERN_ADDR, image_size);
#if defined (ON_BOARD_16M_FLASH_COMPONENT) && (defined (RT2880_ASIC_BOARD) || defined (RT2880_FPGA_BOARD) || defined (RT3052_MP1))
		len = 0x400000 - (CFG_BOOTLOADER_SIZE + CFG_CONFIG_SIZE + CFG_FACTORY_SIZE);
		if (image_size <= len) {
			e_end = CFG_KERN_ADDR + image_size - 1;
			if (get_addr_boundary(&e_end) != 0)
				return -1;
		        printf("Erase from 0x%X To 0x%X\n", CFG_KERN_ADDR, e_end);
			flash_sect_erase(CFG_KERN_ADDR, e_end);
			memcpy(CFG_LOAD_ADDR, (void *)CFG_KERN2_ADDR, image_size);
			ret = flash_write((uchar *)CFG_LOAD_ADDR, (ulong)CFG_KERN_ADDR, image_size);
		}
		else {
			e_end = CFG_KERN_ADDR + len - 1;
			if (get_addr_boundary(&e_end - 1) != 0)
				return -1;
		        printf("Erase from 0x%X To 0x%X\n", CFG_KERN_ADDR, e_end);
			flash_sect_erase(CFG_KERN_ADDR, e_end);
			e_end = PHYS_FLASH_2 + (image_size - len) - 1;
			if (get_addr_boundary(&e_end) != 0)
				return -1;
	        	printf("From 0x%X To 0x%X\n", PHYS_FLASH_2, e_end);
			flash_sect_erase(PHYS_FLASH_2, e_end);
			memcpy(CFG_LOAD_ADDR, (void *)CFG_KERN2_ADDR, image_size);
			ret = flash_write((uchar *)CFG_LOAD_ADDR, (ulong)CFG_KERN_ADDR, len);
			ret = flash_write((uchar *)(CFG_LOAD_ADDR + len), (ulong)PHYS_FLASH_2, image_size - len);
		}
#else
		e_end = CFG_KERN_ADDR + image_size - 1;
		if (get_addr_boundary(&e_end) != 0)
			return -1;
		printf("Erase from 0x%X to 0x%X\n", CFG_KERN_ADDR, e_end);
		flash_sect_erase(CFG_KERN_ADDR, e_end);
		memcpy(CFG_LOAD_ADDR, (void *)CFG_KERN2_ADDR, image_size);
		ret = flash_write((uchar *)CFG_LOAD_ADDR, (ulong)CFG_KERN_ADDR, image_size);
#endif
#endif
		if (ret == 0) {
			setenv("Image1Stable", "0");
			setenv("Image1Try", "0");
			saveenv();
		}
	}
	else
		ret = -1;

	return ret;

}

int debug_mode(void)
{
	printf("Upgrade Mode~~\n");

	return 0;
}

#define MAX_TRY_TIMES 3
int check_image_validation(void)
{
	int ret = 0;
	int broken1 = 0, broken2 = 0;
	unsigned long len = 0, chksum = 0;
	image_header_t hdr1, hdr2;
	unsigned char *hdr1_addr, *hdr2_addr;
	char *stable, *try;
	
	hdr1_addr = (unsigned char *)CFG_KERN_ADDR;
	hdr2_addr = (unsigned char *)CFG_KERN2_ADDR;
	
	ra_flash_read((char*) &hdr1, hdr1_addr, sizeof(image_header_t));
	ra_flash_read((char*) &hdr2, hdr2_addr, sizeof(image_header_t));

	printf("\n=================================================\n");
	printf("Check image validation:\n");

	/* Check header magic number */
	printf ("Image1 Header Magic Number --> ");
	if (ntohl(hdr1.ih_magic) != IH_MAGIC) {
		broken1 = 1;
		printf("Failed\n");
	}
	else
		printf("OK\n");

	printf ("Image2 Header Magic Number --> ");
	if (ntohl(hdr2.ih_magic) != IH_MAGIC) {
		broken2 = 1;
		printf("Failed\n");
	}
	else
		printf("OK\n");

	/* Check header crc */
	/* Skip crc checking if there is no valid header, or it may hang on due to broken header length */
	if (broken1 == 0) {
		printf("Image1 Header Checksum --> ");
		len  = sizeof(image_header_t);
		chksum = ntohl(hdr1.ih_hcrc);
		hdr1.ih_hcrc = 0;
		if (crc32(0, (char *)&hdr1, len) != chksum) {
			broken1 = 1;
			printf("Failed\n");
		}
		else
			printf("OK\n");
	}

	if (broken2 == 0) {
		printf("Image2 Header Checksum --> ");
		len  = sizeof(image_header_t);
		chksum = ntohl(hdr2.ih_hcrc);
		hdr2.ih_hcrc = 0;
		if (crc32(0, (char *)&hdr2, len) != chksum) {
			printf("Failed\n");
			broken2 = 1;
		}
		else
			printf("OK\n");
	}

	/* Check data crc */
	/* Skip crc checking if there is no valid header, or it may hang on due to broken data length */
	if (broken1 == 0) {
		ulong h_addr = hdr1_addr + sizeof(image_header_t);

		printf("Image1 Data Checksum --> ");
		len = ntohl(hdr1.ih_size);
		chksum = ntohl(hdr1.ih_dcrc);
#if defined(CFG_ENV_IS_IN_NAND) || defined(CFG_ENV_IS_IN_SPI)
		h_addr = CFG_SPINAND_LOAD_ADDR;
		ra_flash_read(CFG_SPINAND_LOAD_ADDR, hdr1_addr + sizeof(image_header_t), len);
#endif

		if (crc32(0, (char *)h_addr, len) != chksum)
		{
			broken1 = 1;
			printf("Failed\n");
		}
		else
			printf("OK\n");
	}

	if (broken2 == 0) {
		ulong h_addr = hdr2_addr + sizeof(image_header_t);

		printf("Image2 Data Checksum --> ");
		len  = ntohl(hdr2.ih_size);
		chksum = ntohl(hdr2.ih_dcrc);
#if defined(CFG_ENV_IS_IN_NAND) || defined(CFG_ENV_IS_IN_SPI)
		h_addr = CFG_SPINAND_LOAD_ADDR;
		ra_flash_read(CFG_SPINAND_LOAD_ADDR, hdr2_addr + sizeof(image_header_t), len);
#endif
		if (crc32(0, (char *)h_addr, len) != chksum)
		{
			broken2 = 1;
			printf("Failed\n");
		}
		else
			printf("OK\n");
	}

	/* Check stable flag and try counter */
	stable = getenv("Image1Stable");
	printf("Image1 Stable Flag --> %s\n", !strcmp(stable, "1") ? "Stable" : "Not stable");
	try = getenv("Image1Try");
	printf("Image1 Try Counter --> %s\n", (try == NULL) ? "0" : try);
	if ((strcmp(stable, "1") != 0) && (simple_strtoul(try, NULL, 10)) > MAX_TRY_TIMES 
		&& (broken1 == 0)) {
		printf("\nImage1 is not stable and try counter > %X. Take it as a broken image.", MAX_TRY_TIMES);
		broken1 = 1;
	}

	printf("\nImage1: %s Image2: %s\n", broken1 ? "Broken" : "OK", broken2 ? "Broken" : "OK");
	if (broken1 == 1 && broken2 == 0) {
		len = ntohl(hdr2.ih_size) + sizeof(image_header_t);
		if (len > CFG_KERN_SIZE)
			printf("\nImage1 is broken, but Image2 size(0x%X) is too big(limit=0x%X)!!\
				\nGive up copying image.\n", len, CFG_KERN_SIZE);
		else {
			printf("Image1 is borken. Copy Image2 to Image1\n");
			copy_image(2, len);
		}
	}
	else if (broken1 == 0 && broken2 == 1) {
		len = ntohl(hdr1.ih_size) + sizeof(image_header_t);
		if (len > CFG_KERN2_SIZE)
			printf("\nImage2 is broken, but Image1 size(0x%X) is too big(limit=0x%X)!!\
				\nGive up copying image.\n", len, CFG_KERN2_SIZE);
		else {
			printf("\nImage2 is borken. Copy Image1 to Image2.\n");
			copy_image(1, len);
		}
	}
	else if (broken1 == 1 && broken2 == 1)
		 debug_mode();
	else
		ret = -1;

	printf("\n=================================================\n");

	return ret;
}
#endif


/************************************************************************
 *
 * This is the next part if the initialization sequence: we are now
 * running from RAM and have a "normal" C environment, i. e. global
 * data can be written, BSS has been cleared, the stack size in not
 * that critical any more, etc.
 *
 ************************************************************************
 */

gd_t gd_data;
 
void board_init_r (gd_t *id, ulong dest_addr)
{
	cmd_tbl_t *cmdtp;
	ulong size;
	extern void malloc_bin_reloc (void);
#ifndef CFG_ENV_IS_NOWHERE
	extern char * env_name_spec;
#endif
	char *s, *e;
	bd_t *bd;
	int i;
	int timer1= CONFIG_BOOTDELAY;
	unsigned char BootType='3', confirm=0;
	int my_tmp;
	char addr_str[11];
	char tftp_load_addr[] = "0x80200000XXX";
#if defined (CFG_ENV_IS_IN_FLASH)
	ulong e_end;
#endif
#if defined(UBOOT_STAGE1)
#endif

#if defined (RT2880_FPGA_BOARD) || defined (RT2880_ASIC_BOARD)
	u32 value,kk;

	memcpy(&gd_data, (void *)gd, sizeof(gd_t));
	gd = &gd_data;
#else
	u32 config1,lsize,icache_linesz,icache_sets,icache_ways,icache_size;
	u32 dcache_linesz,dcache_sets,dcache_ways,dcache_size;

	memcpy(&gd_data, (void *)gd, sizeof(gd_t));
	gd = &gd_data;
#endif
	sprintf(tftp_load_addr, "0x%x", CFG_LOAD_ADDR);
#if defined(MT7620_ASIC_BOARD)
	/* Enable E-PHY clock */ /* TODO: remove printf()*/
	printf("enable ephy clock...");
	i = 5;
	rw_rf_reg(1, 29, &i);
	printf("done. ");
	rw_rf_reg(0, 29, &i);
	printf("rf reg 29 = %d\n", i);

	/* print SSC for confirmation */ /* TODO: remove these in formanl release*/
	u32 value = RALINK_REG(0xb0000054);
	value = value >> 4;
	if(value & 0x00000008){
		unsigned long swing = ((value & 0x00000007) + 1) * 1250;
		printf("SSC enabled. swing=%d, upperbound=%d\n", swing, (value >> 4) & 0x3);
	}else{
		printf("SSC disabled.\n");
	}

#endif

#if defined(RT3052_ASIC_BOARD)
	void adjust_voltage(void);
	// adjust core voltage ASAP
	adjust_voltage();
#endif

#if defined (RT3052_FPGA_BOARD) || defined(RT3052_ASIC_BOARD)
#ifdef RALINK_EPHY_INIT
	void enable_mdio(int);
	// disable MDIO access ASAP
	enable_mdio(0);
#endif
#endif


	//debug("\n  New gd=%08X\n",gd);
	//for(kk=0;kk <0x000fffff;kk++);

	//gd = id;
	gd->flags |= GD_FLG_RELOC;	/* tell others: relocation done */

	Init_System_Mode(); /*  Get CPU rate */

#if defined(RT3052_ASIC_BOARD) || defined(RT3352_ASIC_BOARD) || defined(RT5350_ASIC_BOARD)
	//turn on all Ethernet LEDs around 0.5sec.
#if 0
	RALINK_REG(RALINK_ETH_SW_BASE+0xA4)=0xC;
	RALINK_REG(RALINK_ETH_SW_BASE+0xA8)=0xC;
	RALINK_REG(RALINK_ETH_SW_BASE+0xAC)=0xC;
	RALINK_REG(RALINK_ETH_SW_BASE+0xB0)=0xC;
	RALINK_REG(RALINK_ETH_SW_BASE+0xB4)=0xC;
	udelay(500000);
	RALINK_REG(RALINK_ETH_SW_BASE+0xA4)=0x5;
	RALINK_REG(RALINK_ETH_SW_BASE+0xA8)=0x5;
	RALINK_REG(RALINK_ETH_SW_BASE+0xAC)=0x5;
	RALINK_REG(RALINK_ETH_SW_BASE+0xB0)=0x5;
	RALINK_REG(RALINK_ETH_SW_BASE+0xB4)=0x5;
#endif
#endif

#if defined(RT3052_ASIC_BOARD) || defined(RT2883_ASIC_BOARD)
	void config_usbotg(void);
	config_usbotg();
#elif defined(RT3883_ASIC_BOARD) || defined(RT3352_ASIC_BOARD) || defined(RT5350_ASIC_BOARD) || defined(RT6855_ASIC_BOARD) || defined (MT7620_ASIC_BOARD)
	void config_usb_ehciohci(void);
	config_usb_ehciohci();
#endif

	u32 reg = RALINK_REG(RT2880_RSTSTAT_REG);
	if(reg & RT2880_WDRST ){
		printf("***********************\n");
		printf("Watchdog Reset Occurred\n");
		printf("***********************\n");
		RALINK_REG(RT2880_RSTSTAT_REG)|=RT2880_WDRST;
		RALINK_REG(RT2880_RSTSTAT_REG)&=~RT2880_WDRST;
		trigger_hw_reset();
	}else if(reg & RT2880_SWSYSRST){
		printf("******************************\n");
		printf("Software System Reset Occurred\n");
		printf("******************************\n");
		RALINK_REG(RT2880_RSTSTAT_REG)|=RT2880_SWSYSRST;
		RALINK_REG(RT2880_RSTSTAT_REG)&=~RT2880_SWSYSRST;
		trigger_hw_reset();
	}else if (reg & RT2880_SWCPURST){
		printf("***************************\n");
		printf("Software CPU Reset Occurred\n");
		printf("***************************\n");
		RALINK_REG(RT2880_RSTSTAT_REG)|=RT2880_SWCPURST;
		RALINK_REG(RT2880_RSTSTAT_REG)&=~RT2880_SWCPURST;
		trigger_hw_reset();
	}

#ifdef DEBUG
	debug ("Now running in RAM - U-Boot at: %08lx\n", dest_addr);
#endif
	gd->reloc_off = dest_addr - CFG_MONITOR_BASE;

	monitor_flash_len = (ulong)&uboot_end_data - dest_addr;
#ifdef DEBUG	
	debug("\n monitor_flash_len =%d \n",monitor_flash_len);
#endif	
	/*
	 * We have to relocate the command table manually
	 */
	for (cmdtp = &__u_boot_cmd_start; cmdtp !=  &__u_boot_cmd_end; cmdtp++) {
		ulong addr;

		addr = (ulong) (cmdtp->cmd) + gd->reloc_off;
#ifdef DEBUG
		printf ("Command \"%s\": 0x%08lx => 0x%08lx\n",
				cmdtp->name, (ulong) (cmdtp->cmd), addr);
#endif
		cmdtp->cmd =
			(int (*)(struct cmd_tbl_s *, int, int, char *[]))addr;

		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (char *)addr;

		if (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (char *)addr;
		}
#ifdef	CFG_LONGHELP
		if (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (char *)addr;
		}
#endif

	}
	/* there are some other pointer constants we must deal with */
#ifndef CFG_ENV_IS_NOWHERE
	env_name_spec += gd->reloc_off;
#endif

	bd = gd->bd;
#if defined (CFG_ENV_IS_IN_NAND)
	if ((size = ranand_init()) == (ulong)-1) {
		printf("ra_nand_init fail\n");
		while(1);
	}
	bd->bi_flashstart = 0;
	bd->bi_flashsize = size;
	bd->bi_flashoffset = 0;
#elif defined (CFG_ENV_IS_IN_SPI)
	if ((size = raspi_init()) == (ulong)-1) {
		printf("ra_spi_init fail\n");
		while(1);
	}
	bd->bi_flashstart = 0;
	bd->bi_flashsize = size;
	bd->bi_flashoffset = 0;
#else //CFG_ENV_IS_IN_FLASH
	/* configure available FLASH banks */
	size = flash_init();

	bd->bi_flashstart = CFG_FLASH_BASE;
	bd->bi_flashsize = size;
#if CFG_MONITOR_BASE == CFG_FLASH_BASE
	bd->bi_flashoffset = monitor_flash_len;	/* reserved area for U-Boot */
#else
	bd->bi_flashoffset = 0;
#endif
#endif //CFG_ENV_IS_IN_FLASH

#if !defined(CFG_ENV_IS_IN_NAND) && defined(NAND_SUPPORT)
	ranand_init();
#endif

	/* initialize malloc() area */
	mem_malloc_init();
	malloc_bin_reloc();

#if defined (CFG_ENV_IS_IN_NAND)
	nand_env_init();
#elif defined (CFG_ENV_IS_IN_SPI)
	spi_env_init();
#else //CFG_ENV_IS_IN_FLASH
#endif //CFG_ENV_IS_IN_FLASH

#if defined(RT3052_ASIC_BOARD)
	void adjust_frequency(void);
	//adjust_frequency();
#endif
#if defined (RT3352_ASIC_BOARD)
	void adjust_crystal_circuit(void);
	adjust_crystal_circuit();
#endif
#if defined (RT3352_ASIC_BOARD) || defined (RT3883_ASIC_BOARD)
	void adjust_rf_r17(void);
	adjust_rf_r17();
#endif

#if defined(CONFIG_RTAC52U)
#if defined(CFG_MALLOC_LEN)
	printf("Maximum malloc length: %d KBytes\n", CFG_MALLOC_LEN >> 10);
	printf("mem_malloc_start/brk/end: 0x%x/%x/%x\n",
		mem_malloc_start, mem_malloc_brk, mem_malloc_end);
#endif
#endif

	/* relocate environment function pointers etc. */
	env_relocate();

	/* board MAC address */
	s = getenv ("ethaddr");
	for (i = 0; i < 6; ++i) {
		bd->bi_enetaddr[i] = s ? simple_strtoul (s, &e, 16) : 0;
		if (s)
			s = (*e) ? e + 1 : e;
	}

	/* IP Address */
	bd->bi_ip_addr = getenv_IPaddr("ipaddr");

#if defined(CONFIG_PCI)
	/*
	 * Do pci configuration
	 */
	puts("\n  Do pci configuration  !!\n");
	pci_init();
#endif

	/** leave this here (after malloc(), environment and PCI are working) **/
	/* Initialize devices */
	devices_init ();

	jumptable_init ();

	/* Initialize the console (after the relocation and devices init) */
	console_init_r ();

	/* Initialize from environment */
	if ((s = getenv ("loadaddr")) != NULL) {
		load_addr = simple_strtoul (s, NULL, 16);
	}
#if (CONFIG_COMMANDS & CFG_CMD_NET)
	if ((s = getenv ("bootfile")) != NULL) {
		copy_filename (BootFile, s, sizeof (BootFile));
	}
#endif	/* CFG_CMD_NET */

#if defined(CONFIG_MISC_INIT_R)
	/* miscellaneous platform dependent initialisations */
	misc_init_r ();
#endif

	/* RT2880 Boot Loader Menu */
#if defined(RT3352_FPGA_BOARD) || defined (RT3352_ASIC_BOARD) || \
    defined(RT3883_FPGA_BOARD) || defined (RT3883_ASIC_BOARD) || \
    defined(RT5350_FPGA_BOARD) || defined (RT5350_ASIC_BOARD) 

#if defined(CFG_ENV_IS_IN_SPI) || defined (CFG_ENV_IS_IN_NAND)
	{
		int reg, boot_from_eeprom=0;
		reg = RALINK_REG(RT2880_SYSCFG_REG);
		/* Uboot Version and Configuration*/
		printf("============================================ \n");
		printf("Ralink UBoot Version: %s\n", RALINK_LOCAL_VERSION);
		printf("-------------------------------------------- \n");
		printf("%s %s %s\n",CHIP_TYPE, CHIP_VERSION, GMAC_MODE);
		boot_from_eeprom = ((reg>>18) & 0x01);
		if(boot_from_eeprom){
		    printf("DRAM_CONF_FROM: EEPROM \n");
		    printf("DRAM_SIZE: %d Mbits %s\n", DRAM_COMPONENT, DDR_INFO);
		    printf("DRAM_TOTAL_WIDTH: %d bits\n", DRAM_BUS );
		    printf("TOTAL_MEMORY_SIZE: %d MBytes\n", DRAM_SIZE);
		}else{
		int dram_width, is_ddr2, dram_total_width, total_size;
		int _x = ((reg >> 12) & 0x7); 

#if defined(RT3352_FPGA_BOARD) || defined (RT3352_ASIC_BOARD)
		int dram_size = (_x == 6)? 2048 : (_x == 5)? 1024 : (_x == 4)? 512 : (_x == 3)? 256 : (_x == 2)? 128 : \
				(_x == 1)? 64 : (_x == 0)? 16 : 0; 
#elif defined (RT5350_FPGA_BOARD) || defined (RT5350_ASIC_BOARD)
		int dram_size = (_x == 4)? 512 : (_x == 3)? 256 : (_x == 2)? 128 : \
				(_x == 1)? 64 : (_x == 0)? 16 : 0; 
#elif defined (RT3883_FPGA_BOARD) || defined (RT3883_ASIC_BOARD)
		int dram_size = (_x == 6)? 2048 : (_x == 5)? 1024 : (_x == 4)? 512 : \
				(_x == 3)? 256 : (_x == 2)? 128 : (_x == 1)? 64 : \
				(_x == 0)? 16 : 0; 
#endif
		if(((reg >> 15) & 0x1)){
		    dram_total_width = 32;
		}else{
		    dram_total_width = 16;
		}

		is_ddr2 = ((reg >> 17) & 0x1);
		if(is_ddr2){
		  if((reg >> 10) & 0x1){
			dram_width = 16;
		  }else{
			dram_width = 8;
		  }
		}else{
		  if((reg >> 10) & 0x1){
			dram_width = 32;
		  }else{
			dram_width = 16;
		  }
		}
		total_size = (dram_size*(dram_total_width/dram_width))/8;

		printf("DRAM_CONF_FROM: %s \n", boot_from_eeprom ? "EEPROM":"Boot-Strapping");
		printf("DRAM_TYPE: %s \n", is_ddr2 ? "DDR2":"SDRAM");
		printf("DRAM_SIZE: %d Mbits\n", dram_size);
		printf("DRAM_WIDTH: %d bits\n", dram_width);
		printf("DRAM_TOTAL_WIDTH: %d bits\n", dram_total_width );
		printf("TOTAL_MEMORY_SIZE: %d MBytes\n", total_size);
		}
		printf("%s\n", FLASH_MSG);
		printf("%s\n", "Date:" __DATE__ "  Time:" __TIME__ );
		printf("============================================ \n");
	}
#else
	SHOW_VER_STR();
#endif /* defined(CFG_ENV_IS_IN_SPI) || defined (CFG_ENV_IS_IN_NAND) */


#elif (defined (RT6855_ASIC_BOARD) || defined (RT6855_FPGA_BOARD) ||  \
      defined (RT6855A_ASIC_BOARD) || defined (RT6855A_FPGA_BOARD) || \
      defined (MT7620_ASIC_BOARD) || defined (MT7620_FPGA_BOARD) ||  \
      defined (MT7621_ASIC_BOARD) || defined (MT7621_FPGA_BOARD)) && defined (UBOOT_RAM)
	{
		char *m;
		unsigned long chip_mode, dram_comp, dram_bus, is_ddr1, is_ddr2, data, cfg0, cfg1, size=0;
		int dram_type_bit_offset = 0;

#if defined (RT6855A_ASIC_BOARD) || defined(RT6855A_FPGA_BOARD)	
		data = RALINK_REG(RALINK_SYSCTL_BASE+0x8c);
		chip_mode = ((data>>28) & 0x3)|(((data>>22) & 0x3)<<2);
		dram_type_bit_offset = 24;
#else		
		data = RALINK_REG(RALINK_SYSCTL_BASE+0x10);
		chip_mode = (data&0x0F);
		dram_type_bit_offset = 4;
#endif		
		switch((data>>dram_type_bit_offset)&0x3)			
		{
			default:
			case 0:
				is_ddr2 = is_ddr1 = 0;
				break;
#if defined (RT6855A_ASIC_BOARD) || defined(RT6855A_FPGA_BOARD)
#else				
			case 3:
#endif
#if defined (MT7620_ASIC_BOARD) || defined (MT7620_FPGA_BOARD)
				is_ddr1 = 1; 
				is_ddr2 = 0;
#else				
				is_ddr2 = is_ddr1 = 0;
#endif
				break;
#if defined (RT6855A_ASIC_BOARD) || defined(RT6855A_FPGA_BOARD)
			case 2:
#else				
			case 1:
#endif
				is_ddr2 = 0;
				is_ddr1 = 1;
				break;
#if defined (RT6855A_ASIC_BOARD) || defined(RT6855A_FPGA_BOARD)
			case 3:
#else				
			case 2:
#endif
				is_ddr2 = 1;
				is_ddr1 = 0;
				break;
		}
		
		switch((data>>dram_type_bit_offset)&0x3)
		{
			case 0:
#if defined (MT7620_ASIC_BOARD) || defined (MT7620_FPGA_BOARD) || \
	defined (RT6855A_ASIC_BOARD) || defined(RT6855A_FPGA_BOARD)
#else
			case 3:
#endif				
				cfg0 = RALINK_REG(RALINK_MEMCTRL_BASE+0x0);
				cfg1 = RALINK_REG(RALINK_MEMCTRL_BASE+0x4);
				data = cfg1;
				
				dram_comp = 1<<(2+(((data>>16)&0x3)+11)+(((data>>20)&0x3)+8)+1+3-20);
				dram_bus = ((data>>24)&0x1) ? 32 : 16;
				size = 1<<(2 +(((data>>16)&0x3)+11)+(((data>>20)&0x3)+8)+1-20);       	
				break;
			case 1:
			case 2:
#if defined (MT7620_ASIC_BOARD) || defined (MT7620_FPGA_BOARD) || \
	defined (RT6855A_ASIC_BOARD) || defined(RT6855A_FPGA_BOARD)
			case 3:
#endif				
				cfg0 = RALINK_REG(RALINK_MEMCTRL_BASE+0x40);
				cfg1 = RALINK_REG(RALINK_MEMCTRL_BASE+0x44);
				data = cfg1;
				dram_comp = 1<<(((data>>18)&0x7)+5);
			    dram_bus = 1<<(((data>>12)&0x3)+2);	
				if(((data>>16)&0x3) < ((data>>12)&0x3))
				{
					size = 1<<(((data>>18)&0x7) + 22 + 1-20); 
				}
				else
				{
					size = 1<<(((data>>18)&0x7) + 22-20);
				}	
				break;
		}
#if defined (RT6855A_ASIC_BOARD) || defined(RT6855A_FPGA_BOARD)		
		if ((((RALINK_REG(RALINK_SYSCTL_BASE+0x8c)>>30)&0x1)==0) && ((chip_mode==2)||(chip_mode==3))) 
		{
#if defined(ON_BOARD_DDR2)
			is_ddr2 = 1;
			is_ddr1 = 0;
#elif defined(ON_BOARD_DDR1)
			is_ddr2 = 0;
			is_ddr1 = 1;
#else
			is_ddr2 = is_ddr1 = 0;
#endif
			dram_comp = DRAM_COMPONENT;
			dram_bus = DRAM_BUS;
			size = DRAM_SIZE;
		}
#endif
		printf("============================================ \n");
		printf("Ralink UBoot Version: %s\n", RALINK_LOCAL_VERSION);
		printf("-------------------------------------------- \n");
		printf("%s %s %s\n",CHIP_TYPE, CHIP_VERSION, GMAC_MODE);
#if defined (RT6855A_ASIC_BOARD) || defined (RT6855A_FPGA_BOARD)
#if defined (RT6855A_FPGA_BOARD)
		if((!is_ddr2)&&(!is_ddr1))
		{
		printf("[SDR_CFG0=0x%08X, SDR_CFG1=0x%08X]\n", RALINK_REG(RALINK_MEMCTRL_BASE+0x0),\
								RALINK_REG(RALINK_MEMCTRL_BASE+0x4));
		}	
		else
		{		
		printf("[DDR_CFG0 =0x%08X, DDR_CFG1 =0x%08X]\n", RALINK_REG(RALINK_MEMCTRL_BASE+0x40),\
								RALINK_REG(RALINK_MEMCTRL_BASE+0x44));
		printf("[DDR_CFG2 =0x%08X, DDR_CFG3 =0x%08X]\n", RALINK_REG(RALINK_MEMCTRL_BASE+0x48),\
								RALINK_REG(RALINK_MEMCTRL_BASE+0x4C));
		printf("[DDR_CFG4 =0x%08X, DDR_CFG10=0x%08X]\n", RALINK_REG(RALINK_MEMCTRL_BASE+0x50),\
								RALINK_REG(RALINK_MEMCTRL_BASE+0x68));
		}
#endif			
		printf("DRAM_CONF_FROM: %s \n", ((RALINK_REG(RALINK_SYSCTL_BASE+0x8c)>>30)&0x1) ? \
			"From SPI/NAND": (((chip_mode==2)||(chip_mode==3)) ? "From Uboot" : "Boot-strap"));
#elif defined (MT7620_ASIC_BOARD) || defined(MT7620_FPGA_BOARD)
		printf("DRAM_CONF_FROM: %s \n", (((RALINK_REG(RALINK_SYSCTL_BASE+0x10)>>8)&0x1)==0) ? "From SPI/NAND": 
				(((chip_mode==2)||(chip_mode==3)) ? "From Uboot" : "Auto-detection"));
#else
		printf("DRAM_CONF_FROM: %s \n", ((RALINK_REG(RALINK_SYSCTL_BASE+0x10)>>7)&0x1) ? "From SPI/NAND":
				(((chip_mode==2)||(chip_mode==3)) ? "From Uboot" : "Auto-detection"));
#endif		
		printf("DRAM_TYPE: %s \n", is_ddr2 ? "DDR2": (is_ddr1 ? "DDR1" : "SDRAM"));
		printf("DRAM component: %d Mbits\n", dram_comp);
		printf("DRAM bus: %d bit\n", dram_bus);
		printf("Total memory: %d MBytes\n", size);
		printf("%s\n", FLASH_MSG);
		if ((m = ra_flash_id()) != NULL)
			printf("Flash ID: %s\n", m);
		printf("%s\n", "Date:" __DATE__ "  Time:" __TIME__ );
		printf("============================================ \n");
	}
#else
	SHOW_VER_STR();
#endif /* defined(RT3352_FPGA_BOARD) || defined (RT3352_ASIC_BOARD) || defined(RT3883_FPGA_BOARD) || defined (RT3883_ASIC_BOARD) */



#if defined (RT2880_FPGA_BOARD) || defined (RT2880_ASIC_BOARD)
	value = read_32bit_cp0_register_with_select1(CP0_CONFIG);

	kk = value >> 7;
	kk = kk & 0x7;

	if(kk)
	{
		debug(" D-CACHE set to %d way \n",kk + 1);
	}
	else
	{
		debug("\n D-CACHE Direct mapped\n");
	}

	kk = value >> 16;
	kk = kk & 0x7;


	if(kk)
	{
		debug(" I-CACHE set to %d way \n",kk + 1);
	}
	else
	{
		debug("\n I-CACHE Direct mapped\n");
	}
#else

	config1 = read_32bit_cp0_register_with_select1(CP0_CONFIG);

	if ((lsize = ((config1 >> 19) & 7)))
		icache_linesz = 2 << lsize;
	else
		icache_linesz = lsize;
	icache_sets = 64 << ((config1 >> 22) & 7);
	icache_ways = 1 + ((config1 >> 16) & 7);

	icache_size = icache_sets *
		icache_ways *
		icache_linesz;

	printf("icache: sets:%d, ways:%d, linesz:%d, total:%d\n", 
			icache_sets, icache_ways, icache_linesz, icache_size);

	/*
	 * Now probe the MIPS32 / MIPS64 data cache.
	 */

	if ((lsize = ((config1 >> 10) & 7)))
		dcache_linesz = 2 << lsize;
	else
		dcache_linesz = lsize;
	dcache_sets = 64 << ((config1 >> 13) & 7);
	dcache_ways = 1 + ((config1 >> 7) & 7);

	dcache_size = dcache_sets *
		dcache_ways *
		dcache_linesz;

	printf("dcache: sets:%d, ways:%d, linesz:%d, total:%d\n", 
			dcache_sets, dcache_ways, dcache_linesz, dcache_size);

#endif

	debug("\n ##### The CPU freq = %d MHZ #### \n", mips_cpu_feq/1000/1000);

/*
	if(*(volatile u_long *)(RALINK_SYSCTL_BASE + 0x0304) & (1<< 24))
	{
		debug("\n SDRAM bus set to 32 bit \n");
	}
	else
	{
		debug("\nSDRAM bus set to 16 bit \n");
	}
*/
	debug(" estimate memory size = %d Mbytes\n", gd->ram_size /1024/1024 );

#if defined (RT3052_ASIC_BOARD) || defined (RT3052_FPGA_BOARD)  || \
    defined (RT3352_ASIC_BOARD) || defined (RT3352_FPGA_BOARD)  || \
    defined (RT5350_ASIC_BOARD) || defined (RT5350_FPGA_BOARD)  
	rt305x_esw_init();
#elif defined (RT6855_ASIC_BOARD) || defined (RT6855_FPGA_BOARD) || \
      defined (MT7620_ASIC_BOARD) || defined (MT7620_FPGA_BOARD)
	rt_gsw_init();
#elif defined (RT6855A_ASIC_BOARD) || defined (RT6855A_FPGA_BOARD)
#ifdef FPGA_BOARD
	rt6855A_eth_gpio_reset();
#endif
	rt6855A_gsw_init();
#endif
	LANWANPartition();

#ifdef DUAL_IMAGE_SUPPORT
	check_image_validation();
#endif
/*config bootdelay via environment parameter: bootdelay */
	{
	    char * s;
	    s = getenv ("bootdelay");
	    timer1 = s ? (int)simple_strtol(s, NULL, 10) : CONFIG_BOOTDELAY;
	}
	if (timer1 > 5)
		timer1 = 5;
#if defined(UBOOT_STAGE1) || defined(UBOOT_STAGE2)
	if (timer1 <= CONFIG_BOOTDELAY)
		timer1 = 0;
#endif

#ifdef ASUS_PRODUCT
#if defined(ASUS_RTN14U) || defined(ASUS_RTAC52U)
       /* turn off WAN, LAN, WLAN, USB LED */
       mtk7620_set_gpio_pin(WAN_LED, 1);
       mtk7620_set_gpio_pin(LAN_LED, 1);
       mtk7620_set_gpio_pin(WIFI_2G_LED, 1);
       mtk7620_set_gpio_pin(USB_LED, 1);
#elif defined(ASUS_RTAC51U)
       mtk7620_set_gpio_pin(WIFI_2G_LED, 1);
       mtk7620_set_gpio_pin(USB_LED, 1);
#elif defined(ASUS_RTN54U)
       mtk7620_set_gpio_pin(WIFI_2G_LED, 1);
       mtk7620_set_gpio_pin(USB_LED, 1);
#elif defined(ASUS_RTN11P)
       mtk7620_set_gpio_pin(WIFI_2G_LED, 1);
       mtk7620_set_gpio_pin(WAN_LED, 1);
       mtk7620_set_gpio_pin(LAN_LED, 1);
#endif
#if defined(ALL_LED_OFF)
	mtk7620_set_gpio_pin(ALL_LED_OFF_GPIO_NR, 1);	/* Inhibit ALL LED, except PWR LED */
#endif

#if defined(HWNAT_FIX)       
	rst_fengine();
#endif	
#endif 

	ra_flash_init_layout();

	OperationSelect();
	timer1 *= 100;
	if (!timer1)
		timer1 = 20;
	while (timer1 > 0) {
		--timer1;
		/* delay 10ms */
		if ((my_tmp = tstc()) != 0) {	/* we got a key press	*/
			timer1 = 0;	/* no more delay	*/
			BootType = getc();
			if ((BootType < '0' || BootType > '5') && (BootType != '7') && (BootType != '8') && (BootType != '9'))
				BootType = '3';
			printf("\n\rYou choosed %c\n\n", BootType);
			break;
		}
		udelay (10000);
		if ((timer1 / 100 * 100) == timer1)
			printf ("\b\b\b%2d ", timer1 / 100);
	}
	putc ('\n');
	if(BootType == '3') {
		char *argv[2] = {"", ""};
#if !defined(UBOOT_STAGE1)
		sprintf(addr_str, "0x%X", CFG_KERN_ADDR);
		argv[1] = &addr_str[0];
#endif
		printf("   \n3: System Boot %s via Flash.\n", sys_image_name);

#ifdef ASUS_PRODUCT
#if ! defined(UBOOT_STAGE1)
                if(chkVer() == 0)
                       set_ver();      // ham add

                if((chkMAC()) < 0)     // ham test
                       init_mac();
#endif /* ! UBOOT_STAGE1) */

                eth_initialize(gd->bd);         // ASUS chk
                do_tftpd(cmdtp, 0, 2, argv);    // ASUS chk
#if defined(ASUS_RTN14U) || defined(ASUS_RTAC52U)
		/* turn off WAN, LAN, USB LED */
		mtk7620_set_gpio_pin(WAN_LED, 1);
		mtk7620_set_gpio_pin(LAN_LED, 1);
		mtk7620_set_gpio_pin(USB_LED, 1);
#elif defined(ASUS_RTAC51U)
		mtk7620_set_gpio_pin(USB_LED, 1);
#elif defined(ASUS_RTN54U)
		mtk7620_set_gpio_pin(USB_LED, 1);
#elif defined(ASUS_RTN11P)
		mtk7620_set_gpio_pin(WIFI_2G_LED, 1);
		mtk7620_set_gpio_pin(WAN_LED, 1);
		mtk7620_set_gpio_pin(LAN_LED, 1);
#endif
#else          
                do_bootm(cmdtp, 0, 2, argv);
#endif         
	}
	else {
		char *argv[4];
		int argc= 3;

		argv[2] = &file_name_space[0];
		memset(file_name_space,0,ARGV_LEN);

#if (CONFIG_COMMANDS & CFG_CMD_NET)
		eth_initialize(gd->bd);
#endif

		switch(BootType) {
#if !defined(UBOOT_STAGE1)
		case '1':
			printf("   \n%d: System Load %s to SDRAM via TFTP. \n", SEL_LOAD_LINUX_SDRAM, sys_image_name);
			tftp_config(SEL_LOAD_LINUX_SDRAM, argv);           
			argc= 3;
			setenv("autostart", "yes");
			do_tftpb(cmdtp, 0, argc, argv);
			break;
#endif

		case '2':
			printf("   \n%d: System Load %s then write to Flash via TFTP. \n", SEL_LOAD_LINUX_WRITE_FLASH, sys_image_name);
			printf(" Warning!! Erase Linux in Flash then burn new one. Are you sure?(Y/N)\n");
			confirm = getc();
			if (confirm != 'y' && confirm != 'Y') {
				printf(" Operation terminated\n");
				break;
			}
#if defined(UBOOT_STAGE1)
			setenv("bootfile", "uboot_stage2.img");
#endif
			tftp_config(SEL_LOAD_LINUX_WRITE_FLASH, argv);
			argc= 3;
			setenv("autostart", "no");
			do_tftpb(cmdtp, 0, argc, argv);

			{
				unsigned int load_address = simple_strtoul(argv[1], NULL, 16);
#if defined(UBOOT_STAGE1)
				struct stage2_loc *s2 = &g_s2_loc;

				ranand_write_stage2(load_address, NetBootFileXferSize);
				ranand_locate_stage2(s2);
				sprintf(addr_str, "0x%X", s2->good->code);
#else
				ra_flash_erase_write((uchar*)load_address, CFG_KERN_ADDR, NetBootFileXferSize, 0);
#endif
			}
#if defined (CFG_ENV_IS_IN_FLASH)
#if (defined (ON_BOARD_8M_FLASH_COMPONENT) || defined (ON_BOARD_16M_FLASH_COMPONENT)) && (defined (RT2880_ASIC_BOARD) || defined (RT2880_FPGA_BOARD) || defined (RT3052_MP1))
#warning	FIXME: rewrite below code to new flash wrapper function.
			//erase linux
			if (NetBootFileXferSize <= (0x400000 - (CFG_BOOTLOADER_SIZE + CFG_CONFIG_SIZE + CFG_FACTORY_SIZE))) {
				e_end = CFG_KERN_ADDR + NetBootFileXferSize;
				if (0 != get_addr_boundary(&e_end))
					break;
				printf("Erase linux kernel block !!\n");
				printf("From 0x%X To 0x%X\n", CFG_KERN_ADDR, e_end);
				flash_sect_erase(CFG_KERN_ADDR, e_end);
			}
			else if (NetBootFileXferSize <= CFG_KERN_SIZE) {
				e_end = PHYS_FLASH_2 + NetBootFileXferSize - (0x400000 - (CFG_BOOTLOADER_SIZE + CFG_CONFIG_SIZE + CFG_FACTORY_SIZE));
				if (0 != get_addr_boundary(&e_end))
					break;
				printf("Erase linux kernel block !!\n");
				printf("From 0x%X To 0x%X\n", CFG_KERN_ADDR, CFG_FLASH_BASE+0x3FFFFF);
				flash_sect_erase(CFG_KERN_ADDR, CFG_FLASH_BASE+0x3FFFFF);
				printf("Erase linux file system block !!\n");
				printf("From 0x%X To 0x%X\n", PHYS_FLASH_2, e_end);
				flash_sect_erase(PHYS_FLASH_2, e_end);
			}
#else
			if (NetBootFileXferSize <= (bd->bi_flashsize - (CFG_BOOTLOADER_SIZE + CFG_CONFIG_SIZE + CFG_FACTORY_SIZE))) {
				e_end = CFG_KERN_ADDR + NetBootFileXferSize;
				if (0 != get_addr_boundary(&e_end))
					break;
				printf("Erase linux kernel block !!\n");
				printf("From 0x%X To 0x%X\n", CFG_KERN_ADDR, e_end);
				flash_sect_erase(CFG_KERN_ADDR, e_end);
			}
#endif
			else {
				printf("***********************************\n");
				printf("The Linux Image size is too big !! \n");
				printf("***********************************\n");
				break;
			}

			//cp.linux
			argc = 4;
			argv[0]= "cp.linux";
			do_mem_cp(cmdtp, 0, argc, argv);
#endif //CFG_ENV_IS_IN_FLASH

#ifdef DUAL_IMAGE_SUPPORT
			/* Don't do anything to the firmware upgraded in Uboot, since it may be used for testing */
			setenv("Image1Stable", "1");
			saveenv();
#endif

			//bootm bc050000
			argc= 2;
#if !defined(UBOOT_STAGE1)
			sprintf(addr_str, "0x%X", CFG_KERN_ADDR);
#endif
			argv[1] = &addr_str[0];
			do_bootm(cmdtp, 0, argc, argv);            
			break;

#ifdef RALINK_CMDLINE
		case '4':
			printf("   \n%d: System Enter Boot Command Line Interface.\n", SEL_ENTER_CLI);
			printf ("\n%s\n", version_string);
			/* main_loop() can return to retry autoboot, if so just run it again. */
			for (;;) {					
				main_loop ();
			}
			break;
#endif // RALINK_CMDLINE //

#if defined(ASUS_RTN14U) || defined(ASUS_RTAC52U) || defined(ASUS_RTAC51U) || defined(ASUS_RTN11P) || defined(ASUS_RTN54U) 
#if !defined(UBOOT_STAGE1)
		case '5':
			printf("\n%d: System Load %s to SDRAM via Serial (*.bin) \n\n\n", SEL_LOAD_BOOT_SDRAM_VIA_SERIAL, boot_image_name);
			printf("**************************************************************\n");
			printf("*** NOTICE: You MUST use the 'RAM Version' of uboot **********\n");
			printf("**************************************************************\n");
			argc= 4;
			argv[1] = tftp_load_addr;
			setenv("autostart", "yes");
			my_tmp = do_load_serial_bin(cmdtp, 0, argc, argv);
			NetBootFileXferSize=simple_strtoul(getenv("filesize"), NULL, 16);

			if (NetBootFileXferSize > CFG_BOOTLOADER_SIZE || my_tmp == 1) {
				printf("Abort: Bootloader is too big or download aborted!\n");
			}
			break;
#endif
#endif

#ifdef RALINK_UPGRADE_BY_SERIAL
		case '7':
			printf("\n%d: System Load %s then write to Flash via Serial. \n", SEL_LOAD_BOOT_WRITE_FLASH_BY_SERIAL, boot_image_name);
			
#if defined(ASUS_RTN14U) || defined(ASUS_RTAC52U) || defined(ASUS_RTAC51U) || defined(ASUS_RTN11P) || defined(ASUS_RTN54U)
			printf("**************************************************************\n");
			printf("*** NOTICE: You MUST use the 'ROM Version' of uboot **********\n");
			printf("**************************************************************\n");
			argv[1] = tftp_load_addr;
			argc=2;
#else			
			argc= 1;
#endif			

			setenv("autostart", "no");
			my_tmp = do_load_serial_bin(cmdtp, 0, argc, argv);
			NetBootFileXferSize=simple_strtoul(getenv("filesize"), NULL, 16);
#if defined(SMALL_UBOOT_PARTITION)
			if (NetBootFileXferSize > CFG_UBOOT_SIZE || my_tmp == 1)
#elif defined(UBOOT_STAGE1)
			if (NetBootFileXferSize > CFG_BOOTSTAGE1_SIZE || my_tmp == 1)
#else
			if (NetBootFileXferSize > CFG_MAX_BOOTLOADER_BINARY_SIZE || my_tmp == 1)
#endif
			{
				printf("Abort: Bootloader is too big or download aborted!\n");
			}
#if defined(CFG_ENV_IS_IN_NAND) || defined(CFG_ENV_IS_IN_SPI)
			else {
				unsigned int load_address = simple_strtoul(argv[1], NULL, 16);
				if (check_trx(2, argv) <= 0)
				{
					printf("## checksum error ##\n");
					break;
				}
#if defined(CFG_ENV_IS_IN_SPI)
				if (ntohl(((unsigned int *)load_address)[0]) == IH_MAGIC)
				{
					int hdr_len = sizeof(image_header_t);
					load_address += hdr_len;
					NetBootFileXferSize -= hdr_len;
				}
#endif
				printf("[U] raspi/ranand ew: from(%08x), to(%08x), size(%x)\n", load_address, CFG_FLASH_BASE, NetBootFileXferSize);
#if   defined(UBOOT_STAGE1)
				ranand_set_sbb_max_addr(CFG_BOOTSTAGE1_SIZE);
				ra_flash_erase_write((uchar*) load_address, CFG_FLASH_BASE, NetBootFileXferSize, 1);
				ranand_set_sbb_max_addr(0);
#elif defined(UBOOT_STAGE2)
				if (NetBootFileXferSize <= CFG_BOOTSTAGE1_SIZE) {
					ranand_set_sbb_max_addr(CFG_BOOTSTAGE1_SIZE);
					ra_flash_erase_write((uchar*) load_address, CFG_FLASH_BASE, NetBootFileXferSize, 1);
					ranand_set_sbb_max_addr(0);
				} else {
					ranand_write_stage2(CFG_LOAD_ADDR, NetBootFileXferSize);
				}
#else
				ranand_set_sbb_max_addr(CFG_BOOTLOADER_SIZE);
				ra_flash_erase_write((uchar*) load_address, CFG_FLASH_BASE, NetBootFileXferSize, 1);
				ranand_set_sbb_max_addr(0);
#endif
			}
#else //CFG_ENV_IS_IN_FLASH
#warning	FIXME: rewrite below code to new flash wrapper function.
			else {
				//protect off uboot
				flash_sect_protect(0, CFG_FLASH_BASE, CFG_FLASH_BASE+CFG_BOOTLOADER_SIZE-1);

				//erase uboot
				printf("\n Erase U-Boot block !!\n");
				printf("From 0x%X To 0x%X\n", CFG_FLASH_BASE, CFG_FLASH_BASE+CFG_BOOTLOADER_SIZE-1);
				flash_sect_erase(CFG_FLASH_BASE, CFG_FLASH_BASE+CFG_BOOTLOADER_SIZE-1);

				//cp.uboot            
				argc = 4;
				argv[0]= "cp.uboot";
				do_mem_cp(cmdtp, 0, argc, argv);                       

				//protect on uboot
				flash_sect_protect(1, CFG_FLASH_BASE, CFG_FLASH_BASE+CFG_BOOTLOADER_SIZE-1);
			}
#endif //CFG_ENV_IS_IN_FLASH

			//reset            
			do_reset(cmdtp, 0, argc, argv);
			break;
#endif // RALINK_UPGRADE_BY_SERIAL //

#if !defined(UBOOT_STAGE1)
		case '8':
			printf("   \n%d: System Load %s to SDRAM via TFTP.(*.bin) \n", SEL_LOAD_BOOT_SDRAM, boot_image_name);
			printf("**************************************************************\n");
			printf("*** NOTICE: You MUST use the 'RAM Version' of uboot **********\n");
			printf("**************************************************************\n");
			tftp_config(SEL_LOAD_BOOT_SDRAM, argv);
#if defined(ASUS_RTN14U) || defined(ASUS_RTAC52U) || defined(ASUS_RTAC51U) || defined(ASUS_RTN11P) || defined(ASUS_RTN54U)
			argc= 5;
#else			
			argc= 3;
#endif
			setenv("autostart", "yes");
			do_tftpb(cmdtp, 0, argc, argv);
			break;
#endif

		case '9':
			printf("   \n%d: System Load %s then write to Flash via TFTP. (.bin)\n", SEL_LOAD_BOOT_WRITE_FLASH, boot_image_name);
			printf(" Warning!! Erase %s in Flash then burn new one. Are you sure?(Y/N)\n", boot_image_name);
			confirm = getc();
			if (confirm != 'y' && confirm != 'Y') {
				printf(" Operation terminated\n");
				break;
			}
			setenv("bootfile", BOOTFILENAME);
			tftp_config(SEL_LOAD_BOOT_WRITE_FLASH, argv);
			argc= 3;
			setenv("autostart", "no");
			do_tftpb(cmdtp, 0, argc, argv);
#if defined(SMALL_UBOOT_PARTITION)
			if (NetBootFileXferSize > CFG_UBOOT_SIZE)
#elif defined(UBOOT_STAGE1)
			if (NetBootFileXferSize > CFG_BOOTSTAGE1_SIZE)
#else
			if (NetBootFileXferSize > CFG_MAX_BOOTLOADER_BINARY_SIZE)
#endif
			{
				printf("Abort: %s %d too big! \n", boot_image_name, NetBootFileXferSize);
			}
#if defined(CFG_ENV_IS_IN_NAND) || defined(CFG_ENV_IS_IN_SPI)
			else {
				unsigned int load_address = simple_strtoul(argv[1], NULL, 16);
				if (check_trx(2, argv) <= 0)
				{
					printf("## checksum error ##\n");
					break;
				}
#if defined(CFG_ENV_IS_IN_SPI)
				if (ntohl(((unsigned int *)load_address)[0]) == IH_MAGIC)
				{
					int hdr_len = sizeof(image_header_t);
					load_address += hdr_len;
					NetBootFileXferSize -= hdr_len;
				}
#endif
				printf("[U] raspi/ranand ew: offs:%x, size:%x\n", CFG_FLASH_BASE, NetBootFileXferSize);
#if   defined(UBOOT_STAGE1)
				ranand_set_sbb_max_addr(CFG_BOOTSTAGE1_SIZE);
				ra_flash_erase_write((uchar*) load_address, CFG_FLASH_BASE, NetBootFileXferSize, 1);
				ranand_set_sbb_max_addr(0);
#elif defined(UBOOT_STAGE2)
				if (NetBootFileXferSize <= CFG_BOOTSTAGE1_SIZE) {
					ranand_set_sbb_max_addr(CFG_BOOTSTAGE1_SIZE);
					ra_flash_erase_write((uchar*) load_address, CFG_FLASH_BASE, NetBootFileXferSize, 1);
					ranand_set_sbb_max_addr(0);
				} else {
					ranand_write_stage2(load_address, NetBootFileXferSize);
				}
#else
				ranand_set_sbb_max_addr(CFG_BOOTLOADER_SIZE);
				ra_flash_erase_write((uchar*) load_address, CFG_FLASH_BASE, NetBootFileXferSize, 1);
				ranand_set_sbb_max_addr(0);
#endif
			}
#else //CFG_ENV_IS_IN_FLASH
#warning	FIXME: rewrite below code to new flash wrapper function.
			else {
				//protect off uboot
				flash_sect_protect(0, CFG_FLASH_BASE, CFG_FLASH_BASE+CFG_BOOTLOADER_SIZE-1);

				//erase uboot
				printf("\n Erase U-Boot block !!\n");
				printf("From 0x%X To 0x%X\n", CFG_FLASH_BASE, CFG_FLASH_BASE+CFG_BOOTLOADER_SIZE-1);
				flash_sect_erase(CFG_FLASH_BASE, CFG_FLASH_BASE+CFG_BOOTLOADER_SIZE-1);

				//cp.uboot            
				argc = 4;
				argv[0]= "cp.uboot";
				do_mem_cp(cmdtp, 0, argc, argv);                       

				//protect on uboot
				flash_sect_protect(1, CFG_FLASH_BASE, CFG_FLASH_BASE+CFG_BOOTLOADER_SIZE-1);
			}
#endif //CFG_ENV_IS_IN_FLASH

			//reset            
			do_reset(cmdtp, 0, argc, argv);
			break;
#ifdef RALINK_UPGRADE_BY_SERIAL
#if defined (CFG_ENV_IS_IN_NAND) || defined (CFG_ENV_IS_IN_SPI)
		case '0':
			printf("\n%d: System Load %s then write to Flash via Serial. \n", SEL_LOAD_LINUX_WRITE_FLASH_BY_SERIAL, sys_image_name);
			argc= 1;
			setenv("autostart", "no");
			my_tmp = do_load_serial_bin(cmdtp, 0, argc, argv);
			NetBootFileXferSize=simple_strtoul(getenv("filesize"), NULL, 16);
#if defined(UBOOT_STAGE1)
			ranand_write_stage2(CFG_LOAD_ADDR, NetBootFileXferSize);
#else
			ra_flash_erase_write((uchar*) CFG_LOAD_ADDR, CFG_KERN_ADDR, NetBootFileXferSize, 0);
#endif

			//reset            
			do_reset(cmdtp, 0, argc, argv);
			break;
#endif
#endif // RALINK_UPGRADE_BY_SERIAL //


#ifdef RALINK_USB
#if defined (CFG_ENV_IS_IN_NAND) || defined (CFG_ENV_IS_IN_SPI)
		case '5':
			printf("\n%d: System Load Linux then write to Flash via USB Storage. \n", 5);

			argc = 2;
			argv[1] = "start";
			do_usb(cmdtp, 0, argc, argv);
			if( usb_stor_curr_dev < 0){
				printf("No USB Storage found. Upgrade F/W failed.\n");
				break;
			}

			argc= 5;
			argv[1] = "usb";
			argv[2] = "0";
			sprintf(addr_str, "0x%X", CFG_LOAD_ADDR);
			argv[3] = &addr_str[0];
			argv[4] = "root_uImage";
			setenv("autostart", "no");
			if(do_fat_fsload(cmdtp, 0, argc, argv)){
				printf("Upgrade F/W from USB storage failed.\n");
				break;
			}

			NetBootFileXferSize=simple_strtoul(getenv("filesize"), NULL, 16);
#if defined(UBOOT_STAGE1)
			ranand_write_stage2(CFG_LOAD_ADDR, NetBootFileXferSize);
#else
			ra_flash_erase_write((uchar*) CFG_LOAD_ADDR, CFG_KERN_ADDR, NetBootFileXferSize, 0);
#endif

			//reset            
			do_reset(cmdtp, 0, argc, argv);
			break;
#endif
#endif // RALINK_UPGRADE_BY_USB //

		default:
			printf("   \nSystem Boot Linux via Flash.\n");
			do_bootm(cmdtp, 0, 1, argv);
			break;            
		} /* end of switch */   

		do_reset(cmdtp, 0, argc, argv);

	} /* end of else */

	/* NOTREACHED - no way out of command loop except booting */
}


void hang (void)
{
	puts ("### ERROR ### Please RESET the board ###\n");
	for (;;);
}

#if defined (RALINK_RW_RF_REG_FUN)
#if defined (MT7620_ASIC_BOARD)
#define RF_CSR_CFG      0xb0180500
#define RF_CSR_KICK     (1<<0)
int rw_rf_reg(int write, int reg, int *data)
{
	u32	rfcsr, i = 0;

	while (1) {
		rfcsr = RALINK_REG(RF_CSR_CFG);
		if (! (rfcsr & (u32)RF_CSR_KICK) )
			break;
		if (++i > 10000) {
			puts("Warning: Abort rw rf register: too busy\n");
			return -1;
		}
	}
	rfcsr = (u32)(RF_CSR_KICK | ((reg & 0x3f) << 16)  | ((*data & 0xff) << 8));
	if (write)
		rfcsr |= 0x10;

	RALINK_REG(RF_CSR_CFG) = cpu_to_le32(rfcsr);
	i = 0;
	while (1) {
		rfcsr = RALINK_REG(RF_CSR_CFG);
		if (! (rfcsr & (u32)RF_CSR_KICK) )
			break;
		if (++i > 10000) {
			puts("Warning: still busy\n");
			return -1;
		}
	}

	rfcsr = RALINK_REG(RF_CSR_CFG);
	if (((rfcsr & 0x3f0000) >> 16) != (reg & 0x3f)) {
		puts("Error: rw register failed\n");
		return -1;
	}
	*data = (int)( (rfcsr & 0xff00) >> 8) ;
	return 0;
}
#else
#define RF_CSR_CFG      0xb0180500
#define RF_CSR_KICK     (1<<17)
int rw_rf_reg(int write, int reg, int *data)
{
	u32	rfcsr, i = 0;

	while (1) {
		rfcsr = RALINK_REG(RF_CSR_CFG);
		if (! (rfcsr & (u32)RF_CSR_KICK) )
			break;
		if (++i > 10000) {
			puts("Warning: Abort rw rf register: too busy\n");
			return -1;
		}
	}


	rfcsr = (u32)(RF_CSR_KICK | ((reg & 0x3f) << 8)  | (*data & 0xff));
	if (write)
		rfcsr |= 0x10000;

	RALINK_REG(RF_CSR_CFG) = cpu_to_le32(rfcsr);

	i = 0;
	while (1) {
		rfcsr = RALINK_REG(RF_CSR_CFG);
		if (! (rfcsr & (u32)RF_CSR_KICK) )
			break;
		if (++i > 10000) {
			puts("Warning: still busy\n");
			return -1;
		}
	}

	rfcsr = RALINK_REG(RF_CSR_CFG);

	if (((rfcsr&0x1f00) >> 8) != (reg & 0x1f)) {
		puts("Error: rw register failed\n");
		return -1;
	}
	*data = (int)(rfcsr & 0xff);

	return 0;
}
#endif
#endif

#ifdef RALINK_RW_RF_REG_FUN
#ifdef RALINK_CMDLINE
int do_rw_rf(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int write, reg, data;

	if ((argv[1][0] == 'r' || argv[1][0] == 'R') && (argc == 3)) {
		write = 0;
		reg = (int)simple_strtoul(argv[2], NULL, 10);
		data = 0;
	}
	else if ((argv[1][0] == 'w' || argv[1][0] == 'W') && (argc == 4)) {
		write = 1;
		reg = (int)simple_strtoul(argv[2], NULL, 10);
		data = (int)simple_strtoul(argv[3], NULL, 16);
	}
	else {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	rw_rf_reg(write, reg, &data);
	if (!write)
		printf("rf reg <%d> = 0x%x\n", reg, data);
	return 0;
}

U_BOOT_CMD(
	rf,     4,     1,      do_rw_rf,
	"rf      - read/write rf register\n",
	"usage:\n"
	"rf r <reg>        - read rf register\n"
	"rf w <reg> <data> - write rf register (reg: decimal, data: hex)\n"
);
#endif // RALINK_CMDLINE //
#endif

#if defined(RT3352_ASIC_BOARD)
void adjust_crystal_circuit(void)
{
	int d = 0x45;

	rw_rf_reg(1, 18, &d);
}
#endif

#if defined(RT3052_ASIC_BOARD)
/*
 *  Adjust core voltage to 1.2V using RF reg 26 for the 3052 two layer board.
 */
void adjust_voltage(void)
{
	int d = 0x25;
	rw_rf_reg(1, 26, &d);
}

void adjust_frequency(void)
{
	u32 r23;

	//read from EE offset 0x3A
	ra_factory_read(&r23, 0x3a, 1);
	r23 &= 0xff;
	//write to RF R23
	rw_rf_reg(1, 23, &r23);
}
#endif

#if defined (RT3352_ASIC_BOARD) || defined (RT3883_ASIC_BOARD)
void adjust_rf_r17(void)
{
	u32 r17;
	u32 i;
	u32 val;
	u32 j = 0;
	ulong addr = 0x3a;

#if defined(RT3883_ASIC_BOARD) && defined(CFG_ENV_IS_IN_FLASH)
	addr = 0x44;
#endif

	//read from EE offset 0x3A
	ra_factory_read(&r17, addr, 1);
	r17 &= 0xff;
	//printf("EE offset 0x3A is  0x%0X\n", r17);
	if((r17 == 0) || (r17 == 0xff)){
	    r17 = 0x2c;
	}

	if(r17 <= 0xf) {
		for(i=1; i<=r17; i++) {
		//write to RF R17
		val = i;
		val |= 1 << 7;
		rw_rf_reg(1, 17, &val);
		udelay(2000);
		rw_rf_reg(0, 17, &val);
		//printf("Update RF_R17 to 0x%0X\n", val);
		}	
	}
	else{
		for(i=1; i<=0xf; i++) {
		//write to RF R17
		val = i;
		val |= 1 << 7;
		rw_rf_reg(1, 17, &val);
		udelay(2000);
		rw_rf_reg(0, 17, &val);
		printf("Update RF_R17 to 0x%0X\n", val);
		}
		val = 0x1f;
		val |= 1 << 7;
		rw_rf_reg(1, 17, &val);
		udelay(2000);
		rw_rf_reg(0, 17, &val);
		printf("Update RF_R17 to 0x%0X\n", val);
		
		if(r17 <= 0x1f) {
			for(i=0x1e; i>=r17; i--) {
			//write to RF R17
			val = i;
			val |= 1 << 7;
			rw_rf_reg(1, 17, &val);
			udelay(2000);
			rw_rf_reg(0, 17, &val);
			printf("Update RF_R17 to 0x%0X\n", val);
			}
		} else if((r17 > 0x1f) && (r17 <=0x2f)){
			for(i=0x2f; i>=r17; i--) {
			//write to RF R17
			val = i;
			val |= 1 << 7;
			rw_rf_reg(1, 17, &val);
			udelay(2000);
			rw_rf_reg(0, 17, &val);
			//printf("Update RF_R17 to 0x%0X\n", val);
			}
		}else {
			val = 0x2f;
			val |= 1 << 7;
			rw_rf_reg(1, 17, &val);
			udelay(2000);
			rw_rf_reg(0, 17, &val);
			//printf("Update RF_R17 to 0x%0X\n", val);
		}
		if((r17 > 0x2f) && (r17 <= 0x3f)){
			for(i=0x3f; i>=r17; i--) {
			//write to RF R17
			val = i;
			val |= 1 << 7;
			rw_rf_reg(1, 17, &val);
			udelay(2000);
			rw_rf_reg(0, 17, &val);
			//printf("Update RF_R17 to 0x%0X\n", val);
			}
		}
		if(r17 > 0x3f){
			val = 0x3f;
			val |= 1 << 7;
			rw_rf_reg(1, 17, &val);
			udelay(2000);
			rw_rf_reg(0, 17, &val);
			//printf("Only Update RF_R17 to 0x%0X\n", val);
		}
	}
	//rw_rf_reg(0, 17, &val);
	//printf("Read RF_R17 = 0x%0X\n", val);
}
#endif

#if defined(RT3883_ASIC_BOARD) || defined(RT3352_ASIC_BOARD) || defined(RT5350_ASIC_BOARD) || defined(RT6855_ASIC_BOARD) || defined (MT7620_ASIC_BOARD)
/*
 * enter power saving mode
 */
void config_usb_ehciohci(void)
{
	u32 val;
	
	val = RALINK_REG(RT2880_RSTCTRL_REG);    // toggle host & device RST bit
	val = val | RALINK_UHST_RST | RALINK_UDEV_RST;
	RALINK_REG(RT2880_RSTCTRL_REG) = val;

	val = RALINK_REG(RT2880_CLKCFG1_REG);
#if defined(RT5350_ASIC_BOARD) || defined(RT6855_ASIC_BOARD)
	val = val & ~(RALINK_UPHY0_CLK_EN) ;  // disable USB port0 PHY. 
#else
	val = val & ~(RALINK_UPHY0_CLK_EN | RALINK_UPHY1_CLK_EN) ;  // disable USB port0 & port1 PHY. 
#endif
	RALINK_REG(RT2880_CLKCFG1_REG) = val;
}
#endif /* (RT3883_ASIC_BOARD) || defined(RT3352_ASIC_BOARD)|| defined(RT5350_ASIC_BOARD) || defined(RT6855_ASIC_BOARD) || defined (MT7620_ASIC_BOARD) */

#if defined(RT3052_ASIC_BOARD) || defined(RT2883_ASIC_BOARD)
int usbotg_host_suspend(void)
{
	u32 val;
	int i, rc=0, retry_count=0;
	
	printf(".");

retry_suspend:
	val = le32_to_cpu(*(volatile u_long *)(0xB01C0440));
	val = val >> 10;
	val = val & 0x00000003;

	if(val == 0x3){
		if(retry_count++ < 0x100000)
			goto retry_suspend;
		printf("*** Error: D+/D- is 1/1, config usb failed.\n");
		return -1;
	}
	//printf("Config usb otg 2\n");

	val = le32_to_cpu(*(volatile u_long *)(0xB01C0400));
	//printf("1.b01c0400 = 0x%08x\n", val);
	//printf("force \"FS-LS only mode\"\n");
	val = val | (1 << 2);
	*(volatile u_long *)(0xB01C0400) = cpu_to_le32(val);
	val = le32_to_cpu(*(volatile u_long *)(0xB01C0400));
	//printf("2.b01c0400 = 0x%08x\n", val);

    val = le32_to_cpu(*(volatile u_long *)(0xB01C0440));
    //printf("3.b01c0440 = 0x%08x\n", val);

	//printf("port power on\n");
    val = val | (1 << 12);
	*(volatile u_long *)(0xB01C0440) = cpu_to_le32(val);
    val = le32_to_cpu(*(volatile u_long *)(0xB01C0440));
    //printf("4.b01c0440 = 0x%08x\n", val);

	udelay(3000);	// 3ms

	////printf("check port connect status\n");
	val = le32_to_cpu(*(volatile u_long *)(0xB01C0440));
	////printf("5.b01c0440 = 0x%08x\n", val);

	////printf("port reset --set\n");
	val = val | (1 << 8);
	*(volatile u_long *)(0xB01C0440) = cpu_to_le32(val);
	val = le32_to_cpu(*(volatile u_long *)(0xB01C0440));
	//printf("6.b01c0440 = 0x%08x\n", val);

	udelay(10000);
	
	//printf("port reset -- clear\n");
	val = val & ~(1 << 8);
	*(volatile u_long *)(0xB01C0440) = cpu_to_le32(val);
	val = le32_to_cpu(*(volatile u_long *)(0xB01C0440));
	//printf("7.b01c0440 = 0x%08x\n", val);

	udelay(1000);

	//printf("port suspend --set\n");
	val = le32_to_cpu(*(volatile u_long *)(0xB01C0440));
	//printf("b.b01c0440 = 0x%08x\n", val);
	val = val | (1 << 7);
	/* avoid write 1 to port enable */
	val = val & 0xFFFFFFF3;

	*(volatile u_long *)(0xB01C0440) = cpu_to_le32(val);

	val = le32_to_cpu(*(volatile u_long *)(0xB01C0440));
	//printf("c.b01c0440 = 0x%08x\n", val);

	//printf(" stop pclk\n");
	*(volatile u_long *)(0xB01C0E00) = cpu_to_le32(0x1);
	val = le32_to_cpu(*(volatile u_long *)(0xB01C0E00));
	//printf("8.b01c0e00 = 0x%08x\n", val);

	//printf("wait for suspend...");
	for(i=0; i<200000; i++){
		val = le32_to_cpu(*(volatile u_long *)(0xB01C0440));
		val = val & (1 << 7);
		if(val)
			break;			// done.
		udelay(1);
	}

	if(i==200000){
		//printf("timeout, ignore...(0x%08x)\n", val);
		rc = -1;
	}
	val = le32_to_cpu(*(volatile u_long *)(0xB01C0440));
	//printf("val = 0x%08x\n", val);
	udelay(100000);

	val = RALINK_REG(RT2880_RSTCTRL_REG);    // reset OTG
	val = val | RALINK_OTG_RST;
	RALINK_REG(RT2880_RSTCTRL_REG) = val;
	val = val & ~(RALINK_OTG_RST);
	RALINK_REG(RT2880_RSTCTRL_REG) = val;
	udelay(200000);

	udelay(200000);

	return rc;
}


int usbotg_device_suspend(void)
{
	u32 val;
	int rc = -1, try_count;

	printf(".");

	RALINK_REG(0xB01C0E00) = 0xF;
	udelay(100000);
	val = le32_to_cpu(*(volatile u_long *)(0xB01C0808));
	//printf("B01c0808 = 0x%08x\n", val);

	RALINK_REG(0xB01C000C) = 0x40001408;    // force device mode
	udelay(50000);
	RALINK_REG(0xB01C0E00) = 0x1;           // stop pclock
	udelay(100000);

	/* Some layouts need more time. */
	for(try_count=0 ; try_count< 1000 /* ms */; try_count++)
	{
		val = le32_to_cpu(*(volatile u_long *)(0xB01C0808));
		//printf("B01C0808 = 0x%08x\n", val);
		if((val & 0x1)){
			rc = 0;
			break;
		}
		udelay(1000);
	}

	val = RALINK_REG(RT2880_RSTCTRL_REG);    // reset OTG
	val = val | RALINK_OTG_RST;
	RALINK_REG(RT2880_RSTCTRL_REG) = val;
	val = val & ~(RALINK_OTG_RST);
	RALINK_REG(RT2880_RSTCTRL_REG) = val;
	udelay(200000);

	return rc;
}

void config_usbotg(void)
{
	int i, host_rc, device_rc;	

	printf("config usb");
	for(i=0;i<2;i++){
		device_rc = usbotg_device_suspend();
		host_rc = usbotg_host_suspend();

		if(host_rc == -1 && device_rc == -1)
			continue;
		else
			break;
	}
	
	RALINK_REG(0xB01C0E00) = 0xF;        //disable USB module, optimize for power-saving
	printf("\n");
	return;
}

#endif

#if defined (RT6855A_ASIC_BOARD) || defined(RT6855A_FPGA_BOARD)	
static int watchdog_reset()
{
	unsigned int word;
	unsigned int i;

	/* check if do watch dog reset */
	if ((RALINK_REG(RALINK_HIR_REG) & 0xffff0000) == 0x40000) {
		if (!(RALINK_REG(0xbfb00080) >> 31)) {
			/* set delay counter */
			RALINK_REG(RALINK_TIMER5_LDV) = 1000;
			/* enable watch dog timer */
			word = RALINK_REG(RALINK_TIMER_CTL);
			word |= ((1 << 5) | (1 << 25));
			RALINK_REG(RALINK_TIMER_CTL) = word;
			while(1);
		}
	}

	return 0;
}
#endif

#if ! defined(UBOOT_STAGE1)
/* Restore to default. */
int reset_to_default(void)
{
	ulong addr, size;

#if defined(UBI_SUPPORT)
	unsigned char *p;

	addr = CFG_FLASH_BASE + CFG_BOOTLOADER_SIZE + CFG_CONFIG_SIZE;
	size = CFG_NVRAM_SIZE;
	p = malloc(CFG_NVRAM_SIZE);
	if (!p)
		p = (unsigned char*) CFG_LOAD_ADDR;

	memset(p, 0xFF, CFG_NVRAM_SIZE);
	ra_flash_erase_write(p, addr, size, 0);

	if (p != (unsigned char*) CFG_LOAD_ADDR)
		free(p);
#endif
	/* erase U-Boot environment whether it shared same block with nvram or not. */
	addr = CFG_ENV_ADDR;
	size = CFG_CONFIG_SIZE;
	printf("Erase 0x%08x size 0x%x\n", addr, size);
	ranand_set_sbb_max_addr(addr + size);
	ra_flash_erase(addr, size);
	ranand_set_sbb_max_addr(0);

	return 0;
}
#endif /* ! UBOOT_STAGE1 */

