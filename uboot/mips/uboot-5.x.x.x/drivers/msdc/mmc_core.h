/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef MMC_CORE_H
#define MMC_CORE_H

#include <linux/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MMC_BLOCK_BITS                  (9)
#define MMC_BLOCK_SIZE                  (1 << MMC_BLOCK_BITS)
#define MMC_MAX_BLOCK_SIZE              (1 << MMC_BLOCK_BITS)

#define SDIO_MAX_FUNCS                  (7)

#define SD_CMD_BIT                      (1 << 7)
#define SD_CMD_APP_BIT                  (1 << 8)
#define SD_CMD_AUTO_BIT                 (1 << 9)

/* MMC command numbers */
#define MMC_CMD_GO_IDLE_STATE           (0)              /* bc.   no response */
#define MMC_CMD_SEND_OP_COND            (1)              /* bcr.  R3          */
#define MMC_CMD_ALL_SEND_CID            (2)              /* bcr.  R2          */
#define MMC_CMD_SET_RELATIVE_ADDR       (3)              /* ac.   R1          */
#define MMC_CMD_SET_DSR                 (4)              /* bc.   no response */
#define MMC_CMD_SLEEP_AWAKE             (5)              /* ac.   R1b         */
#define MMC_CMD_SWITCH                  (6)              /* ac.   R1b         */
#define MMC_CMD_SELECT_CARD             (7)              /* ac.   R1/R1b      */
#define MMC_CMD_SEND_EXT_CSD            (8)              /* adtc. R1          */
#define MMC_CMD_SEND_CSD                (9)              /* ac.   R2          */
#define MMC_CMD_SEND_CID                (10)             /* ac.   R2          */
#define MMC_CMD_READ_DAT_UNTIL_STOP     (11)             /* adtc. R1          */
#define MMC_CMD_STOP_TRANSMISSION       (12)             /* ac.   R1/R1b      */
#define MMC_CMD_SEND_STATUS             (13)             /* ac.   R1          */
#define MMC_CMD_BUSTEST_R               (14)             /* adtc. R1          */
#define MMC_CMD_GO_INACTIVE_STATE       (15)             /* ac.   no response */
#define MMC_CMD_SET_BLOCKLEN            (16)             /* ac.   R1          */
#define MMC_CMD_READ_SINGLE_BLOCK       (17)             /* adtc. R1          */
#define MMC_CMD_READ_MULTIPLE_BLOCK     (18)             /* adtc. R1          */
#define MMC_CMD_BUSTEST_W               (19)             /* adtc. R1          */
#define MMC_CMD_WRITE_DAT_UNTIL_STOP    (20)             /* adtc. R1          */
#define MMC_CMD_SET_BLOCK_COUNT         (23)             /* ac.   R1          */
#define MMC_CMD_WRITE_BLOCK             (24)             /* adtc. R1          */
#define MMC_CMD_WRITE_MULTIPLE_BLOCK	(25)             /* adtc. R1          */
#define MMC_CMD_PROGRAM_CID             (26)             /* adtc. R1          */
#define MMC_CMD_PROGRAM_CSD             (27)             /* adtc. R1          */

#define MMC_CMD_SET_WRITE_PROT          (28)             /* ac.   R1b         */
#define MMC_CMD_CLR_WRITE_PROT          (29)             /* ac.   R1b         */
#define MMC_CMD_SEND_WRITE_PROT         (30)             /* adtc. R1          */
#define MMC_CMD_SEND_WRITE_PROT_TYPE    (31)             /* adtc. R1          */
#define MMC_CMD_ERASE_WR_BLK_START      (32)
#define MMC_CMD_ERASE_WR_BLK_END        (33)
#define MMC_CMD_ERASE_GROUP_START       (35)             /* ac.   R1          */
#define MMC_CMD_ERASE_GROUP_END         (36)             /* ac.   R1          */
#define MMC_CMD_ERASE                   (38)             /* ac.   R1b         */
#define MMC_CMD_FAST_IO                 (39)             /* ac.   R4          */
#define MMC_CMD_GO_IRQ_STATE            (40)             /* bcr.  R5          */
#define MMC_CMD_LOCK_UNLOCK             (42)             /* adtc. R1          */
#define MMC_CMD_APP_CMD                 (55)             /* ac.   R1          */
#define MMC_CMD_GEN_CMD                 (56)             /* adtc. R1          */

/* SD Card command numbers */
#define SD_CMD_SEND_RELATIVE_ADDR       (3 | SD_CMD_BIT)
#define SD_CMD_SWITCH                   (6 | SD_CMD_BIT)
#define SD_CMD_SEND_IF_COND             (8 | SD_CMD_BIT)
#define SD_CMD_VOL_SWITCH               (11 | SD_CMD_BIT)
#define SD_CMD_SEND_TUNING_BLOCK        (19 | SD_CMD_BIT)
#define SD_CMD_SPEED_CLASS_CTRL         (20 | SD_CMD_BIT)

#define SD_ACMD_SET_BUSWIDTH	        (6  | SD_CMD_APP_BIT)
#define SD_ACMD_SD_STATUS               (13 | SD_CMD_APP_BIT)
#define SD_ACMD_SEND_NR_WR_BLOCKS       (22 | SD_CMD_APP_BIT)
#define SD_ACMD_SET_WR_ERASE_CNT        (23 | SD_CMD_APP_BIT)
#define SD_ACMD_SEND_OP_COND            (41 | SD_CMD_APP_BIT)
#define SD_ACMD_SET_CLR_CD              (42 | SD_CMD_APP_BIT)
#define SD_ACMD_SEND_SCR                (51 | SD_CMD_APP_BIT)

/* SDIO Card command numbers */
#define SD_IO_SEND_OP_COND              (5 | SD_CMD_BIT) /* bcr. R4           */
#define SD_IO_RW_DIRECT                 (52 | SD_CMD_BIT)/* ac.  R5           */
#define SD_IO_RW_EXTENDED               (53 | SD_CMD_BIT)/* adtc. R5          */

/* platform dependent command */
#define SD_ATOCMD_STOP_TRANSMISSION     (12 | SD_CMD_AUTO_BIT)
#define SD_ATOCMD_SET_BLOCK_COUNT       (23 | SD_CMD_AUTO_BIT)

#define MMC_VDD_145_150	0x00000001	/* VDD voltage 1.45 - 1.50 */
#define MMC_VDD_150_155	0x00000002	/* VDD voltage 1.50 - 1.55 */
#define MMC_VDD_155_160	0x00000004	/* VDD voltage 1.55 - 1.60 */
#define MMC_VDD_160_165	0x00000008	/* VDD voltage 1.60 - 1.65 */
#define MMC_VDD_165_170	0x00000010	/* VDD voltage 1.65 - 1.70 */
#define MMC_VDD_17_18	0x00000020	/* VDD voltage 1.7 - 1.8 */
#define MMC_VDD_18_19	0x00000040	/* VDD voltage 1.8 - 1.9 */
#define MMC_VDD_19_20	0x00000080	/* VDD voltage 1.9 - 2.0 */
#define MMC_VDD_20_21	0x00000100	/* VDD voltage 2.0 ~ 2.1 */
#define MMC_VDD_21_22	0x00000200	/* VDD voltage 2.1 ~ 2.2 */
#define MMC_VDD_22_23	0x00000400	/* VDD voltage 2.2 ~ 2.3 */
#define MMC_VDD_23_24	0x00000800	/* VDD voltage 2.3 ~ 2.4 */
#define MMC_VDD_24_25	0x00001000	/* VDD voltage 2.4 ~ 2.5 */
#define MMC_VDD_25_26	0x00002000	/* VDD voltage 2.5 ~ 2.6 */
#define MMC_VDD_26_27	0x00004000	/* VDD voltage 2.6 ~ 2.7 */
#define MMC_VDD_27_28	0x00008000	/* VDD voltage 2.7 ~ 2.8 */
#define MMC_VDD_28_29	0x00010000	/* VDD voltage 2.8 ~ 2.9 */
#define MMC_VDD_29_30	0x00020000	/* VDD voltage 2.9 ~ 3.0 */
#define MMC_VDD_30_31	0x00040000	/* VDD voltage 3.0 ~ 3.1 */
#define MMC_VDD_31_32	0x00080000	/* VDD voltage 3.1 ~ 3.2 */
#define MMC_VDD_32_33	0x00100000	/* VDD voltage 3.2 ~ 3.3 */
#define MMC_VDD_33_34	0x00200000	/* VDD voltage 3.3 ~ 3.4 */
#define MMC_VDD_34_35	0x00400000	/* VDD voltage 3.4 ~ 3.5 */
#define MMC_VDD_35_36	0x00800000	/* VDD voltage 3.5 ~ 3.6 */
#define MMC_CARD_BUSY	0x80000000	/* Card Power up status bit */

#define MMC_ERR_NONE        0
#define MMC_ERR_TIMEOUT     1
#define MMC_ERR_BADCRC      2
#define MMC_ERR_FIFO        3
#define MMC_ERR_FAILED      4
#define MMC_ERR_INVALID     5

#define MMC_POWER_OFF       0
#define MMC_POWER_UP        1
#define MMC_POWER_ON        2

#define MMC_BUS_WIDTH_1     0
#define MMC_BUS_WIDTH_4     2

#define SD_BUS_WIDTH_1      0
#define SD_BUS_WIDTH_4      2

#define MMC_STATE_PRESENT       (1<<0)      /* present in sysfs */
#define MMC_STATE_READONLY      (1<<1)      /* card is read-only */
#define MMC_STATE_HIGHSPEED     (1<<2)      /* card is in high speed mode */
#define MMC_STATE_BLOCKADDR     (1<<3)      /* card uses block-addressing */
#define MMC_STATE_HIGHCAPS      (1<<4)
#define MMC_STATE_UHS1          (1<<5)      /* card is in ultra high speed mode */
#define MMC_STATE_DDR           (1<<6)      /* card is in ddr mode */

#define R1_OUT_OF_RANGE         (1UL << 31) /* er, c */
#define R1_ADDRESS_ERROR        (1 << 30)   /* erx, c */
#define R1_BLOCK_LEN_ERROR      (1 << 29)   /* er, c */
#define R1_ERASE_SEQ_ERROR      (1 << 28)   /* er, c */
#define R1_ERASE_PARAM          (1 << 27)   /* ex, c */
#define R1_WP_VIOLATION         (1 << 26)   /* erx, c */
#define R1_CARD_IS_LOCKED       (1 << 25)   /* sx, a */
#define R1_LOCK_UNLOCK_FAILED   (1 << 24)   /* erx, c */
#define R1_COM_CRC_ERROR        (1 << 23)   /* er, b */
#define R1_ILLEGAL_COMMAND      (1 << 22)   /* er, b */
#define R1_CARD_ECC_FAILED      (1 << 21)   /* ex, c */
#define R1_CC_ERROR             (1 << 20)   /* erx, c */
#define R1_ERROR                (1 << 19)   /* erx, c */
#define R1_UNDERRUN             (1 << 18)   /* ex, c */
#define R1_OVERRUN              (1 << 17)   /* ex, c */
#define R1_CID_CSD_OVERWRITE    (1 << 16)   /* erx, c, CID/CSD overwrite */
#define R1_WP_ERASE_SKIP        (1 << 15)   /* sx, c */
#define R1_CARD_ECC_DISABLED    (1 << 14)   /* sx, a */
#define R1_ERASE_RESET          (1 << 13)   /* sr, c */
#define R1_STATUS(x)            (x & 0xFFFFE000)
#define R1_CURRENT_STATE(x)     ((x & 0x00001E00) >> 9) /* sx, b (4 bits) */
#define R1_READY_FOR_DATA       (1 << 8)    /* sx, a */
#define R1_SWITCH_ERROR         (1 << 7)    /* ex, b */
#define R1_URGENT_BKOPS         (1 << 6)    /* sr, a */
#define R1_APP_CMD              (1 << 5)    /* sr, c */

/*
 * Card Command Classes (CCC)
 */
#define CCC_BASIC               (1<<0)  /* (0) Basic protocol functions */
                                        /* (CMD0,1,2,3,4,7,9,10,12,13,15) */
#define CCC_STREAM_READ         (1<<1)  /* (1) Stream read commands */
                                        /* (CMD11) */
#define CCC_BLOCK_READ          (1<<2)  /* (2) Block read commands */
                                        /* (CMD16,17,18) */
#define CCC_STREAM_WRITE        (1<<3)  /* (3) Stream write commands */
                                        /* (CMD20) */
#define CCC_BLOCK_WRITE         (1<<4)  /* (4) Block write commands */
                                        /* (CMD16,24,25,26,27) */
#define CCC_ERASE               (1<<5)  /* (5) Ability to erase blocks */
                                        /* (CMD32,33,34,35,36,37,38,39) */
#define CCC_WRITE_PROT          (1<<6)  /* (6) Able to write protect blocks */
                                        /* (CMD28,29,30) */
#define CCC_LOCK_CARD           (1<<7)  /* (7) Able to lock down card */
                                        /* (CMD16,CMD42) */
#define CCC_APP_SPEC            (1<<8)  /* (8) Application specific */
                                        /* (CMD55,56,57,ACMD*) */
#define CCC_IO_MODE             (1<<9)  /* (9) I/O mode */
                                        /* (CMD5,39,40,52,53) */
#define CCC_SWITCH              (1<<10) /* (10) High speed switch */
                                        /* (CMD6,34,35,36,37,50) */
                                        /* (11) Reserved */
                                        /* (CMD?) */

/*
 * CSD field definitions
 */

#define CSD_STRUCT_VER_1_0  0   /* Valid for system specification 1.0 - 1.2 */
#define CSD_STRUCT_VER_1_1  1   /* Valid for system specification 1.4 - 2.2 */
#define CSD_STRUCT_VER_1_2  2   /* Valid for system specification 3.1 - 3.2 - 3.31 - 4.0 - 4.1 */
#define CSD_STRUCT_EXT_CSD  3   /* Version is coded in CSD_STRUCTURE in EXT_CSD */

#define CSD_SPEC_VER_0      0   /* Implements system specification 1.0 - 1.2 */
#define CSD_SPEC_VER_1      1   /* Implements system specification 1.4 */
#define CSD_SPEC_VER_2      2   /* Implements system specification 2.0 - 2.2 */
#define CSD_SPEC_VER_3      3   /* Implements system specification 3.1 - 3.2 - 3.31 */
#define CSD_SPEC_VER_4      4   /* Implements system specification 4.0 - 4.1 */

/*
 * EXT_CSD fields
 */

#define EXT_CSD_BADBLK_MGMT             134 /* R/W */
#define EXT_CSD_ENH_START_ADDR          136 /* R/W 4 bytes */
#define EXT_CSD_ENH_SIZE_MULT           140 /* R/W 3 bytes */
#define EXT_CSD_GP1_SIZE_MULT           143 /* R/W 3 bytes */
#define EXT_CSD_GP2_SIZE_MULT           146 /* R/W 3 bytes */
#define EXT_CSD_GP3_SIZE_MULT           149 /* R/W 3 bytes */
#define EXT_CSD_GP4_SIZE_MULT           152 /* R/W 3 bytes */
#define EXT_CSD_PART_SET_COMPL          155 /* R/W */
#define EXT_CSD_PART_ATTR               156 /* R/W 3 bytes */
#define EXT_CSD_MAX_ENH_SIZE_MULT       157 /* R/W 3 bytes */
#define EXT_CSD_PART_SUPPORT            160 /* R */
#define EXT_CSD_HPI_MGMT                161 /* R/W/E_P (4.41) */
#define EXT_CSD_RST_N_FUNC              162 /* R/W */
#define EXT_CSD_BKOPS_EN                163 /* R/W (4.41) */
#define EXT_CSD_BKOPS_START             164 /* W/E_P (4.41) */
#define EXT_CSD_WR_REL_PARAM            166 /* R (4.41) */
#define EXT_CSD_WR_REL_SET              167 /* R/W (4.41) */
#define EXT_CSD_RPMB_SIZE_MULT          168 /* R */
#define EXT_CSD_FW_CONFIG               169 /* R/W */
#define EXT_CSD_USR_WP                  171 /* R/W, R/W/C_P & R/W/E_P */
#define EXT_CSD_BOOT_WP                 173 /* R/W, R/W/C_P */
#define EXT_CSD_ERASE_GRP_DEF           175 /* R/W/E */
#define EXT_CSD_BOOT_BUS_WIDTH          177 /* R/W/E */
#define EXT_CSD_BOOT_CONFIG_PROT        178 /* R/W & R/W/C_P */
#define EXT_CSD_PART_CFG                179 /* R/W/E & R/W/E_P */
#define EXT_CSD_ERASED_MEM_CONT         181 /* R */
#define EXT_CSD_BUS_WIDTH               183 /* R/W */
#define EXT_CSD_HS_TIMING               185 /* R/W */
#define EXT_CSD_PWR_CLASS               187 /* R/W/E_P */
#define EXT_CSD_CMD_SET_REV             189 /* R */
#define EXT_CSD_CMD_SET                 191 /* R/W/E_P */
#define EXT_CSD_REV                     192 /* R */
#define EXT_CSD_STRUCT                  194 /* R */
#define EXT_CSD_CARD_TYPE               196 /* RO */
#define EXT_CSD_OUT_OF_INTR_TIME        198 /* R (4.41) */
#define EXT_CSD_PART_SWITCH_TIME        199 /* R (4.41) */
#define EXT_CSD_PWR_CL_52_195           200 /* R */
#define EXT_CSD_PWR_CL_26_195           201 /* R */
#define EXT_CSD_PWR_CL_52_360           202 /* R */
#define EXT_CSD_PWR_CL_26_360           203 /* R */
#define EXT_CSD_MIN_PERF_R_4_26         205 /* R */
#define EXT_CSD_MIN_PERF_W_4_26         206 /* R */
#define EXT_CSD_MIN_PERF_R_8_26_4_25    207 /* R */
#define EXT_CSD_MIN_PERF_W_8_26_4_25    208 /* R */
#define EXT_CSD_MIN_PERF_R_8_52         209 /* R */
#define EXT_CSD_MIN_PERF_W_8_52         210 /* R */
#define EXT_CSD_SEC_CNT                 212 /* RO, 4 bytes */
#define EXT_CSD_S_A_TIMEOUT             217 /* R */
#define EXT_CSD_S_C_VCCQ                219 /* R */
#define EXT_CSD_S_C_VCC                 220 /* R */
#define EXT_CSD_HC_WP_GPR_SIZE          221 /* R */
#define EXT_CSD_REL_WR_SEC_C            222 /* R */
#define EXT_CSD_ERASE_TIMEOUT_MULT      223 /* R */
#define EXT_CSD_HC_ERASE_GRP_SIZE       224 /* R */
#define EXT_CSD_ACC_SIZE                225 /* R */
#define EXT_CSD_BOOT_SIZE_MULT          226 /* R */
#define EXT_CSD_BOOT_INFO               228 /* R */
#define EXT_CSD_SEC_TRIM_MULT           229 /* R */
#define EXT_CSD_SEC_ERASE_MULT          230 /* R */
#define EXT_CSD_SEC_FEATURE_SUPPORT     231 /* R */
#define EXT_CSD_TRIM_MULT               232 /* R */
#define EXT_CSD_MIN_PERF_DDR_R_8_52     234 /* R */
#define EXT_CSD_MIN_PERF_DDR_W_8_52     235 /* R */
#define EXT_CSD_PWR_CL_DDR_52_195       238 /* R */
#define EXT_CSD_PWR_CL_DDR_52_360       239 /* R */
#define EXT_CSD_INI_TIMEOUT_AP          241 /* R */
#define EXT_CSD_CORRECT_PRG_SECTS_NUM   242 /* R, 4 bytes (4.41) */
#define EXT_CSD_BKOPS_STATUS            246 /* R (4.41) */
#define EXT_CSD_BKOPS_SUPP              502 /* R (4.41) */
#define EXT_CSD_HPI_FEATURE             503 /* R (4.41) */
#define EXT_CSD_S_CMD_SET               504 /* R */

/*
 * EXT_CSD field definitions
 */

/* SEC_FEATURE_SUPPORT[231] */
#define EXT_CSD_SEC_FEATURE_ER_EN       (1<<0)
#define EXT_CSD_SEC_FEATURE_BD_BLK_EN   (1<<2)
#define EXT_CSD_SEC_FEATURE_GB_CL_EN    (1<<4)

/* BOOT_INFO[228] */
#define EXT_CSD_BOOT_INFO_ALT_BOOT      (1<<0)
#define EXT_CSD_BOOT_INFO_DDR_BOOT      (1<<1)
#define EXT_CSD_BOOT_INFO_HS_BOOT       (1<<2)

#define EXT_CSD_CMD_SET_NORMAL          (1<<0)
#define EXT_CSD_CMD_SET_SECURE          (1<<1)
#define EXT_CSD_CMD_SET_CPSECURE        (1<<2)

#define EXT_CSD_CARD_TYPE_26            (1<<0)  /* Card can run at 26MHz */
#define EXT_CSD_CARD_TYPE_52            (1<<1)  /* Card can run at 52MHz */
#define EXT_CSD_CARD_TYPE_DDR_52        (1<<2)  /* Card can run at DDR 52MHz@1.8V or 3V */
#define EXT_CSD_CARD_TYPE_DDR_52_1_2V   (1<<3)  /* Card can run at DDR 52MHz@1.2V */

/* BUS_WIDTH[183] */
#define EXT_CSD_BUS_WIDTH_1             (0) /* Card is in 1 bit mode */
#define EXT_CSD_BUS_WIDTH_4             (1) /* Card is in 4 bit mode */
#define EXT_CSD_BUS_WIDTH_8             (2) /* Card is in 8 bit mode */
#define EXT_CSD_BUS_WIDTH_4_DDR         (5) /* Card is in 4 bit mode + DDR */
#define EXT_CSD_BUS_WIDTH_8_DDR         (6) /* Card is in 8 bit mode + DDR */

/* ERASED_MEM_CONT[181] */
#define EXT_CSD_ERASED_MEM_CONT_0       (0)
#define EXT_CSD_ERASED_MEM_CONT_1       (1)

/* PARTITION CONFIG[179] */
#define EXT_CSD_PART_CFG_DEFT_PART      (0)
#define EXT_CSD_PART_CFG_BOOT_PART_1    (1)
#define EXT_CSD_PART_CFG_BOOT_PART_2    (2)
#define EXT_CSD_PART_CFG_RPMB_PART      (3)
#define EXT_CSD_PART_CFG_GP_PART_1      (4)
#define EXT_CSD_PART_CFG_GP_PART_2      (5)
#define EXT_CSD_PART_CFG_GP_PART_3      (6)
#define EXT_CSD_PART_CFG_GP_PART_4      (7)
#define EXT_CSD_PART_CFG_EN_NO_BOOT     (0 << 3)
#define EXT_CSD_PART_CFG_EN_BOOT_PART_1 (1 << 3)
#define EXT_CSD_PART_CFG_EN_BOOT_PART_2 (2 << 3)
#define EXT_CSD_PART_CFG_EN_USER_AREA   (7 << 3)
#define EXT_CSD_PART_CFG_EN_NO_ACK      (0 << 6)
#define EXT_CSD_PART_CFG_EN_ACK         (1 << 6)

/* BOOT_CONFIG_PROT[178] */
#define EXT_CSD_EN_PWR_BOOT_CFG_PROT    (1)
#define EXT_CSD_EN_PERM_BOOT_CFG_PROT   (1<<4)  /* Carefully */

/* BOOT_BUS_WIDTH[177] */
#define EXT_CSD_BOOT_BUS_WIDTH_1        (0)
#define EXT_CSD_BOOT_BUS_WIDTH_4        (1)
#define EXT_CSD_BOOT_BUS_WIDTH_8        (2)
#define EXT_CSD_BOOT_BUS_RESET          (1 << 2)
#define EXT_CSD_BOOT_BUS_MODE_DEFT      (0 << 3)
#define EXT_CSD_BOOT_BUS_MODE_HS        (1 << 3)
#define EXT_CSD_BOOT_BUS_MODE_DDR       (2 << 3)

/* ERASE_GROUP_DEF[175] */
#define EXT_CSD_ERASE_GRP_DEF_EN        (1)

/* BOOT_WP[173] */
#define EXT_CSD_BOOT_WP_EN_PWR_WP       (1)
#define EXT_CSD_BOOT_WP_EN_PERM_WP      (1 << 2)
#define EXT_CSD_BOOT_WP_DIS_PERM_WP     (1 << 4)
#define EXT_CSD_BOOT_WP_DIS_PWR_WP      (1 << 6)

/* USER_WP[171] */
#define EXT_CSD_USR_WP_EN_PWR_WP        (1)
#define EXT_CSD_USR_WP_EN_PERM_WP       (1<<2)
#define EXT_CSD_USR_WP_DIS_PWR_WP       (1<<3)
#define EXT_CSD_USR_WP_DIS_PERM_WP      (1<<4)
#define EXT_CSD_USR_WP_DIS_CD_PERM_WP   (1<<6)
#define EXT_CSD_USR_WP_DIS_PERM_PWD     (1<<7)

/* RST_n_FUNCTION[162] */
#define EXT_CSD_RST_N_TEMP_DIS          (0)
#define EXT_CSD_RST_N_PERM_EN           (1) /* carefully */
#define EXT_CSD_RST_N_PERM_DIS          (2) /* carefully */

/* PARTITIONING_SUPPORT[160] */
#define EXT_CSD_PART_SUPPORT_PART_EN     (1)
#define EXT_CSD_PART_SUPPORT_ENH_ATTR_EN (1<<1)

/* PARTITIONS_ATTRIBUTE[156] */
#define EXT_CSD_PART_ATTR_ENH_USR       (1<<0)
#define EXT_CSD_PART_ATTR_ENH_1         (1<<1)
#define EXT_CSD_PART_ATTR_ENH_2         (1<<2)
#define EXT_CSD_PART_ATTR_ENH_3         (1<<3)
#define EXT_CSD_PART_ATTR_ENH_4         (1<<4)

/* PARTITION_SETTING_COMPLETED[156] */
#define EXT_CSD_PART_SET_COMPL_BIT      (1<<0)

/*
 * MMC_SWITCH access modes
 */

#define MMC_SWITCH_MODE_CMD_SET		0x00	/* Change the command set */
#define MMC_SWITCH_MODE_SET_BITS	0x01	/* Set bits which are 1 in value */
#define MMC_SWITCH_MODE_CLEAR_BITS	0x02	/* Clear bits which are 1 in value */
#define MMC_SWITCH_MODE_WRITE_BYTE	0x03	/* Set target to value */

#define MMC_SWITCH_MODE_SDR12       0
#define MMC_SWITCH_MODE_SDR25       1
#define MMC_SWITCH_MODE_SDR50       2
#define MMC_SWITCH_MODE_SDR104      3
#define MMC_SWITCH_MODE_DDR50       4

#define MMC_SWITCH_MODE_DRV_TYPE_B  0
#define MMC_SWITCH_MODE_DRV_TYPE_A  1
#define MMC_SWITCH_MODE_DRV_TYPE_C  2
#define MMC_SWITCH_MODE_DRV_TYPE_D  3

#define MMC_SWITCH_MODE_CL_200MA    0
#define MMC_SWITCH_MODE_CL_400MA    1
#define MMC_SWITCH_MODE_CL_600MA    2
#define MMC_SWITCH_MODE_CL_800MA    3

/* 
 * MMC_ERASE arguments
 */
#define MMC_ERASE_SECURE_REQ        (1 << 31)
#define MMC_ERASE_GC_REQ            (1 << 15)
#define MMC_ERASE_TRIM              (1 << 0)
#define MMC_ERASE_NORMAL            (0)

#define HOST_BUS_WIDTH_1            (1)
#define HOST_BUS_WIDTH_4            (4)
#define HOST_BUS_WIDTH_8            (8)

#define EMMC_BOOT_PULL_CMD_MODE     (0)
#define EMMC_BOOT_RST_CMD_MODE      (1)

enum {
    EMMC_BOOT_PWR_RESET,
    EMMC_BOOT_RST_N_SIG,
    EMMC_BOOT_PRE_IDLE_CMD
};

enum {
    RESP_NONE = 0,
    RESP_R1,
    RESP_R2,
    RESP_R3,
    RESP_R4,
    RESP_R5,
    RESP_R6,
    RESP_R7,
    RESP_R1B
};

struct mmc_cid {
    unsigned int   manfid;    
    char           prod_name[8];
    unsigned int   serial;
    unsigned short oemid;
    unsigned short year;
    unsigned char  hwrev;
    unsigned char  fwrev;
    unsigned char  month;
    unsigned char  cbx;                 /* device type: card(0) BGA(1) POP(2) */
};

struct mmc_csd {
    unsigned char  csd_struct;          /* csd structure version */
    unsigned char  mmca_vsn;
    unsigned short cmdclass;            /* card command classes */
    unsigned short tacc_clks;           /* data read access-time-1 in clks */
    unsigned int   tacc_ns;             /* data read access-time-2 */
    unsigned int   r2w_factor;          /* write speed factor */
    unsigned int   max_dtr;             /* max. data transfer rate */
    unsigned int   read_blkbits;        /* max. read data block length */
    unsigned int   write_blkbits;       /* max. write data block length */
    unsigned int   capacity;            /* card capacity */
    unsigned int   erase_sctsz;         /* erase sector size */
    unsigned int   write_prot_grpsz;
    unsigned int   read_partial:1,
                   read_misalign:1,
                   write_partial:1,
                   write_misalign:1,
                   write_prot_grp:1,
                   perm_wr_prot:1,
                   tmp_wr_prot:1,
                   erase_blk_en:1,
                   copy:1,
                   dsr:1;
};

struct mmc_raw_ext_csd {
    /* mode segment */
    unsigned char   rsv1[134];
    unsigned char   sec_bad_blk_mgmt;
    unsigned char   rsv2[1];
    unsigned char   enh_start_addr[4];
    unsigned char   enh_sz_mult[3];
    unsigned char   gp_sz_mult[12];
    unsigned char   part_set_cmpl;
    unsigned char   part_attr;
    unsigned char   max_enh_sz_mult[3];
    unsigned char   part_supp;
    unsigned char   rsv3[1];
    unsigned char   rst_n_func;
    unsigned char   rsv4[5];
    unsigned char   rpmb_sz_mult;
    unsigned char   fw_cfg;
    unsigned char   rsv5[1];
    unsigned char   user_wp;
    unsigned char   rsv6[1];
    unsigned char   boot_wp;
    unsigned char   rsv7[1];
    unsigned char   erase_grp_def;
    unsigned char   rsv8[1];
    unsigned char   boot_bus_width;
    unsigned char   boot_cfg_prot;
    unsigned char   part_cfg;
    unsigned char   rsv9[1];
    unsigned char   erase_mem_cont;
    unsigned char   rsv10[1];
    unsigned char   bus_width;
    unsigned char   rsv11[1];
    unsigned char   hs_timing;
    unsigned char   rsv12[1];
    unsigned char   pwr_cls;
    unsigned char   rsv13[1];
    unsigned char   cmd_set_rev;
    unsigned char   rsv14[1];
    unsigned char   cmd_set;

    /* propertities segment */
    unsigned char   ext_csd_rev;
    unsigned char   rsv15[1];
    unsigned char   csd_struct;
    unsigned char   rsv16[1];
    unsigned char   card_type;
    unsigned char   rsv17[1];
    unsigned char   pwr_cls_52_195;
    unsigned char   pwr_cls_26_195;
    unsigned char   pwr_cls_52_360;
    unsigned char   pwr_cls_26_360;
    unsigned char   rsv18[1];
    unsigned char   min_perf_r_4_26;
    unsigned char   min_perf_w_4_26;
    unsigned char   min_perf_r_8_26_4_52;
    unsigned char   min_perf_w_8_26_4_52;
    unsigned char   min_perf_r_8_52;
    unsigned char   min_perf_w_8_52;
    unsigned char   rsv19[1];
    unsigned char   sec_cnt[4];
    unsigned char   rsv20[1];
    unsigned char   slp_awake_tmo;
    unsigned char   rsv21[1];
    unsigned char   slp_curr_vccq;
    unsigned char   slp_curr_vcc;
    unsigned char   hc_wp_grp_sz;
    unsigned char   rel_wr_sec_cnt;
    unsigned char   erase_tmo_mult;
    unsigned char   hc_erase_grp_sz;
    unsigned char   acc_sz;
    unsigned char   boot_sz_mult;
    unsigned char   rsv22[1];
    unsigned char   boot_info;    
    unsigned char   sec_trim_mult;
    unsigned char   sec_erase_mult;
    unsigned char   sec_supp;
    unsigned char   trim_mult;
    unsigned char   rsv23[1];
    unsigned char   min_perf_ddr_r_8_52;
    unsigned char   min_perf_ddr_w_8_52;
    unsigned char   rsv24[2];
    unsigned char   pwr_cls_ddr_52_195;
    unsigned char   pwr_cls_ddr_52_360;
    unsigned char   rsv25[1];
    unsigned char   ini_tmo_ap;
    unsigned char   rsv26[262];
    unsigned char   supp_cmd_set;
    unsigned char   rsv27[7];
};

struct mmc_ext_csd {
    unsigned int    trim_tmo_ms;
    unsigned int    hc_wp_grp_sz;
    unsigned int    hc_erase_grp_sz;
    unsigned int    sectors;
    unsigned int    hs_max_dtr;
    unsigned int    boot_part_sz;
    unsigned int    rpmb_sz;
    unsigned int    access_sz;
    unsigned int    enh_sz;
    unsigned int    enh_start_addr;
    unsigned char   rev;
    unsigned char   boot_info;    
    unsigned char   part_en:1,
                    enh_attr_en:1,
                    ddr_support:1;
    unsigned char   erased_mem_cont;
};

#define SD_SCR_BUS_WIDTH_1	(1<<0)
#define SD_SCR_BUS_WIDTH_4	(1<<2)

struct sd_scr {
    unsigned char   scr_struct;
    unsigned char   sda_vsn;
    unsigned char   data_bit_after_erase;
    unsigned char   security;
    unsigned char   bus_widths;
    unsigned char   sda_vsn3;
    unsigned char   ex_security;
    unsigned char   cmd_support;
};

#define SD_DRV_TYPE_B       (0)
#define SD_DRV_TYPE_A       (1<<0)
#define SD_DRV_TYPE_C       (1<<1)
#define SD_DRV_TYPE_D       (1<<2)

#define SD_MAX_CUR_200MA    (0)
#define SD_MAX_CUR_400MA    (1<<0)
#define SD_MAX_CUR_600MA    (1<<1)
#define SD_MAX_CUR_800MA    (1<<2)

struct sd_switch_caps {
    unsigned int    hs_max_dtr;
    unsigned int    ddr;
    unsigned int    drv_strength;
    unsigned int    max_cur;
};

struct sdio_cccr {
    unsigned int    sdio_vsn;
    unsigned int    sd_vsn;
    unsigned int    multi_block:1,
                    low_speed:1,
                    wide_bus:1,
                    high_power:1,
                    high_speed:1,
                    intr_multi_block:1;
};

/* SDIO function CIS tuple (unknown to the core) */
struct sdio_func_tuple {
    struct sdio_func_tuple *next;
    unsigned char code;
    unsigned char size;
    unsigned char data[1];
};

struct sdio_func;
typedef void (sdio_irq_handler_t)(struct sdio_func *);
typedef void (*hw_irq_handler_t)(void);

/* SDIO function devices */
struct sdio_func {
    struct mmc_card *card;          /* the card this device belongs to */
    sdio_irq_handler_t *irq_handler;/* IRQ callback */
    unsigned int     num;           /* function number */

    unsigned char    class;         /* standard interface class */
    unsigned short   vendor;        /* vendor id */
    unsigned short   device;        /* device id */

    unsigned int     max_blksize;   /* maximum block size */
    unsigned int     cur_blksize;   /* current block size */

    unsigned int     enable_timeout;/* max enable timeout in msec */

    unsigned int     state;         /* function state */

    u8               tmpbuf[4];     /* DMA:able scratch buffer */

    unsigned int     num_info;      /* number of info strings */
    const char     **info;          /* info strings */

    struct sdio_func_tuple *tuples;
};

struct sdio_cis {
    unsigned short      vendor;
    unsigned short      device;
    unsigned short      blksize;
    unsigned int        max_dtr;
};

#define MMC_CAP_4_BIT_DATA      (1 << 0) /* Can the host do 4 bit transfers */
#define MMC_CAP_MULTIWRITE      (1 << 1) /* Can accurately report bytes sent to card on error */
#define MMC_CAP_BYTEBLOCK       (1 << 2) /* Can do non-log2 block sizes */
#define MMC_CAP_MMC_HIGHSPEED   (1 << 3) /* Can do MMC high-speed timing */
#define MMC_CAP_SD_HIGHSPEED    (1 << 4) /* Can do SD high-speed timing */
#define MMC_CAP_8_BIT_DATA      (1 << 5) /* Can the host do 8 bit transfers */
#define MMC_CAP_SD_UHS1         (1 << 6) /* Can do SD ultra-high-speed timing */
#define MMC_CAP_DDR             (1 << 7) /* The host support dual data rate */

struct mmc_host
{
    struct mmc_card *card;
    u32 max_hw_segs;
    u32 max_phys_segs;
    u32 max_seg_size;
    u32 max_blk_size;
    u32 max_blk_count;
    u32 base;         /* host base address */
    u32 caps;         /* Host capabilities */
    u32 f_min;        /* host min. frequency */
    u32 f_max;        /* host max. frequency */
    u32 clk;          /* host clock speed */
    u32 sclk;         /* SD/MS clock speed */
    u32 blklen;       /* block len */
    u32 blkbits;      /* block bits */
    u32 ocr;          /* current ocr */
    u32 ocr_avail;    /* available ocr */    
    u32 timeout_ns;   /* data timeout ns */
    u32 timeout_clks; /* data timeout clks */
    u8  clksrc;       /* clock source */
    u8  id;           /* host id number */
    u8  boot_type;    /* boot type */
	bool card_detect_flag; /*flag for card detection*/
    void *priv;       /* private data */
    int (*blk_read)(struct mmc_host *host, uchar *dst, ulong src, ulong nblks);
    int (*blk_write)(struct mmc_host *host, ulong dst, uchar *src, ulong nblks);
};

#define MMC_TYPE_UNKNOWN    (0)          /* Unknown card */
#define MMC_TYPE_MMC        (0x00000001) /* MMC card */
#define MMC_TYPE_SD         (0x00000002) /* SD card */
#define MMC_TYPE_SDIO       (0x00000004) /* SDIO card */

/* MMC device */
struct mmc_card {
    struct mmc_host        *host;       /* the host this device belongs to */
    unsigned int            nblks;
    unsigned int            blklen;
    unsigned int            ocr;
    unsigned int            maxhz;
    unsigned int            uhs_mode;
    unsigned int            rca;        /* relative card address of device */
    unsigned int            type;       /* card type */
    unsigned int            sdio_funcs; /* number of SDIO functions */
    unsigned short          state;      /* (our) card state */
    unsigned short          ready;      /* card is ready or not */
    u32                     raw_cid[4]; /* raw card CID */
    u32                     raw_csd[4]; /* raw card CSD */
    u32                     raw_scr[2]; /* raw card SCR */
    u8                      raw_ext_csd[512]; /* raw card EXT_CSD */
    struct mmc_cid          cid;        /* card identification */
    struct mmc_csd          csd;        /* card specific */
    struct mmc_ext_csd      ext_csd;    /* mmc v4 extended card specific */
    struct sd_scr           scr;        /* extra SD information */
    struct sd_switch_caps   sw_caps;    /* switch (CMD6) caps */
    struct sdio_cccr        cccr;       /* common card info */
    struct sdio_cis         cis;        /* common tuple info */
    struct sdio_func       *io_func[SDIO_MAX_FUNCS]; /* SDIO functions (devices) */
    struct sdio_func_tuple *tuples;     /* unknown common tuples */
    unsigned int            num_info;   /* number of info strings */
    const char            **info;       /* info strings */
};

struct mmc_command {
    u32 opcode;
    u32 arg;
    u32 rsptyp;
    u32 resp[4];
    u32 timeout;
    u32 retries;    /* max number of retries */
    u32 error;      /* command error */ 
};

#define mmc_card_mmc(c)             ((c)->type & MMC_TYPE_MMC)
#define mmc_card_sd(c)              ((c)->type & MMC_TYPE_SD)
#define mmc_card_sdio(c)            ((c)->type & MMC_TYPE_SDIO)

#define mmc_card_set_host(c,h)      ((c)->host = (h))
#define mmc_card_set_unknown(c)     ((c)->type = MMC_TYPE_UNKNOWN)
#define mmc_card_set_mmc(c)         ((c)->type |= MMC_TYPE_MMC)
#define mmc_card_set_sd(c)          ((c)->type |= MMC_TYPE_SD)
#define mmc_card_set_sdio(c)        ((c)->type |= MMC_TYPE_SDIO)

#define mmc_card_present(c)         ((c)->state & MMC_STATE_PRESENT)
#define mmc_card_readonly(c)        ((c)->state & MMC_STATE_READONLY)
#define mmc_card_highspeed(c)       ((c)->state & MMC_STATE_HIGHSPEED)
#define mmc_card_uhs1(c)            ((c)->state & MMC_STATE_UHS1)
#define mmc_card_ddr(c)             ((c)->state & MMC_STATE_DDR)
#define mmc_card_blockaddr(c)       ((c)->state & MMC_STATE_BLOCKADDR)
#define mmc_card_highcaps(c)        ((c)->state & MMC_STATE_HIGHCAPS)

#define mmc_card_set_present(c)     ((c)->state |= MMC_STATE_PRESENT)
#define mmc_card_set_readonly(c)    ((c)->state |= MMC_STATE_READONLY)
#define mmc_card_set_highspeed(c)   ((c)->state |= MMC_STATE_HIGHSPEED)
#define mmc_card_set_uhs1(c)        ((c)->state |= MMC_STATE_UHS1)
#define mmc_card_set_ddr(c)         ((c)->state |= MMC_STATE_DDR)  
#define mmc_card_set_blockaddr(c)   ((c)->state |= MMC_STATE_BLOCKADDR)

#define mmc_card_name(c)            ((c)->cid.prod_name)
#define mmc_card_id(c)              ((c)->host->id)

typedef struct {
    u16 max_current;
    u16 grp6_info;
    u16 grp5_info;
    u16 grp4_info;
    u16 grp3_info;
    u16 grp2_info;
    u16 grp1_info;
    u8  grp6_result:4;
    u8  grp5_result:4;
    u8  grp4_result:4;
    u8  grp3_result:4;
    u8  grp2_result:4;
    u8  grp1_result:4;
    u8  ver;
    u16 grp6_busy;
    u16 grp5_busy;
    u16 grp4_busy;
    u16 grp3_busy;
    u16 grp2_busy;
    u16 grp1_busy;
    u8  rev[34];
} mmc_switch_t;

int mmc_init(int id);
void mmc_hard_reset(void);
struct mmc_host *mmc_get_host(int id);
struct mmc_card *mmc_get_card(int id);
int mmc_init_host(int id, struct mmc_host *host, int clksrc, u8 mode);
int mmc_init_card(struct mmc_host *host, struct mmc_card *card);
int mmc_set_blk_length(struct mmc_host *host, u32 blklen);
int mmc_set_bus_width(struct mmc_host *host, struct mmc_card *card, int width);
int mmc_card_avail(struct mmc_host *host);
int mmc_card_protected(struct mmc_host *host);
void mmc_set_clock(struct mmc_host *host, int ddr, u32 hz);
int mmc_block_read(int dev_num, unsigned long blknr, u32 blkcnt, unsigned long *dst);
int mmc_block_write(int dev_num, unsigned long blknr, u32 blkcnt, unsigned long *src);
int mmc_deselect_all_card(struct mmc_host *host);
int mmc_select_card(struct mmc_host *host, struct mmc_card *card);

int mmc_read_ext_csd(struct mmc_host *host, struct mmc_card *card);
int mmc_switch(struct mmc_host *host, struct mmc_card *card, u8 set, u8 index, u8 value);
int mmc_read(ulong src, uchar *dst, int size);
int mmc_write(uchar *src, ulong dst, int size);

int mmc_erase_start(struct mmc_card *card, u32 addr);
int mmc_erase_end(struct mmc_card *card, u32 addr);
int mmc_erase(struct mmc_card *card, u32 arg);

int mmc_io_rw_direct(struct mmc_card *card, int write, unsigned fn,	
    unsigned addr, u8 in, u8* out);
int mmc_io_rw_extended(struct mmc_card *card, int write, unsigned fn,
	unsigned addr, int incr_addr, u8 *buf, unsigned blocks, unsigned blksz);
int mmc_sdio_proc_pending_irqs(struct mmc_card *card);
int mmc_sdio_enable_irq_gap(struct mmc_card *card, int enable);


#ifdef __cplusplus
}
#endif

#endif /* MMC_CORE_H */

