/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    ac_ioctl.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Steven Liu  2007-02-15      Initial version
*/

#ifndef	__AC_IOCTL_H__
#define	__AC_IOCTL_H__

#include "common.h"

#define AC_ADD_MAC_UL_ENTRY		(0)
#define AC_ADD_MAC_DL_ENTRY		(1)
#define AC_DEL_MAC_UL_ENTRY		(2)
#define AC_DEL_MAC_DL_ENTRY		(3)
#define AC_ADD_IP_UL_ENTRY		(4)
#define AC_ADD_IP_DL_ENTRY		(5)
#define AC_DEL_IP_UL_ENTRY		(6)
#define AC_DEL_IP_DL_ENTRY		(7)
#define AC_ADD_VLAN_UL_ENTRY		(8)
#define AC_ADD_VLAN_DL_ENTRY		(9)
#define AC_DEL_VLAN_UL_ENTRY		(10)
#define AC_DEL_VLAN_DL_ENTRY		(11)
#define AC_GET_MAC_UL_PKT_CNT		(12)
#define AC_GET_MAC_DL_PKT_CNT		(13)
#define AC_GET_MAC_UL_BYTE_CNT		(14)
#define AC_GET_MAC_DL_BYTE_CNT		(15)
#define AC_GET_IP_UL_PKT_CNT		(16)
#define AC_GET_IP_DL_PKT_CNT		(17)
#define AC_GET_IP_UL_BYTE_CNT		(18)
#define AC_GET_IP_DL_BYTE_CNT		(19)
#define AC_GET_VLAN_UL_PKT_CNT		(20)
#define AC_GET_VLAN_DL_PKT_CNT		(21)
#define AC_GET_VLAN_UL_BYTE_CNT		(22)
#define AC_GET_VLAN_DL_BYTE_CNT		(23)
#define AC_CLEAN_TBL			(24)

#define AC_DEVNAME                     "ac0"
#define AC_MAJOR                       (240)

enum AcResult {
	AC_SUCCESS=0,
	AC_FAIL=1,
	AC_TBL_FULL=2
};

struct ac_args {
    unsigned char  mac[6];
    enum AcResult  result;
    unsigned long  ip_s; /* Start Ip Address */
    unsigned long  ip_e; /* End Ip Address */
    unsigned short  vid:12; /* VID */
    unsigned int   cnt; /* pkt_cnt or byt_cnt */
    unsigned int   ag_num; /* account group number */
};


int AcRegIoctlHandler(void);
void AcUnRegIoctlHandler(void);

#endif
