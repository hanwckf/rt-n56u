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
    dot11r_ft.h
 
    Abstract:
	Defined status code, IE and frame structures that FT (802.11rD9.0) needed.
 
    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------
    Fonchi Wu  12-02-2008    created for 11r soft-AP
 */


#ifndef __DOT11R_FT_H
#define __DOT11R_FT_H

#define GNU_PACKED  __attribute__ ((packed))


#if defined(DOT11R_FT_SUPPORT) || defined(DOT11Z_TDLS_SUPPORT)
#define FT_MIC_LEN					16
#define FT_NONCE_LEN				32			
#endif


#if defined(DOT11R_FT_SUPPORT) || defined(DOT11Z_TDLS_SUPPORT)
/* Information element ID defined in 802.11rD9.0 specification. */
#define IE_FT_MDIE				54
#define IE_FT_FTIE				55
#define IE_FT_TIMEOUT_INTERVAL	56
#define IE_FT_RIC_DATA			57
#define IE_FT_RIC_DESCRIPTOR	75


/* RIC Type */
#define FT_RIC_TYPE_BA			1

/* AKM SUITE */
#define FT_AKM_SUITE_1X		3
#define FT_AKM_SUITE_PSK	4
#endif


#if defined(DOT11R_FT_SUPPORT) || defined(DOT11Z_TDLS_SUPPORT)
typedef union GNU_PACKED _FT_MIC_CTR_FIELD
{
	/*
		IECnt: contains the number of IEs
		that are included int eht MIC calculation.
	*/
	struct GNU_PACKED
	{
#ifdef RT_BIG_ENDIAN
	UINT16 IECnt:8;
	UINT16 :8;
#else
	UINT16 :8;
	UINT16 IECnt:8;
#endif
	} field;
	UINT16 word;
} FT_MIC_CTR_FIELD, *PFT_MIC_CTR_FIELD;

/*
** FTIE: Fast Transition IE.
*/
typedef struct GNU_PACKED _FT_FTIE
{
	FT_MIC_CTR_FIELD MICCtr;		/* 2 Octects. */
	UINT8 MIC[FT_MIC_LEN];			/* 16 Octects. */
	UINT8 ANonce[FT_NONCE_LEN];		/* 32 Octects. */
	UINT8 SNonce[FT_NONCE_LEN];		/* 32 Octects. */
	UINT8 Option[0];				/* 1:R1KHID, 2:GTK, 3:ROKHId, else:Res */ 
} FT_FTIE, *PFT_FTIE;
#endif


#if defined(DOT11R_FT_SUPPORT) || defined(DOT11Z_TDLS_SUPPORT)
/*
** Timeout Interval IE.
*/
typedef enum _FT_TIMEOUT_INTERVAL_TYPE
{
	REASSOC_DEADLINE_INTERVAL = 1,	/* TUs */
	KEY_LIFETIME_INTERVAL,				/* seconds. */
	RESERVED_INTERVAL
} FT_TIMEOUT_INTERVAL_TYPE, *PFT_TIMEOUT_INTERVAL_TYPE;

typedef struct GNU_PACKED _FT_TIMEOUT_INTERVAL_IE
{
	UINT8 TimeoutIntervalType;
	UINT32 TimeoutIntervalValue;
} FT_TIMEOUT_INTERVAL_IE, *PFT_TIMEOUT_INTERVAL_IE;
#endif


#endif /* __DOT11R_FT_H */

