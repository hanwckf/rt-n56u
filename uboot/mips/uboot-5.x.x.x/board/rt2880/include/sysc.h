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
*  File Name: sysc.h
*     Author: Linda Yang
*
*******************************************************************************
*
* Revision History:
*
*      Date    Name  Comments
*    --------  ---   ------------------------------------
*    11/08/00  LYT   Created
*
*******************************************************************************/

/* FILE_DESC ******************************************************************/
//
// Purpose:
//    This file contains the register addresses and the bit definitions for 
//    all the system and processor control related registers. 
//
// Sp. Notes:
//
/******************************************************************************/

#ifndef SYSC_H
#define SYSC_H

/*=====================*
 *  Include Files      *
 *=====================*/
//#include "pubdefs.h"
#include "product.h"
#include "mem_map.h"


/*=====================*
 *  Defines            *
 *=====================*/
/* Define register offsets which are needed by assembly code */
#define CPU_CONFIG_REG          (SYSC_BASE + 0x0010)
#define TEST_RESULT_REG         (SYSC_BASE + 0x0018)
#define TEST_RESULT2_REG        (SYSC_BASE + 0x001C)
#define RESET_STATUS_REG        (SYSC_BASE + 0x0038)
#define DLL_CONFIG_REG          (SYSC_BASE + 0x004C)

/* Define of value for TEST_RESULT2_REG to indicate NMI exception */
#define TEST_RESULT2_NMI	(0xC0000001)


/* Chip Name 0 & 1 register bit definitions */
#define CHIPNAME0               (0x70617247)   /* "Grap" */
#define CHIPNAME1               (0x65746968)   /* "hite" */


/* Chip ID register bit definitions */
#define CHIP_REV_MASK           (0xFFFF)
#define CHIP_REV_SHIFT          (0)
#define CHIP_NUM_MASK           (0xFFFF)
#define CHIP_NUM_SHIFT          (16)

#define CHIP_REV                (0x0000)
#define CHIP_NUM                (0x0000)
#define CHIP_ID                 ((CHIP_NUM << CHIP_NUM_SHIFT) | (CHIP_REV << CHIP_REV_SHIFT))

/* CPU Config register bit definitions */
/* CPU Error Status register bit definitions */
#define ALL_REMAP_BITS_MASK	(0x001F)
#define REMAP_ROM               (0x0001)
#define REMAP_VECTMEM		(0x0002)	/* May not be implemented */
#define REMAP_SDRAM2VEC		(0x0004)	/* May not be implemented */
#define REMAP_SDRAM2ROM		(0x0008)	/* May not be implemented */
#define REMAP_SRAM    		(0x0010)	/* May not be implemented */
#define ALL_CPU_ERRORS          (0x0F80)
#define CPU_DMA_ERROR           (0x0080)
#define CPU_DATA_ADDR_ERROR     (0x0100)
#define CPU_INST_ADDR_ERROR     (0x0200)
#define CPU_DATA_ALIGN_ERROR    (0x0400)
#define CPU_INST_ALIGN_ERROR    (0x0800)
#define PERIPH_8_AND_16_WR_ENA  (0x1000)        /* Palmbus 8/16 bit writes */
#define LCD_MUX_ENA             (0x2000)        /* Enables muxed LCD signals */
#define POST_WRITE_DIS          (0x4000)        /* Palmbus postwrite disable */

#define CPU_DATA_ERRORS         (CPU_DATA_ADDR_ERROR | CPU_DATA_ALIGN_ERROR)
#define CPU_INST_ERRORS         (CPU_INST_ADDR_ERROR | CPU_INST_ALIGN_ERROR)

/* Bad Data Access Info register bit definitions */
#define BAD_DATA_INFO_MASK      (0x7F)
#define BAD_DATA_BYTESEL_MASK   (0x78)
#define BAD_DATA_BYTESEL_SHIFT  (3)
#define BAD_DATA_DIR_MASK       (0x04)
#define BAD_DATA_SIZE_MASK      (0x03)

#define BAD_DATA_READ           (0x00)
#define BAD_DATA_WRITE          (0x04)

#define BAD_DATA_TYPE_MASK      (0x07)	/* DIR and SIZE */
#define BAD_BYTE_READ           (0x00)
#define BAD_WORD_READ           (0x01)
#define BAD_DWORD_READ          (0x02)
#define BAD_BYTE_WRITE          (0x04)
#define BAD_WORD_WRITE          (0x05)
#define BAD_DWORD_WRITE         (0x06)

/* Bad Instruction Access Info register bit definitions */
#define BAD_INST_INFO_MASK      (0x3F)
#define BAD_INST_BYTESEL_MASK   (0x3C)
#define BAD_INST_BYTESEL_SHIFT  (2)
#define BAD_INST_SIZE_MASK      (0x03)

#define BAD_BYTE_INST           (0x00)
#define BAD_WORD_INST           (0x01)
#define BAD_DWORD_INST          (0x02)

/* Power Management, Clock Config register bit definitions */
#define SYS_SLEEP_ENABLE        (0x1)
#define CPU_SLEEP_ENABLE        (0x2)

/* Reset Control register bit definitions */
#define RESET_SYSC              (0x00000001)
#define RESET_TMR               (0x00000002)
#define RESET_INTC              (0x00000004)
#define RESET_MAC               (0x00000008)
#define RESET_CPU               (0x00000010)
#define RESET_UART              (0x00000020)
#define RESET_PIO               (0x00000040)
#define RESET_DMA               (0x00000080)
#define RESET_LCD               (0x00000100)
#define RESET_I2C               (0x00000200)
#define RESET_RTC               (0x00000400)
#define RESET_SPI               (0x00000800)
#define RESET_UARTLITE          (0x00001000)
#define RESET_AUX0              (0x00010000)
#define RESET_AUX1              (0x00020000)
#define RESET_AUX2              (0x00040000)
#define RESET_AUX3              (0x00080000)
#define RESET_AUX4              (0x00100000)
#define RESET_AUX5              (0x00200000)
#define RESET_AUX6              (0x00400000)
#define RESET_AUX7              (0x00800000)
#define RESET_AUX8              (0x01000000)
#define RESET_AUX9              (0x02000000)
#define RESET_AUX10             (0x04000000)

/* Reset Status register bit definitions */
#define POR_RESET_STATUS        (0)
#define ALL_RESETS              (0xF)
#define EXT_PIN_RESET           (0x1)
#define WATCHDOG_RESET          (0x2)
#define SW_SYSTEM_RESET         (0x4)
#define SW_CPU_RESET            (0x8)

/* PLL Control (pllCtrl) Register bit definitions */
#define PLL_BYPASS			(0x00000001)
#define PLL_ENABLE			(0x00000002)
#define PLL_RESET			(0x00000004)

/* PLL Configuration (pllCfg) Register bit definitions */
#define PLL_REFCLK_MDIV_MASK		(0xFF)
#define PLL_REFCLK_MDIV_SHIFT		(0)
#define PLL_FEEDBACK_NDIV_MASK		(0xFF)
#define PLL_FEEDBACK_NDIV_SHIFT		(8)

/* PLL Status (pllStat) Register bit definitions */
#define PLL_LOCKED			(0x00000001)

/* DLL Configuration (dllCfg) Register bit definitions */
#define DLL_PHASE_MASK			(0x3)
#define DLL_PHASE_SHIFT			(0)
#define DLL_INVERT_SYSCLK		(0x00000004)
#define DLL_ENABLE			(0x00000008)

#define DLL_PHASE_0			(0x0)
#define DLL_PHASE_90			(0x1)
#define DLL_PHASE_180			(0x2)
#define DLL_PHASE_270			(0x3)


/* DLL Status (dllStat) Register bit definitions */
#define DLL_LOCKED			(0x00000001)

#endif /* SYSC_H */
