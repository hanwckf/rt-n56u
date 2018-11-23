#ifdef MTK_LICENSE
/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mt_hif_sdio.h
*/
#endif /* MTK_LICENSE */
#ifndef __MT_HIF_SDIO_H__
#define __MT_HIF_SDIO_H__

//leonardo temporally
#define CFG_SDIO_INTR_ENHANCE 1
#define CFG_SDIO_RX_AGG 1
#define CFG_SDIO_RX_ENHANCE 1

#define CFG_SDIO_TX_AGG 1

#define CFG_SDIO_DRIVING_TUNE 0
#define CFG_SDIO_BIST 0

#define CFG_SDIO_RX_THREAD 1
#define CFG_SDIO_TX_THREAD 0

#define CFG_SDIO_STAT 0
/*! Maximum RX packet size, if exceed this value, drop incoming packet */
/* 7.2.3 Maganement frames */
#define CFG_RX_MAX_PKT_SIZE   ( 28 + 2312 + 12 /*HIF_RX_HEADER_T*/ )  //TODO: it should be 4096 under emulation mode

#define CFG_NUM_OF_RX0_HIF_DESC                 16

/* extra size for CS_STATUS and enhanced response */
#define CFG_RX_COALESCING_BUFFER_SIZE		((CFG_NUM_OF_RX0_HIF_DESC  + 1) \
											* CFG_RX_MAX_PKT_SIZE)

#define HIF_RX_CSO_APPENDED_LEN              4
#define HIF_RX_ZERO_APPENDED_LEN              4
#define CFG_MAX_RX_ENHANCE_LOOP_COUNT		3

#define SDIO_TX_DESC_LONG_FORMAT_LENGTH_DW       8       //in unit of double word, in MT7636 case, TXD in long format = 8 (including USB DMA scheduler DWORD, although not in used for SDIO case)
#define SDIO_TX_DESC_LONG_FORMAT_LENGTH          (SDIO_TX_DESC_LONG_FORMAT_LENGTH_DW << 2)
#define SDIO_TX_DESC_SHORT_FORMAT_LENGTH_DW      3       //in unit of double word, in MT7636 case, TXD in short format = 3 (including USB DMA scheduler DWORD, although not in used for SDIO case)
#define SDIO_TX_DESC_SHORT_FORMAT_LENGTH         (SDIO_TX_DESC_SHORT_FORMAT_LENGTH_DW << 2)
#define SDIO_TX_DESC_PADDING_LENGTH_DW           0       //in unit of double word
#define SDIO_TX_DESC_PADDING_LENGTH              (SDIO_TX_DESC_PADDING_LENGTH_DW << 2)
#define SDIO_TX_PAGE_SIZE_IS_POWER_OF_2          TRUE
#define SDIO_TX_PAGE_SIZE_IN_POWER_OF_2          7
#define SDIO_TX_PAGE_SIZE                        128
#define NIC_TX_TOTAL_PAGE	 (676) /* configured by FW */
#define SDIO_TX_MAX_SIZE_PER_FRAME               1532
#define SDIO_TX_MAX_PAGE_PER_FRAME  \
                                      ((SDIO_TX_DESC_LONG_FORMAT_LENGTH + SDIO_TX_DESC_PADDING_LENGTH + \
                                        SDIO_TX_MAX_SIZE_PER_FRAME + SDIO_TX_PAGE_SIZE - 1) / SDIO_TX_PAGE_SIZE)





#if(CFG_SDIO_RX_AGG == 1) && (CFG_SDIO_INTR_ENHANCE == 1)
    #define CFG_TX_COALESCING_BUFFER_SIZE          (SDIO_TX_PAGE_SIZE * NIC_TX_TOTAL_PAGE)
#else
    #define CFG_TX_COALESCING_BUFFER_SIZE          (SDIO_TX_MAX_PAGE_PER_FRAME)
#endif /* CFG_SDIO_TX_AGG || CFG_TX_BUFFER_IS_SCATTER_LIST */

/* Normally all traffics are BE. Initially maximum pages go to TC0/BE.
 * BK/VI/VO (TC1/TC2/TC3) gets no page initially.
 * TC4 (BMC) - Not needed for STA mode
 * TC5 (AC4) (mgmt) gets fixed pages (not adjusted by adaptive algorithm
 * TC6 (cmd/cpu) gets fixed pages (not adjusted by adaptive algorithm
 * TC7 (BCN) - Not needed for STA mode
 * TODO: For AP mode give fixed pages to BCN
 *       and BMC will be part of adaptive algorithm.
 */
#define SDIO_TX_PAGE_COUNT_TC1   (0)    /* BK */
#define SDIO_TX_PAGE_COUNT_TC2   (0)    /* VI */
#define SDIO_TX_PAGE_COUNT_TC3   (0)    /* VO */
#define SDIO_TX_PAGE_COUNT_TC4   (0)    /* BMC */
#define SDIO_TX_PAGE_COUNT_TC5   (42)   /* MGMT/AC4 */
#define SDIO_TX_PAGE_COUNT_TC6   (52)   /* CMD/CPU */
#define SDIO_TX_PAGE_COUNT_TC7   (10)    /* BCN */

/* remaining pages to TC0 - BE*/
#define SDIO_TX_PAGE_COUNT_TC0   (NIC_TX_TOTAL_PAGE - (SDIO_TX_PAGE_COUNT_TC1 +	\
					SDIO_TX_PAGE_COUNT_TC2 +		\
					SDIO_TX_PAGE_COUNT_TC3 +		\
					SDIO_TX_PAGE_COUNT_TC4 +		\
					SDIO_TX_PAGE_COUNT_TC5 +		\
					SDIO_TX_PAGE_COUNT_TC6 +		\
					SDIO_TX_PAGE_COUNT_TC7))


#define MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL   1

#ifdef MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL
#define TC_PAGE_BASED_DEMAND    1
#define DEBUG_ADAPTIVE_QUOTA    0

/* pages reserved for TC5, TC6, TC7 - mgmt frame, cmd, beacon */
#define NIC_TOTAL_RESERVED_PAGE 	(SDIO_TX_PAGE_COUNT_TC5 + SDIO_TX_PAGE_COUNT_TC6 + SDIO_TX_PAGE_COUNT_TC7)
/* pages free to be allocated to TC0-TC4 */
#define NIC_TOTAL_ADJUSTABLE_RESOURCES      (NIC_TX_TOTAL_PAGE - NIC_TOTAL_RESERVED_PAGE)

#define NUM_ADJUSTABLE_TC   5


#define SDIO_GUARANTEED_TC0_RESOURCE     52
#define SDIO_GUARANTEED_TC1_RESOURCE     52
#define SDIO_GUARANTEED_TC2_RESOURCE     117
#define SDIO_GUARANTEED_TC3_RESOURCE     143
#define SDIO_GUARANTEED_TC4_RESOURCE     0
#define SDIO_GUARANTEED_TC5_RESOURCE     42
#define SDIO_GUARANTEED_TC6_RESOURCE     52
#define SDIO_GUARANTEED_TC7_RESOURCE     10


#define SDIO_INIT_TIME_TO_UPDATE_QUE_LEN  60
#define SDIO_INIT_TIME_TO_ADJUST_TC_RSC   3
#define MOVING_AVARAGE_WINDOW_SIZE      4

#endif	/* MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL */

#define WCIR 0x0000
#define CHIP_ID_MASK (0xffff)
#define GET_CHIP_ID(p) (((p) & CHIP_ID_MASK))
#define REVISION_ID_MASK (0xf << 16)
#define GET_REVISION_ID(p) (((p) & REVISION_ID_MASK) >> 16)
#define POR_INDICATOR (1 << 20)
#define GET_POR_INDICATOR(p) (((p) & POR_INDICATOR) >> 20)
#define W_FUNC_RDY (1 << 21)
#define GET_W_FUNC_RDY(p) (((p) & W_FUNC_RDY) >> 21)
#define DEVICE_STATUS_MASK (0xff << 24)
#define GET_DEVICE_STATUS(p) (((p) & DEVICE_STATUS_MASK) >> 24)

#define WHLPCR 0x0004
#define W_INT_EN_SET (1 << 0)
#define W_INT_EN_CLR (1 << 1)
#define W_FW_OWN_REQ_SET (1 << 8)
#define GET_W_FW_OWN_REQ_SET(p) (((p) & W_FW_OWN_REQ_SET) >> 8)
#define W_FW_OWN_REQ_CLR (1 << 9)

#define WSDIOCSR 0x0008

#define WHCR 0x000C
#define W_INT_CLR_CTRL (1 << 1)
#define RECV_MAILBOX_RD_CLR_EN (1 << 2)
#define RPT_OWN_RX_PACKET_LEN (1 << 3)
#define MAX_HIF_RX_LEN_NUM_MASK (0x3f << 8)
#define MAX_HIF_RX_LEN_NUM(p) (((p) & 0x3f) << 8)
#define GET_MAX_HIF_RX_LEN_NUM(p) (((p) & MAX_HIF_RX_LEN_NUM_MASK) >> 8)
#define RX_ENHANCE_MODE (1 << 16)

#define WHISR 0x0010
#define TX_DONE_INT (1 << 0)
#define RX0_DONE_INT (1 << 1)
#define RX1_DONE_INT (1 << 2)
#define ABNORMAL_INT (1 << 6)
#define FW_OWN_BACK_INT (1 << 7)
#define D2H_SW_INT (0xffffff << 8)
#define D2H_SW_INT_MASK (0xffffff << 8)
#define GET_D2H_SW_INT(p) (((p) & D2H_SW_INT_MASK) >> 8)

#define WHIER 0x0014
#define TX_DONE_INT_EN (1 << 0)
#define RX0_DONE_INT_EN (1 << 1)
#define RX1_DONE_INT_EN (1 << 2)
#define ABNORMAL_INT_EN (1 << 6)
#define FW_OWN_BACK_INT_EN (1 << 7)
#define D2H_SW_INT_EN_MASK (0xffffff << 8)
#define D2H_SW_INT_EN(p) (((p) & 0xffffff) << 8)
#define GET_D2H_SW_INT_EN(p) (((p) & D2H_SW_INT_EN_MASK) >> 8)

#define WHIER_DEFAULT (TX_DONE_INT_EN | RX0_DONE_INT_EN | RX1_DONE_INT_EN\
						| ABNORMAL_INT_EN\
						| D2H_SW_INT_EN_MASK)


#define WASR 0x0020
#define TX1_OVERFLOW (1 << 1)
#define RX0_UNDERFLOW (1 << 8)
#define RX1_UNDERFLOW (1 << 9)
#define FW_OWN_INVALID_ACCESS (1 << 16)

#define WSICR 0x0024
#define WTDR1 0x0034
#define WRDR0 0x0050
#define WRDR1 0x0054
#define H2DSM0R 0x0070
#define H2DSM1R 0x0074
#define D2HRM0R 0x0078
#define D2HRM1R 0x007c

#define WRPLR 0x0090
#define RX0_PACKET_LENGTH_MASK (0xffff)
#define GET_RX0_PACKET_LENGTH(p) (((p) & RX0_PACKET_LENGTH_MASK))
#define RX1_PACKET_LENGTH_MASK (0xffff << 16)
#define GET_RX1_PACKET_LENGTH(p) (((p) & RX1_PACKET_LENGTH_MASK) >> 16)

#define WTMDR 0x00b0
#define WTMCR 0x00b4
#define WTMDPCR0 0x00b8
#define WTMDPCR1 0x00bc

#define WPLRCR 0x00d4
#define RX0_RPT_PKT_LEN_MASK (0x3f)
#define RX0_RPT_PKT_LEN(p) (((p) & 0x3f))
#define GET_RPT_PKT_LEN(p) (((p) & RX0_RPT_PKT_LEN_MASK))
#define RX1_RPT_PKT_LEN_MASK (0x3f << 8)
#define RX1_RPT_PKT_LEN(p) (((p) & 0x3f) << 8)
#define GET_RX1_RPT_PKT_LEN(p) (((p) & RX1_RPT_PKT_LEN_MASK) >> 8)

#define WSR 0x00D8
#define CLKIOCR 0x0100
#define CMDIOCR 0x0104
#define DAT0IOCR 0x0108
#define DAT1IOCR 0x010C
#define DAT2IOCR 0x0110
#define DAT3IOCR 0x0114
#define CLKDLYCR 0x0118
#define CMDDLYCR 0x011C
#define ODATDLYCR 0x0120
#define IDATDLYCR1 0x0124
#define IDATDLYCR2 0x0128
#define ILCHCR 0x012C
#define WTQCR0 0x0130
#define WTQCR1 0x0134
#define WTQCR2 0x0138
#define WTQCR3 0x013C
#define WTQCR4 0x0140
#define WTQCR5 0x0144
#define WTQCR6 0x0148
#define WTQCR7 0x014C

#define SDIO_CFG_MAX_HIF_RX_LEN_NUM(_prAdapter, _ucNumOfRxLen) \
    { \
    UINT32 u4Value, ucNum; \
    ucNum = ((_ucNumOfRxLen >= 32) ? 0 : _ucNumOfRxLen); \
    u4Value = 0; \
    MTSDIORead32(_prAdapter, \
        WHCR, \
        &u4Value); \
    u4Value &= ~MAX_HIF_RX_LEN_NUM_MASK; \
    u4Value |= ((((UINT32)ucNum) << 8) & MAX_HIF_RX_LEN_NUM_MASK); \
    MTSDIOWrite32(_prAdapter, \
        WHCR, \
        u4Value); \
    }

typedef struct _ENHANCE_MODE_DATA_STRUCT_T {
    UINT32             u4WHISR;
    union {
        struct {
            UINT16             u2TQ0Cnt;
            UINT16             u2TQ1Cnt;
            UINT16             u2TQ2Cnt;
            UINT16             u2TQ3Cnt;
            UINT16             u2TQ4Cnt;
            UINT16             u2TQ5Cnt;
            UINT16             u2TQ6Cnt;
            UINT16             u2TQ7Cnt;
            UINT16             u2TQ8Cnt;
            UINT16             u2TQ9Cnt;
            UINT16             u2TQ10Cnt;
            UINT16             u2TQ11Cnt;
            UINT16             u2TQ12Cnt;
            UINT16             u2TQ13Cnt;
            UINT16             u2TQ14Cnt;
            UINT16             u2TQ15Cnt;

        } u;
        UINT32                 au4WTSR[8];
    } rTxInfo;
    union {
        struct {
            UINT16             u2NumValidRx0Len;
            UINT16             u2NumValidRx1Len;
            UINT16             au2Rx0Len[16];
            UINT16             au2Rx1Len[16];
        } u;
        UINT32                 au4RxStatusRaw[17];
    } rRxInfo;
    UINT32                     u4RcvMailbox0;
    UINT32                     u4RcvMailbox1;
} ENHANCE_MODE_DATA_STRUCT_T, *P_ENHANCE_MODE_DATA_STRUCT_T;
// #endif /* ENHANCE_MODE_DATA_STRUCT_T */

/* HIF Tx interrupt status queue index*/
typedef enum _ENUM_HIF_TX_INDEX_T {
    HIF_TX_AC0_INDEX = 0,   /* HIF TX: AC0 packets */
    HIF_TX_AC1_INDEX,       /* HIF TX: AC1 packets */
    HIF_TX_AC2_INDEX,       /* HIF TX: AC2 packets */
    HIF_TX_AC3_INDEX,       /* HIF TX: AC3 packets */
    HIF_TX_AC4_INDEX,       /* HIF TX: AC4 packets */
    HIF_TX_AC5_INDEX,       /* HIF TX: AC5 packets */
    HIF_TX_AC6_INDEX,       /* HIF TX: AC6 packets */
    HIF_TX_BCN_INDEX,       /* HIF TX: BCN packets */
    HIF_TX_BMC_INDEX,       /* HIF TX: BMC packets */
    HIF_TX_AC10_INDEX,      /* HIF TX: AC10 packets */
    HIF_TX_AC11_INDEX,      /* HIF TX: AC11 packets */
    HIF_TX_AC12_INDEX,      /* HIF TX: AC12 packets */
    HIF_TX_AC13_INDEX,      /* HIF TX: AC13 packets */
    HIF_TX_AC14_INDEX,      /* HIF TX: AC14 packets */
    HIF_TX_FFA_INDEX,       /* HIF TX: free-for-all */
    HIF_TX_CPU_INDEX,       /* HIF TX: CPU */
    HIF_TX_NUM       /* Maximum number of HIF TX port. */
} ENUM_HIF_TX_INDEX_T;

#endif

