/*
 * (C) Copyright 2000-2003
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

#ifndef	__VERSION_H__
#define	__VERSION_H__

#include "../uboot_version.h"
#include <configs/rt2880.h>

#define U_BOOT_VERSION	"U-Boot 1.1.3"

#if defined (RT2880_ASIC_BOARD) || defined (RT2883_ASIC_BOARD) || defined (RT3052_ASIC_BOARD) || defined (RT3352_ASIC_BOARD) || defined (RT3883_ASIC_BOARD) || defined (RT5350_ASIC_BOARD)
#define CHIP_TYPE	"ASIC"
#elif defined (RT2880_FPGA_BOARD) || defined (RT2883_FPGA_BOARD) || defined (RT3052_FPGA_BOARD) || defined (RT3352_FPGA_BOARD) || defined (RT3883_FPGA_BOARD) || defined (RT5350_FPGA_BOARD)
#define CHIP_TYPE	"FPGA"
#else
#error "PLATFORM_TYPE not defined in config.mk"
#endif

#if defined (RT2880_SHUTTLE)
#define CHIP_VERSION	"2880_Shuttle"
#elif defined (RT2880_MP)
#define CHIP_VERSION	"2880_MP"
#elif defined (RT2883_MP)
#define CHIP_VERSION	"2883_MP"
#elif defined (RT3052_MP2)
#define CHIP_VERSION	"3052_MP2"
#elif defined (RT3352_MP)
#define CHIP_VERSION	"3352_MP"
#elif defined (RT3883_MP)
#define CHIP_VERSION	"3883_MP"
#elif defined (RT5350_MP)
#define CHIP_VERSION	"5350_MP"
#else
#error "CHIP_VER not defined in config.mk"
#endif


#if defined (MAC_TO_100SW_MODE)
#define GMAC_MODE	"(MAC to 100SW Mode)"
#elif defined (MAC_TO_100PHY_MODE)
#define GMAC_MODE	"(MAC to 100PHY Mode)"
#elif defined (MAC_TO_GIGAPHY_MODE)
#define GMAC_MODE	"(MAC to GigaPHY Mode)"
#ifndef MAC_TO_GIGAPHY_MODE_ADDR
#error "MAC_TO_GIGAPHY_MODE_ADDR not defined in config.mk"
#endif
#elif defined (MAC_TO_VITESSE_MODE)
#define GMAC_MODE	"(MAC to VITESSE Mode)"
#elif defined (RT3052_ASIC_BOARD) || defined (RT3052_FPGA_BOARD) || defined (RT3352_ASIC_BOARD) || defined (RT3352_FPGA_BOARD) || \
      defined (RT5350_ASIC_BOARD) || defined (RT5350_FPGA_BOARD)

#if defined (P5_MAC_TO_NONE_MODE)
#define GMAC_MODE  "(Port5<->None)"
#elif defined (P5_MAC_TO_PHY_MODE)
#define GMAC_MODE  "(Port5<->Phy)"
#elif defined (P5_RGMII_TO_MAC_MODE)
#define GMAC_MODE  "(Port5<->GigaSW)"
#elif defined (P5_MII_TO_MAC_MODE)
#define GMAC_MODE  "(Port5<->RvMII)"
#elif defined (P5_RMII_TO_MAC_MODE)
#define GMAC_MODE  "(Port5<->MII)"
#endif

#else
#error	"GMAC_MODE not defined"
#endif


#if defined (ON_BOARD_64M_DRAM_COMPONENT)
#define DRAM_COMPONENT	64
#elif defined (ON_BOARD_128M_DRAM_COMPONENT)
#define DRAM_COMPONENT	128
#elif defined (ON_BOARD_256M_DRAM_COMPONENT)
#define DRAM_COMPONENT	256
#elif defined (ON_BOARD_512M_DRAM_COMPONENT)
#define DRAM_COMPONENT	512
#elif defined (ON_BOARD_1024M_DRAM_COMPONENT)
#define DRAM_COMPONENT	1024
#elif defined (RT3883_FPGA_BOARD) || defined (RT3883_ASIC_BOARD) || defined (RT3352_FPGA_BOARD) || defined (RT3352_ASIC_BOARD)
#define DRAM_COMPONENT	({ int _x = ((RALINK_REG(RT2880_SYSCFG_REG) >> 12) & 0x7); \
		(_x == 6)? 2048 : (_x == 5)? 1024 : (_x == 4)? 512 : \
		(_x == 3)? 256 : (_x == 2)? 128 : (_x == 1)? 64 : 16; })
#elif defined CFG_ENV_IS_IN_SPI
#define DRAM_COMPONENT	({ int _x = ((RALINK_REG(RT2880_SYSCFG_REG) >> 26) & 0x3); \
		(_x == 0x2)? 256 : (_x == 0x1)? 128 : 64; })
#else
#error "DRAM SIZE not defined"
#endif

#if defined (ON_BOARD_16BIT_DRAM_BUS)
#define DRAM_BUS	16
#elif defined (ON_BOARD_32BIT_DRAM_BUS)
#define DRAM_BUS	32
#elif defined (RT3883_FPGA_BOARD) || defined (RT3883_ASIC_BOARD) || defined (RT3352_FPGA_BOARD) || defined (RT3352_ASIC_BOARD)
#define DRAM_BUS	({ ((RALINK_REG(RT2880_SYSCFG_REG) >> 15) & 0x1)? 32 : 16; })
#elif defined (CFG_ENV_IS_IN_SPI)
#define DRAM_BUS	({ ((RALINK_REG(RT2880_SYSCFG_REG) >> 28) & 0x1)? 32 : 16; })
#else
#error "DRAM BUS not defined"
#endif

#if defined (CFG_ENV_IS_IN_SPI) && !defined (RT3883_FPGA_BOARD) && !defined (RT3883_FPGA_BOARD)
#if defined (RT3352_FPGA_BOARD) || defined (RT3352_ASIC_BOARD)
#define DDR_INFO	({ ((RALINK_REG(RT2880_SYSCFG_REG) >> 17) & 0x1)? \
		(((RALINK_REG(RT2880_SYSCFG_REG) >> 10) & 0x1)? "DDR, width 16" : "DDR, width 8") : \
		"SDR"; })
#else
#define DDR_INFO	"SDR"
#endif
#else
#ifdef ON_BOARD_DDR 
  #ifdef ON_BOARD_DDR_WIDTH_8 
  #define DDR_INFO "DDR, width 8"
  #else
  #define DDR_INFO "DDR, width 16"
  #endif
#else
#define DDR_INFO	"SDR"
#endif
#endif

#define DRAM_SIZE ((DRAM_COMPONENT/8)*(DRAM_BUS/16))

#if defined (ON_BOARD_2M_FLASH_COMPONENT)
#define FLASH_MSG "Flash component: 2 MBytes NOR Flash"
#elif defined (ON_BOARD_4M_FLASH_COMPONENT)
#define FLASH_MSG "Flash component: 4 MBytes NOR Flash"
#elif defined (ON_BOARD_8M_FLASH_COMPONENT)
#define FLASH_MSG "Flash component: 8 MBytes NOR Flash"
#elif defined (ON_BOARD_16M_FLASH_COMPONENT)
#define FLASH_MSG "Flash component: 16 MBytes NOR Flash"
#elif defined (ON_BOARD_32M_FLASH_COMPONENT)
#define FLASH_MSG "Flash component: 32 MBytes NOR Flash"
  #ifndef RT3052_MP2
  #error "32MB flash is only supported by RT3052 MP2 currently"
  #endif
#else
  #if defined (RT2880_ASIC_BOARD) || defined (RT2880_FPGA_BOARD) 
  #error "FLASH SIZE not defined"
  #elif defined CFG_ENV_IS_IN_FLASH
  #define FLASH_MSG "Flash component: NOR Flash"
  #elif defined CFG_ENV_IS_IN_NAND
  #define FLASH_MSG "Flash component: NAND Flash"
  #elif defined CFG_ENV_IS_IN_SPI
  #define FLASH_MSG "Flash component: SPI Flash"
  #else
  #error "FLASH TYPE not defined"
  #endif
#endif


#define SHOW_VER_STR()	\
	do {	\
		printf("============================================ \n"); \
		printf("Ralink UBoot Version: %s\n", RALINK_LOCAL_VERSION); \
		printf("-------------------------------------------- \n"); \
		printf("%s %s %s\n",CHIP_TYPE, CHIP_VERSION, GMAC_MODE); \
		printf("DRAM component: %d Mbits %s\n", DRAM_COMPONENT, DDR_INFO); \
		printf("DRAM bus: %d bit\n", DRAM_BUS); \
		printf("Total memory: %d MBytes\n", DRAM_SIZE); \
		printf("%s\n", FLASH_MSG); \
		printf("%s\n", "Date:" __DATE__ "  Time:" __TIME__ ); \
		printf("============================================ \n"); \
	}while(0)

#endif	/* __VERSION_H__ */
