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
	wf_cfgoff.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __WF_CFG_OFF_H__
#define __WF_CFG_OFF_H__

#define WF_CFG_OFF_BASE		0x20600

/* 820F0800	WOSWRST_EN SW Reset	00000000 */
#define CFG_OFF_WOSWRST_EN      (WF_CFG_OFF_BASE)
#define BB_LOGRST_MASK          0x01
#define BB_LOGRST_EN            BIT0
#define MACOFF_REGRST_MASK      0x02
#define MACOFF_REGRST_EN        BIT1
#define MACOFF_LOGRST_MASK      0x04
#define MACOFF_LOGRST_EN        BIT2



#endif /* __WF_CFG_OFF_H__ */

