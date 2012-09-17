/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
    dot11z_tdls.h
 
    Abstract:
	Defined status code, IE and frame structures that TDLS (802.11zD4.0) needed.
 
    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------
    Arvin Tai  17-04-2009    created for 11z
 */

#ifdef DOT11Z_TDLS_SUPPORT
#include "rtmp_type.h"

#ifndef __DOT11Z_TDLS_H
#define __DOT11Z_TDLS_H

#define GNU_PACKED  __attribute__ ((packed))

// TDLS definitions
#define	PROTO_NAME_TDLS				3	// <ANA>
#define	TDLS_AKM_SUITE_1X			5	// <ANA>
#define	TDLS_AKM_SUITE_PSK			6	// <ANA>

// TDLS Action frame definition
#define TDLS_ACTION_CODE_SETUP_REQUEST				0
#define TDLS_ACTION_CODE_SETUP_RESPONSE				1
#define TDLS_ACTION_CODE_SETUP_CONFIRM				2
#define TDLS_ACTION_CODE_TEARDOWN					3
#define TDLS_ACTION_CODE_PEER_TRAFFIC_INDICATION	4
#define TDLS_ACTION_CODE_CHANNEL_SWITCH_REQUEST		5
#define TDLS_ACTION_CODE_CHANNEL_SWITCH_RESPONSE	6
#define TDLS_ACTION_CODE_PEER_PSM_REQUEST			7
#define TDLS_ACTION_CODE_PEER_PSM_RESPONSE			8
#define TDLS_ACTION_CODE_AP_PHY_DATA_RATE_REQUEST	9
#define TDLS_ACTION_CODE_AP_PHY_DATA_RATE_RESPONSE	10

/* Status codes defined in 802.11zD4.0 specification. */
#define TDLS_STATUS_CODE_WAKEUP_SCHEDULE_REJECT_BUT_ALTERNATIVE_SCHEDULE_PROVIDED	1
#define TDLS_STATUS_CODE_WAKEUP_SCHEDULE_REJECT										2
#define TDLS_STATUS_CODE_DIRECT_LINK_NOT_ALLOW_BY_THE_BSS							3
#define TDLS_STATUS_CODE_SECURITY_DISABLED											4
#define TDLS_STATUS_CODE_UNACCEPTABLE_LIFETIME										5

/* Reason codes defined in 802.11zD4.0 specification. */
#define TDLS_REASON_CODE_TEARDOWN_DUE_TO_UNACCEPTABLE_FRAME_LOSS					50
#define TDLS_REASON_CODE_TEARDOWN_FOR_UNSPECIFIED_REASON							51

/* Information element ID defined in 802.11zD4.0 specification. */
#define IE_TDLS_LINK_IDENTIFIER			80
#define IE_TDLS_WAKEUP_SCHEDULE			81
#define IE_TDLS_AP_PHY_DATA_RATE		82
#define IE_TDLS_CHANNEL_SWITCH_TIMING	83

#define TDLS_ELM_LEN_LINK_IDENTIFIER		20
#define TDLS_ELM_LEN_WAKEUP_SCHEDULE		12
#define TDLS_ELM_LEN_AP_PHY_DATA_RATE		6
#define TDLS_ELM_LEN_CHANNEL_SWITCH_TIMING	6

#define TDLS_KEY_TIMEOUT			3600      // unit: sec

typedef struct GNU_PACKED _TDLS_LINK_IDENTIFIER_IE
{
	UCHAR	BSSID[MAC_ADDR_LEN];
	UCHAR	InitSTA_MAC[MAC_ADDR_LEN];
	UCHAR	RespSTA_MAC[MAC_ADDR_LEN];
} TDLS_LINK_IDENTIFIER_IE, *PTDLS_LINK_IDENTIFIER_IE;

typedef struct GNU_PACKED _TDLS_WAKEUP_SCHEDULE_IE
{
	UINT32	Interval;
	UINT32	AwakeDuration;
	UINT16	IdleCount;
} TDLS_WAKEUP_SCHEDULE_IE, *PTDLS_WAKEUP_SCHEDULE_IE;

typedef struct GNU_PACKED _TDLS_AP_PHY_DATA_RATE_IE
{
	UINT32	AP_PHY_Data_Rate;
} TDLS_AP_PHY_DATA_RATE_IE, *PTDLS_AP_PHY_DATA_RATE_IE;

typedef struct GNU_PACKED _TDLS_CHANNEL_SWITCH_TIMING_IE
{
	UINT16	SwitchTime;
	UINT16	SwitchTimeout;
} TDLS_CHANNEL_SWITCH_TIMING_IE, *PTDLS_CHANNEL_SWITCH_TIMING_IE;

#endif /* __DOT11Z_TDLS_H */

#endif // DOT11Z_TDLS_SUPPORT //

