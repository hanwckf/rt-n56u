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
 ac_policy.h

Abstract:

Revision History:
Who         When            What
--------    ----------      ----------------------------------------------
Name        Date            Modification logs
Steven Liu  2007-01-23      Initial version
 */

#ifndef _AC_POLICY_WANTED
#define _AC_POLICY_WANTED

#include "policy.h"
#include "ac_ioctl.h"

enum AcRuleType {
	AC_MAC_GROUP=0,
	AC_IP_GROUP=1,
	AC_VLAN_GROUP=2
};

enum AcType {
	PRE_AC=0,
	POST_AC=1
};

enum AcCntType {
	AC_BYTE_CNT=0,
	AC_PKT_CNT=1
};

typedef struct {
    struct list_head List;
    enum AcType Type;
    enum AcRuleType RuleType;
    uint8_t Mac[6];
    uint16_t PortS; /* start of port */
    uint16_t PortE; /* end of port */
    uint32_t IpS; /* start of ip */
    uint32_t IpE; /* end of ip */
    uint32_t IpProto; /* ip protocol */
    uint16_t VLAN:12; /* VLAN ID */
    uint8_t  AgNum; /* accounting group number */
} AcPlcyNode;

/*
 * EXPORT FUNCTION
 */

uint32_t AcInsMac(AcPlcyNode *node);
uint32_t AcInsIp(AcPlcyNode *node);
uint32_t AcInsVlan(AcPlcyNode *node);

uint32_t AcAddNode(AcPlcyNode *NewNode);
uint32_t AcDelNode(AcPlcyNode *NewNode);

void SyncAcTbl(void);
uint32_t AcCleanTbl(void);

void  PpeSetPreAcEbl(uint32_t PreAclEbl);
void  PpeSetPostAcEbl(uint32_t PostAclEbl);

uint16_t PpeGetPreAcStr(void);
uint16_t PpeGetPreAcEnd(void);
uint16_t PpeGetPostAcStr(void);
uint16_t PpeGetPostAcEnd(void);

int PpeGetFreeAcGrp(void);
void PpeSetFreeAcGrp(uint32_t AcNum);

AcPlcyNode *AcExistNode(AcPlcyNode *NewNode);
uint32_t AcGetCnt(AcPlcyNode *SearchNode, enum AcCntType AcCntType);

#endif
