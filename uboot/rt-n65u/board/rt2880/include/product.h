/******************************************************************************
 *  This program is free software; you can redistribute  it and/or modify it                                                                                           *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your                                                                                           *  option) any later version.
 *                                                                                                                                                                     *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF                                                                                           *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,                                                                                            *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF                                                                                           *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT                                                                                           *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                                                                                  *
 *  You should have received a copy of the  GNU General Public License along                                                                                           *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.                                                                                                                            *
 */

/*******************************************************************************
*
*  File Name: product.h
*     Author: Robin Bhagat 
*
*******************************************************************************
*
* Revision History:
*
*      Date    Name  Comments
*    --------  ---   ------------------------------------
*    08/12/97  RWB   Created.
*    02/09/01  LRF   Removed UART Default definitions.
*
*
*
*******************************************************************************/
/* FILE_DESC ******************************************************************
//
// Purpose:
//    This file contains the product dependent values, so that the entire
//    library can be customised by changing this simple file. 
//
//    It also includes the product's memory map header file and a
//    header file containing common product-independent definitions.
//
// Sp. Notes:
//
 ******************************************************************************/

#ifndef PRODUCT_H
#define PRODUCT_H

#include <config.h>	/* include/configs/rt2880.h for CFG_RUN_CODE_IN_RAM */

#if defined(CFG_RUN_CODE_IN_RAM)
#define CODE_IN_SDRAM
#endif


#define USE_DEVBOARD	1
#define USE_ROM32	1
//#define USE_SRAM	1
#define USE_SDRAM	1
#define USE_SDRAM32	1
#define USE_ESRAM	1
#define USE_BUFFERED_MAC	1
#define USE_UART_FIFO	1
#define USE_CACHE	1




// Remap vector memory for bev0 vectors
#define REMAPPED_VECTOR_MEM  1

//#define REMAPPED_SDRAM  1

#define SRAM32		1 
#define SDRAM32		1 
#define SDRAM_MEM_WIDTH	32


/*=====================*
 *  Include Files      *
 *=====================*/
//#include "pubdefs.h"
#include "mem_map.h"
#include "chip_reg_map.h"


/*=====================*
 *  Defines            *
 *=====================*/


/*----------------------------------------------------------------------------*/
// Instructions for configuring the system clock:
// 
// 1. Define the external crystal speed.
// 2. Define whether DLL will be used (for SDRAM only, at higher clock speeds).
// 3. Define whether PLL will be used.
// 4. If DLL used, define appropriate values for DLL_INVERT and DLL_PHASE.
// 5. If PLL used, define the desired system clock value, SYS_CLK_KHZ.
/*----------------------------------------------------------------------------*/



/*------------------*/
/* External Crystal */
/*------------------*/
/* Define the external crystal speed.
** The actual system clock speed is determined in the PLL section below.
*/

#define XTAL_KHZ			(16667)		/* Default value */
//#define XTAL_KHZ			(14746)
//#define XTAL_KHZ			(16000)
//#define XTAL_KHZ			(32000)
//#define XTAL_KHZ			(33330)

#ifndef XTAL_KHZ
  #error "No XTAL_KHZ defined in product.h"
#endif



/*--------------------*/
/* PLL and DLL Enable */
/*--------------------*/
#if !defined(CFG_RUN_CODE_IN_RAM)
#define USE_DLL_INIT	 1 /* Define to setup and enable DLL */
#define USE_PLL_INIT	 1 /* Define to setup and enable PLL */
#endif



/*-----------*/
/* DLL Setup */
/*-----------*/
/* At higher system clock rates, the DLL must be enabled for correct signal
** timing at SDRAM chips.  Lower speeds should not need the DLL.
** NOTE:  When the DLL is enabled, the system clock is divided by two.
*/

/* DLL Configuration */
#define DLL_INVERT			(0)		/* 1= TRUE, 0=FALSE */
#define DLL_PHASE			(DLL_PHASE_270)	/* defines in sysc.h */



/*-----------*/
/* PLL Setup */
/*-----------*/
/* Calculation of the system clock, SYS_CLK_KHZ */

/* When PLL is not enabled, the system clock is defined based on the DLL and
** the external crystal speed.
*/
#ifndef USE_PLL_INIT
 #ifdef USE_DLL_INIT
  #define SYS_CLK_KHZ			(XTAL_KHZ / 2)
 #else
  #define SYS_CLK_KHZ			(XTAL_KHZ)
 #endif
#else
/* When PLL is enabled, one SYS_CLK_KHZ appropriate for the selected XTAL_KHZ
** must be defined/uncommented.
*/
#if (XTAL_KHZ == 16667)
  //#define SYS_CLK_KHZ			(16667)		/* XTAL_KHZ * 1 */
  //#define SYS_CLK_KHZ			(33334)		/* XTAL_KHZ * 2 */
  //#define SYS_CLK_KHZ			(50001)		/* XTAL_KHZ * 3 */
  //#define SYS_CLK_KHZ			(66668)		/* XTAL_KHZ * 4 */
  //#define SYS_CLK_KHZ			(83335)		/* XTAL_KHZ * 5 */
  //#define SYS_CLK_KHZ			(100002)	/* XTAL_KHZ * 6 */
  //#define SYS_CLK_KHZ			(116669)	/* XTAL_KHZ * 7 */
  #define SYS_CLK_KHZ			(133336)	/* XTAL_KHZ * 8 */
#else
#if (XTAL_KHZ == 33330)
  //#define SYS_CLK_KHZ			(16665)		/* XTAL_KHZ / 2 */
  //#define SYS_CLK_KHZ			(33330)		/* XTAL_KHZ * 1 */
  //#define SYS_CLK_KHZ			(66660)		/* XTAL_KHZ * 2 */
  //#define SYS_CLK_KHZ			(133320)	/* XTAL_KHZ * 4 */
#else
#if (XTAL_KHZ == 14746)
  //#define SYS_CLK_KHZ			(14746)		/* XTAL_KHZ * 1 */
  //#define SYS_CLK_KHZ			(29492)		/* XTAL_KHZ * 1 */
  //#define SYS_CLK_KHZ			(132714)	/* XTAL_KHZ * 9 */
#endif /* 14746 */
#endif /* 33330 */
#endif /* 16667 */


/* Add/Modify this list only if multipliers other than .5 and 1-9 are needed.
** PLL_MULT = NDIV + 1.  PLL_DIV = MDIV + 1.
** NOTE:  SyscEnableDllPll() adjusts the PLL_MULT as needed when DLL is enabled.
*/
#if (SYS_CLK_KHZ == (XTAL_KHZ / 2))
  #define PLL_MULT			(1)
  #define PLL_DIV			(2)
#else
#if (SYS_CLK_KHZ == (XTAL_KHZ * 1))
  #define PLL_MULT			(1)
  #define PLL_DIV			(1)
#else
#if (SYS_CLK_KHZ == (XTAL_KHZ * 2))
  #define PLL_MULT			(2)
  #define PLL_DIV			(1)
#else
#if (SYS_CLK_KHZ == (XTAL_KHZ * 3))
  #define PLL_MULT			(3)
  #define PLL_DIV			(1)
#else
#if (SYS_CLK_KHZ == (XTAL_KHZ * 4))
  #define PLL_MULT			(4)
  #define PLL_DIV			(1)
#else
#if (SYS_CLK_KHZ == (XTAL_KHZ * 5))
  #define PLL_MULT			(5)
  #define PLL_DIV			(1)
#else
#if (SYS_CLK_KHZ == (XTAL_KHZ * 6))
  #define PLL_MULT			(6)
  #define PLL_DIV			(1)
#else
#if (SYS_CLK_KHZ == (XTAL_KHZ * 7))
  #define PLL_MULT			(7)
  #define PLL_DIV			(1)
#else
#if (SYS_CLK_KHZ == (XTAL_KHZ * 8))
  #define PLL_MULT			(8)
  #define PLL_DIV			(1)
#else
#if (SYS_CLK_KHZ == (XTAL_KHZ * 9))
  #define PLL_MULT			(9)
  #define PLL_DIV			(1)
#else
  #error "Invalid SYS_CLK_KHZ defined in product.h"
#endif /* 9 */
#endif /* 8 */
#endif /* 7 */
#endif /* 6 */
#endif /* 5 */
#endif /* 4 */
#endif /* 3 */
#endif /* 2 */
#endif /* 1 */
#endif /* 1/2 */

#endif /* USE_PLL_INIT */


#ifndef SYS_CLK_KHZ
  #error "No SYS_CLK_KHZ defined in product.h"
#endif



#define SYS_CLK_MHZ			(SYS_CLK_KHZ / 1000)



/* Audio sysclock must be less than sysclock */
#define AUD_SYS_CLK_HZ			(11289600)


#define SYS_CLK_NS			(1000 / SYS_CLK_MHZ)



/* Kernel Type Flag */
/* Uncomment only one of these for appropriate Kernel */
//#define PARALOOGOS 1
//#define HARDHAT 1
#define MVISTA 1

#if PARALOGOS || HARDHAT || MVISTA
#else
#error "Kernel Type Flag not defined in product.h"
#endif


//#define USE_XIP_ROM


//#define USE_DEBUGGER_TO_LOAD


/* Wakeboard Ethernet Config */
/* Uncomment to use ethernet MAC configuration specific to Wakeboard with
** Altera 1500 FPGA part (1.5 million gates).
*/
//#define WAKEBOARD1500 1


/* Ethernet MAC address */
/* This number is added to the ET0_MAC_ADDR_LO and ET1_MAC_ADDR_LO in
** linux_init.c to create the actual MAC_ADDR_LO's used.
** Must use even number.
*/
#define ENET_MAC_ADDRNUM	0xA


/* Hardcoded kernel_entry address in vmlinux.  See System.map, vmlinux.sym,
** or vmlinux.dis.
** If the linux kernel entry point is moved, change these values.
*/
#define KERNEL_ENTRY_ADDR	(0x8a1b6040)  // module support

/* See hal_src/linux_init.c for ROM Allocation map */
#ifdef BUILD_BL_PROG	// For use with bootloader+flash burner program
    #define LINUX_BIN_BASE      (0x9fc40000)	// offset = 0x40000
    #define MAX_LINUX_BIN_SIZE  (0x001c0000)
#else 
    //#define LINUX_BIN_BASE      (0x9fc04000)	// offset = 0x4000
    //#define MAX_LINUX_BIN_SIZE  (0x001fc000)
    #define LINUX_BIN_BASE      (0x9fc20000)	// offset = 0x20000
    #define MAX_LINUX_BIN_SIZE  (0x00200000)
#endif 


/*=====================*
 *  External Variables *
 *=====================*/

/*=====================*
 *  External Functions *
 *=====================*/


#endif /* PRODUCT_H */


