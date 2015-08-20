/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	ral_nmac.h

	Abstract:
	Ralink Wireless Chip RAL MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef __RAL_NMAC_H__
#define __RAL_NMAC_H__

enum INFO_TYPE {
	NORMAL_PACKET,
	CMD_PACKET,
};

enum D_PORT {
	WLAN_PORT,
	CPU_RX_PORT,
	CPU_TX_PORT,
	HOST_PORT,
	VIRTUAL_CPU_RX_PORT,
	VIRTUAL_CPU_TX_PORT,
	DISCARD,
};

#include "rtmp_type.h"

#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TXINFO_NMAC_PKT{
	UINT32 info_type:2;
	UINT32 d_port:3;
	UINT32 QSEL:2;
	UINT32 wiv:1;
	UINT32 rsv1:2;
	UINT32 cso:1;
	UINT32 tso:1;
	UINT32 pkt_80211:1;
	UINT32 rsv0:1;
	UINT32 tx_burst:1;
	UINT32 next_vld:1;
	UINT32 pkt_len:16;
}TXINFO_NMAC_PKT;
#else
typedef struct GNU_PACKED _TXINFO_NMAC_PKT {
	UINT32 pkt_len:16;
	UINT32 next_vld:1;
	UINT32 tx_burst:1;
	UINT32 rsv0:1;
	UINT32 pkt_80211:1;
	UINT32 tso:1;
	UINT32 cso:1;
	UINT32 rsv1:2;
	UINT32 wiv:1;
	UINT32 QSEL:2;
	UINT32 d_port:3;
	UINT32 info_type:2;
}TXINFO_NMAC_PKT;
#endif /* RT_BIG_ENDIAN */

#define TxInfoWIV			txinfo_nmac_pkt.wiv
#define TxInfoQSEL			txinfo_nmac_pkt.QSEL
#define TxInfoPktLen			txinfo_nmac_pkt.pkt_len
#define TxInfoSwLstRnd		txinfo_nmac_pkt.rsv0
#define TxInfoUDMATxburst	txinfo_nmac_pkt.tx_burst
#define TxInfoUDMANextVld	txinfo_nmac_pkt.next_vld
#define TxInfoPkt80211		txinfo_nmac_pkt.pkt_80211

#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TXINFO_NMAC_CMD{
	UINT32 info_type:2;
	UINT32 d_port:3;
	UINT32 cmd_type:7;
	UINT32 cmd_seq:4;
	UINT32 pkt_len:16;
}TXINFO_NMAC_CMD;
#else
typedef struct GNU_PACKED _TXINFO_NMAC_CMD{
	UINT32 pkt_len:16;
	UINT32 cmd_seq:4;
	UINT32 cmd_type:7;
	UINT32 d_port:3;	
	UINT32 info_type:2;
}TXINFO_NMAC_CMD;
#endif /* RT_BIG_ENDIAN */

#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _LED_NMAC_CMD{
	UINT32  rsv:8;
	UINT32 CmdID:8;
	UINT32 Arg0:8;
	UINT32 Arg1:8;
}LED_NMAC_CMD;
#else
typedef struct GNU_PACKED _LED_NMAC_CMD{
	UINT32 Arg1:8;
	UINT32 Arg0:8;
	UINT32 CmdID:8;
	UINT32 rsv:8;	
}LED_NMAC_CMD;
#endif /* RT_BIG_ENDIAN */

typedef union GNU_PACKED _TXINFO_NMAC{
	struct _TXINFO_NMAC_PKT txinfo_pkt;
	struct _TXINFO_NMAC_CMD txinfo_cmd;
}TXINFO_NMAC;


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _TXWI_NMAC {
	/* Word 0 */
	UINT32		PHYMODE:3;
	UINT32		iTxBF:1;
	UINT32		eTxBF:1;
	UINT32		STBC:1;
	UINT32		ShortGI:1;
	UINT32		BW:2;			/* channel bandwidth 20/40/80 MHz */
	UINT32		MCS:7;
	
	UINT32		lut_en:1;
	UINT32		rsv0:1;
	UINT32		Sounding:1;
	UINT32		NDPSndBW:1;	/* NDP sounding BW */
	UINT32		NDPSndRate:2;	/* 0 : MCS0, 1: MCS8, 2: MCS16, 3: reserved */
	UINT32		txop:2;

	UINT32		MpduDensity:3;
	UINT32		AMPDU:1;
	UINT32		TS:1;
	UINT32		CFACK:1;
	UINT32		MIMOps:1;	/* the remote peer is in dynamic MIMO-PS mode */
	UINT32		FRAG:1;		/* 1 to inform TKIP engine this is a fragment. */
	
	/* Word 1 */
	UINT32		Rsv1:2;
	UINT32		MPDUtotalByteCnt:14;
	UINT32		wcid:8;
	UINT32		BAWinSize:6;
	UINT32		NSEQ:1;
	UINT32		ACK:1;

	/* Word 2 */
	UINT32		IV;
	/* Word 3 */
	UINT32		EIV;

	/* Word 4 */
	UINT32		TxPktId:8;
	UINT32		Rsv4:4;
	UINT32		TxPwrAdj:4;
	UINT32		TxStreamMode:8;
	UINT32		TxEAPId:8;
}	TXWI_NMAC, *PTXWI_NMAC;
#else
typedef	struct GNU_PACKED _TXWI_NMAC {
	/* Word	0 */
	/* ex: 00 03 00 40 means txop = 3, PHYMODE = 1 */
	UINT32		FRAG:1;		/* 1 to inform TKIP engine this is a fragment. */
	UINT32		MIMOps:1;	/* the remote peer is in dynamic MIMO-PS mode */
	UINT32		CFACK:1;
	UINT32		TS:1;
	UINT32		AMPDU:1;
	UINT32		MpduDensity:3;

	UINT32		txop:2;
	UINT32		NDPSndRate:2; /* 0 : MCS0, 1: MCS8, 2: MCS16, 3: reserved */
	UINT32		NDPSndBW:1; /* NDP sounding BW */
	UINT32		Sounding:1;
	UINT32		rsv0:1;
	UINT32		lut_en:1;
	
	UINT32		MCS:7;
	UINT32		BW:2;		/*channel bandwidth 20/40/80 MHz */
	UINT32		ShortGI:1;
	UINT32		STBC:1;
	UINT32		eTxBF:1;
	UINT32		iTxBF:1;
	UINT32		PHYMODE:3;  

	/* Word1 */
	/* ex:  1c ff 38 00 means ACK=0, BAWinSize=7, MPDUtotalByteCnt = 0x38 */
	UINT32		ACK:1;
	UINT32		NSEQ:1;
	UINT32		BAWinSize:6;
	UINT32		wcid:8;
	UINT32		MPDUtotalByteCnt:14;
	UINT32		Rsv1:2;
	
	/*Word2 */
	UINT32		IV;
	
	/*Word3 */
	UINT32		EIV;

	/* Word 4 */
	UINT32		TxEAPId:8;
	UINT32		TxStreamMode:8;
	UINT32		TxPwrAdj:4;
	UINT32		Rsv4:4;	
	UINT32		TxPktId:8;
}	TXWI_NMAC, *PTXWI_NMAC;
#endif


#define TxWIMPDUByteCnt	TXWI_N.MPDUtotalByteCnt
#define TxWIWirelessCliID	TXWI_N.wcid
#define TxWIFRAG			TXWI_N.FRAG
#define TxWICFACK			TXWI_N.CFACK
#define TxWITS				TXWI_N.TS
#define TxWIAMPDU			TXWI_N.AMPDU
#define TxWIACK				TXWI_N.ACK
#define TxWITXOP			TXWI_N.txop
#define TxWINSEQ			TXWI_N.NSEQ
#define TxWIBAWinSize		TXWI_N.BAWinSize
#define TxWIShortGI			TXWI_N.ShortGI
#define TxWISTBC			TXWI_N.STBC
#define TxWIPacketId		TXWI_N.TxPktId
#define TxWIBW				TXWI_N.BW
#define TxWIMCS				TXWI_N.MCS
#define TxWIPHYMODE		TXWI_N.PHYMODE
#define TxWIMIMOps			TXWI_N.MIMOps
#define TxWIMpduDensity		TXWI_N.MpduDensity
#define TxWILutEn			TXWI_N.lut_en


/*
	Rx Memory layout:

	1. Rx Descriptor
		Rx Descriptor(12 Bytes) + RX_FCE_Info(4 Bytes)
	2. Rx Buffer
		RxInfo(4 Bytes) + RXWI() + 802.11 packet
*/


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RXFCE_INFO{
	UINT32 info_type:2;
	UINT32 s_port:3;
	UINT32 qsel:2;
	UINT32 pcie_intr:1;

	UINT32 mac_len:3;
	UINT32 l3l4_done:1;
	UINT32 pkt_80211:1;
	UINT32 ip_err:1;
	UINT32 tcp_err:1;
	UINT32 udp_err:1;
	
	UINT32 rsv:2;
	UINT32 pkt_len:14;
}RXFCE_INFO;
#else
typedef struct GNU_PACKED _RXFCE_INFO{
	UINT32 pkt_len:14;
	UINT32 rsv:2;

	UINT32 udp_err:1;
	UINT32 tcp_err:1;
	UINT32 ip_err:1;
	UINT32 pkt_80211:1;
	UINT32 l3l4_done:1;
	UINT32 mac_len:3;

	UINT32 pcie_intr:1;
	UINT32 qsel:2;
	UINT32 s_port:3;
	UINT32 info_type:2;
}RXFCE_INFO;
#endif /* RT_BIG_ENDIAN */

#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RXFCE_INFO_CMD{
	UINT32 info_type:2;
	UINT32 d_port:3;
	UINT32 qsel:2;
	UINT32 pcie_intr:1;
	UINT32 evt_type:4;
	UINT32 cmd_seq:4;
	UINT32 self_gen:1;
	UINT32 rsv:1;
	UINT32 pkt_len:14;
}RXFCE_INFO_CMD;
#else
typedef struct GNU_PACKED _RXFCE_INFO_CMD{
	UINT32 pkt_len:14;
	UINT32 rsv:1;
	UINT32 self_gen:1;
	UINT32 cmd_seq:4;
	UINT32 evt_type:4;
	UINT32 pcie_intr:1;
	UINT32 qsel:2;
	UINT32 d_port:3;
	UINT32 info_type:2;
}RXFCE_INFO_CMD;
#endif


#ifdef RT_BIG_ENDIAN
typedef struct GNU_PACKED _RXINFO_NMAC{
	UINT32 ic_err:1;
	UINT32 tc_err:1;
	UINT32 ichk_bps:1;
	UINT32 tchk_bps:1;
	UINT32 rsv0:6;
	UINT32 pn_len:3;
	UINT32 wapi_kid:1;
	UINT32 bssid_idx3:1;
	UINT32 dec:1;
	UINT32 ampdu:1;
	UINT32 l2pad:1;
	UINT32 rssi:1;
	UINT32 htc:1;
	UINT32 amsdu:1;
	UINT32 mic_err:1;
	UINT32 icv_err:1;
	UINT32 crc_err:1;
	UINT32 mybss:1;
	UINT32 bc:1;
	UINT32 mc:1;
	UINT32 u2me:1;
	UINT32 frag:1;
	UINT32 null:1;
	UINT32 data:1;
	UINT32 ba:1;
}RXINFO_NMAC;
#else
typedef struct GNU_PACKED _RXINFO_NMAC{
	UINT32 ba:1;
	UINT32 data:1;
	UINT32 null:1;
	UINT32 frag:1;
	UINT32 u2me:1;
	UINT32 mcast:1;
	UINT32 bcast:1;
	UINT32 mybss:1;
	UINT32 crc_err:1;
	UINT32 icv_err:1;	
	UINT32 mic_err:1;
	UINT32 amsdu:1;
	UINT32 htc:1;
	UINT32 rssi:1;
	UINT32 l2pad:1;
	UINT32 ampdu:1;
	UINT32 dec:1;
	UINT32 bssid_idx3:1;
	UINT32 wapi_kid:1;
	UINT32 pn_len:3;
	UINT32 rsv0:6;
	UINT32 tchk_bps:1;
	UINT32 ichk_bps:1;
	UINT32 tc_err:1;
	UINT32 ic_err:1;
}RXINFO_NMAC;
#endif /* RT_BIG_ENDIAN */


/*
	RXWI wireless information format.
*/
#ifdef RT_BIG_ENDIAN
typedef	struct GNU_PACKED _RXWI_NMAC{
	/* Word 0 */
	UINT32 eof:1;
	UINT32 rsv:1;
	UINT32 MPDUtotalByteCnt:14;  /* mpdu_total_byte = rxfceinfo_len - rxwi_len- rxinfo_len - l2pad */
	UINT32 udf:3;
	UINT32 bss_idx:3;
	UINT32 key_idx:2;
	UINT32 wcid:8;

	/* Word 1 */
	UINT32 phy_mode:3;
	UINT32 i_txbf:1;
	UINT32 e_txbf:1;
	UINT32 stbc:1;
	UINT32 sgi:1;
	UINT32 bw:2;
	UINT32 mcs:7;
	UINT32 sn:12;
	UINT32 tid:4;

	/* Word 2 */
	UINT8 rssi[4];

	/* Word 3~6 */
	UINT8 bbp_rxinfo[16];
}	RXWI_NMAC;
#else
typedef	struct GNU_PACKED _RXWI_NMAC {
	/* Word 0 */
	UINT32 wcid:8;
	UINT32 key_idx:2;
	UINT32 bss_idx:3;
	UINT32 udf:3;
	UINT32 MPDUtotalByteCnt:14; /* mpdu_total_byte = rxfceinfo_len - rxwi_len- rxinfo_len - l2pad */
	UINT32 rsv:1;
	UINT32 eof:1;

	/* Word 1 */
	UINT32 tid:4;
	UINT32 sn:12;
	UINT32 mcs:7;
	UINT32 bw:2;
	UINT32 sgi:1;
	UINT32 stbc:1;
	UINT32 e_txbf:1;
	UINT32 i_txbf:1;
	UINT32 phy_mode:3;

	/* Word 2 */
	UINT8 rssi[4];

	/* Word 3~6 */
	UINT8 bbp_rxinfo[16];
}	RXWI_NMAC;
#endif /* RT_BIG_ENDIAN */


#define RxWIMPDUByteCnt	RXWI_N.MPDUtotalByteCnt
#define RxWIWirelessCliID	RXWI_N.wcid
#define RxWIKeyIndex		RXWI_N.key_idx
#define RxWIMCS				RXWI_N.mcs
#define RxWIBW				RXWI_N.bw
#define RxWIBSSID			RXWI_N.bss_idx
#define RxWISGI				RXWI_N.sgi
#define RxWIPhyMode		RXWI_N.phy_mode
#define RxWISTBC			RXWI_N.stbc
#define RxWITID				RXWI_N.tid
#define RxWIRSSI0			RXWI_N.rssi[0]
#define RxWIRSSI1			RXWI_N.rssi[1]
#define RxWIRSSI2			RXWI_N.rssi[2]
#define RxWIRSSI3			RXWI_N.rssi[3]
#define RxWISNR0			RXWI_N.bbp_rxinfo[0]
#define RxWISNR1			RXWI_N.bbp_rxinfo[1]
#define RxWISNR2			RXWI_N.bbp_rxinfo[2]
#define RxWIFOFFSET			RXWI_N.bbp_rxinfo[3]


typedef struct GNU_PACKED _HW_RATE_CTRL_STRUCT_{
#ifdef RT_BIG_ENDIAN
	UINT16 PHYMODE:3;
	UINT16 iTxBF:1;
	UINT16 eTxBF:1;
	UINT16 STBC:1;
	UINT16 ShortGI:1;
	UINT16 BW:2;			/* channel bandwidth 20/40/80 MHz */
	UINT16 MCS:7;
#else
	UINT16 MCS:7;
	UINT16 BW:2;
	UINT16 ShortGI:1;
	UINT16 STBC:1;
	UINT16 eTxBF:1;
	UINT16 iTxBF:1;
	UINT16 PHYMODE:3;  
#endif /* RT_BIG_ENDIAN */
}HW_RATE_CTRL_STRUCT;


/* ================================================================================= */
/* Register format */
/* ================================================================================= */


#define WLAN_FUN_CTRL		0x80
#ifdef RT_BIG_ENDIAN
typedef union _WLAN_FUN_CTRL_STRUC{
	struct{
		UINT32 GPIO0_OUT_OE_N:8;
		UINT32 GPIO0_OUT:8;
		UINT32 GPIO0_IN:8;
		UINT32 WLAN_ACC_BT:1;
		UINT32 INV_TR_SW0:1;
		UINT32 FRC_WL_ANT_SET:1;
		UINT32 PCIE_APP0_CLK_REQ:1;
		UINT32 WLAN_RESET:1;
		UINT32 WLAN_RESET_RF:1;
		UINT32 WLAN_CLK_EN:1;
		UINT32 WLAN_EN:1;
	}field;
	UINT32 word;
}WLAN_FUN_CTRL_STRUC, *PWLAN_FUN_CTRL_STRUC;
#else
typedef union _WLAN_FUN_CTRL_STRUC{
	struct{
		UINT32 WLAN_EN:1;
		UINT32 WLAN_CLK_EN:1;
		UINT32 WLAN_RESET_RF:1;
		UINT32 WLAN_RESET:1;
		UINT32 PCIE_APP0_CLK_REQ:1;
		UINT32 FRC_WL_ANT_SET:1;
		UINT32 INV_TR_SW0:1;
		UINT32 WLAN_ACC_BT:1;
		UINT32 GPIO0_IN:8;
		UINT32 GPIO0_OUT:8;
		UINT32 GPIO0_OUT_OE_N:8;
	}field;
	UINT32 word;
}WLAN_FUN_CTRL_STRUC, *PWLAN_FUN_CTRL_STRUC;
#endif


#define WLAN_FUN_INFO		0x84
#ifdef RT_BIG_ENDIAN
typedef union _WLAN_FUN_INFO_STRUC{
	struct{
		UINT32		BT_EEP_BUSY:1; /* Read-only for WLAN Driver */
		UINT32		Rsv1:26;		
		UINT32		COEX_MODE:5; /* WLAN function enable */
	}field;
	UINT32 word;
}WLAN_FUN_INFO_STRUC, *PWLAN_FUN_INFO_STRUC;
#else
typedef union _WLAN_FUN_INFO_STRUC{
	struct{
		UINT32		COEX_MODE:5; /* WLAN function enable */
		UINT32		Rsv1:26;
		UINT32		BT_EEP_BUSY:1; /* Read-only for WLAN Driver */
	}field;
	UINT32 word;
}WLAN_FUN_INFO_STRUC, *PWLAN_FUN_INFO_STRUC;
#endif


#define BT_FUN_CTRL		0xC0
#ifdef RT_BIG_ENDIAN
typedef union _BT_FUN_CTRL_STRUC{
	struct{
		UINT32		GPIO1_OUT_OE_N:8;
		UINT32		GPIO1_OUT:8;
		UINT32		GPIO1_IN:8;
		UINT32		BT_ACC_WLAN:1;
		UINT32		INV_TR_SW1:1;
		UINT32		URXD_GPIO_MODE:1;
		UINT32		PCIE_APP1_CLK_REQ:1;
		UINT32		BT_RESET:1;
		UINT32		BT_RF_EN:1;
		UINT32		BT_CLK_EN:1;
		UINT32		BT_EN:1;
	}field;
	UINT32 word;
}BT_FUN_CTRL_STRUC, *PBT_FUN_CTRL_STRUC;
#else
typedef union _BT_FUN_CTRL_STRUC	{
	struct{
		UINT32		BT_EN:1;
		UINT32		BT_CLK_EN:1;
		UINT32		BT_RF_EN:1;
		UINT32		BT_RESET:1;
		UINT32		PCIE_APP1_CLK_REQ:1;
		UINT32		URXD_GPIO_MODE:1;
		UINT32		INV_TR_SW1:1;
		UINT32		BT_ACC_WLAN:1;
		UINT32		GPIO1_IN:8;
		UINT32		GPIO1_OUT:8;
		UINT32		GPIO1_OUT_OE_N:8;
	}field;
	UINT32 word;
}BT_FUN_CTRL_STRUC, *PBT_FUN_CTRL_STRUC;
#endif


#define BT_FUN_INFO		0xC4
#ifdef RT_BIG_ENDIAN
typedef union _BT_FUN_INFO_STRUC{
	struct{
		UINT32		WLAN_EEP_BUSY:1;
		UINT32		BTPower1:7;	/* Peer */
		UINT32		BTPower0:8; /* Self */
		UINT32		AFH_END_CH:8;		
		UINT32		AFH_START_CH:8;
	}field;
	UINT32 word;
} BT_FUN_INFO_STRUC, *PBT_FUN_INFO_STRUC;
#else
typedef	union _BT_FUN_INFO_STRUC	{
	struct{
		UINT32		AFH_START_CH:8;
		UINT32		AFH_END_CH:8;
		UINT32		BTPower0:8;	/* Self */
		UINT32		BTPower1:7;	/* Peer */
		UINT32		WLAN_EEP_BUSY:1;			
	}field;
	UINT32 word;
}BT_FUN_INFO_STRUC, *PBT_FUN_INFO_STRUC;
#endif

// TODO: shiang, this data structure is not defined for register. may can move to other place
typedef struct _WLAN_BT_COEX_SETTING
{
	BOOLEAN					ampduOff;
	BOOLEAN					coexSettingRunning;
	BOOLEAN					RateSelectionForceToUseRSSI;
	UCHAR					TxQualityFlag;
	ULONG					alc;
	ULONG					slna;
}WLAN_BT_COEX_SETTING, *PWLAN_BT_COEX_SETTING;


#define MCU_CMD_CFG 0x0234


#define TSO_CTRL	0x0250
#ifdef RT_BIG_ENDIAN
typedef union _TSO_CTRL_STRUC {
	struct {
		UINT32 rsv:13;
		UINT32 TSO_WR_LEN_EN:1;
		UINT32 TSO_SEG_EN:1;
		UINT32 TSO_EN:1;
		UINT32 RXWI_LEN:4;
		UINT32 RX_L2_FIX_LEN:4;
		UINT32 TXWI_LEN:4;
		UINT32 TX_L2_FIX_LEN:4;
	} field;
	UINT32 word;
} TSO_CTRL_STRUC;
#else
typedef union _TSO_CTRL_STRUC {
	struct {
		UINT32 TX_L2_FIX_LEN:4;
		UINT32 TXWI_LEN:4;
		UINT32 RX_L2_FIX_LEN:4;
		UINT32 RXWI_LEN:4;
		UINT32 TSO_EN:1;
		UINT32 TSO_SEG_EN:1;
		UINT32 TSO_WR_LEN_EN:1;
		UINT32 rsv:13;
	} field;
	UINT32 word;
} TSO_CTRL_STRUC;
#endif /* RT_BIG_ENDIAN */


#define TX_PROT_CFG6    0x13E0    // VHT 20 Protection
#define TX_PROT_CFG7    0x13E4    // VHT 40 Protection
#define TX_PROT_CFG8    0x13E8    // VHT 80 Protection
#define PIFS_TX_CFG     0x13EC    // PIFS setting

//----------------------------------------------------------------
// Header Translation 
//----------------------------------------------------------------

/* 
	WIFI INFO for TX translation 
	|TXINO|TXWI|WIFI INFO|802.3 MAC Header|Pyaload| 
*/

#ifdef HDR_TRANS_SUPPORT
#define WIFI_INFO_SIZE		4
#else
#define WIFI_INFO_SIZE		0
#endif
#ifdef RT_BIG_ENDIAN
typedef union GNU_PACKED _WIFI_INFO_STRUC {
	struct {
    	UINT32 More_Data:1;
    	UINT32 WEP:1;
    	UINT32 PS:1;
    	UINT32 RDG:1;
    	UINT32 QoS:1;
    	UINT32 EOSP:1;
    	UINT32 TID:4;
    	UINT32 Mode:2;
    	UINT32 VLAN:1;
    	UINT32 Rsv:3;
    	UINT32 BssIdx:4;
    	UINT32 Seq_Num:12;
	} field;
	UINT32 word;
} WIFI_INFO_STRUC, *PWIFI_INFO_STRUC;
#else
typedef union GNU_PACKED _WIFI_INFO_STRUC {
	struct {
    	UINT32 Seq_Num:12;
    	UINT32 BssIdx:4;
    	UINT32 Rsv:3;
    	UINT32 VLAN:1;
    	UINT32 Mode:2;
    	UINT32 TID:4;
    	UINT32 EOSP:1;
    	UINT32 QoS:1;
    	UINT32 RDG:1;
    	UINT32 PS:1;
    	UINT32 WEP:1;
    	UINT32 More_Data:1;
	} field;
	UINT32 word;
} WIFI_INFO_STRUC, *PWIFI_INFO_STRUC;
#endif /* RT_BIG_ENDIAN */

/* 
	header translation control register 
	bit0 --> TX translation enable
	bit1 --> RX translation enable 
*/
#define HEADER_TRANS_CTRL_REG	0x0260
#define HT_TX_ENABLE			0x1
#define HT_RX_ENABLE			0x2

#define HT_MAC_ADDR_DW0		0x02A4
#define HT_MAC_ADDR_DW1		0x02A8
#define HT_MAC_BSSID_DW0		0x02AC
#define HT_MAC_BSSID_DW1		0x02B0

#ifdef RT_BIG_ENDIAN
typedef union GNU_PACKED _HDR_TRANS_CTRL_STRUC {
	struct {
    	UINT32 Rsv:30;
    	UINT32 Rx_En:1;
    	UINT32 Tx_En:1;
	} field;
	UINT32 word;	
} HDR_TRANS_CTRL_STRUC, *PHDR_TRANS_CTRL_STRUC;
#else
typedef union GNU_PACKED _HDR_TRANS_CTRL_STRUC {
	struct {
    	UINT32 Tx_En:1;
    	UINT32 Rx_En:1;
    	UINT32 Rsv:30;
	} field;
	UINT32 word;
} HDR_TRANS_CTRL_STRUC, *PHDR_TRANS_CTRL_STRUC;
#endif /* RT_BIG_ENDIAN */

/* RX header translation enable by WCID */ 
#define HT_RX_WCID_EN_BASE	0x0264
#define HT_RX_WCID_OFFSET	32
#if defined(RT63xx)
#define HT_RX_WCID_SIZE		(HT_RX_WCID_OFFSET * 4)	/* 128 WCIDs */
#elif defined(RT65xx)
#define HT_RX_WCID_SIZE		(HT_RX_WCID_OFFSET * 8)	/*	256 WCIDs */
#endif /* defined(RT63xx) */
#ifdef RT_BIG_ENDIAN
typedef union GNU_PACKED _HT_RX_WCID_EN_STRUC {
	struct {
    	UINT32 RX_WCID31_TRAN_EN:1;
    	UINT32 RX_WCID30_TRAN_EN:1;
    	UINT32 RX_WCID29_TRAN_EN:1;
    	UINT32 RX_WCID28_TRAN_EN:1;
    	UINT32 RX_WCID27_TRAN_EN:1;
    	UINT32 RX_WCID26_TRAN_EN:1;
    	UINT32 RX_WCID25_TRAN_EN:1;
    	UINT32 RX_WCID24_TRAN_EN:1;
    	UINT32 RX_WCID23_TRAN_EN:1;
    	UINT32 RX_WCID22_TRAN_EN:1;
    	UINT32 RX_WCID21_TRAN_EN:1;
    	UINT32 RX_WCID20_TRAN_EN:1;
    	UINT32 RX_WCID19_TRAN_EN:1;
    	UINT32 RX_WCID18_TRAN_EN:1;
    	UINT32 RX_WCID17_TRAN_EN:1;
    	UINT32 RX_WCID16_TRAN_EN:1;
    	UINT32 RX_WCID15_TRAN_EN:1;
    	UINT32 RX_WCID14_TRAN_EN:1;
    	UINT32 RX_WCID13_TRAN_EN:1;
    	UINT32 RX_WCID12_TRAN_EN:1;
    	UINT32 RX_WCID11_TRAN_EN:1;
    	UINT32 RX_WCID10_TRAN_EN:1;
    	UINT32 RX_WCID9_TRAN_EN:1;
    	UINT32 RX_WCID8_TRAN_EN:1;
    	UINT32 RX_WCID7_TRAN_EN:1;
    	UINT32 RX_WCID6_TRAN_EN:1;
    	UINT32 RX_WCID5_TRAN_EN:1;
    	UINT32 RX_WCID4_TRAN_EN:1;
    	UINT32 RX_WCID3_TRAN_EN:1;
    	UINT32 RX_WCID2_TRAN_EN:1;
    	UINT32 RX_WCID1_TRAN_EN:1;
    	UINT32 RX_WCID0_TRAN_EN:1;
	} field;
	UINT32 word;	
} HT_RX_WCID_EN_STRUC, *PHT_RX_WCID_EN_STRUC;
#else
typedef union GNU_PACKED _HT_RX_WCID_EN_STRUC {
	struct {
    	UINT32 RX_WCID0_TRAN_EN:1;
    	UINT32 RX_WCID1_TRAN_EN:1;
    	UINT32 RX_WCID2_TRAN_EN:1;
    	UINT32 RX_WCID3_TRAN_EN:1;
    	UINT32 RX_WCID4_TRAN_EN:1;
    	UINT32 RX_WCID5_TRAN_EN:1;
    	UINT32 RX_WCID6_TRAN_EN:1;
    	UINT32 RX_WCID7_TRAN_EN:1;
    	UINT32 RX_WCID8_TRAN_EN:1;
    	UINT32 RX_WCID9_TRAN_EN:1;
    	UINT32 RX_WCID10_TRAN_EN:1;
    	UINT32 RX_WCID11_TRAN_EN:1;
    	UINT32 RX_WCID12_TRAN_EN:1;
    	UINT32 RX_WCID13_TRAN_EN:1;
    	UINT32 RX_WCID14_TRAN_EN:1;
    	UINT32 RX_WCID15_TRAN_EN:1;
    	UINT32 RX_WCID16_TRAN_EN:1;
    	UINT32 RX_WCID17_TRAN_EN:1;
    	UINT32 RX_WCID18_TRAN_EN:1;
    	UINT32 RX_WCID19_TRAN_EN:1;
    	UINT32 RX_WCID20_TRAN_EN:1;
    	UINT32 RX_WCID21_TRAN_EN:1;
    	UINT32 RX_WCID22_TRAN_EN:1;
    	UINT32 RX_WCID23_TRAN_EN:1;
    	UINT32 RX_WCID24_TRAN_EN:1;
    	UINT32 RX_WCID25_TRAN_EN:1;
    	UINT32 RX_WCID26_TRAN_EN:1;
    	UINT32 RX_WCID27_TRAN_EN:1;
    	UINT32 RX_WCID28_TRAN_EN:1;
    	UINT32 RX_WCID29_TRAN_EN:1;
    	UINT32 RX_WCID30_TRAN_EN:1;
    	UINT32 RX_WCID31_TRAN_EN:1;
	} field;
	UINT32 word;
} HT_RX_WCID_EN_STRUC, *PHT_RX_WCID_EN_STRUC;
#endif /* RT_BIG_ENDIAN */

/* RX header translation - black list */
#define HT_RX_BL_BASE		0x0284
#define HT_RX_BL_OFFSET		2
#define HT_RX_BL_SIZE		8
#ifdef RT_BIG_ENDIAN
typedef union GNU_PACKED _HT_RX_BLACK_LIST_STRUC {
	struct {
    	UINT32 BLACK_ETHER_TYPE1:16;
    	UINT32 BLACK_ETHER_TYPE0:16;
	} field;
	UINT32 word;	
} HT_RX_BLACK_LIST_STRUC, *PHT_RX_BLACK_LIST_STRUC;
#else
typedef union GNU_PACKED _HT_RX_BLACK_LIST_STRUC {
	struct {
    	UINT32 BLACK_ETHER_TYPE0:16;
    	UINT32 BLACK_ETHER_TYPE1:16;
	} field;
	UINT32 word;
} HT_RX_BLACK_LIST_STRUC, *PHT_RX_BLACK_LIST_STRUC;
#endif /* RT_BIG_ENDIAN */

/* RX VLAN Mapping (TCI) */
#define HT_BSS_VLAN_BASE	0x0294
#define HT_BSS_VLAN_OFFSET	2
#define HT_BSS_VLAN_SIZE	8
#ifdef RT_BIG_ENDIAN
typedef union GNU_PACKED _HT_BSS_VLAN_STRUC {
	struct {
    	UINT32 TCI1_VID:12;
    	UINT32 TCI1_CFI:1;
    	UINT32 TCI1_PCP:3;
    	UINT32 TCI0_VID:12;
    	UINT32 TCI0_CFI:1;
    	UINT32 TCI0_PCP:3;
	} field;
	UINT32 word;	
} HT_BSS_VLAN_STRUC, *PHT_BSS_VLAN_STRUC;
#else
typedef union GNU_PACKED _HT_BSS_VLAN_STRUC {
	struct {
    	UINT32 TCI0_PCP:3;
    	UINT32 TCI0_CFI:1;
    	UINT32 TCI0_VID:12;
    	UINT32 TCI1_PCP:3;
    	UINT32 TCI1_CFI:1;
    	UINT32 TCI1_VID:12;
	} field;
	UINT32 word;
} HT_BSS_VLAN_STRUC, *PHT_BSS_VLAN_STRUC;
#endif /* RT_BIG_ENDIAN */


// TODO: shiang-6590, following definitions are dummy and not used for RT6590, shall remove/correct these!
#define GPIO_CTRL_CFG	0x0228
#define PBF_MAX_PCNT	0x0408 //actually, it's the TX_MAX_PCNT
// TODO:shiang-6590 --------------------------


#define PAIRWISE_KEY_TABLE_BASE     0x8000      /* 32-byte * 256-entry =  -byte */
#define HW_KEY_ENTRY_SIZE           0x20

#define PAIRWISE_IVEIV_TABLE_BASE     0xa000      /* 8-byte * 256-entry =  -byte */
#define MAC_IVEIV_TABLE_BASE     0xa000      /* 8-byte * 256-entry =  -byte */
#define HW_IVEIV_ENTRY_SIZE   8

#define MAC_WCID_ATTRIBUTE_BASE     0xa800      /* 4-byte * 256-entry =  -byte */
#define HW_WCID_ATTRI_SIZE   4

#define SHARED_KEY_TABLE_BASE       0xac00      /* 32-byte * 16-entry = 512-byte */
#define SHARED_KEY_MODE_BASE       0xb000      /* 32-byte * 16-entry = 512-byte */

#define SHARED_KEY_TABLE_BASE_EXT      0xb400      /* for BSS_IDX=8~15, 32-byte * 16-entry = 512-byte */
#define SHARED_KEY_MODE_BASE_EXT       0xb3f0      /* for BSS_IDX=8~15, 32-byte * 16-entry = 512-byte */

#define HW_SHARED_KEY_MODE_SIZE   4
#define SHAREDKEYTABLE			0
#define PAIRWISEKEYTABLE			1

/* This resgiser is ONLY be supported for RT3883 or later.
   It conflicted with BCN#0 offset of previous chipset. */
#define WAPI_PN_TABLE_BASE		0xb800		
#define WAPI_PN_ENTRY_SIZE   		8

#define RF_MISC	0x0518
#ifdef RT_BIG_ENDIAN
typedef union _RF_MISC_STRUC{
	struct{
		UINT32 Rsv1:29;
		UINT32 EXT_PA_EN:1;
		UINT32 ADDAC_LDO_ADC9_EN:1;
		UINT32 ADDAC_LDO_ADC6_EN:1;
	}field;
	UINT32 word;
}RF_MISC_STRUC, *PRF_MISC_STRUC;
#else
typedef union _RF_MISC_STRUC{
	struct{
		UINT32 ADDAC_LDO_ADC6_EN:1;
		UINT32 ADDAC_LDO_ADC9_EN:1;
		UINT32 EXT_PA_EN:1;
		UINT32 Rsv1:29;
	}field;
	UINT32 word;
}RF_MISC_STRUC, *PRF_MISC_STRUC;
#endif

VOID ral_wlan_chip_onoff(
	IN struct _RTMP_ADAPTER *pAd,
	IN BOOLEAN bOn,
	IN BOOLEAN bResetWLAN);

#define AUX_CLK_CFG		0x120C
#define BB_PA_MODE_CFG0	0x1214
#define BB_PA_MODE_CFG1	0x1218
#define RF_PA_MODE_CFG0 0x121C
#define RF_PA_MODE_CFG1	0x1220
#define TX_ALC_CFG_0	0x13B0
#define TX_ALC_CFG_0_CH_INT_0_MASK (0x3f)
#define TX_ALC_CFG_0_CH_INT_0(p) ((p) & 0x3f)
#define TX_ALC_CFG_0_CH_INT_1_MASK ((0x3f) << 8)
#define TX_ALC_CFG_0_CH_INT_1(p) (((p) & 0x3f) << 8)

#define TX_ALC_CFG_1	0x13B4

#define TX0_RF_GAIN_CORR	0x13A0
#define TX0_RF_GAIN_ATTEN	0x13A8
#define TX0_BB_GAIN_ATTEN	0x13C0
#define TX_ALC_VGA3			0x13C8

#ifdef RT65xx
#define CPU_CTL				0x0704
#define RESET_CTL			0x070C
#define INT_LEVEL			0x0718
#define COM_REG0			0x0730
#define COM_REG1			0x0734
#define COM_REG2			0x0738
#define COM_REG3			0x073C
#define PCIE_REMAP_BASE1	0x0740
#define PCIE_REMAP_BASE2	0x0744
#define PCIE_REMAP_BASE3	0x0748
#define	PCIE_REMAP_BASE4	0x074C
#define LED_CTRL			0x0770
#define LED_TX_BLINK_0		0x0774
#define	LED_TX_BLINK_1		0x0778
#define LED0_S0				0x077C
#define LED0_S1				0x0780
#define SEMAPHORE_00		0x07B0
#endif /* RT65xx */

#define APCLI_BSSID_IDX			8
#define MAC_APCLI_BSSID_DW0		0x1090
#define MAC_APCLI_BSSID_DW1		0x1094

#ifdef MAC_REPEATER_SUPPORT
#define MAC_ADDR_EXT_EN			0x147C
#define MAC_ADDR_EXT0_31_0		0x1480
#define MAC_ADDR_EXT0_47_32		0x1484
#define MAX_EXT_MAC_ADDR_SIZE	16

#define UNKOWN_APCLI_IDX		0xFF
#define CLIENT_APCLI			0x00
#define CLIENT_STA				0x01
#define CLIENT_ETH				0x02
#define EXTERNDER_CLI			0x08
#endif /* MAC_REPEATER_SUPPORT */

#endif /* __RAL_NMAC_H__ */

