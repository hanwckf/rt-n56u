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
*  File Name: mac.h
*     Author: Robin Bhagat 
*
*******************************************************************************
*
* Revision History:
*
*      Date    Name  Comments
*    --------  ---   ------------------------------------
*    09/02/97  RWB   ...
*    10/24/00  IST   Merged header files for PalmPak 2.0
*    11/09/00  IST   Updated for PalmPak 2.0 MAC register file.
*
*
*
*******************************************************************************/
/* FILE_DESC ******************************************************************/
//
// Purpose:
//    This file contains all the definitions
//    of the Memory Access Controller (MAC) block. 
//
// Sp. Notes:
//
/******************************************************************************/

#ifndef MAC_H
#define MAC_H

/*=====================*
 *  Include Files      *
 *=====================*/
#include "pubdefs.h"
#include "product.h"


/*=====================*
 *  Defines            *
 *=====================*/

/* These must match the type definition below!  Used in sysboot.s */
#define MAC_SDRAM_REFR_CNTL_REG (MAC_BASE + 0x00)
#define MAC_ROM_CONFIG_REG	(MAC_BASE + 0x14)
#define MAC_SRAM_CONFIG_REG	(MAC_BASE + 0x24)

#define MAC_SDRAM_CNTL_REG      (MAC_BASE + 0x30)
#define MAC_SDRAM_CONFIG_REG    (MAC_BASE + 0x34)
#define MAC_SDRAM_MODE_REG      (MAC_BASE + 0x38)

#define MAC_SDRAM2_CNTL_REG     (MAC_BASE + 0x40)
#define MAC_SDRAM2_CONFIG_REG   (MAC_BASE + 0x44)
#define MAC_SDRAM2_MODE_REG     (MAC_BASE + 0x48)

#define MAC_WRITESTAT_REG	(MAC_BASE + 0x90)
#define MAC_POSTWRITE_EN_REG	(MAC_BASE + 0x94)


/* ROM and SRAM Config register bit definitions */
#define MAC_WIDTH_8		(0x0)
#define MAC_WIDTH_16		(0x1)
#define MAC_WIDTH_32		(0x2)
#define MAC_WIDTH_SHIFT         (0)
#define MAC_WIDTH_MASK		(0x3)

#define MAC_BANKTYPE_ASYNC      (0x2)
#define MAC_BANKTYPE_SHIFT      (2)
#define MAC_BANKTYPE_MASK	(0x3)

#define MAC_RHOLD_SHIFT		(4)
#define MAC_RHOLD_MASK		(0x3)
#define MAC_WHOLD_SHIFT		(6)
#define MAC_WHOLD_MASK		(0x3)

#define MAC_OE_SHIFT		(8)	        /* read enable pulse width */
#define MAC_OE_MASK		(0xF)
#define MAC_WE_SHIFT		(12)	        /* write enable pulse width */
#define MAC_WE_MASK		(0xF)

#define MAC_RADDR_SETUP_SHIFT	(16)    	/* read address setup */
#define MAC_RADDR_SETUP_MASK	(0x3)
#define MAC_WADDR_SETUP_SHIFT	(18)    	/* write address setup */
#define MAC_WADDR_SETUP_MASK	(0x3)

#define MAC_ADDR2CS_SETUP_SHIFT (20)            /* addr to chip select setup */
#define MAC_ADDR2CS_SETUP_MASK  (0x3)

#define MAC_CS2ADDR_HOLD_SHIFT  (22)            /* chip select to addr hold */
#define MAC_CS2ADDR_HOLD_MASK   (0x1)

#define MAC_BYTE_EN_SHIFT       (23)            /* chip byte enable */
#define MAC_BYTE_EN_MASK        (0x1)



/* SDRAM Refresh Control register bit definitions */
#define MAC_REFRESH_RATE_SHIFT      (0)
#define MAC_REFRESH_RATE_MASK       (0xFFF)
#define MAC_REFRESH_PRESCALE_SHIFT  (13)
#define MAC_REFRESH_PRESCALE_MASK   (0x7)


/* SDRAM Control register bit definitions */
#define MAC_CTRL_SDRAMCLK	(1)
#define MAC_CTRL_SDRAMINI	(2)


/* SDRAM Config register bit definitions */
#define MAC_SDRAM_WIDTH_16	(0x0)
#define MAC_SDRAM_WIDTH_32	(0x1)
#define MAC_SDRAM_WIDTH_SHIFT	(0)
#define MAC_SDRAM_WIDTH_MASK	(0x3)

#define MAC_BANKTYPE_SDRAM	(0x3)

#define MAC_PGSIZE_SHIFT	(4)
#define MAC_PGSIZE_MASK	(0x3)

#define MAC_PCABIT_SHIFT	(6)
#define MAC_PCABIT_MASK	(0x3)

#define MAC_SDRAMTYP_SHIFT	(8)
#define MAC_SDRAMTYP_MASK	(0x1)

#define MAC_NUMROWADR_SHIFT	(9)
#define MAC_NUMROWADR_MASK	(0x7)

#define MAC_PRECHGOPT_SHIFT	(12)
#define MAC_PRECHGOPT_MASK	(0x3)

#define MAC_PRECHRG_SHIFT	(16)
#define MAC_PRECHRG_MASK	(0x3)

#define MAC_ACTIVE_SHIFT	(18)
#define MAC_ACTIVE_MASK		(0x3)

#define MAC_MODE_SHIFT		(20)
#define MAC_MODE_MASK		(0x3)

#define MAC_TERM_SHIFT		(22)
#define MAC_TERM_MASK		(0x3)

#define MAC_REFR_SHIFT		(24)
#define MAC_REFR_MASK		(0xf)


/* SDRAM Mode data register bit definitions */
#define MAC_MODEDATA_SHIFT	(0)
#define MAC_MODEDATA_MASK	(0x3ff)

/* SDRAM Mode data defines for NEC uPD4502161 2M-bit SDRAM */
#define MAC_MD_BURSTLEN_SHIFT		(0)
#define MAC_MD_BURSTLEN_MASK		(0x7)

#define MAC_MD_WRAPTYPE_SEQUENTIAL	(0x0)
#define MAC_MD_WRAPTYPE_INTERLEAVE	(0x8)

#define MAC_MD_LATMODE_SHIFT		(4)	/* Latency mode */
#define MAC_MD_LATMODE_MASK		(0x7)

#define MAC_MD_BURST_WRITE		(0x0)
#define MAC_MD_SINGLE_WRITE		(0x200)


/*=====================*
 *  Type Defines       *
 *=====================*/

/* This must match the definitions above! */
typedef struct macRegs_t 
{
        volatile uint32 sdramRefrCtrl;  /* 0x00 */
	volatile uint32 reserved1[4];
	volatile uint32 romConfig;	/* 0x14 */
	volatile uint32 reserved2[3];
	volatile uint32 sramConfig;	/* 0x24 */
	volatile uint32 reserved3[2];
	volatile uint32 sdramCtrl;	/* 0x30 */
	volatile uint32 sdramConfig;	/* 0x34 */
	volatile uint32 sdramMode;	/* 0x38 */
	volatile uint32 reserved4;
	volatile uint32 sdram2Ctrl;	/* 0x40 */
	volatile uint32 sdram2Config;	/* 0x44 */
	volatile uint32 sdram2Mode;	/* 0x48 */
	volatile uint32 reserved5[17];
	volatile uint32 writeStat;	/* 0x90 */
	volatile uint32 postwriteEn;	/* 0x94 */
} macRegs;

typedef enum macBootMemWidth_t 
{
	BootMem8,
	BootMem16,
	BootMem32
} macBootMemWidth;


/*=====================*
 *  External Variables *
 *=====================*/

/*=====================*
 *  External Functions *
 *=====================*/


/*=====================*
 *  Macro Functions    *
 *=====================*/

#define MacIsRomWidth8(macPtr)	((((macPtr)->romConfig >> MAC_WIDTH_SHIFT) & MAC_WIDTH_MASK) == MAC_WIDTH_8)
#define MacIsRomWidth16(macPtr)	((((macPtr)->romConfig >> MAC_WIDTH_SHIFT) & MAC_WIDTH_MASK) == MAC_WIDTH_16)
#define MacIsRomWidth32(macPtr)	((((macPtr)->romConfig >> MAC_WIDTH_SHIFT) & MAC_WIDTH_MASK) == MAC_WIDTH_32)

#define MacIsSramWidth8(macPtr)	((((macPtr)->sramConfig >> MAC_WIDTH_SHIFT) & MAC_WIDTH_MASK) == MAC_WIDTH_8)
#define MacIsSramWidth16(macPtr)	((((macPtr)->sramConfig >> MAC_WIDTH_SHIFT) & MAC_WIDTH_MASK) == MAC_WIDTH_16)
#define MacIsSramWidth32(macPtr)	((((macPtr)->sramConfig >> MAC_WIDTH_SHIFT) & MAC_WIDTH_MASK) == MAC_WIDTH_32)

#define MacIsSdramWidth16(macPtr)	((((macPtr)->sdramConfig >> MAC_SDRAM_WIDTH_SHIFT) & MAC_SDRAM_WIDTH_MASK) == MAC_SDRAM_WIDTH_16)
#define MacIsSdramWidth32(macPtr)	((((macPtr)->sdramConfig >> MAC_SDRAM_WIDTH_SHIFT) & MAC_SDRAM_WIDTH_MASK) == MAC_SDRAM_WIDTH_32)

#define MacIsSdram2Width16(macPtr)	((((macPtr)->sdram2Config >> MAC_SDRAM_WIDTH_SHIFT) & MAC_SDRAM_WIDTH_MASK) == MAC_SDRAM_WIDTH_16)
#define MacIsSdram2Width32(macPtr)	((((macPtr)->sdram2Config >> MAC_SDRAM_WIDTH_SHIFT) & MAC_SDRAM_WIDTH_MASK) == MAC_SDRAM_WIDTH_32)



#endif /* MAC_H */
