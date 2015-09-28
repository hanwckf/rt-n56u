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

/*
 * This file contains the configuration parameters for the RT2880 board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#ifndef __ASSEMBLY__ 
#if defined(CFG_ENV_IS_IN_NAND) /* Environment is in NAND Flash */
#if defined(MTK_NAND) 
extern unsigned int  CFG_BLOCKSIZE;
#else
#include "../../drivers/ralink_nand.h"
#endif
#endif
#endif

#include "../../autoconf.h"
//#define DEBUG				1
//#define ET_DEBUG
#define CONFIG_RT2880_ETH		1	/* Enable built-in 10/100 Ethernet */

#define CONFIG_MIPS32		1	/* MIPS 4Kc CPU core	*/
//CONFIG_INCA_IP
#if defined (RT3052_FPGA_BOARD) || defined (RT3352_FPGA_BOARD) || \
    defined (RT2883_FPGA_BOARD) || defined (RT3883_FPGA_BOARD) || \
    defined (RT5350_FPGA_BOARD) || defined (RT6855_FPGA_BOARD) || \
    defined (MT7620_FPGA_BOARD) || defined (MT7621_FPGA_BOARD) || \
    defined (RT6855A_FPGA_BOARD) || defined (MT7628_FPGA_BOARD)
#define FPGA_BOARD_CLOCK_RATE 40000000
#else
#define FPGA_BOARD_CLOCK_RATE 25000000
#endif
#define PALMCHIP_CLOCK_DIVISOR 16
#define FPGA_BOARD_CLOCK_DIVISOR 32

/* allowed values: 100000000, 133000000, and 150000000 */
#if 1 /* for CFG_HZ only */

#ifdef RT2880_ASIC_BOARD
#define CPU_CLOCK_RATE	266666666 /* default: 150 MHz clock for the MIPS core */
#elif defined (RT3052_ASIC_BOARD)
#define CPU_CLOCK_RATE	384000000 
#elif defined (RT3352_ASIC_BOARD)
#define CPU_CLOCK_RATE	400000000 
#elif defined (RT6855_ASIC_BOARD) || defined (RT6855A_ASIC_BOARD)
#define CPU_CLOCK_RATE	500000000 
#elif defined (MT7620_ASIC_BOARD)
#define CPU_CLOCK_RATE	600000000 
#elif defined (MT7628_ASIC_BOARD)
#define CPU_CLOCK_RATE	600000000 
#elif defined (MT7621_ASIC_BOARD)
#if defined (MT7621_CPU_FREQUENCY)
#define CPU_CLOCK_RATE	(MT7621_CPU_FREQUENCY*1000000)
#else
#define CPU_CLOCK_RATE  (800000000)
#endif
#elif defined (RT2883_ASIC_BOARD)
#define CPU_CLOCK_RATE	400000000 
#elif defined (RT3883_ASIC_BOARD)
#define CPU_CLOCK_RATE	500000000 
#elif defined (RT5350_ASIC_BOARD)
#define CPU_CLOCK_RATE	360000000 
#else
#define CPU_CLOCK_RATE	FPGA_BOARD_CLOCK_RATE /* default: 150 MHz clock for the MIPS core */
#endif

#endif 

#define SERIAL_CLOCK_DIVISOR 16

#define CONFIG_BOOTDELAY	1	/* autoboot after 1 seconds	*/

#define CONFIG_BAUDRATE		115200

#define CONFIG_SERVERIP 192.168.1.2
#define CONFIG_IPADDR 192.168.1.1
#define CONFIG_ETHADDR "00:0C:43:30:52:11"
/* valid baudrates */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

//#define	CONFIG_TIMESTAMP		/* Print image info with timestamp */

#undef	CONFIG_BOOTARGS

#define CONFIG_BOOTCOMMAND	"tftp" //"run flash_self"


#include <cmd_confdefs.h>

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP				/* undef to save memory      */

#if defined (RT2880_FPGA_BOARD) || defined (RT2880_ASIC_BOARD) 
#define	CFG_PROMPT		"RT2880 # "
#elif defined (RT2883_FPGA_BOARD) || defined (RT2883_ASIC_BOARD) 
#define	CFG_PROMPT		"RT2883 # "
#elif defined (RT3052_FPGA_BOARD) || defined (RT3052_ASIC_BOARD) 
#define	CFG_PROMPT		"RT3052 # "
#elif defined (RT3352_FPGA_BOARD) || defined (RT3352_ASIC_BOARD) 
#define	CFG_PROMPT		"RT3352 # "
#elif defined (RT3883_FPGA_BOARD) || defined (RT3883_ASIC_BOARD) 
#define	CFG_PROMPT		"RT3883 # "
#elif defined (RT5350_FPGA_BOARD) || defined (RT5350_ASIC_BOARD) 
#define	CFG_PROMPT		"RT5350 # "
#elif defined (RT6855_FPGA_BOARD) || defined (RT6855_ASIC_BOARD) 
#define	CFG_PROMPT		"RT6855 # "
#elif defined (RT6855A_FPGA_BOARD) || defined (RT6855A_ASIC_BOARD) 
#define	CFG_PROMPT		"RT6855A # "
#elif defined (MT7620_FPGA_BOARD) || defined (MT7620_ASIC_BOARD) 
#define	CFG_PROMPT		"MT7620 # "
#elif defined (MT7621_FPGA_BOARD) || defined (MT7621_ASIC_BOARD) 
#define	CFG_PROMPT		"MT7621 # "
#elif defined (MT7628_FPGA_BOARD) || defined (MT7628_ASIC_BOARD) 
#define	CFG_PROMPT		"MT7628 # "
#else
#define	CFG_PROMPT		"RTxxxx # "
#endif
#define	CFG_CBSIZE		256		/* Console I/O Buffer Size   */
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)  /* Print Buffer Size */
#define	CFG_MAXARGS		16		/* max number of command args*/

#if defined (MTK_NAND)
#define CFG_MALLOC_LEN      1*1024*1024
#else
#define CFG_MALLOC_LEN		256*1024
#endif

#define CFG_BOOTPARAMS_LEN	128*1024

#define CFG_HZ			CPU_CLOCK_RATE/2

#if defined (RT2880_FPGA_BOARD) || defined (RT2880_ASIC_BOARD)
#define CFG_SDRAM_BASE		0x8A000000
#else
#define CFG_SDRAM_BASE		0x80000000
#endif


/* 
 * for TEST 
 */
#define CFG_CONSOLE_INFO_QUIET	
#define	CFG_LOAD_ADDR		(CFG_SDRAM_BASE + (gd->ram_size)/2)	/* default load address	*/

#if defined (RT2880_FPGA_BOARD) || defined (RT2880_ASIC_BOARD)
#define CFG_HTTP_DL_ADDR	0x8A300000
#define CFG_MEMTEST_START	0x8A100000
#define CFG_MEMTEST_END		0x8A400000
#else
#define CFG_HTTP_DL_ADDR	0x80300000
#if defined(RT6855A_FPGA_BOARD) || defined(RT6855A_ASIC_BOARD) || defined(MT7620_FPGA_BOARD) || defined(MT7620_ASIC_BOARD) || defined(MT7628_FPGA_BOARD) || defined(MT7628_ASIC_BOARD)
#define CFG_SPINAND_LOAD_ADDR	0x80c00000
#else
#define CFG_SPINAND_LOAD_ADDR	0x80500000
#endif

#define CFG_MEMTEST_START	0x80100000
#define CFG_MEMTEST_END		0x80400000
#endif


#define CFG_EMBEDED_SRAM_START 0xA0800000
#define CFG_EMBEDED_SRAM_FOR_RXRING0_DESC 0xA0800000
#define CFG_EMBEDED_SRAM_FOR_TXRING0_DESC 0xA0800100
#define CFG_EMBEDED_SRAM_FOR_TXRING1_DESC 0xA0800200
#define CFG_EMBEDED_SRAM_BUF_START 0xA0800300
#define CFG_EMBEDED_SRAM_SDP0_BUF_START 0xA0804000
#define CFG_EMBEDED_SRAM_END   0xA0807FFF

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define CFG_MAX_FLASH_SECT	(263)	/* max number of sectors on one chip */

#if defined (RT2880_FPGA_BOARD) || defined (RT2880_ASIC_BOARD)
#ifdef DUAL_IMAGE_SUPPORT
#define PHYS_FLASH_START	0xBC000000 /* Address for issuing flash command */
#if defined (ON_BOARD_2M_FLASH_COMPONENT)
#define PHYS_FLASH_1		0xBC000000 /* Image1 Bank #1 */
#define PHYS_FLASH2_1		0xBC100000 /* Image2 Bank #1 */
#elif defined (ON_BOARD_4M_FLASH_COMPONENT)
#define PHYS_FLASH_1		0xBC000000 /* Image1 Bank #1 */
#define PHYS_FLASH2_1		0xBC200000 /* Image2 Bank #1 */
#elif defined (ON_BOARD_8M_FLASH_COMPONENT)
#define PHYS_FLASH_1		0xBC400000 /* Image1 Bank #1 */
#define PHYS_FLASH_2		0xBC000000 /* Image1 Bank #2 */
#define PHYS_FLASH2_1		0xBC000000 /* Image2 Bank #1 */
#elif defined (ON_BOARD_16M_FLASH_COMPONENT)
#define PHYS_FLASH_1		0xBCC00000 /* Image1 Bank #1 */
#define PHYS_FLASH_2		0xBC000000 /* Image1 Bank #2 */
#define PHYS_FLASH2_1		0xBC400000 /* Image2 Bank #1 */
#define PHYS_FLASH2_2		0xBC800000 /* Image2 Bank #2 */
#endif 
#else //Non Dual Image
#ifdef ON_BOARD_8M_FLASH_COMPONENT
#define PHYS_FLASH_1		0xBC400000 /* Flash Bank #1 */
#else
#define PHYS_FLASH_1		0xBCC00000 /* Flash Bank #1 */
#endif
#define PHYS_FLASH_2		0xBC000000 /* Flash Bank #2 */
#if defined (ON_BOARD_8M_FLASH_COMPONENT) || defined (ON_BOARD_16M_FLASH_COMPONENT)
#define PHYS_FLASH_START	PHYS_FLASH_2 /* Address for issuing flash command */
#else
#define PHYS_FLASH_START	PHYS_FLASH_1 /* Address for issuing flash command */
#endif
#endif
#elif defined (RT2883_FPGA_BOARD) || defined (RT2883_ASIC_BOARD) || \
      defined (RT3883_FPGA_BOARD) || defined (RT3883_ASIC_BOARD) || \
      defined (RT3352_FPGA_BOARD) || defined (RT3352_ASIC_BOARD) || \
      defined (RT5350_FPGA_BOARD) || defined (RT5350_ASIC_BOARD) || \
      defined (RT6855_FPGA_BOARD) || defined (RT6855_ASIC_BOARD) || \
      defined (MT7620_FPGA_BOARD) || defined (MT7620_ASIC_BOARD) || \
      defined (MT7621_FPGA_BOARD) || defined (MT7621_ASIC_BOARD) || \
      defined (MT7628_FPGA_BOARD) || defined (MT7628_ASIC_BOARD)
#define PHYS_FLASH_START	0xBC000000 /* Flash Bank #2 */
#define PHYS_FLASH_1		0xBC000000 /* Flash Bank #1 */
  #ifdef DUAL_IMAGE_SUPPORT
  #if defined (ON_BOARD_2M_FLASH_COMPONENT)
  #define PHYS_FLASH2_1		0xBC100000 /* Flash Bank #2 */
  #elif defined (ON_BOARD_4M_FLASH_COMPONENT)
  #define PHYS_FLASH2_1		0xBC200000 /* Flash Bank #2 */
  #elif defined (ON_BOARD_8M_FLASH_COMPONENT)
  #define PHYS_FLASH2_1		0xBC400000 /* Flash Bank #2 */
  #elif defined (ON_BOARD_16M_FLASH_COMPONENT)
  #define PHYS_FLASH2_1		0xBC800000 /* Flash Bank #2 */
  #elif defined (ON_BOARD_32M_FLASH_COMPONENT)
  #define PHYS_FLASH2_1		0xBD000000 /* Flash Bank #2 */
  #endif
  #endif // DUAL_IMAGE_SUPPORT
#elif  defined (MT7621_FPGA_BOARD) || defined (MT7621_ASIC_BOARD)
#define PHYS_FLASH_START	0xBFC00000 /* Flash Bank #2 */
#define PHYS_FLASH_1		0xBFC00000 /* Flash Bank #1 */
  #ifdef DUAL_IMAGE_SUPPORT
	/* TODO */
  #endif
#elif defined (RT3052_FPGA_BOARD) || defined (RT3052_ASIC_BOARD)
  // RT3052_MP2 and 32M_FLASH
  #define PHYS_FLASH_START	0xBF000000 /* Address for issuing flash command */
  #define PHYS_FLASH_1		0xBF000000 /* Flash Bank #1 */
  #ifdef DUAL_IMAGE_SUPPORT
  #if defined (ON_BOARD_2M_FLASH_COMPONENT)
  #define PHYS_FLASH2_1		0xBF100000 /* Flash Bank #2 */
  #elif defined (ON_BOARD_4M_FLASH_COMPONENT)
  #define PHYS_FLASH2_1		0xBF200000 /* Flash Bank #2 */
  #elif defined (ON_BOARD_8M_FLASH_COMPONENT)
  #define PHYS_FLASH2_1		0xBF400000 /* Flash Bank #2 */
  #elif defined (ON_BOARD_16M_FLASH_COMPONENT)
  #define PHYS_FLASH2_1		0xBF800000 /* Flash Bank #2 */
  #elif defined (ON_BOARD_32M_FLASH_COMPONENT)
  #define PHYS_FLASH2_1		0xBB000000 /* Flash Bank #2 */
  #undef CFG_MAX_FLASH_BANKS
  #define CFG_MAX_FLASH_BANKS	2
  #endif
  //#define PHYS_FLASH_2		0xBF000000 /* Flash Bank #2 */
  #else // Non Dual Image
  #ifdef ON_BOARD_32M_FLASH_COMPONENT
  #define PHYS_FLASH2_START	0xBB000000 /* Flash Bank #2 */
  #define PHYS_FLASH_2		0xBB000000 /* Flash Bank #2 */
  #undef CFG_MAX_FLASH_BANKS
  #define CFG_MAX_FLASH_BANKS	2
  #endif
 #endif
#elif defined (RT6855A_FPGA_BOARD) || defined (RT6855A_ASIC_BOARD)
#define PHYS_FLASH_1		0xB0000000
#endif // defined (RT2880_FPGA_BOARD) || defined (RT2880_ASIC_BOARD)

/* The following #defines are needed to get flash environment right */
#define	CFG_MONITOR_BASE	TEXT_BASE

#define	CFG_MONITOR_LEN		(192 << 10)

#define CFG_INIT_SP_OFFSET	0x400000

#define CFG_FLASH_BASE		PHYS_FLASH_1
#ifdef DUAL_IMAGE_SUPPORT
#define CFG_FLASH2_BASE		PHYS_FLASH2_1
#endif

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(15UL * CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(5 * CFG_HZ) /* Timeout for Flash Write */
#define CFG_ETH_AN_TOUT	(5 * CFG_HZ) /* Timeout for Flash Write */
#define CFG_ETH_LINK_UP_TOUT	(5 * CFG_HZ) /* Timeout for Flash Write */
#define CFG_FLASH_STATE_DISPLAY_TOUT  (2 * CFG_HZ) /* Timeout for Flash Write */

#if defined (ON_BOARD_32M_FLASH_COMPONENT) && !defined (DUAL_IMAGE_SUPPORT)
#define CFG_BOOTLOADER_SIZE	0x40000
#define CFG_CONFIG_SIZE		0x20000
#define CFG_FACTORY_SIZE	0x20000
#define CFG_ENV_ADDR		(PHYS_FLASH_2 + 0x1000000 - CFG_BOOTLOADER_SIZE)
#define CFG_FACTORY_ADDR	(PHYS_FLASH_2 + 0x1000000 - CFG_FACTORY_SIZE)
#define CFG_KERN_ADDR		(CFG_FLASH_BASE + CFG_BOOTLOADER_SIZE)
#define CFG_KERN2_ADDR		(CFG_FLASH2_BASE + CFG_BOOTLOADER_SIZE)
#else
#if defined(MTK_NAND) || defined (CFG_ENV_IS_IN_NAND)
#define CFG_BOOTLOADER_SIZE	(CFG_BLOCKSIZE*NAND_BLK_BOOTLOADER)	// 4x blocks ( 512K)
#define CFG_CONFIG_SIZE		(CFG_BLOCKSIZE*NAND_BLK_CONFIG)		//10x blocks (1280K)
#define CFG_FACTORY_SIZE	(CFG_BLOCKSIZE*NAND_BLK_FACTORY)	// 2x blocks ( 256K), kernel start from 0x200000
#elif defined(SMALL_UBOOT_PARTITION)
#define CFG_BOOTLOADER_SIZE	0x20000
#define CFG_CONFIG_SIZE		0x10000
#define CFG_FACTORY_SIZE	0x00000
#else
#define CFG_BOOTLOADER_SIZE	0x30000
#define CFG_CONFIG_SIZE		0x10000
#define CFG_FACTORY_SIZE	0x10000
#endif
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + CFG_BOOTLOADER_SIZE)
#define CFG_FACTORY_ADDR	(CFG_FLASH_BASE + CFG_BOOTLOADER_SIZE + CFG_CONFIG_SIZE)
#define CFG_KERN_ADDR		(CFG_FLASH_BASE + (CFG_BOOTLOADER_SIZE + CFG_CONFIG_SIZE + CFG_FACTORY_SIZE))
#ifdef DUAL_IMAGE_SUPPORT
#define CFG_KERN2_ADDR		(CFG_FLASH2_BASE + (CFG_BOOTLOADER_SIZE + CFG_CONFIG_SIZE + CFG_FACTORY_SIZE))
#endif
#endif

#define CFG_ENV_SECT_SIZE	CFG_CONFIG_SIZE
#define CFG_ENV_SIZE		0x1000

#if defined(SMALL_UBOOT_PARTITION)
#define CFG_UBOOT_SECT_SIZE	CFG_BOOTLOADER_SIZE
#define CFG_UBOOT_SIZE		0x19000 // must <= CFG_FACTORY_ADDR
#define CFG_RF_PARAM_SIZE	0x800
#undef CFG_FACTORY_ADDR
#define CFG_FACTORY_ADDR	(CFG_BOOTLOADER_SIZE - (2 * CFG_RF_PARAM_SIZE))
#define CFG_FACTORY_ADDR2	(CFG_BOOTLOADER_SIZE - CFG_RF_PARAM_SIZE)
#endif

#if defined (DUAL_IMAGE_SUPPORT)
#if defined (ON_BOARD_2M_FLASH_COMPONENT)
#define CFG_KERN_SIZE		(0x100000 - (CFG_BOOTLOADER_SIZE + CFG_CONFIG_SIZE + CFG_FACTORY_SIZE))
#elif defined (ON_BOARD_4M_FLASH_COMPONENT)
#define CFG_KERN_SIZE		(0x200000 - (CFG_BOOTLOADER_SIZE + CFG_CONFIG_SIZE + CFG_FACTORY_SIZE))
#elif defined (ON_BOARD_8M_FLASH_COMPONENT)
#define CFG_KERN_SIZE		(0x400000 - (CFG_BOOTLOADER_SIZE + CFG_CONFIG_SIZE + CFG_FACTORY_SIZE))
#elif defined (ON_BOARD_16M_FLASH_COMPONENT)
#define CFG_KERN_SIZE		(0x800000 - (CFG_BOOTLOADER_SIZE + CFG_CONFIG_SIZE + CFG_FACTORY_SIZE))
#endif
#define CFG_KERN2_SIZE		CFG_KERN_SIZE
#else // Non Dual Image
#if defined (ON_BOARD_2M_FLASH_COMPONENT)
#define CFG_KERN_SIZE		(0x200000 - (CFG_BOOTLOADER_SIZE + CFG_CONFIG_SIZE + CFG_FACTORY_SIZE))
#elif defined (ON_BOARD_4M_FLASH_COMPONENT)
#define CFG_KERN_SIZE		(0x400000 - (CFG_BOOTLOADER_SIZE + CFG_CONFIG_SIZE + CFG_FACTORY_SIZE))
#elif defined (ON_BOARD_8M_FLASH_COMPONENT)
#define CFG_KERN_SIZE		(0x800000 - (CFG_BOOTLOADER_SIZE + CFG_CONFIG_SIZE + CFG_FACTORY_SIZE))
#elif defined (ON_BOARD_16M_FLASH_COMPONENT)
#define CFG_KERN_SIZE		(0x1000000 - (CFG_BOOTLOADER_SIZE + CFG_CONFIG_SIZE + CFG_FACTORY_SIZE))
#endif
#endif


#define UBOOT_FILE_SIZE_MIN	0x8000	/* for prevent flash damaged Uboot */
#if defined(SMALL_UBOOT_PARTITION)
#define UBOOT_FILE_SIZE_MAX	CFG_UBOOT_SIZE
#else
#define UBOOT_FILE_SIZE_MAX	CFG_BOOTLOADER_SIZE
#endif


#define CONFIG_FLASH_16BIT

#define CONFIG_NR_DRAM_BANKS	1
//#define CONFIG_NET_MULTI
#define CFG_RX_ETH_BUFFER		60

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_DCACHE_SIZE		(16*1024)
#define CFG_ICACHE_SIZE		(16*1024)
#define CFG_CACHELINE_SIZE	16

#define RT2880_REGS_BASE			0xA0000000


/*
 * System Controller	(0x00300000)
 *   Offset
 *   0x10  -- SYSCFG		System Configuration Register
 *   0x30  -- CLKCFG1		Clock Configuration Register
 *   0x34  -- RSTCTRL		Reset Control Register
 *   0x38  -- RSTSTAT		Reset Status Register 
 *   0x60  -- GPIOMODE		GPIO Mode Control Register 
 */
#define RT2880_SYS_CNTL_BASE			(RALINK_SYSCTL_BASE)
#if defined (RT6855A_FPGA_BOARD) || defined (RT6855A_ASIC_BOARD)
#define RT2880_SYSCFG_REG                       (RT2880_SYS_CNTL_BASE+0x8c)
#define RT2880_RSTCTRL_REG                      (RT2880_SYS_CNTL_BASE+0x834)
#define RT2880_RSTSTAT_REG                      (RT2880_SYS_CNTL_BASE+0x38)
#define RT2880_GPIOMODE_REG                     (RT2880_SYS_CNTL_BASE+0x860)
#else
#define RT2880_CHIP_REV_ID_REG			(RT2880_SYS_CNTL_BASE+0x0c)
#define RT2880_SYSCFG_REG			(RT2880_SYS_CNTL_BASE+0x10)
#define RT2880_SYSCFG1_REG			(RT2880_SYS_CNTL_BASE+0x14)
#define RT2880_CLKCFG1_REG			(RT2880_SYS_CNTL_BASE+0x30)
#define RT2880_RSTCTRL_REG			(RT2880_SYS_CNTL_BASE+0x34)
#define RT2880_RSTSTAT_REG			(RT2880_SYS_CNTL_BASE+0x38)
#define RT2880_SYSCLKCFG_REG			(RT2880_SYS_CNTL_BASE+0x3c)
#if defined (MT7628_ASIC_BOARD)
#define RT2880_AGPIOCFG_REG			(RT2880_SYS_CNTL_BASE+0x3c)
#endif
#define RT2880_GPIOMODE_REG			(RT2880_SYS_CNTL_BASE+0x60)
#endif

#define RT2880_PRGIO_ADDR       (RALINK_SYSCTL_BASE + 0x600) // Programmable I/O
#if defined (MT7621_FPGA_BOARD) || defined (MT7621_ASIC_BOARD) || \
    defined (MT7628_FPGA_BOARD) || defined (MT7628_ASIC_BOARD)
#define RT2880_REG_PIOINT       (RT2880_PRGIO_ADDR + 0x90)
#define RT2880_REG_PIOEDGE      (RT2880_PRGIO_ADDR + 0xA0)
#define RT2880_REG_PIORENA      (RT2880_PRGIO_ADDR + 0x50)
#define RT2880_REG_PIOFENA      (RT2880_PRGIO_ADDR + 0x60)
#define RT2880_REG_PIODATA      (RT2880_PRGIO_ADDR + 0x20)
#define RT2880_REG_PIODIR       (RT2880_PRGIO_ADDR + 0x00)
#define RT2880_REG_PIOSET       (RT2880_PRGIO_ADDR + 0x30)
#define RT2880_REG_PIORESET     (RT2880_PRGIO_ADDR + 0x40)
#else
#define RT2880_REG_PIOINT       (RT2880_PRGIO_ADDR + 0x00)
#define RT2880_REG_PIOEDGE      (RT2880_PRGIO_ADDR + 0x04)
#define RT2880_REG_PIORENA      (RT2880_PRGIO_ADDR + 0x08)
#define RT2880_REG_PIOFENA      (RT2880_PRGIO_ADDR + 0x0C)
#define RT2880_REG_PIODATA      (RT2880_PRGIO_ADDR + 0x20)
#define RT2880_REG_PIODIR       (RT2880_PRGIO_ADDR + 0x24)
#define RT2880_REG_PIOSET       (RT2880_PRGIO_ADDR + 0x2c)
#define RT2880_REG_PIORESET     (RT2880_PRGIO_ADDR + 0x30)
#endif

#define RALINK_REG(x)		(*((volatile u32 *)(x)))	
#if defined (RT6855A_FPGA_BOARD) || defined (RT6855A_ASIC_BOARD) || \
    defined (MT7621_FPGA_BOARD) || defined (MT7621_ASIC_BOARD) || defined (MT7628_FPGA_BOARD) || defined (MT7628_ASIC_BOARD)
#define ra_inb(offset)		(*(volatile unsigned char *)(offset))
#define ra_inw(offset)		(*(volatile unsigned short *)(offset))
#define ra_inl(offset)		(*(volatile unsigned long *)(offset))

#define ra_outb(offset,val)	(*(volatile unsigned char *)(offset) = val)
#define ra_outw(offset,val)	(*(volatile unsigned short *)(offset) = val)
#define ra_outl(offset,val)	(*(volatile unsigned long *)(offset) = val)

#define ra_and(addr, value) ra_outl(addr, (ra_inl(addr) & (value)))
#define ra_or(addr, value) ra_outl(addr, (ra_inl(addr) | (value)))
#endif
#define RT2880_WDRST            (1<<1)
#define RT2880_SWSYSRST         (1<<2)
#define RT2880_SWCPURST         (1<<3)


#define RT2880_UPHY0_CLK_EN		(1<<18)
#define RT2880_UPHY1_CLK_EN		(1<<20)


/*
* for USB
*/
#if defined (RALINK_USB) || defined (MTK_USB)
#ifdef CONFIG_RALINK_MT7621
#define CONFIG_USB_STORAGE    1
#define CONFIG_DOS_PARTITION	1
#define LITTLEENDIAN
#define CONFIG_CRC32_VERIFY
#define CONFIG_SYS_USB_XHCI_MAX_ROOT_PORTS      2
#else
#define CONFIG_USB_OHCI		1
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2
#define CONFIG_SYS_USB_OHCI_REGS_BASE		0x101C1000
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"rt3680"
#define CONFIG_USB_EHCI		1
#define CONFIG_USB_STORAGE    1
#define CONFIG_DOS_PARTITION	1
#define LITTLEENDIAN
#define CONFIG_CRC32_VERIFY
#endif
#endif /* RALINK_USB */

#if defined (MT7621_ASIC_BOARD) || defined (MT7621_FPGA_BOARD)
//#define USE_PIO_DBG		1
#endif

#if defined(MT7628_ASIC_BOARD)
#define PHY_BASE                0xB0120000
#define SIFSLV_FM_FEG_BASE      (PHY_BASE+0xf00)
#define U2_PHY_BASE             (PHY_BASE+0x800)
#endif

#endif	/* __CONFIG_H */
