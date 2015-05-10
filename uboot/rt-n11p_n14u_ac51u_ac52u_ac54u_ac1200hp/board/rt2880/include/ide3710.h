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
*  File Name: ide3710.h
*     Author: Jenny Tong
*
*******************************************************************************
*
* Revision History:
*
*      Date    Name  Comments
*    --------  ---   ------------------------------------
*    05/07/01  JCT   Created
*
*******************************************************************************/
/* FILE_DESC ******************************************************************/
//
// Purpose:
//    This file contains all the Hardware Abstraction Layer (HAL) functions,
//    definitions, and macros for the Ide3710 Controller. 
//
// Sp. Notes:
//    The following must be defined in chip_reg_map.h and intc.h according to 
//    which AUX block and the IDE block is connected:
//
//        IDE3710            Test Class code (ex. AUX1)
//
//        IDE3710_BASE       Address base for IDE (ex. AUX1_BASE)    
//
//        ide3710Int         IDE's Interrupt enumerated bit 
//                            position (see intc.h)
//                            (ex. aux1Int)   
//
//        INTC_IDE3710_INT   IDE's Interrupt Bit position in
//                            Interrupt Controller's registers (see intc.h)
//                            (ex. INTC_AUX1_INT)
//
//    The Task File Data Register is defined by the IDE Spec as 16-bits wide.
//    However, all the addresses of all Task File registers are 8-bit offset,
//    therefore this register is defined as a uint8.
//
//    Further, Intel IDE Controllers resolve 32-bit reads/writes to the Task 
//    File Data Register as 2 16-bit reads/writes.
//
//    By default, slew control registers are not implemented in hardware and
//    should be treated as reserved registers.
//
/******************************************************************************/

#ifndef IDE3710_H
#define IDE3710_H

/*=====================*
 *  Include Files      *
 *=====================*/
#include "product.h"
#include "pubdefs.h"
#include "timer.h"


/*=====================*
 *  Defines            *
 *=====================*/
/* Failure codes */
/* 0x01 t0 0x0F reserved for use within test files src/*.c */
#define IDE_ERR_BAD_LBA_MODE                   (0x10)

#define IDE_ERR_CMDDONE_STAT_DEV_ERROR         (0x20)
#define IDE_ERR_CMDDONE_STAT_DEV_FAULT         (0x21)
#define IDE_ERR_CMDDONE_STAT_OTHER_ERROR       (0x22)

#define IDE_ERR_IDENTDEV_NO_DRDY               (0x30)
#define IDE_ERR_IDENTDEV_CMDWAIT_TIMED_OUT     (0x31)
#define IDE_ERR_IDENTDEV_BAD_PRE_STATUS        (0x32)

#define IDE_ERR_SETFEAT_XFERMODE_NO_DRDY       (0x40)
#define IDE_ERR_SETMULT_NO_DRDY                (0x41)
 
#define IDE_ERR_PIOWAIT_TIMED_OUT              (0x50)

#define IDE_ERR_PIO_INVALID_CMD                (0x60)
#define IDE_ERR_PIO_CANNOT_SELECT_DRIVE        (0x61)
#define IDE_ERR_PIO_BAD_PRE_STATUS             (0x62)
#define IDE_ERR_PIO_BAD_POST_STATUS            (0x63)
#define IDE_ERR_PIO_BAD_FINAL_STATUS           (0x64)

#define IDE_ERR_DMASETUP_ODD_ADDR              (0x70)
#define IDE_ERR_DMASETUP_ODD_COUNT             (0x71)
    
#define IDE_ERR_DMAWAIT_BM_XFER_ERROR          (0x81)
#define IDE_ERR_DMAWAIT_INT_TIMED_OUT          (0x82)
#define IDE_ERR_DMAWAIT_POLL_TIMED_OUT         (0x83)

#define IDE_ERR_DMA_INVALID_CMD                (0x90)
#define IDE_ERR_DMA_CANNOT_SELECT_DRIVE        (0x91)
#define IDE_ERR_DMA_CMDWAIT_TIMED_OUT          (0x92)
#define IDE_ERR_DMA_BAD_FINAL_STATUS           (0x94)
#define IDE_ERR_DMA_BAD_BM_STATUS              (0x95)

#define IDE_ERR_ABTCHK_TIMED_OUT               (0xA0)
#define IDE_ERR_ABTCHK_DEV_ERROR               (0xA1)
#define IDE_ERR_ABTCHK_DEV_FAULT               (0xA2)

#define IDE_ERR_ABTDMA_INVALID_CMD             (0xB0)
#define IDE_ERR_ABTDMA_CANNOT_SELECT_DRIVE     (0xB1)
#define IDE_ERR_ABTDMA_CMDDONE_TIMED_OUT       (0xB2)
#define IDE_ERR_ABTDMA_BAD_FINAL_STATUS        (0xB3)
#define IDE_ERR_ABTDMA_BAD_BM_STATUS           (0xB4)

#define IDE_ERR_IDE0_HARD_RESET_PROC           (0xC0)
#define IDE_ERR_IDE1_HARD_RESET_PROC           (0xC1)


/* IDE Block Specific Defines */
#define IDE_BM_OFFSET                       (0x00)
#define IDE_CFG_REG_OFFSET                  (0x40)
#define IDE_PRI_TASKFILE_OFFSET             (0x1F0)
#define IDE_SEC_TASKFILE_OFFSET             (0x170)

#define IdeBmBase(ideBase)                  (ideBase + IDE_BM_OFFSET)
#define IdeCfgRegBase(ideBase)              (ideBase + IDE_CFG_REG_OFFSET)
/* IdeTaskFileBase check for ideIf.  Default = primary */
#define IdeTaskFileBase(ideBase, ideIf)                                       \
    (((ideIf) == IDE_IF_PRIMARY) ? ((ideBase) + IDE_PRI_TASKFILE_OFFSET) :    \
     ((ideIf) == IDE_IF_SECONDARY) ? ((ideBase) + IDE_SEC_TASKFILE_OFFSET) :  \
                                     ((ideBase) + IDE_PRI_TASKFILE_OFFSET))
#define IdePriTaskFileBase(ideBase)         (ideBase + IDE_PRI_TASKFILE_OFFSET)
#define IdeSecTaskFileBase(ideBase)         (ideBase + IDE_SEC_TASKFILE_OFFSET)



/*
** IDE Bus Master Control Registers
*/

/* IDE Bus Master Command Register (ideBMICP, ideBMICS) bit definitions */
#define IDE_DMA_XFER_START                  (0x0001) /* start transfer */
#define IDE_DMA_BM_WRITE                    (0x0008) /* write to memory 
                                                        (ata read) */

/* IDE Bus Master Status Register (ideBMISP, ideBMISS) bit definitions */
#define IDE_BM_SR_MASK_ERR                  (0x02)  /* error */
#define IDE_BM_SR_MASK_INT                  (0x04)  /* INTRQ signal asserted */

/* IDE Bus Master Descriptor Pointer Register (ideBMIDTPP_LO, ideBMIDTPP_HI,
** ideBMIDTPS_LO, ideBMIDTPS_HI) bit definitions
*/
#define IDE_BMIDTP_MASK                     (0xFFFF)
#define IDE_BMIDTP_HI_SHIFT                 (16)


/*
** IDE Registers
*/

/* IDE Primary/Secondary Timing Register (priIdeTimLo, priIdeTimHi, secIdeTimLo,
** secIdeTimHi) bit definitions
*/
/* Lower 8-bit register */
#define IDE_TIM_0_PIO_FAST_ENABLE           (0x01)
#define IDE_TIM_0_IORDY_ENABLE              (0x02)
#define IDE_TIM_0_PIO_PREPOST_ENABLE        (0x04)
#define IDE_TIM_0_DMA_FAST_ENABLE           (0x08)
#define IDE_TIM_1_PIO_FAST_ENABLE           (0x10)
#define IDE_TIM_1_IORDY_ENABLE              (0x20)
#define IDE_TIM_1_PIO_PREPOST_ENABLE        (0x40)
#define IDE_TIM_1_DMA_FAST_ENABLE           (0x80)
/* Upper 8-bit register */
#define IDE_TIM_RDY_RECOVERY_MASK           (0x3)
#define IDE_TIM_RDY_RECOVERY_SHIFT          (0)
#define IDE_TIM_RDY_SAMPLE_MASK             (0x3)
#define IDE_TIM_RDY_SAMPLE_SHIFT            (4)
#define IDE_TIM_SLAVE_TIM_ENABLE            (0x40)
#define IDE_TIM_DECODE_ENABLE               (0x80)

/* IDE Slave Timing Register (sIdeTim) bit definitions */
#define IDE_SLTIM_PRI_RDY_RECOVERY_MASK     (0x3)
#define IDE_SLTIM_PRI_RDY_RECOVERY_SHIFT    (0)
#define IDE_SLTIM_PRI_RDY_SAMPLE_MASK       (0x3)
#define IDE_SLTIM_PRI_RDY_SAMPLE_SHIFT      (2)
#define IDE_SLTIM_SEC_RDY_RECOVERY_MASK     (0x3)
#define IDE_SLTIM_SEC_RDY_RECOVERY_SHIFT    (4)
#define IDE_SLTIM_SEC_RDY_SAMPLE_MASK       (0x3)
#define IDE_SLTIM_SEC_RDY_SAMPLE_SHIFT      (6)

/* Specific values for RDY_RECOVERY and RDY_SAMPLE based on IDE mode */
#define IDE_REC_PIO_MODE1                   (0x0)
#define IDE_SAM_PIO_MODE1                   (0x0)
#define IDE_REC_PIO_MODE2                   (0x1)
#define IDE_SAM_PIO_MODE2                   (0x1)
#define IDE_REC_PIO_MODE3                   (0x2)
#define IDE_SAM_PIO_MODE3                   (0x2)
#define IDE_REC_PIO_MODE4                   (0x3)
#define IDE_SAM_PIO_MODE4                   (0x3)

#define IDE_REC_MDMA_MODE0                  (0x0)
#define IDE_SAM_MDMA_MODE0                  (0x0)
#define IDE_REC_MDMA_MODE1                  (0x1)
#define IDE_SAM_MDMA_MODE1                  (0x1)
#define IDE_REC_MDMA_MODE2                  (0x2)
#define IDE_SAM_MDMA_MODE2                  (0x2)

/* IDE Slew Rate Control Register (ideSlewLo, ideSlewHi) bit definitions */
#define IDE_SLEW_MASK                       (0x00FF)
#define IDE_SLEW_HI_SHIFT                   (8)
 
/* IDE Ultra DMA Control Register (udmaCtl) bit definitions */
#define IDE_PRI_0_UDMA_ENABLE               (0x1)
#define IDE_PRI_1_UDMA_ENABLE               (0x2)
#define IDE_SEC_0_UDMA_ENABLE               (0x4)
#define IDE_SEC_1_UDMA_ENABLE               (0x8)

/* IDE Ultra DMA Timing Register (udmaTimLo, udmaTimHi) bit definitions */
#define IDE_UDMA_TIM_MASK                   (0x00FF)
#define IDE_UDMA_TIM_HI_SHIFT               (8)
#define IDE_PRI_0_TMODE_MASK                (0x0007)
#define IDE_PRI_0_TMODE_SHIFT               (0)
#define IDE_PRI_1_TMODE_MASK                (0x0007)
#define IDE_PRI_1_TMODE_SHIFT               (4)

#define IDE_UTIM_0_MODE_MASK                (0x7)
#define IDE_UTIM_0_MODE_SHIFT               (0)
#define IDE_UTIM_1_MODE_MASK                (0x7)
#define IDE_UTIM_1_MODE_SHIFT               (4)

/* IDE UDMA Timing Mode */
#define IDE_UDMA_MODE0                      (0)    /* slowest mode */
#define IDE_UDMA_MODE1                      (1)
#define IDE_UDMA_MODE2                      (2)
#define IDE_UDMA_MODE3                      (3)
#define IDE_UDMA_MODE4                      (4)
#define IDE_UDMA_MODE5                      (5)
#define IDE_UDMA_MODE6                      (6)    /* fastest mode */
#define IDE_UDMA_DEFAULT_MODE               (IDE_UDMA_MODE0)

/* IDE Timing Override Control Register (timOverride) bit definitions */
#define IDE_TIMORIDE_OVERRIDE_ENA           (0x1)

/* IDE 8-bit Reg Access Strobe Width Register (regStbWidth) bit definitions */
#define IDE_REGSTB_0_WIDTH_MASK             (0x7F)
#define IDE_REGSTB_0_WIDTH_SHIFT            (0)
#define IDE_REGSTB_1_WIDTH_MASK             (0x7F)
#define IDE_REGSTB_1_WIDTH_SHIFT            (8)
#define IDE_REGSTB_DEF_WIDTH_MASK           (0x7F)
#define IDE_REGSTB_DEF_WIDTH_SHIFT          (16)

/* IDE 8-bit Reg Access Recovery Time Register (regRcvrTim) bit definitions */
#define IDE_REGRCVR_0_TIM_MASK              (0x7F)
#define IDE_REGRCVR_0_TIM_SHIFT             (0)
#define IDE_REGRCVR_1_TIM_MASK              (0x7F)
#define IDE_REGRCVR_1_TIM_SHIFT             (8)
#define IDE_REGRCVR_DEF_TIM_MASK            (0x7F)
#define IDE_REGRCVR_DEF_TIM_SHIFT           (16)

/* IDE Data Reg Access Strobe Width Register (dataStbWidth) bit definitions */
#define IDE_DATASTB_0_WIDTH_MASK            (0x7F)
#define IDE_DATASTB_0_WIDTH_SHIFT           (0)
#define IDE_DATASTB_1_WIDTH_MASK            (0x7F)
#define IDE_DATASTB_1_WIDTH_SHIFT           (8)

/* IDE Data Reg Access Recovery Time Register (dataRcvrTim) bit definitions */
#define IDE_DATARCVR_0_WIDTH_MASK           (0x7F)
#define IDE_DATARCVR_0_WIDTH_SHIFT          (0)
#define IDE_DATARCVR_1_WIDTH_MASK           (0x7F)
#define IDE_DATARCVR_1_WIDTH_SHIFT          (8)

/* IDE DMA Access Strobe Width Register (dmaStbWidth) bit definitions */
#define IDE_DMASTB_0_WIDTH_MASK             (0x7F)
#define IDE_DMASTB_0_WIDTH_SHIFT            (0)
#define IDE_DMASTB_1_WIDTH_MASK             (0x7F)
#define IDE_DMASTB_1_WIDTH_SHIFT            (8)

/* IDE DMA Access Recovery Time Register (dmaRcvrTim) bit definitions */
#define IDE_DMARCVR_0_WIDTH_MASK            (0x7F)
#define IDE_DMARCVR_0_WIDTH_SHIFT           (0)
#define IDE_DMARCVR_1_WIDTH_MASK            (0x7F)
#define IDE_DMARCVR_1_WIDTH_SHIFT           (8)

/* IDE UDMA Access Strobe Width Register (udmaStbWidth) bit definitions */
#define IDE_UDMASTB_0_WIDTH_MASK            (0xF)
#define IDE_UDMASTB_0_WIDTH_SHIFT           (0)
#define IDE_UDMASTB_1_WIDTH_MASK            (0xF)
#define IDE_UDMASTB_1_WIDTH_SHIFT           (8)

/* IDE UDMA Access Recovery Time Register (udmaRTPTim) bit definitions */
#define IDE_UDMARTP_0_WIDTH_MASK            (0x1F)
#define IDE_UDMARTP_0_WIDTH_SHIFT           (0)
#define IDE_UDMARTP_1_WIDTH_MASK            (0x1F)
#define IDE_UDMARTP_1_WIDTH_SHIFT           (8)

/* IDE Pin Control Register (pinCtrl) bit definitions */
#define IDE_PINCTRL_RESETPIN_ENA            (0x2)
#define IDE_PINCTRL_ASSERT_RESET            (0x1)


/* IDE interface */
#define IDE_IF_PRIMARY                      (1)
#define IDE_IF_SECONDARY                    (2)

/* IDE Device transfer Device Head parameter */
#define IDE_DEV_MASTER                      (0) 
#define IDE_DEV_SLAVE                       (1) 

/* IDE Device transfer Feature parameter */
#define IDE_NO_FEATURE                      (0)

/* DMA Flush Cache parameter */
#define BCOUNT                              (2048)


/*
** Task File Registers
*/

/* Device Control (altStatDevCtrl) Register bit definitions */
#define IDE_CB_DC_IEN                       (0x00)      /* enable interrupt */
#define IDE_CB_DC_NIEN                      (0x02)      /* disable interrupt */
#define IDE_CB_DC_SRST                      (0x04)      /* soft reset */
#define IDE_CB_DC_HD15                      (0x08)      /* bit should always 
                                                           be set to one */
/* Device/Head (devHead) Register bit definitions */
#define IDE_CB_DH_LBA                       (0x40)      /* select device 0 */
#define IDE_CB_DH_DEV0                      (0xa0)      /* select device 0 */
#define IDE_CB_DH_DEV1                      (0xb0)      /* select device 1 */

/* Status (statCmd, altStatDevCtrl) Register bit definitions */
#define IDE_CB_STAT_ERR                     (0x01)      /* error (ATA) */
#define IDE_CB_STAT_DRQ                     (0x08)      /* data request */
#define IDE_CB_STAT_DF                      (0x20)      /* device fault */
#define IDE_CB_STAT_DRDY                    (0x40)      /* device ready */
#define IDE_CB_STAT_BSY                     (0x80)      /* busy */

/* IDE Task File Device Commands (statCmd) */
#define IDE_CMD_DEVICE_RESET                (0x08)
#define IDE_CMD_READ_SECT                   (0x20)
#define IDE_CMD_WRITE_SECT                  (0x30)
#define IDE_CMD_EXE_DEV_DIAG                (0x90)
#define IDE_CMD_READ_MULT                   (0xC4)
#define IDE_CMD_WRITE_MULT                  (0xC5)
#define IDE_CMD_READ_DMA                    (0xC8)
#define IDE_CMD_WRITE_DMA                   (0xCA)
#define IDE_CMD_CFA_WRITE_MULT_NO_ERASE     (0xCD)
#define IDE_CMD_READ_LONG                   (0x22)
#define IDE_CMD_WRITE_LONG                  (0x32)
#define IDE_CMD_READ_BUFFER                 (0xE4)
#define IDE_CMD_WRITE_BUFFER                (0xE8)
#define IDE_CMD_ID_DEV                      (0xEC)
#define IDE_CMD_SET_MULT_MODE               (0xC6)
#define IDE_CMD_SET_FEATURES                (0xEF)
#define IDE_CMD_READ_SECT_EXT               (0x24)
#define IDE_CMD_READ_MULT_EXT               (0x29)
#define IDE_CMD_READ_DMA_EXT                (0x25)
#define IDE_CMD_WRITE_SECT_EXT              (0x34)
#define IDE_CMD_WRITE_MULT_EXT              (0x39)
#define IDE_CMD_WRITE_DMA_EXT               (0x35)
#define IDE_CMD_READ_VERIFY_EXT             (0x42)


/*
** IDE Task File Subcommands for IDE_CMD_SET_FEATURES command
*/

/* IDE_CMD_SET_FEATURES subCommand values for Features (errFeat) register */
#define IDE_SETFEAT_SET_XFERMODE_VIA_SECTCNT (0x03)

/* IDE_CMD_SET_FEATURES subCommand values for Sector Count (sectCnt) register.
** bits [7:3] = transfer type.  bits [2:0] = mode value.
*/
/* PIO Transfer Modes */
/* transfer type 1:  PIO flow control transfer mode */
#define IDE_SETFEAT_XFERMODE_PIO_MODE0      (0x08)
#define IDE_SETFEAT_XFERMODE_PIO_MODE1      (0x09)
#define IDE_SETFEAT_XFERMODE_PIO_MODE2      (0x0A)
#define IDE_SETFEAT_XFERMODE_PIO_MODE3      (0x0B)
#define IDE_SETFEAT_XFERMODE_PIO_MODE4      (0x0C)
/* DMA Transfer Modes */
/* transfer type 4:  Multiword DMA mode */
#define IDE_SETFEAT_XFERMODE_MDMA_MODE0     (0x20)
#define IDE_SETFEAT_XFERMODE_MDMA_MODE1     (0x21)
#define IDE_SETFEAT_XFERMODE_MDMA_MODE2     (0x22)
/* UDMA Transfer Modes */
/* transfer type 8:  Ultra DMA mode */
#define IDE_SETFEAT_XFERMODE_UDMA_MODE0     (0x40)
#define IDE_SETFEAT_XFERMODE_UDMA_MODE1     (0x41)
#define IDE_SETFEAT_XFERMODE_UDMA_MODE2     (0x42)
#define IDE_SETFEAT_XFERMODE_UDMA_MODE3     (0x43)
#define IDE_SETFEAT_XFERMODE_UDMA_MODE4     (0x44)
#define IDE_SETFEAT_XFERMODE_UDMA_MODE5     (0x45)
#define IDE_SETFEAT_XFERMODE_UDMA_MODE6     (0x46)


/* Configuration data for device 0 and 1 */
#define IDE_REG_CONFIG_TYPE_ATAPI           (3)


/* Physical Region Descriptor bit defintions */
#define IDE_PRD_ADDR_SHIFT                  (16)
#define IDE_PRD_ADDR_MASK                   (0xFFFF)
#define IDE_PRD_EOT                         (0x80000000)
#define IDE_PRD_BYTE_CNT_SHIFT              (0)
#define IDE_PRD_BYTE_CNT_MASK               (0xFFFF)
#define IDE_PRD_MAX_SIZE                    (0x00010000)   /* 64K Maximum */


/* Info Structure definitions */
#define IDE_CYL_HI_SHIFT                    (8)
#define IDE_CYL_HI_MASK                     (0x00FF)
#define IDE_CYL_LO_SHIFT                    (0)
#define IDE_CYL_LO_MASK                     (0x00FF)
#define IDE_DEV_HEAD_SHIFT                  (0)
#define IDE_DEV_HEAD_MASK                   (0x004F)


/* Defines for IDENTIFY DEVICE */
#define ID_DEVICE_INFO_WORDS                (256)     /* No. words in ID info */
#define ID_MULT_SUPPORT_WORD                (47)
#define ID_MULT_SUPPORT_MAXSECT_MASK        (0xFF)       /* bits 7-0 */
#define ID_CAPABILITIES_WORD                (49)               
#define ID_CAPABILITIES_LBA_SUPPORTED       (0x0100)     /* bit 8 */
#define ID_CAPABILITIES_DMA_SUPPORTED       (0x0200)     /* bit 9 */
#define ID_FIELDVALID_WORD                  (53)
#define ID_FIELDVALID_PIODMA_VALID          (0x0002)     /* bit 1 */
#define ID_FIELDVALID_UDMA_VALID            (0x0004)     /* bit 2 */
#define ID_MULTISECT_WORD                   (59)
#define ID_MULTISECT_SELECTED               (0x0100)     /* bit 8 */
#define ID_MULTISECT_CURR_SECTCNT_MASK      (0xFF)       /* bits 7-0 */
#define ID_MAX_LBA28_WORD1                  (60)
#define ID_MAX_LBA28_WORD2                  (61)
#define ID_MAX_LBA28_VAL                    (0x0FFFFFFF) /* 268,435,455 */
#define ID_MDMA_WORD                        (63)
#define ID_MDMA_MODE0_SUPPORTED             (0x0001)     /* bit 0 */
#define ID_MDMA_MODE1_SUPPORTED             (0x0002)     /* bit 1 */
#define ID_MDMA_MODE2_SUPPORTED             (0x0004)     /* bit 2 */
#define ID_MDMA_MODE0_SELECTED              (0x0100)     /* bit 8 */
#define ID_MDMA_MODE1_SELECTED              (0x0200)     /* bit 9 */
#define ID_MDMA_MODE2_SELECTED              (0x0400)     /* bit 10 */
#define ID_PIO_WORD                         (64)
#define ID_PIO_MODE3_SUPPORTED              (0x0001)     /* bit 0 */
#define ID_PIO_MODE4_SUPPORTED              (0x0002)     /* bit 1 */
#define ID_FEATCMD_WORD2                    (83)
#define ID_FEATCMD_LBA48_SUPPORTED          (0x0400)     /* bit 10 */
#define ID_UDMA_WORD                        (88)
#define ID_UDMA_MODE0_SUPPORTED             (0x0001)     /* bit 0 */
#define ID_UDMA_MODE1_SUPPORTED             (0x0002)     /* bit 1 */
#define ID_UDMA_MODE2_SUPPORTED             (0x0004)     /* bit 2 */
#define ID_UDMA_MODE3_SUPPORTED             (0x0008)     /* bit 3 */
#define ID_UDMA_MODE4_SUPPORTED             (0x0010)     /* bit 4 */
#define ID_UDMA_MODE5_SUPPORTED             (0x0020)     /* bit 5 */
#define ID_UDMA_MODE6_SUPPORTED             (0x0040)     /* bit 6 */
#define ID_UDMA_MODE0_SELECTED              (0x0100)     /* bit 8 */
#define ID_UDMA_MODE1_SELECTED              (0x0200)     /* bit 9 */
#define ID_UDMA_MODE2_SELECTED              (0x0400)     /* bit 10 */
#define ID_UDMA_MODE3_SELECTED              (0x0800)     /* bit 11 */
#define ID_UDMA_MODE4_SELECTED              (0x1000)     /* bit 12 */
#define ID_UDMA_MODE5_SELECTED              (0x2000)     /* bit 13 */
#define ID_UDMA_MODE6_SELECTED              (0x4000)     /* bit 14 */
#define ID_MAX_LBA48_WORD1                  (100)
#define ID_MAX_LBA48_WORD2                  (101)
#define ID_MAX_LBA48_WORD3                  (102)
#define ID_MAX_LBA48_WORD4                  (103)
#define ID_MAX_LBA48_VAL_HI                 (0x0000FFFF)
#define ID_MAX_LBA48_VAL_LO                 (0xFFFFFFFF)


/*=====================*
 *  Type Defines       *
 *=====================*/

typedef struct ideRegs_t
{
    volatile uint8 priIdeTimLo;           /* 0x40 */
    volatile uint8 priIdeTimHi;           /* 0x41 */
    volatile uint8 secIdeTimLo;           /* 0x42 */
    volatile uint8 secIdeTimHi;           /* 0x43 */
    volatile uint8 sIdeTim;               /* 0x44 */ 
    volatile uint8 ideSlewLo;             /* 0x45 */
    volatile uint8 ideSlewHi;             /* 0x46 */
    volatile uint8 ideStatus;             /* 0x47 */
    volatile uint8 udmaCtl;               /* 0x48 */
    volatile uint8 reserved;              /* 0x49 */
    volatile uint8 udmaTimLo;             /* 0x4A */
    volatile uint8 udmaTimHi;             /* 0x4B */
    volatile uint32 reserved1;            /* 0x4C */
    volatile uint32 timOverride;          /* 0x50 */
    volatile uint32 regStbWidth;          /* 0x54 */
    volatile uint32 regRcvrTim;           /* 0x58 */
    volatile uint32 dataStbWidth;         /* 0x5C */
    volatile uint32 dataRcvrTim;          /* 0x60 */
    volatile uint32 dmaStbWidth;          /* 0x64 */
    volatile uint32 dmaRcvrTim;           /* 0x68 */
    volatile uint32 udmaStbWidth;         /* 0x6C */
    volatile uint32 udmaRTPTim;           /* 0x70 */
    volatile uint32 reserved2[3];         /* 0x74-0x7C */
    volatile uint32 pioDmaEna;            /* 0x80 */
    volatile uint8 pinCtrl;               /* 0x84 */
} ideRegs;
 

/* Structure of Bus Master Interface for each channel */ 
typedef struct ideBM_t
{
    volatile uint16 ideBMICP;             /* 0x00 */
    volatile uint16 ideBMISP;             /* 0x02 */
    volatile uint16 ideBMIDTPP_LO;        /* 0x04 */
    volatile uint16 ideBMIDTPP_HI;        /* 0x06 */
    volatile uint16 ideBMICS;             /* 0x08 */
    volatile uint16 ideBMISS;             /* 0x0A */
    volatile uint16 ideBMIDTPS_LO;        /* 0x0C */
    volatile uint16 ideBMIDTPS_HI;        /* 0x0E */
    volatile uint32 ideDmaPtr;            /* 0x10 - for SATA only */
    volatile uint32 ideDmaLen;            /* 0x14 - for SATA only */
    volatile uint32 ideDmaState;          /* 0x18 - for SATA only */
    volatile uint32 ideSectSize;          /* 0x1C - for SATA only */
} ideBM;

/* Task File registers */
typedef struct ideTaskFile_t
{
    volatile uint8 data;                  /* 0x0000 */ /* See Sp. Note Above */
    volatile uint8 errFeat;               /* 0x0001 */
    volatile uint8 sectCnt;               /* 0x0002 */
    volatile uint8 sectNum;               /* 0x0003 */ /* aka LbaLo */
    volatile uint8 cylLo;                 /* 0x0004 */ /* aka LbaMid */
    volatile uint8 cylHi;                 /* 0x0005 */ /* aka LbaHi */
    volatile uint8 devHead;               /* 0x0006 */ /* aka Device */
    volatile uint8 statCmd;               /* 0x0007 */
    volatile uint8 reserved[510];
    volatile uint8 altStatDevCtrl;        /* 0x0206 */
    volatile uint8 devAddr;               /* 0x0207 */

} ideTaskFile;

/* IDE PIO and DMA Timing Modes */
typedef enum ideTimingMode_t
{
    /* IDE PIO Timing Mode */
    IDE_PIO_MODE0 = 0,
    IDE_PIO_MODE1,
    IDE_PIO_MODE2,
    IDE_PIO_MODE3,
    IDE_PIO_MODE4,
    /* IDE MDMA Timing Mode */
    IDE_MDMA_MODE0,    /* NOTE: IdeSetTimMode() assumes MODES less than this */
    IDE_MDMA_MODE1,    /*       value are PIO modes, and greater or equal to */
    IDE_MDMA_MODE2,    /*       this value are DMA modes.                    */
    IDE_DEFAULT_MODE = IDE_PIO_MODE0
} ideTimingMode;


/* Structure for variables needed by the IDE timeout timer.
** Values are initialized by IdeTimerSetup function, which should be called
** before using the IDE timer.
*/
typedef struct ideTimer_t
{
   tmrBlkRegs *ideTmrPtr;
   uint32 ideTmrNum;
   uint32 ideTmrFreqkHz;
   uint32 cmdTimeout;
} ideTimer;

typedef enum ideLbaMode_t
{
  ideChs,
  ideLba28,
  ideLba48
} ideLbaMode;

typedef struct ideXferInfo_t
{
    ideTimer tmrInfo;                     /* IDE timer info */
    uint32 sectSize;                      /* Sector size in words */
    uint32 blkSize;                       /* Number of sectors per block */
    uint32 ideIf;                         /* IDE interface */
    uint32 dev;                           /* device */
    uint32 cmd;                           /* command */
    uint32 sc;                            /* sector count */
    ideLbaMode lbaMode;                   /* LBA mode */
    uint32 lbaLo;                         /* LBA sector addressing mode */
    uint32 lbaHi;                         /* LBA sector addressing mode */
    uint32 sect;                          /* Sector number */
    uint32 cylLo;                         /* Cylinder Low */
    uint32 cylHi;                         /* Cylinder High */
    uint32 head;                          /* Device head */
    uint32 extSect;                       /* Extended LBA - Sector number */
    uint32 extCylLo;                      /* Extended LBA - Cylinder Low */
    uint32 extCylHi;                      /* Extended LBA - Cylinder High */
    uint32 extHead;                       /* Extended LBA - Device head */
    uint32 *addr;                         /* Address pointer to data array */
    uint32 vendCnt;                       /* No. of vendor bytes (long xfers) */
} ideXferInfo;


/*=====================*
 *  Global Variables   *
 *=====================*/
 

/*=====================*
 *  External Variables *
 *=====================*/


/*=====================*
 *  External Functions *
 *=====================*/
uint32 IdeHardResetProc (void);

void IdeOverrideTimeTo33 ( ideRegs *idePtr );

uint32 IdeAbortDma ( uint32 ideBase, ideXferInfo *xferInfo, uint32 *prdBufPtr,
                     uint32 *ideIntIntrFlag, uint32 ideIntUseIntrFlag );

uint32 IdeDmaDataXfer ( uint32 ideBase, ideXferInfo *xferInfo, 
                        uint32 *prdBufPtr, uint32 *ideIntIntrFlag, 
                        uint32 ideIntUseIntrFlag );

uint32 IdePioDataXfer ( uint32 ideBase, ideXferInfo *xferInfo,
                        uint32 *ideIntIntrFlag, uint32 ideIntUseIntrFlag );

uint32 IdeIdentifyDevice ( ideTaskFile *taskFilePtr, uint32 ideDev,
                           ideTimer *ideTmr,
                           uint32 *ideIntIntrFlag, uint32 ideIntUseIntrFlag,
                           uint16 *buf );

uint32 IdeSetFeaturesXferMode ( ideTaskFile *taskFilePtr, uint32 ideDev,
                                uint32 mode );
uint32 IdeSetMultipleMode ( ideTaskFile *taskFilePtr, uint32 ideDev,
                            uint32 blkSize );

void IdeSetTimMode ( ideRegs *idePtr, uint32 ideIf,
                    ideTimingMode dev0mode, ideTimingMode dev1mode );
void IdeSetUdmaTimMode ( ideRegs *idePtr, uint32 ideIf, uint32 dev0mode,
                         uint32 dev1mode );

void IdeTimerSetup ( ideTimer *ideTmr, tmrBlkRegs *tmrBlkPtr,
                     uint32 timerNum );


/*=====================*
 *  Macro Functions    *
 *=====================*/


/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeIsPrimary()
//
// SYNOPSIS       void IdeIsPrimary ( uint32 ideIf )
//
// TYPE           Macro
//
// INPUT          uint32 ideIf: Ide interface
//                              (IDE_IF_PRIMARY, IDE_IF_SECONDARY, ...)
//
// OUTPUT         TRUE/FALSE:  TRUE if ideIf is set to value IDE_IF_PRIMARY
//
// DESCRIPTION    Returns TRUE if the value passed in indicates the Primary
//                IDE Interface.
//
// NOTE           This macro allows src/.c test code to compare variables to
//                the value IDE_IF_PRIMARY without using the "IDE_IF_PRIMARY"
//                define directly.
//                The release_setup script may replace any occurence of the text
//                "IDE_IF_PRIMARY" in src/.c files with the text 
//                "IDE_IF_SECONDARY".
//
/******************************************************************************/
#define IdeIsPrimary(ideIf)                    ((ideIf) == IDE_IF_PRIMARY)

 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeClearDmaStatus()
//
// SYNOPSIS       void IdeClearDmaStatus ( ideBM *bmPtr, uint32 ideIf )
//
// TYPE           Macro
//
// INPUT          ideBM *bmPtr: Base Pointer of the IDE Bus Master Registers
//                uint32 ideIf: Ide interface
//                              (IDE_IF_PRIMARY, IDE_IF_SECONDARY, ...)
//
// OUTPUT         None
//
// DESCRIPTION    Clear the DMA Status register.
//
// NOTE           None
//
/******************************************************************************/
#define IdeClearDmaStatus(bmPtr, ideIf)                                       \
do                                                                            \
{                                                                             \
    if ((ideIf) == IDE_IF_SECONDARY)                                          \
        (bmPtr)->ideBMISS |= (IDE_BM_SR_MASK_INT | IDE_BM_SR_MASK_ERR);       \
    else                                                                      \
        (bmPtr)->ideBMISP |= (IDE_BM_SR_MASK_INT | IDE_BM_SR_MASK_ERR);       \
} while (0)

 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeEnablePri0PioFastTim()
//
// SYNOPSIS       IdeEnablePri0PioFastTim ( ideRegs *idePtr, uint32 ideIf,
//                                          uint32 ideDev )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//                uint32 ideIf: Ide interface
//                              (IDE_IF_PRIMARY, IDE_IF_SECONDARY, ...)
//                uint32 ideDev: Device number on the ide interface
//                               (IDE_DEV_MASTER (drive0) or
//                               IDE_DEV_SLAVE (drive1)
//
// OUTPUT         None
//
// DESCRIPTION    Enable the IDE Pio Fast Timing for the device
//
// NOTE           None
//
/******************************************************************************/
#define IdeEnablePioFastTim(idePtr, ideIf, ideDev)                            \
do                                                                            \
{                                                                             \
    uint8 bit;                                                                \
                                                                              \
    if ((ideDev) == IDE_DEV_SLAVE)        bit = IDE_TIM_1_PIO_FAST_ENABLE;    \
    else  /* default = IDE_DEV_MASTER */  bit = IDE_TIM_0_PIO_FAST_ENABLE;    \
    if ((ideIf) == IDE_IF_SECONDARY)      (idePtr)->secIdeTimLo |= bit;       \
    else  /* default = IDE_IF_PRIMARY */  (idePtr)->priIdeTimLo |= bit;       \
} while (0)
 

/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeEnablePri0Iordy()
//
// SYNOPSIS       IdeEnablePri0Iordy ( ideRegs *idePtr, uint32 ideIf,
//                                     uint32 ideDev )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//                uint32 ideIf: Ide interface
//                              (IDE_IF_PRIMARY, IDE_IF_SECONDARY, ...)
//                uint32 ideDev: Device number on the ide interface
//                               (IDE_DEV_MASTER (drive0) or
//                               IDE_DEV_SLAVE (drive1)
//
// OUTPUT         None
//
// DESCRIPTION    Enable the IDE IORDY Sample Point for the device
//
// NOTE           None
//
/******************************************************************************/
#define IdeEnableIordy(idePtr, ideIf, ideDev)                                 \
do                                                                            \
{                                                                             \
    uint8 bit;                                                                \
                                                                              \
    if ((ideDev) == IDE_DEV_SLAVE)        bit = IDE_TIM_1_IORDY_ENABLE;       \
    else  /* default = IDE_DEV_MASTER */  bit = IDE_TIM_0_IORDY_ENABLE;       \
    if ((ideIf) == IDE_IF_SECONDARY)      (idePtr)->secIdeTimLo |= bit;       \
    else  /* default = IDE_IF_PRIMARY */  (idePtr)->priIdeTimLo |= bit;       \
} while (0)
 

/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeEnablePreFetch()
//
// SYNOPSIS       IdeEnablePreFetch ( ideRegs *idePtr, uint32 ideIf,
//                                    uint32 ideDev )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//                uint32 ideIf: Ide interface
//                              (IDE_IF_PRIMARY, IDE_IF_SECONDARY, ...)
//                uint32 ideDev: Device number on the ide interface
//                               (IDE_DEV_MASTER (drive0) or
//                               IDE_DEV_SLAVE (drive1)
//
// OUTPUT         None
//
// DESCRIPTION    Enable the IDE PreFetch and PostWrite for the device
//
// NOTE           None
//
/******************************************************************************/
#define IdeEnablePreFetch(idePtr, ideIf, ideDev)                              \
do                                                                            \
{                                                                             \
    uint8 bit;                                                                \
                                                                              \
    if ((ideDev) == IDE_DEV_SLAVE)        bit = IDE_TIM_1_PIO_PREPOST_ENABLE; \
    else  /* default = IDE_DEV_MASTER */  bit = IDE_TIM_0_PIO_PREPOST_ENABLE; \
    if ((ideIf) == IDE_IF_SECONDARY)      (idePtr)->secIdeTimLo |= bit;       \
    else  /* default = IDE_IF_PRIMARY */  (idePtr)->priIdeTimLo |= bit;       \
}                                                                             \
while (0)
 

/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeEnableDmaFastTim()
//
// SYNOPSIS       IdeEnableDmaFastTim ( ideRegs *idePtr, uint32 ideIf,
//                                      uint32 ideDev )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//                uint32 ideIf: Ide interface
//                              (IDE_IF_PRIMARY, IDE_IF_SECONDARY, ...)
//                uint32 ideDev: Device number on the ide interface
//                               (IDE_DEV_MASTER (drive0) or
//                               IDE_DEV_SLAVE (drive1)
//
// OUTPUT         None
//
// DESCRIPTION    Enable the IDE Dma Fast Timing for the device
//
// NOTE           None
//
/******************************************************************************/
#define IdeEnableDmaFastTim(idePtr, ideIf, ideDev)                            \
do                                                                            \
{                                                                             \
    uint8 bit;                                                                \
                                                                              \
    if ((ideDev) == IDE_DEV_SLAVE)        bit = IDE_TIM_0_DMA_FAST_ENABLE;    \
    else  /* default = IDE_DEV_MASTER */  bit = IDE_TIM_1_DMA_FAST_ENABLE;    \
    if ((ideIf) == IDE_IF_SECONDARY)      (idePtr)->secIdeTimLo |= bit;       \
    else  /* default = IDE_IF_PRIMARY */  (idePtr)->priIdeTimLo |= bit;       \
} while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeGetRcvryTim()
//
// SYNOPSIS       uint8 IdeGetRcvryTim ( ideRegs *idePtr, uint32 ideIf,
//                                       uint32 ideDev )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//                uint32 ideIf: Ide interface
//                              (IDE_IF_PRIMARY, IDE_IF_SECONDARY, ...)
//                uint32 ideDev: Device number on the ide interface
//                               (IDE_DEV_MASTER (drive0) or
//                               IDE_DEV_SLAVE (drive1)
//
// OUTPUT         uint8: 2-bit value of the IDE IORDY recovery time
//
// DESCRIPTION    Returns the value of the IDE IORDY recovery time of the
//                device.
//
// NOTE           Check specifications for equivalent value in nanoseconds.
//
/******************************************************************************/
#define IdeGetRcvryTim(idePtr, ideIf, ideDev)                                 \
    (((ideIf) == IDE_IF_SECONDARY) ?                                          \
        (((ideDev) == IDE_IF_SLAVE) ?                                         \
            IdeGetSec1RcvryTim(idePtr) : IdeGetSec0RcvryTim(idePtr))          \
      : (((ideDev) == IDE_IF_SLAVE) ?                                         \
            IdeGetPri1RcvryTim(idePtr) : IdeGetPri0RcvryTim(idePtr)))

/* Macros used by the above macro */
#define IdeGetPri0RcvryTim(idePtr)                                            \
    (((idePtr)->priIdeTimHi >> IDE_TIM_RDY_RECOVERY_SHIFT)                    \
     & IDE_TIM_RDY_RECOVERY_MASK)
#define IdeGetPri1RcvryTim(idePtr)                                            \
    (((idePtr)->sIdeTim >> IDE_SLTIM_PRI_RDY_RECOVERY_SHIFT)                  \
     & IDE_SLTIM_PRI_RDY_RECOVERY_MASK)
#define IdeGetSec0RcvryTim(idePtr)                                            \
    (((idePtr)->secIdeTimHi >> IDE_TIM_RDY_RECOVERY_SHIFT)                    \
     & IDE_TIM_RDY_RECOVERY_MASK)
#define IdeGetSec1RcvryTim(idePtr)                                            \
    (((idePtr)->sIdeTim >> IDE_SLTIM_SEC_RDY_RECOVERY_SHIFT)                  \
     & IDE_SLTIM_SEC_RDY_RECOVERY_MASK)


/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeLoadRcvryTim()
//
// SYNOPSIS       IdeLoadRcvryTim ( ideRegs *idePtr, uint32 ideIf, uint32 ideDev
//                                  uint8 val )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//                uint32 ideIf: Ide interface
//                              (IDE_IF_PRIMARY, IDE_IF_SECONDARY, ...)
//                uint32 ideDev: Device number on the ide interface
//                               (IDE_DEV_MASTER (drive0) or
//                               IDE_DEV_SLAVE (drive1)
//                uint8 val: 2-bit value of the IDE IORDY recovery time
//
// OUTPUT         None
//
// DESCRIPTION    Loads the value of the IDE IORDY recovery time of the device.
//
// NOTE           Check specifications for equivalent value in nanoseconds.
//
/******************************************************************************/
#define IdeLoadRcvryTim(idePtr, ideIf, ideDev, val)                           \
do                                                                            \
{                                                                             \
    if ((ideIf) == IDE_IF_SECONDARY)                                          \
    {                                                                         \
        if ((ideDev) == IDE_DEV_SLAVE)                                        \
            IdeLoadSec1RcvryTim((idePtr), (val));                             \
        else /* default = IDE_DEV_MASTER */                                   \
            IdeLoadSec0RcvryTim((idePtr), (val));                             \
    }                                                                         \
    else /* default = IDE_IF_PRIMARY */                                       \
    {                                                                         \
        if ((ideDev) == IDE_DEV_SLAVE)                                        \
            IdeLoadPri1RcvryTim((idePtr), (val));                             \
        else /* default = IDE_DEV_MASTER */                                   \
            IdeLoadPri0RcvryTim((idePtr), (val));                             \
    }                                                                         \
}                                                                             \
while (0)

/* Macros used by the above macro */
#define IdeLoadPri0RcvryTim(idePtr, val)                                      \
do                                                                            \
{                                                                             \
    (idePtr)->priIdeTimHi = ((idePtr)->priIdeTimHi                            \
                             & ~(IDE_TIM_RDY_RECOVERY_MASK                    \
                                 << IDE_TIM_RDY_RECOVERY_SHIFT))              \
                            | (((val) & IDE_TIM_RDY_RECOVERY_MASK)            \
                               << IDE_TIM_RDY_RECOVERY_SHIFT);                \
} while (0)
#define IdeLoadPri1RcvryTim(idePtr, val)                                      \
do                                                                            \
{                                                                             \
    (idePtr)->sIdeTim = ((idePtr)->sIdeTim                                    \
                         & ~(IDE_SLTIM_PRI_RDY_RECOVERY_MASK                  \
                             << IDE_SLTIM_PRI_RDY_RECOVERY_SHIFT))            \
                        | (((val) & IDE_SLTIM_PRI_RDY_RECOVERY_MASK)          \
                           << IDE_SLTIM_PRI_RDY_RECOVERY_SHIFT);              \
} while (0)
#define IdeLoadSec0RcvryTim(idePtr, val)                                      \
do                                                                            \
{                                                                             \
    (idePtr)->secIdeTimHi = ((idePtr)->secIdeTimHi                            \
                             & ~(IDE_TIM_RDY_RECOVERY_MASK                    \
                                 << IDE_TIM_RDY_RECOVERY_SHIFT))              \
                            | (((val) & IDE_TIM_RDY_RECOVERY_MASK)            \
                               << IDE_TIM_RDY_RECOVERY_SHIFT);                \
} while (0)
#define IdeLoadSec1RcvryTim(idePtr, val)                                      \
do                                                                            \
{                                                                             \
    (idePtr)->sIdeTim = ((idePtr)->sIdeTim                                    \
                         & ~(IDE_SLTIM_SEC_RDY_RECOVERY_MASK                  \
                             << IDE_SLTIM_SEC_RDY_RECOVERY_SHIFT))            \
                        | (((val) & IDE_SLTIM_SEC_RDY_RECOVERY_MASK)          \
                           << IDE_SLTIM_SEC_RDY_RECOVERY_SHIFT);              \
} while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeGetIordySp()
//
// SYNOPSIS       uint8 IdeGetIordySp ( ideRegs *idePtr, uint32 ideIf,
//                                      uint32 ideDev)
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//                uint32 ideIf: Ide interface
//                              (IDE_IF_PRIMARY, IDE_IF_SECONDARY, ...)
//                uint32 ideDev: Device number on the ide interface
//                               (IDE_DEV_MASTER (drive0) or
//                               IDE_DEV_SLAVE (drive1)
//
// OUTPUT         uint8: 2-bit value of the IDE IORDY sample point
//
// DESCRIPTION    Returns the value of the IDE IORDY sample point of the device.
//
// NOTE           Check specifications for equivalent value in nanoseconds.
//
/******************************************************************************/
#define IdeGetIordySp(idePtr, ideIf, ideDev)                                  \
    (((ideIf) == IDE_IF_SECONDARY) ?                                          \
        (((ideDev) == IDE_IF_SLAVE) ?                                         \
            IdeGetSec1IordySp(idePtr) : IdeGetSec0IordySp(idePtr))            \
      : (((ideDev) == IDE_IF_SLAVE) ?                                         \
            IdeGetPri1IordySp(idePtr) : IdeGetPri0IordySp(idePtr)))

/* Macros used by the above macro */
#define IdeGetPri0IordySp(idePtr)                                             \
    (((idePtr)->priIdeTimHi >> IDE_TIM_RDY_SAMPLE_SHIFT)                      \
     & IDE_TIM_RDY_SAMPLE_MASK)
#define IdeGetPri1IordySp(idePtr)                                             \
    (((idePtr)->sIdeTim >> IDE_SLTIM_PRI_RDY_SAMPLE_SHIFT)                    \
     & IDE_SLTIM_PRI_RDY_SAMPLE_MASK)
#define IdeGetSec0IordySp(idePtr)                                             \
    (((idePtr)->secIdeTimHi >> IDE_TIM_RDY_SAMPLE_SHIFT)                      \
     & IDE_TIM_RDY_SAMPLE_MASK)
#define IdeGetSec1IordySp(idePtr)                                             \
    (((idePtr)->sIdeTim >> IDE_SLTIM_SEC_RDY_SAMPLE_SHIFT)                    \
     & IDE_SLTIM_SEC_RDY_SAMPLE_MASK)


/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeLoadIordySp()
//
// SYNOPSIS       IdeLoadIordySp ( ideRegs *idePtr, uint32 ideIf, uint32 ideDev,
//                                 uint8 val )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//                uint32 ideIf: Ide interface
//                              (IDE_IF_PRIMARY, IDE_IF_SECONDARY, ...)
//                uint32 ideDev: Device number on the ide interface
//                               (IDE_DEV_MASTER (drive0) or
//                               IDE_DEV_SLAVE (drive1)
//                uint8 val: 2-bit value of the IDE IORDY sample point
//
// OUTPUT         None
//
// DESCRIPTION    Loads the value of the IDE IORDY sample point of the device.
//
// NOTE           Check specifications for equivalent value in nanoseconds.
//
/******************************************************************************/
#define IdeLoadIordySp(idePtr, ideIf, ideDev, val)                            \
do                                                                            \
{                                                                             \
    if ((ideIf) == IDE_IF_SECONDARY)                                          \
    {                                                                         \
        if ((ideDev) == IDE_DEV_SLAVE)                                        \
            IdeLoadSec1IordySp((idePtr), (val));                              \
        else /* default = IDE_DEV_MASTER */                                   \
            IdeLoadSec0IordySp((idePtr), (val));                              \
    }                                                                         \
    else /* default = IDE_IF_PRIMARY */                                       \
    {                                                                         \
        if ((ideDev) == IDE_DEV_SLAVE)                                        \
            IdeLoadPri1IordySp((idePtr), (val));                              \
        else /* default = IDE_DEV_MASTER */                                   \
            IdeLoadPri0IordySp((idePtr), (val));                              \
    }                                                                         \
}                                                                             \
while (0)

/* Macros used by the above macro */
#define IdeLoadPri0IordySp(idePtr, val)                                       \
do                                                                            \
{                                                                             \
    (idePtr)->priIdeTimHi = ((idePtr)->priIdeTimHi                            \
                             & ~(IDE_TIM_RDY_SAMPLE_MASK                      \
                                 << IDE_TIM_RDY_SAMPLE_SHIFT))                \
                            | (((val) & IDE_TIM_RDY_SAMPLE_MASK)              \
                               << IDE_TIM_RDY_SAMPLE_SHIFT);                  \
} while (0)
#define IdeLoadPri1IordySp(idePtr, val)                                       \
do                                                                            \
{                                                                             \
    (idePtr)->sIdeTim = ((idePtr)->sIdeTim                                    \
                         & ~(IDE_SLTIM_PRI_RDY_SAMPLE_MASK                    \
                             << IDE_SLTIM_PRI_RDY_SAMPLE_SHIFT))              \
                        | (((val) & IDE_SLTIM_PRI_RDY_SAMPLE_MASK)            \
                           << IDE_SLTIM_PRI_RDY_SAMPLE_SHIFT);                \
} while (0)
#define IdeLoadSec0IordySp(idePtr, val)                                       \
do                                                                            \
{                                                                             \
    (idePtr)->secIdeTimHi = ((idePtr)->secIdeTimHi                            \
                             & ~(IDE_TIM_RDY_SAMPLE_MASK                      \
                                 << IDE_TIM_RDY_SAMPLE_SHIFT))                \
                            | (((val) & IDE_TIM_RDY_SAMPLE_MASK)              \
                               << IDE_TIM_RDY_SAMPLE_SHIFT);                  \
} while (0)
#define IdeLoadSec1IordySp(idePtr, val)                                       \
do                                                                            \
{                                                                             \
    (idePtr)->sIdeTim = ((idePtr)->sIdeTim                                    \
                         & ~(IDE_SLTIM_SEC_RDY_SAMPLE_MASK                    \
                             << IDE_SLTIM_SEC_RDY_SAMPLE_SHIFT))              \
                        | (((val) & IDE_SLTIM_SEC_RDY_SAMPLE_MASK)            \
                           << IDE_SLTIM_SEC_RDY_SAMPLE_SHIFT);                \
} while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeGetIdeTim()
//
// SYNOPSIS       uint16 IdeGetIdeTim ( ideRegs *idePtr, uint32 ideIf )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//                uint32 ideIf: Ide interface
//                              (IDE_IF_PRIMARY, IDE_IF_SECONDARY, ...)
//
// OUTPUT         uint16: The 16 bits value of the IDE Timing Register
//
// DESCRIPTION    Returns the 16 bits value of the relevant IDE Timing Register
//                The 16-bit ideTim register is split into two 8-bit registers.
//
// NOTE           None
//
/******************************************************************************/
#define IdeGetIdeTim(idePtr, ideIf)                                           \
    (((ideIf) == IDE_IF_SECONDARY) ?                                          \
                ((idePtr)->secIdeTimLo | ((idePtr)->secIdeTimHi << 8))        \
              : ((idePtr)->priIdeTimLo | ((idePtr)->priIdeTimHi << 8))
 

/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeLoadIdeTim()
//
// SYNOPSIS       IdeLoadIdeTim ( ideRegs *idePtr, uint32 ideIf, uint16 val )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//                uint32 ideIf: Ide interface
//                              (IDE_IF_PRIMARY, IDE_IF_SECONDARY, ...)
//                uint16 val: 16 bits value of IDE Timing Register
//
// OUTPUT         None
//
// DESCRIPTION    Loading the 16 bit value into 2 8-bits relevant IDE Timing
//                Registers
//
// NOTE           None
//
/******************************************************************************/
#define IdeLoadIdeTim(idePtr, ideIf, val)                                     \
do                                                                            \
{                                                                             \
    if ((ideIf) == IDE_IF_SECONDARY)                                          \
    {                                                                         \
        (idePtr)->secIdeTimLo = (val) & 0xFF;                                 \
        (idePtr)->secIdeTimHi = ((val) & 0xFF00) >> 8;                        \
    }                                                                         \
    else /* default = primary */                                              \
    {                                                                         \
        (idePtr)->priIdeTimLo = (val) & 0xFF;                                 \
        (idePtr)->priIdeTimHi = ((val) & 0xFF00) >> 8;                        \
    }                                                                         \
} while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeEnableSlaveTim()
//
// SYNOPSIS       IdeEnableSlaveTim ( ideRegs *idePtr, uint32 ideIf )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//                uint32 ideIf: Ide interface
//                              (IDE_IF_PRIMARY, IDE_IF_SECONDARY, ...)
//
// OUTPUT         None
//
// DESCRIPTION    Enable the IDE Slave Timing for both IDE devices (master and
//                slave)
//
// NOTE           None
//
/******************************************************************************/
#define IdeEnableSlaveTim(idePtr, ideIf)                                      \
do                                                                            \
{                                                                             \
    if ((ideIf) == IDE_IF_SECONDARY)                                          \
        (idePtr)->secIdeTimHi |= IDE_TIM_SLAVE_TIM_ENABLE;                    \
    else /* default = primary */                                              \
        (idePtr)->priIdeTimHi |= IDE_TIM_SLAVE_TIM_ENABLE;                    \
} while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeEnableDecode()
//
// SYNOPSIS       IdeEnableDecode ( ideRegs *idePtr, uint32 ideIf )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//                uint32 ideIf: Ide interface
//                              (IDE_IF_PRIMARY, IDE_IF_SECONDARY, ...)
//
// OUTPUT         None
//
// DESCRIPTION    Enable the IDE Decode for both IDE devices (master and slave)
//
// NOTE           None
//
/******************************************************************************/
#define IdeEnableDecode(idePtr, ideIf)                                        \
do                                                                            \
{                                                                             \
    if ((ideIf) == IDE_IF_SECONDARY)                                          \
        (idePtr)->secIdeTimHi |= IDE_TIM_DECODE_ENABLE;                       \
    else /* default = primary */                                              \
        (idePtr)->priIdeTimHi |= IDE_TIM_DECODE_ENABLE;                       \
} while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeDisableDecode()
//
// SYNOPSIS       IdeDisableDecode ( ideRegs *idePtr, uint32 ideIf )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//                uint32 ideIf: Ide interface
//                              (IDE_IF_PRIMARY, IDE_IF_SECONDARY, ...)
//
// OUTPUT         None
//
// DESCRIPTION    Disable the IDE Decode for both IDE devices (master and slave)
//
// NOTE           None
//
/******************************************************************************/
#define IdeDisableDecode(idePtr, ideIf)                                       \
do                                                                            \
{                                                                             \
    if ((ideIf) == IDE_IF_SECONDARY)                                          \
        (idePtr)->secIdeTimHi &= ~IDE_TIM_DECODE_ENABLE;                      \
    else /* default = primary */                                              \
        (idePtr)->priIdeTimHi &= ~IDE_TIM_DECODE_ENABLE;                      \
} while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeIsDecodeEnabled()
//
// SYNOPSIS       IdeIsDecodeEnabled ( ideRegs *idePtr, uint32 ideIf )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//                uint32 ideIf: Ide interface
//                              (IDE_IF_PRIMARY, IDE_IF_SECONDARY, ...)
//
// OUTPUT         TRUE/FALSE:  TRUE if decode is enabled
//
// DESCRIPTION    Checks if the IDE Decode for the relevant IDE Interface (both
//                master and slave devices) are enabled
//
// NOTE           None
//
/******************************************************************************/
#define IdeIsDecodeEnabled(idePtr, ideIf)                                     \
    (((ideIf) == IDE_IF_SECONDARY) ?                                          \
                ((bool)((idePtr)->secIdeTimHi & IDE_TIM_DECODE_ENABLE))       \
              : ((bool)((idePtr)->priIdeTimHi & IDE_TIM_DECODE_ENABLE)))


/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeGetSIdeTim()
//
// SYNOPSIS       uint8 IdeGetSIdeTim ( ideRegs *idePtr )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//
// OUTPUT         uint8: value of the IDE Slave IDE Timing Register
//
// DESCRIPTION    Returns the value of the IDE Slave IDE Timing Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeGetSIdeTim(idePtr)                                                 \
    ((idePtr)->sIdeTim)

 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeLoadSIdeTim()
//
// SYNOPSIS       IdeLoadSIdeTim ( ideRegs *idePtr, uint8 val )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//                uint8 val: IDE Slave IDE Timing Register value
//
// OUTPUT         None
//
// DESCRIPTION    Load the value into the IDE Slave IDE Timing Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeLoadSIdeTim(idePtr, val)                                           \
do                                                                            \
{                                                                             \
    (idePtr)->sIdeTim = (val);                                                \
} while (0)
 
 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeGetIdeSlewLo()
//
// SYNOPSIS       uint8 IdeGetIdeSlewLo ( ideRegs *idePtr )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//
// OUTPUT         uint8: The lower 8 bits value of IDE Slew Rate Control
//                       Register
//
// DESCRIPTION    Returns the value of the lower 8 bits of IDE Slew Rate Control
//                Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeGetIdeSlewLo(idePtr)                                               \
    ((idePtr)->ideSlewLo)
 
 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeLoadIdeSlewLo()
//
// SYNOPSIS       IdeLoadIdeSlewLo ( ideRegs *idePtr, uint8 val )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//                uint8 val: Lower 8 bits of IDE Slew Rate Control value
//
// OUTPUT         None
//
// DESCRIPTION    Load the value into the lower 8 bits of the IDE Slew Rate 
//                Control Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeLoadIdeSlewLo(idePtr, val)                                         \
do                                                                            \
{                                                                             \
    (idePtr)->ideSlewLo = (uint8)((val) & IDE_SLEW_MASK);                     \
} while (0)
 
 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeGetIdeSlewHi()
//
// SYNOPSIS       uint8 IdeGetIdeSlewHi ( ideRegs *idePtr )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//
// OUTPUT         uint8: The upper 8 bits value of IDE Slew Rate Control
//                       Register
//
// DESCRIPTION    Returns the value of the upper 8 bits of IDE Slew Rate Control
//                Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeGetIdeSlewHi(idePtr)                                               \
    ((idePtr)->ideSlewHi)

 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeLoadIdeSlewHi()
//
// SYNOPSIS       IdeLoadIdeSlewHi ( ideRegs *idePtr, uint8 val )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//                uint8 val: Upper 8 bits of IDE Slew Rate Control value
//
// OUTPUT         None
//
// DESCRIPTION    Load the value into the upper 8 bits of the IDE Slew Rate 
//                Control Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeLoadIdeSlewHi(idePtr, val)                                         \
do                                                                            \
{                                                                             \
    (idePtr)->ideSlewHi = (uint8)((val) & IDE_SLEW_MASK);                     \
} while (0)
 
 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeGetIdeSlew()
//
// SYNOPSIS       uint16 IdeGetIdeSlew ( ideRegs *idePtr )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//
// OUTPUT         uint16: Controller Register value
//
// DESCRIPTION    Returns Controller Register value.
//
// NOTE           None
//
/******************************************************************************/
#define IdeGetIdeSlew(idePtr)                                                 \
    (((IdeGetIdeSlewHi(idePtr) & IDE_SLEW_MASK) << IDE_SLEW_HI_SHIFT) |       \
      (IdeGetIdeSlewLo(idePtr) & IDE_SLEW_MASK))
 

 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeLoadIdeSlew()
//
// SYNOPSIS       IdeLoadIdeSlew ( ideRegs *idePtr, uint16 val )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//                uint16: 16 bits of IDE Slew Rate Control value
//
// OUTPUT         None
//
// DESCRIPTION    Load the 16 bits value into IDE Slew Rate Control Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeLoadIdeSlew(idePtr, val)                                           \
do                                                                            \
{                                                                             \
    IdeLoadIdeSlewLo(idePtr, (uint8)((val) & IDE_SLEW_MASK));                 \
    IdeLoadIdeSlewHi(idePtr, (uint8)(((val) >> IDE_SLEW_HI_SHIFT) &           \
                                  IDE_SLEW_MASK));                            \
} while (0)
 

/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeGetIdeStatus()
//
// SYNOPSIS       uint8 IdeGetIdeStatus ( ideRegs *idePtr )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//
// OUTPUT         uint8: The 8 bits value of IDE Status Register
//
// DESCRIPTION    Returns the 8 bits value of IDE Status Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeGetIdeStatus(idePtr)                                               \
    ((idePtr)->ideStatus)
 
 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeLoadIdeStatus()
//
// SYNOPSIS       IdeLoadIdeStatus ( ideRegs *idePtr, uint8 val )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//                uint8 val: 8 bits of IDE Status value
//
// OUTPUT         None
//
// DESCRIPTION    Loads the 8 bits value into IDE Status Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeLoadIdeStatus(idePtr,val)                                          \
do                                                                            \
{                                                                             \
    (idePtr)->ideStatus = (uint8)(val);                                       \
} while (0)
 

/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeGetUdmaCtl()
//
// SYNOPSIS       uint8 IdeGetUdmaCtl ( ideRegs *idePtr )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//
// OUTPUT         uint8: The 8 bits value of Ultra DMA Control Register
//
// DESCRIPTION    Returns the 8 bits value of Ultra DMA Control Register 
//
// NOTE           None
//
/******************************************************************************/
#define IdeGetUdmaCtl(idePtr)                                                 \
    ((idePtr)->udmaCtl)
 
 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeLoadUdmaCtl()
//
// SYNOPSIS       IdeLoadUdmaCtl ( ideRegs *idePtr, uint8 val )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//                uint8 val: 8 bits of Ultra DMA Control value
//
// OUTPUT         None
//
// DESCRIPTION    Loads the 8 bits value into Ultra DMA Control Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeLoadUdmaCtl(idePtr,val)                                            \
do                                                                            \
{                                                                             \
    (idePtr)->udmaCtl = (uint8)(val);                                         \
} while (0)
 

/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeEnableUdma()
//
// SYNOPSIS       IdeEnableUdma ( ideRegs *idePtr, uint32 ideIf, uint32 ideDev )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//                uint32 ideIf: Ide interface
//                              (IDE_IF_PRIMARY, IDE_IF_SECONDARY, ...)
//                uint32 ideDev: Device number on the ide interface
//                               (IDE_DEV_MASTER (drive0) or
//                               IDE_DEV_SLAVE (drive1)
//
// OUTPUT         None
//
// DESCRIPTION    Enable UDMA on the device
//
// NOTE           None
//
/******************************************************************************/
#define IdeEnableUdma(idePtr, ideIf, ideDev)                                  \
do                                                                            \
{                                                                             \
    if ((ideIf) == IDE_IF_SECONDARY)                                          \
        (idePtr)->udmaCtl |= (((ideDev) == IDE_DEV_SLAVE) ?                   \
                              IDE_SEC_1_UDMA_ENABLE : IDE_SEC_0_UDMA_ENABLE); \
    else /* default = IDE_IF_PRIMARY */                                       \
        (idePtr)->udmaCtl |= (((ideDev) == IDE_DEV_SLAVE) ?                   \
                              IDE_PRI_1_UDMA_ENABLE : IDE_PRI_0_UDMA_ENABLE); \
} while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeDisableUdma()
//
// SYNOPSIS       IdeDisableUdma ( ideRegs *idePtr, uint32 ideIf, uint32 ideDev )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//                uint32 ideIf: Ide interface
//                              (IDE_IF_PRIMARY, IDE_IF_SECONDARY, ...)
//                uint32 ideDev: Device number on the ide interface
//                               (IDE_DEV_MASTER (drive0) or
//                               IDE_DEV_SLAVE (drive1)
//
// OUTPUT         None
//
// DESCRIPTION    Enable UDMA on the device
//
// NOTE           None
//
/******************************************************************************/
#define IdeDisableUdma(idePtr, ideIf, ideDev)                                 \
do                                                                            \
{                                                                             \
    if ((ideIf) == IDE_IF_SECONDARY)                                          \
        (idePtr)->udmaCtl &= ~(((ideDev) == IDE_DEV_SLAVE) ?                  \
                              IDE_SEC_1_UDMA_ENABLE : IDE_SEC_0_UDMA_ENABLE); \
    else /* default = IDE_IF_PRIMARY */                                       \
        (idePtr)->udmaCtl &= ~(((ideDev) == IDE_DEV_SLAVE) ?                  \
                              IDE_PRI_1_UDMA_ENABLE : IDE_PRI_0_UDMA_ENABLE); \
} while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeGetUdmaTimLo()
//
// SYNOPSIS       uint8 IdeGetUdmaTimLo ( ideRegs *idePtr )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//
// OUTPUT         uint8: The lower 8 bits value of Ultra DMA Timing Register
//
// DESCRIPTION    Returns the value of the lower 8 bits of Ultra DMA Timing
//                Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeGetUdmaTimLo(idePtr)                                               \
    ((idePtr)->udmaTimLo)
 
 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeLoadUdmaTimLo()
//
// SYNOPSIS       IdeLoadUdmaTimLo ( ideRegs *idePtr, uint8 val )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//                uint8 val: Lower 8 bits of Ultra DMA Timing value
//
// OUTPUT         None
//
// DESCRIPTION    Load the value into the lower 8 bits of the Ultra DMA Timing
//                Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeLoadUdmaTimLo(idePtr, val)                                         \
do                                                                            \
{                                                                             \
    (idePtr)->udmaTimLo = (uint8)((val) & IDE_UDMA_TIM_MASK);                 \
} while (0)
 
 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeGetUdmaTimHi()
//
// SYNOPSIS       uint8 IdeGetUdmaTimHi ( ideRegs *idePtr )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//
// OUTPUT         uint8: The upper 8 bits value of Ultra DMA Timing Register
//
// DESCRIPTION    Returns the value of the upper 8 bits of Ultra DMA Timing 
//                Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeGetUdmaTimHi(idePtr)                                               \
    ((idePtr)->udmaTimHi)

 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeLoadUdmaTimHi()
//
// SYNOPSIS       IdeLoadUdmaTimHi ( ideRegs *idePtr, uint8 val )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//                uint8 val: Upper 8 bits of Ultra DMA Timing value
//
// OUTPUT         None
//
// DESCRIPTION    Load the value into the lower 8 bits of the Ultra DMA Timing
//                Register 
//
// NOTE           None
//
/******************************************************************************/
#define IdeLoadUdmaTimHi(idePtr, val)                                         \
do                                                                            \
{                                                                             \
    (idePtr)->udmaTimHi = (uint8)((val) & IDE_UDMA_TIM_MASK);                 \
} while (0)
 
 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeGetUdmaTim()
//
// SYNOPSIS       uint16 IdeGetUdmaTim ( ideRegs *idePtr )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//
// OUTPUT         uint16: The 16 bits value of Ultra DMA Timing Register
//
// DESCRIPTION    Returns the 16 bits value of the Ultra DMA Timing Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeGetUdmaTim(idePtr)                                                 \
   (((IdeGetUdmaTimHi(idePtr) & IDE_UDMA_TIM_MASK) << IDE_UDMA_TIM_HI_SHIFT) |\
     (IdeGetUdmaTimLo(idePtr) & IDE_UDMA_TIM_MASK))
 

 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeLoadUdmaTim()
//
// SYNOPSIS       IdeLoadUdmaTim ( ideRegs *idePtr, uint16 val )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//                uint16 val: 16 bits of Ultra DMA Timing value
//
// OUTPUT         None
//
// DESCRIPTION    Load the 16 bits value into 2 8 bits Ultra DMA Timing Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeLoadUdmaTim(idePtr, val)                                           \
do                                                                            \
{                                                                             \
    IdeLoadUdmaTimLo(idePtr, (uint8)((val) & IDE_UDMA_TIM_MASK));             \
    IdeLoadUdmaTimHi(idePtr, (uint8)(((val) >> IDE_UDMA_TIM_HI_SHIFT) &       \
                                  IDE_UDMA_TIM_MASK));                        \
} while (0)
 
 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeSetPri0UdmaTimMode()
//                IdeSetPri1UdmaTimMode()
//                IdeSetSec0UdmaTimMode()
//                IdeSetSec1UdmaTimMode()
//
// SYNOPSIS       IdeSetPri0UdmaTimMode ( ideRegs *idePtr, uint8 mode )
//                IdeSetPri1UdmaTimMode ( ideRegs *idePtr, uint8 mode )
//                IdeSetSec0UdmaTimMode ( ideRegs *idePtr, uint8 mode )
//                IdeSetSec1UdmaTimMode ( ideRegs *idePtr, uint8 mode )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//                uint8 mode: The speed of the cycle time.
//
// OUTPUT         None
//
// DESCRIPTION    Set the Udma timing mode for IDE device
//
// NOTE           None
//
/******************************************************************************/
#define IdeSetPri0UdmaTimMode(idePtr, mode)                                   \
do                                                                            \
{                                                                             \
    (idePtr)->udmaTimLo = ((idePtr)->udmaTimLo                                \
                             & ~(IDE_UTIM_0_MODE_MASK                         \
                                 << IDE_UTIM_0_MODE_SHIFT))                   \
                            | (((mode) & IDE_UTIM_0_MODE_MASK)                \
                               << IDE_UTIM_0_MODE_SHIFT);                     \
} while (0)
#define IdeSetPri1UdmaTimMode(idePtr, mode)                                   \
do                                                                            \
{                                                                             \
    (idePtr)->udmaTimLo = ((idePtr)->udmaTimLo                                \
                             & ~(IDE_UTIM_1_MODE_MASK                         \
                                 << IDE_UTIM_1_MODE_SHIFT))                   \
                            | (((mode) & IDE_UTIM_1_MODE_MASK)                \
                               << IDE_UTIM_1_MODE_SHIFT);                     \
} while (0)
#define IdeSetSec0UdmaTimMode(idePtr, mode)                                   \
do                                                                            \
{                                                                             \
    (idePtr)->udmaTimHi = ((idePtr)->udmaTimHi                                \
                             & ~(IDE_UTIM_0_MODE_MASK                         \
                                 << IDE_UTIM_0_MODE_SHIFT))                   \
                            | (((mode) & IDE_UTIM_0_MODE_MASK)                \
                               << IDE_UTIM_0_MODE_SHIFT);                     \
} while (0)
#define IdeSetSec1UdmaTimMode(idePtr, mode)                                   \
do                                                                            \
{                                                                             \
    (idePtr)->udmaTimHi = ((idePtr)->udmaTimHi                                \
                             & ~(IDE_UTIM_1_MODE_MASK                         \
                                 << IDE_UTIM_1_MODE_SHIFT))                   \
                            | (((mode) & IDE_UTIM_1_MODE_MASK)                \
                               << IDE_UTIM_1_MODE_SHIFT);                     \
} while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeGetBMICP()
//
// SYNOPSIS       uint8 IdeGetBMICP ( ideBM *bmPtr )
//
// TYPE           Macro
//
// INPUT          ideBM *bmPtr: Base Pointer of the IDE Bus Master Registers
//
// OUTPUT         uint8: The 8 bits value of Bus Master IDE Command Register
//
// DESCRIPTION    Returns the 8 bits value of the Bus Master IDE Command 
//                Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeGetBMICP(bmPtr)                                                    \
    ((bmPtr)->ideBMICP)
 
 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeLoadBMICP()
//
// SYNOPSIS       IdeLoadBMICP ( ideBM *bmPtr, uint16 val )
//
// TYPE           Macro
//
// INPUT          ideBM *bmPtr: Base Pointer of the IDE Bus Master Registers
//                uint16 val: 16 bits of Bus Master IDE Command value
//
// OUTPUT         None
//
// DESCRIPTION    Loads the 16 bits value into Bus Master IDE Command Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeLoadBMICP(bmPtr,val)                                               \
do                                                                            \
{                                                                             \
    (bmPtr)->ideBMICP = (uint16)(val);                                        \
} while (0)
 

 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeGetBMISP()
//
// SYNOPSIS       uint16 IdeGetBMISP ( ideBM *bmPtr )
//
// TYPE           Macro
//
// INPUT          ideBM *bmPtr: Base Pointer of the IDE Bus Master Registers
//
// OUTPUT         uint16: The 16 bits value of Bus Master IDE Status Register
//
// DESCRIPTION    Returns the 16 bits value of Bus Master IDE Status Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeGetBMISP(bmPtr)                                                    \
    ((bmPtr)->ideBMISP)
 
 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeLoadBMISP()
//
// SYNOPSIS       IdeLoadBMISP ( ideBM *bmPtr, uint16 val )
//
// TYPE           Macro
//
// INPUT          ideBM *bmPtr: Base Pointer of the IDE Bus Master Registers
//                uint16 val: 16 bits of Bus Master IDE Status value
//
// OUTPUT         None
//
// DESCRIPTION    Loads the 16 bits value into Bus Master IDE Status Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeLoadBMISP(bmPtr,val)                                               \
do                                                                            \
{                                                                             \
    (bmPtr)->ideBMISP = (uint16)(val);                                        \
} while (0)
 

/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeGetBMIDTPPLo()
//
// SYNOPSIS       uint16 IdeGetBMIDTPPLo ( ideBM *bmPtr )
//
// TYPE           Macro
//
// INPUT          ideBM *bmPtr: Base Pointer of the IDE Bus Master Registers
//
// OUTPUT         uint16: The lower 16 bits value of Bus Master IDE Descriptor
//                        Table Pointer Register 
//
// DESCRIPTION    Returns the value of the lower 16 bits of Bus Master IDE
//                Descriptor Table Pointer Primary Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeGetBMIDTPPLo(bmPtr)                                                \
    ((bmPtr)->ideBMIDTPP_LO)
 
 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeLoadBMIDTPPLo()
//
// SYNOPSIS       IdeLoadBMIDTPPLo ( ideBM *bmPtr, uint16 val )
//
// TYPE           Macro
//
// INPUT          ideBM *bmPtr: Base Pointer of the IDE Bus Master Registers
//                uint16 val: Lower 16 bits of Bus Master IDE Descriptor
//                            Table Pointer value
//
// OUTPUT         None
//
// DESCRIPTION    Load the value into the lower 16 bits of the Bus Master IDE
//                Descriptor Table Pointer Primary Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeLoadBMIDTPPLo(bmPtr, val)                                          \
do                                                                            \
{                                                                             \
    (bmPtr)->ideBMIDTPP_LO = (uint16)((val) & IDE_BMIDTP_MASK);               \
} while (0)
 
 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeGetBMIDTPPHi()
//
// SYNOPSIS       uint16 IdeGetBMIDTPPHi ( ideBM *bmPtr )
//
// TYPE           Macro
//
// INPUT          ideBM *bmPtr: Base Pointer of the IDE Bus Master Registers
//
// OUTPUT         uint16: The upper 16 bits value of Bus Master IDE Descriptor
//                        Table Pointer Register
//
// DESCRIPTION    Returns the value of the upper 16 bits of Bus Master IDE 
//                Descriptor Table Pointer Primary Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeGetBMIDTPPHi(bmPtr)                                                \
    ((bmPtr)->ideBMIDTPP_HI)

 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeLoadBMIDTPPHi()
//
// SYNOPSIS       IdeLoadBMIDTPPHi ( ideBM *bmPtr, uint16 val )
//
// TYPE           Macro
//
// INPUT          ideBM *bmPtr: Base Pointer of the IDE Bus Master Registers
//                uint16 val: Upper 16 bits of Bus Master IDE Descriptor
//                            Table Pointer value
//
// OUTPUT         None
//
// DESCRIPTION    Load the value into the upper 16 bits of the Bus Master IDE
//                Descriptor Table Pointer Primary Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeLoadBMIDTPPHi(bmPtr, val)                                          \
do                                                                            \
{                                                                             \
    (bmPtr)->ideBMIDTPP_HI = (uint16)((val) & IDE_BMIDTP_MASK);               \
} while (0)
 
 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeGetBMIDTPP()
//
// SYNOPSIS       uint32 IdeGetBMIDTPP ( ideBM *idePtr )
//
// TYPE           Macro
//
// INPUT          ideBM *idePtr: Base Pointer of the IDE Controller Register
//
// OUTPUT         uint32: The 32 bits value of Bus Master IDE Descriptor Table
//                        Pointer Register
//
// DESCRIPTION    Returns the 32 bits value of Bus Master IDE Descriptor Table
//                Pointer Primary Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeGetBMIDTPP(idePtr)                                                 \
    (((IdeGetBMIDTPPHi(idePtr) & IDE_BMIDTP_MASK) << IDE_BMIDTP_HI_SHIFT) |   \
      (IdeGetBMIDTPPLo(idePtr) & IDE_BMIDTP_MASK))
 
 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeLoadBMIDTPP()
//
// SYNOPSIS       IdeLoadBMIDTPP ( ideBM *idePtr, uint32 val )
//
// TYPE           Macro
//
// INPUT          ideBM *idePtr: Base Pointer of the IDE Controller Register
//                uint32 val: 32 bits of Bus Master IDE Descriptor Table
//                            Pointer value
//
// OUTPUT         None
//
// DESCRIPTION    Returns the 32 bits value of the Bus Master IDE Descriptor
//                Table Pointer Primary Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeLoadBMIDTPP(idePtr, val)                                           \
do                                                                            \
{                                                                             \
    IdeLoadBMIDTPPLo(idePtr, (uint16)((val) & IDE_BMIDTP_MASK));              \
    IdeLoadBMIDTPPHi(idePtr, (uint16)(((val) >> IDE_BMIDTP_HI_SHIFT) &        \
                                  IDE_BMIDTP_MASK));                          \
} while (0)
 

 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeGetBMICS()
//
// SYNOPSIS       uint16 IdeGetBMICS ( ideBM *bmPtr )
//
// TYPE           Macro
//
// INPUT          ideBM *bmPtr: Base Pointer of the IDE Bus Master Registers
//
// OUTPUT         uint16: The 16 bits value of Bus Master IDE Command Register
//
// DESCRIPTION    Returns the 16 bits value of the Bus Master IDE Command 
//                Secondary Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeGetBMICS(bmPtr)                                                    \
    ((bmPtr)->ideBMICS)
 
 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeLoadBMICS()
//
// SYNOPSIS       IdeLoadBMICS ( ideBM *bmPtr, uint16 val )
//
// TYPE           Macro
//
// INPUT          ideBM *bmPtr: Base Pointer of the IDE Bus Master Registers
//                uint16 val: 16 bits of Bus Master IDE Command Register
//
// OUTPUT         None
//
// DESCRIPTION    Loads the 16 bits value into the Bus Master IDE Command 
//                Secondary Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeLoadBMICS(bmPtr,val)                                               \
do                                                                            \
{                                                                             \
    (bmPtr)->ideBMICS = (uint16)(val);                                        \
} while (0)
 

 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeGetBMISS()
//
// SYNOPSIS       uint16 IdeGetBMISS ( ideBM *bmPtr )
//
// TYPE           Macro
//
// INPUT          ideBM *bmPtr: Base Pointer of the IDE Bus Master Registers
//
// OUTPUT         uint16: The 16 bits value of Bus Master IDE Status Register
//
// DESCRIPTION    Returns the 16 bits value of the Bus Master IDE Status 
//                Secondary Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeGetBMISS(bmPtr)                                                    \
    ((bmPtr)->ideBMISS)
 
 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeGetBMIStatus()
//
// SYNOPSIS       uint16 IdeGetBMIStatus ( ideBM *bmPtr, uint32 ideIf )
//
// TYPE           Macro
//
// INPUT          ideBM *bmPtr: Base Pointer of the IDE Bus Master Registers
//                uint32 ideIf: Ide interface
//                              (IDE_IF_PRIMARY, IDE_IF_SECONDARY, ...)
//
// OUTPUT         uint16: The 16 bits value of Bus Master IDE Status Register
//
// DESCRIPTION    Returns the 16 bits value of the Bus Master IDE Status 
//                Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeGetBMIStatus(bmPtr, ideIf)                                         \
    (((ideIf) == IDE_IF_SECONDARY) ? (bmPtr)->ideBMISS : (bmPtr)->ideBMISP)
 
 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeLoadBMISS()
//
// SYNOPSIS       IdeLoadBMISS ( ideBM *bmPtr, uint16 val )
//
// TYPE           Macro
//
// INPUT          ideBM *bmPtr: Base Pointer of the IDE Bus Master Registers
//                uint16 val: 16 bits of Bus Master IDE Status value
//
// OUTPUT         None
//
// DESCRIPTION    Loads the 16 bits value into the Bus Master IDE Status 
//                Secondary Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeLoadBMISS(bmPtr,val)                                               \
do                                                                            \
{                                                                             \
    (bmPtr)->ideBMISS = (uint16)(val);                                        \
} while (0)
 

/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeGetBMIDTPSLo()
//
// SYNOPSIS       uint16 IdeGetBMIDTPSLo ( ideBM *bmPtr )
//
// TYPE           Macro
//
// INPUT          ideBM *bmPtr: Base Pointer of the IDE Bus Master Registers
//
// OUTPUT         uint16: The lower 16 bits value of Bus Master IDE Descriptor
//                        Table Pointer value
//
// DESCRIPTION    Returns the value of the lower 16 bits of Bus Master IDE 
//                Descriptor Table Pointer Secondary Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeGetBMIDTPSLo(bmPtr)                                                \
    ((bmPtr)->ideBMIDTPS_LO)
 
 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeLoadBMIDTPSLo()
//
// SYNOPSIS       IdeLoadBMIDTPSLo ( ideBM *bmPtr, uint16 val )
//
// TYPE           Macro
//
// INPUT          ideBM *bmPtr: Base Pointer of the IDE Bus Master Registers
//                uint16 val: Lower 16 bits of Bus Master IDE Descriptor
//                            Table Pointer value
//
// OUTPUT         None
//
// DESCRIPTION    Load the value into the lower 16 bits of the Bus Master IDE
//                Descriptor Table Pointer Secondary Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeLoadBMIDTPSLo(bmPtr, val)                                          \
do                                                                            \
{                                                                             \
    (bmPtr)->ideBMIDTPS_LO = ((uint16)(val) & IDE_BMIDTP_MASK);               \
} while (0)
 
 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeGetBMIDTPSHi()
//
// SYNOPSIS       uint16 IdeGetBMIDTPSHi ( ideBM *bmPtr )
//
// TYPE           Macro
//
// INPUT          ideBM *bmPtr: Base Pointer of the IDE Bus Master Registers
//
// OUTPUT         uint16: The upper 16 bits value of Bus Master IDE Descriptor
//                        Table Pointer Register
//
// DESCRIPTION    Returns the value of the upper 16 bits of Bus Master IDE 
//                Descriptor Table Pointer Secondary Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeGetBMIDTPSHi(bmPtr)                                                \
    ((bmPtr)->ideBMIDTPS_HI)

 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeLoadBMIDTPSHi()
//
// SYNOPSIS       IdeLoadBMIDTPSHi ( ideBM *bmPtr, uint16 val )
//
// TYPE           Macro
//
// INPUT          ideBM *bmPtr: Base Pointer of the IDE Bus Master Registers
//                uint16 val: Upper 16 bits of Bus Master IDE Descriptor
//                            Table Pointer value
//
// OUTPUT         None
//
// DESCRIPTION    Load the value into the upper 16 bit of the Bus Master IDE
//                Descriptor Table Pointer Secondary Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeLoadBMIDTPSHi(bmPtr, val)                                          \
do                                                                            \
{                                                                             \
    (bmPtr)->ideBMIDTPS_HI = (uint16)((val) & IDE_BMIDTP_MASK);               \
} while (0)
 
 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeGetBMIDTPS()
//
// SYNOPSIS       uint32 IdeGetBMIDTPS ( ideBM *idePtr )
//
// TYPE           Macro
//
// INPUT          ideBM *idePtr: Base Pointer of the IDE Controller Register
//
// OUTPUT         uint32: The 32 bits value of Bus Master IDE Descriptor Table
//                        Pointer Register
//
// DESCRIPTION    Returns the 32 bits value of the Bus Master IDE Descriptor
//                Table Pointer Secondary Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeGetBMIDTPS(idePtr)                                                 \
    (((IdeGetBMIDTPSHi(idePtr) & IDE_BMIDTP_MASK) << IDE_BMIDTP_HI_SHIFT) |   \
      (IdeGetBMIDTPSLo(idePtr) & IDE_BMIDTP_MASK))
 

 
/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeLoadBMIDTPS()
//
// SYNOPSIS       IdeLoadBMIDTPS ( ideBM *idePtr, uint32 val )
//
// TYPE           Macro
//
// INPUT          ideBM *idePtr: Base Pointer of the IDE Controller Register
//                uint32 val: 32 bits of Bus Master IDE Descriptor Table
//                            Pointer value
//
// OUTPUT         None
//
// DESCRIPTION    Load the 32 bits value into the Bus Master IDE Desciptor 
//                Table Pointer Secondary Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeLoadBMIDTPS(idePtr, val)                                           \
do                                                                            \
{                                                                             \
    IdeLoadBMIDTPSLo(idePtr, (uint16)((val) & IDE_BMIDTP_MASK));              \
    IdeLoadBMIDTPSHi(idePtr, (uint16)(((val) >> IDE_BMIDTP_HI_SHIFT) &        \
                                  IDE_BMIDTP_MASK));                          \
} while (0)
 

/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeLoadBMIDescTabPtr()
//
// SYNOPSIS       IdeLoadBMIDescTabPtr ( ideBM *idePtr, uint32, ideIf,
//                                       uint32 val )
//
// TYPE           Macro
//
// INPUT          ideBM *idePtr: Base Pointer of the IDE Controller Register
//                uint32 ideIf: Ide interface
//                              (IDE_IF_PRIMARY, IDE_IF_SECONDARY, ...)
//                uint32 val: 32 bits of Bus Master IDE Descriptor Table
//                            Pointer value
//
// OUTPUT         None
//
// DESCRIPTION    Load the 32 bits value into the Bus Master IDE Desciptor 
//                Table Pointer Register
//
// NOTE           None
//
/******************************************************************************/
#define IdeLoadBMIDescTabPtr(idePtr, ideIf, val)                              \
do                                                                            \
{                                                                             \
    if ((ideIf) == IDE_IF_SECONDARY)  { IdeLoadBMIDTPS((idePtr), (val)); }    \
    else                              { IdeLoadBMIDTPP((idePtr), (val)); }    \
} while (0)
 

/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeSetDmaWriteAta()
//
// SYNOPSIS       IdeSetDmaWriteAta ( ideBM *bmPtr, uint32 ideIf )
//
// TYPE           Macro
//
// INPUT          ideBM *bmPtr: Base Pointer of the IDE Bus Master Registers
//                uint32 ideIf: Ide interface
//                              (IDE_IF_PRIMARY, IDE_IF_SECONDARY, ...)
//
// OUTPUT         None
//
// DESCRIPTION    Set the direction of the DMA transfer to write to the ATA
//                device.  (This means a read for the bus master.)
//
// NOTE           The direction must not be changed while the bus master
//                operation is active (i.e., while dma is transferring data)
//
/******************************************************************************/
#define IdeSetDmaWriteAta(bmPtr, ideIf)                                       \
do                                                                            \
{                                                                             \
    if ((ideIf) == IDE_IF_SECONDARY)                                          \
        (bmPtr)->ideBMICS &= ~((uint16)IDE_DMA_BM_WRITE);                     \
    else                                                                      \
        (bmPtr)->ideBMICP &= ~((uint16)IDE_DMA_BM_WRITE);                     \
} while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeSetDmaReadAta()
//
// SYNOPSIS       IdeSetDmaReadAta ( ideBM *bmPtr, uint32 ideIf )
//
// TYPE           Macro
//
// INPUT          ideBM *bmPtr: Base Pointer of the IDE Bus Master Registers
//                uint32 ideIf: Ide interface
//                              (IDE_IF_PRIMARY, IDE_IF_SECONDARY, ...)
//
// OUTPUT         None
//
// DESCRIPTION    Set the direction of the DMA transfer to read from the ATA
//                device.  (This means a write for the bus master.)
//
// NOTE           The direction must not be changed while the bus master
//                operation is active (i.e., while dma is transferring data)
//
/******************************************************************************/
#define IdeSetDmaReadAta(bmPtr, ideIf)                                        \
do                                                                            \
{                                                                             \
    if ((ideIf) == IDE_IF_SECONDARY)                                          \
        (bmPtr)->ideBMICS |= (uint16)IDE_DMA_BM_WRITE;                        \
    else                                                                      \
        (bmPtr)->ideBMICP |= (uint16)IDE_DMA_BM_WRITE;                        \
} while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeStartDmaXfer()
//
// SYNOPSIS       IdeStartDmaXfer ( ideBM *bmPtr, uint32 ideIf )
//
// TYPE           Macro
//
// INPUT          ideBM *bmPtr: Base Pointer of the IDE Bus Master Registers
//                uint32 ideIf: Ide interface
//                              (IDE_IF_PRIMARY, IDE_IF_SECONDARY, ...)
//
// OUTPUT         None
//
// DESCRIPTION    Start the IDE DMA transfer by enabling bus master operation
//                of the controller
//
// NOTE           None
//
/******************************************************************************/
#define IdeStartDmaXfer(bmPtr, ideIf)                                         \
do                                                                            \
{                                                                             \
    if ((ideIf) == IDE_IF_SECONDARY)                                          \
        (bmPtr)->ideBMICS |= (uint16)IDE_DMA_XFER_START;                      \
    else                                                                      \
        (bmPtr)->ideBMICP |= (uint16)IDE_DMA_XFER_START;                      \
} while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeStopDmaXfer()
//
// SYNOPSIS       IdeStopDmaXfer ( ideBM *bmPtr, uint32 ideIf )
//
// TYPE           Macro
//
// INPUT          ideBM *bmPtr: Base Pointer of the IDE Bus Master Registers
//                uint32 ideIf: Ide interface
//                              (IDE_IF_PRIMARY, IDE_IF_SECONDARY, ...)
//
// OUTPUT         None
//
// DESCRIPTION    Stop the IDE DMA transfer
//
// NOTE           None
//
/******************************************************************************/
#define IdeStopDmaXfer(bmPtr, ideIf)                                          \
do                                                                            \
{                                                                             \
    if ((ideIf) == IDE_IF_SECONDARY)                                          \
        (bmPtr)->ideBMICS &= ~((uint16)IDE_DMA_XFER_START);                   \
    else                                                                      \
        (bmPtr)->ideBMICP &= ~((uint16)IDE_DMA_XFER_START);                   \
} while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeIsDmaXferStarted()
//
// SYNOPSIS       IdeIsDmaXferStarted ( ideRegs *idePtr )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//
// OUTPUT         Returns 1 if start bit is set, return 0 if not
//
// DESCRIPTION    Return the status of the DMA transfer
//
// NOTE           None
//
/******************************************************************************/
#define IdeIsDmaXferStarted(idePtr)                                           \
    ((IdeGetBMICP(idePtr) & IDE_DMA_XFER_START)


/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeEnableTimOverride()
//
// SYNOPSIS       IdeEnableTimOverride ( ideRegs *idePtr )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//
// OUTPUT         None
//
// DESCRIPTION    Enable the IDE timing override
//
// NOTE           None
//
/******************************************************************************/
#define IdeEnableTimOverride(idePtr)                                          \
do                                                                            \
{                                                                             \
    (idePtr)->timOverride |= IDE_TIMORIDE_OVERRIDE_ENA;                       \
} while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           IdeDisableTimOverride()
//
// SYNOPSIS       IdeDisableTimOverride ( ideRegs *idePtr )
//
// TYPE           Macro
//
// INPUT          ideRegs *idePtr: Base Pointer of the IDE Controller Register
//
// OUTPUT         None
//
// DESCRIPTION    Disable the IDE timing override
//
// NOTE           None
//
/******************************************************************************/
#define IdeDisableTimOverride(idePtr)                                         \
do                                                                            \
{                                                                             \
    (idePtr)->timOverride &= ~IDE_TIMORIDE_OVERRIDE_ENA;                      \
} while (0)


#endif /* IDE3710_H */
