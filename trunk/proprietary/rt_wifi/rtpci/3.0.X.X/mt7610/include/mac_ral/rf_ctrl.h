/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering	the source code	is stricitly prohibited, unless	the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rf_ctrl.h

	Abstract:
	Ralink wireless chip RF related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef __RF_CTRL_H__
#define __RF_CTRL_H__

#ifdef RTMP_MAC
#include "mac_ral/omac/ral_omac_rf_ctrl.h"
#endif /* RTMP_MAC */

#ifdef RLT_MAC

#endif /* RLT_MAC */


/* ================================================================================= */
/* Register format  for RFCTRL                                                                                                                                               */
/* ================================================================================= */

#define	RF_CSR_CFG	0x500

#ifdef RLT_RF

#define RF_BANK0	0
#define RF_BANK1	1
#define RF_BANK2	2
#define RF_BANK3	3
#define RF_BANK4	4
#define RF_BANK5	5
#define RF_BANK6	6
#define RF_BANK7	7
#define RF_BANK8	8
#define RF_BANK9	9
#define RF_BANK10	10
#define RF_BANK11	11
#define RF_BANK12	12
#define RF_BANK13	13
#define RF_BANK14	14
#define RF_BANK15	15

/* @!Release
	RF_CSR_KICK:1
			Write - kick RF register read/write
				0: do nothing        
				1: kick read/write process
			Read - Polling RF register read/write
				0: idle
				1: busy
	RF_CSR_RW:1
			0: read  1: write
	rsv:12
	RF_CSR_REG_ID:10
			RF register ID, 0 for R0, 1 for R1 and so on
				Bits [17:15] 3 bits, indicates the bank number
				Bits [14:08] 7 bits, indicates the register number

	RF_CSR_DATA:8
			DATA written to/read from RF
*/
typedef	union _RLT_RF_CSR_CFG {
#ifdef RT_BIG_ENDIAN
	struct {
		UINT RF_CSR_KICK:1;
		UINT RF_CSR_WR:1;
		UINT rsv:12;
		UINT RF_CSR_REG_BANK:3;
		UINT RF_CSR_REG_ID:7;
		UINT RF_CSR_DATA:8;
	} field;
#else
	struct {
		UINT RF_CSR_DATA:8;
		UINT RF_CSR_REG_ID:7;
		UINT RF_CSR_REG_BANK:3;
		UINT rsv:12;
		UINT RF_CSR_WR:1;
		UINT RF_CSR_KICK:1;
	} field;
#endif /* RT_BIG_ENDIAN */
	UINT word;
}RLT_RF_CSR_CFG;
#endif /* RLT_RF */


typedef	union _RF_CSR_CFG_STRUC {
#ifdef RT_BIG_ENDIAN
	struct {
		UINT32	Rsvd1:14;				/* Reserved */
		UINT32	RF_CSR_KICK:1;			/* kick RF register read/write */
		UINT32	RF_CSR_WR:1;			/* 0: read  1: write */
		UINT32	TESTCSR_RFACC_REGNUM:8;	/* RF register ID */
		UINT32	RF_CSR_DATA:8;			/* DATA */
	} field;
#else
	struct {
		UINT32	RF_CSR_DATA:8;
		UINT32	TESTCSR_RFACC_REGNUM:8;
		UINT32	RF_CSR_WR:1;
		UINT32	RF_CSR_KICK:1;
		UINT32	Rsvd1:14;
	} field;
#endif /* RT_BIG_ENDIAN */
	UINT32 word;
}RF_CSR_CFG_STRUC;

#define RF_BYPASS_0		0x0504

#define RF_SETTING_0	0x050C

#endif /* __RF_CTRL_H__ */

