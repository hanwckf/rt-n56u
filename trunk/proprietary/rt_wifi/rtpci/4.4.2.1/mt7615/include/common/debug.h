/****************************************************************************
 * Mediatek Inc.
 * 5F., No.5, Taiyuan 1st St., Zhubei City,
 * Hsinchu County 302, Taiwan, R.O.C.
 * (c) Copyright 2014, Mediatek, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
	****************************************************************************

    Module Name:
        debug.h

    Abstract:
        All function prototypes and macro are provided from debug message.

    Revision History:
    Who             When            What
    ---------    ----------    ----------------------------------------------
    Name           Date              Modification logs
    UnifyLOGTF   2014.07.11     Initial version

***************************************************************************/

#ifndef __DEBUG_H__
#define __DEBUG_H__


/* */
/*  Debug information verbosity: lower values indicate higher urgency */
/* */

/* Debug Level */
#define DBG_LVL_OFF		0
#define DBG_LVL_ERROR	1
#define DBG_LVL_WARN	2
#define DBG_LVL_TRACE	3
#define DBG_LVL_INFO	4
#define DBG_LVL_LOUD	5
#define DBG_LVL_NOISY	6
#define DBG_LVL_MAX		DBG_LVL_NOISY
#if !defined(EVENT_TRACING)
/* Debug Category */
#define DBG_CAT_INIT    0x00000001u /* initialization/shutdown */
#define DBG_CAT_HW      0x00000002u /* MAC/BBP/RF/Chip */
#define DBG_CAT_FW      0x00000004u /* FW related command, response, CR that FW care about */
#define DBG_CAT_HIF     0x00000008u /* Host interface: usb/sdio/pcie/rbus */
#define DBG_CAT_FPGA    0x00000010u /* FPGA Chip verify, DVT */
#define DBG_CAT_TEST    0x00000020u /* ATE, QA, UT, FPGA?, TDT, SLT, WHQL, and other TEST */
#define DBG_CAT_RA      0x00000040u /* Rate Adaption/Throughput related */
#define DBG_CAT_AP      0x00000080u /* AP, MBSS, WDS */
#define DBG_CAT_CLIENT  0x00000100u /* STA, ApClient, AdHoc, Mesh */
#define DBG_CAT_TX      0x00000200u /* Tx data path */
#define DBG_CAT_RX      0x00000400u /* Rx data path */
#define DBG_CAT_CFG     0x00000800u /* ioctl/oid/profile/cfg80211/Registry */
#define DBG_CAT_MLME    0x00001000u /* 802.11 fundamental connection flow, auth, assoc, disconnect, etc */
#define DBG_CAT_PROTO   0x00002000u /* protocol, ex. TDLS */
#define DBG_CAT_SEC     0x00004000u /* security/key/WPS/WAPI/PMF/11i related*/
#define DBG_CAT_PS      0x00008000u /* power saving/UAPSD */
#define DBG_CAT_POWER   0x00010000u /* power Setting, Single Sku, Temperature comp, etc */
#define DBG_CAT_COEX    0x00020000u /* BT, BT WiFi Coex, LTE, TVWS*/
#define DBG_CAT_P2P     0x00040000u /* P2P, Miracast */
#define DBG_CAT_TOKEN	0x00080000u
#define DBG_CAT_CMW     0x00100000u /* CMW Link Test */
#define DBG_CAT_RSV1    0x40000000u /* reserved index for code development */
#define DBG_CAT_RSV2    0x80000000u /* reserved index for code development */
#define DBG_CAT_ALL     0xFFFFFFFFu
#endif

/* Debug SubCategory */
#define DBG_SUBCAT_ALL	0xFFFFFFFFu

/* Sub-Category of  DBG_CAT_HW */
#define CATHW_SA		0x00000001u	/* debug flag for smart antenna */

/* Sub-Category of  DBG_CAT_HIF */
#define CATHIF_PCI		0x00000001u
#define CATHIF_USB		0x00000002u
#define CATHIF_SDIO		0x00000004u

/* Sub-Category of  DBG_CAT_AP */
#define CATAP_MBSS		0x00000001u
#define CATAP_WDS		0x00000002u

/* Sub-Category of  DBG_CAT_CLIENT */
#define CATCLIENT_ADHOC	0x00000001u
#define CATCLIENT_APCLI	0x00000002u
#define CATCLIENT_MESH	0x00000004u

/* Sub-Category of  DBG_CAT_TX */
#define CATTX_TMAC		0x00000001u	/* debug flag for tmac info dump */

/*  Sub-Category of DBG_CAT_TOKEN */
#define TOKEN_INFO		0x00000001u
#define TOKEN_PROFILE	0x00000002u
#define TOKEN_TRACE		0x00000004u

/* Sub-Category of  DBG_CAT_PROTO */
#define CATPROTO_ACM	0x00000001u
#define CATPROTO_BA	0x00000002u
#define CATPROTO_TDLS	0x00000004u
#define CATPROTO_WNM	0x00000008u
#define CATPROTO_IGMP	0x00000010u
#define CATPROTO_MAT	0x00000020u
#define CATPROTO_RRM	0x00000040u
#define CATPROTO_DFS	0x00000080u
#define CATPROTO_FT	0x00000100u
#define CATPROTO_SCAN	0x00000200u
#define CATPROTO_FTM    0x00000400u

/* Sub-Category of  DBG_CAT_SEC */
#define CATSEC_KEY	    0x00000001u
#define CATSEC_WPS	    0x00000002u
#define CATSEC_WAPI	    0x00000004u
#define CATSEC_PMF	    0x00000008u

/* Sub-Category of  DBG_CAT_PS */
#define CATPS_UAPSD		0x00000001u


/* The bitmap of enabled debug categories */
#define DBG_CAT_EN_BITMAP	(0|\
		DBG_CAT_INIT	|\
		DBG_CAT_HW		|\
		DBG_CAT_FW		|\
		DBG_CAT_HIF		|\
		DBG_CAT_FPGA	|\
		DBG_CAT_TEST	|\
		DBG_CAT_RA		|\
		DBG_CAT_AP		|\
		DBG_CAT_CLIENT	|\
		DBG_CAT_TX		|\
		DBG_CAT_RX		|\
		DBG_CAT_CFG		|\
		DBG_CAT_MLME	|\
		DBG_CAT_PROTO	|\
		DBG_CAT_SEC		|\
		DBG_CAT_PS		|\
		DBG_CAT_POWER	|\
		DBG_CAT_COEX	|\
		DBG_CAT_P2P		|\
		DBG_CAT_TOKEN	|\
		DBG_CAT_CMW     |\
		DBG_CAT_RSV1	|\
		DBG_CAT_RSV2)


/***********************************************************************************
 *	Debugging and printing related definitions and prototypes
 ***********************************************************************************/
#define PRINT_MAC(addr)	\
	addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]

extern int			DebugLevel;
#ifdef DBG
extern ULONG		DebugCategory;
extern ULONG		DebugSubCategory[];

static inline ULONG GET_BIT(ULONG pwr_of_2)
{
	ULONG bit = 0;
	if (pwr_of_2) {
		for (bit = 0; bit < 31; bit ++) {
			if ((pwr_of_2 & 1) == 0)
				pwr_of_2 >>= 1;
			else
				break;
		}
	}
	return bit;
}

#define MTWF_LOG(Category, SubCategory, Level, Fmt)	\
do{	\
	if ((Category) & DBG_CAT_EN_BITMAP) {	\
		if ((Category) & (DebugCategory)) {	\
			if (Level <= DebugLevel) { \
				if ((SubCategory) == DBG_SUBCAT_ALL)	\
					MTWF_PRINT Fmt;	\
				else {	\
					ULONG bit = GET_BIT(Category);	\
					if ((DebugSubCategory[bit] == DBG_SUBCAT_ALL) || \
						((SubCategory) & DebugSubCategory[bit]))	\
						MTWF_PRINT Fmt; \
				}	\
			}	\
		}	\
	}	\
}while(0)

#else
#define MTWF_LOG(Category, SubCategory, Level, Fmt)
#endif

void hex_dump(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen);



#endif /* __DEBUG_H__ */

