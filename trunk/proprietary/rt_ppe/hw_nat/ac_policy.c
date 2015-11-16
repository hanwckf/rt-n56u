/*
    Module Name:
    
    ac_policy.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Steven Liu  2007-01-24      Initial version
*/

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/list.h>

#include "util.h"
#include "ac_policy.h"
#include "frame_engine.h"

#if defined (CONFIG_RA_HW_NAT_IPV6)
extern int ipv6_offload;
#endif

static AcPlcyNode AcPlcyList = {.List = LIST_HEAD_INIT(AcPlcyList.List) };
static char AcFreeList[8];

void SyncAcTbl(void)
{
	struct list_head *pos = NULL, *tmp;
	AcPlcyNode *node = NULL;
	int PreRuleFound = 0, PostRuleFound = 0;

	PpeSetPreAcEbl(0);
	PpeSetPostAcEbl(0);

	list_for_each_safe(pos, tmp, &AcPlcyList.List) {
		node = list_entry(pos, AcPlcyNode, List);

		switch (node->RuleType) {
		case AC_MAC_GROUP:
			AcInsMac(node);
			break;
		case AC_IP_GROUP:
			AcInsIp(node);
			break;
		case AC_VLAN_GROUP:
			AcInsVlan(node);
			break;
		default:
			break;
		}

		if (node->Type == PRE_AC) {
			PreRuleFound = 1;
		} else {
			PostRuleFound = 1;
		}

	}

	if (PreRuleFound) {
		PpeSetPreAcEbl(1);
	}
	if (PostRuleFound) {
		PpeSetPostAcEbl(1);
	}
}

AcPlcyNode *AcExistNode(AcPlcyNode * NewNode)
{
	struct list_head *pos = NULL, *tmp;
	AcPlcyNode *node = NULL;

	list_for_each_safe(pos, tmp, &AcPlcyList.List) {
		node = list_entry(pos, AcPlcyNode, List);

		switch (NewNode->RuleType) {
		case AC_MAC_GROUP:
			if (memcmp(node->Mac, NewNode->Mac, 6) == 0 &&
			    node->Type == NewNode->Type) {
				return node;
			}
			break;
		case AC_IP_GROUP:
			if (node->IpS == NewNode->IpS
			    && node->IpE == NewNode->IpE
			    && node->Type == NewNode->Type) {
				return node;
			}
			break;
		case AC_VLAN_GROUP:
			if (node->VLAN == NewNode->VLAN
			    && node->Type == NewNode->Type) {
				return node;
			}
			break;
		}
	}

	return NULL;
}

uint32_t AcAddNode(AcPlcyNode * NewNode)
{
	AcPlcyNode *node = NULL;

	if ((node = AcExistNode(NewNode))) {
		return AC_SUCCESS;
	}

	node = (AcPlcyNode *) kmalloc(sizeof(AcPlcyNode), GFP_ATOMIC);

	if (node == NULL) {
		return AC_FAIL;
	}

	memcpy(node, NewNode, sizeof(AcPlcyNode));
	node->AgIdx = PpeGetFreeAcGrp();

	list_add_tail(&node->List, &AcPlcyList.List);
	SyncAcTbl();
	return AC_SUCCESS;
}

uint32_t AcDelNode(AcPlcyNode * DelNode)
{
	struct list_head *pos = NULL, *tmp;
	AcPlcyNode *node;

	list_for_each_safe(pos, tmp, &AcPlcyList.List) {
		node = list_entry(pos, AcPlcyNode, List);

		switch (DelNode->RuleType) {
		case AC_MAC_GROUP:
			if (memcmp(node->Mac, DelNode->Mac, ETH_ALEN) == 0) {
				goto found;
			}
			break;
		case AC_IP_GROUP:
			if (node->IpS == DelNode->IpS
			    && node->IpE == DelNode->IpE) {
				goto found;
			}
			break;
		case AC_VLAN_GROUP:
			if (node->VLAN == DelNode->VLAN) {
				goto found;
			}
			break;
		}
	}

	return AC_FAIL;

      found:
	PpeSetFreeAcGrp(node->AgIdx);
	list_del(pos);
	kfree(node);
	SyncAcTbl();
	return AC_SUCCESS;
}

/*
 * Pre Ac Function
 */
uint32_t PpeGetPreAcEbl(void)
{
	uint32_t PpeFlowSet = 0;

	PpeFlowSet = RegRead(PPE_FLOW_SET);

	if ((PpeFlowSet & ~BIT_FUC_PREA) ||
	    (PpeFlowSet & ~BIT_FMC_PREA) || (PpeFlowSet & ~BIT_FBC_PREA)) {
		return 1;
	} else {
		return 0;
	}

}

void PpeSetPreAcEbl(uint32_t PreAcEbl)
{
	uint32_t PpeFlowSet = 0;

	PpeFlowSet = RegRead(PPE_FLOW_SET);

	/* Pre-Account engine for unicast/multicast/broadcast flow */
	if (PreAcEbl == 1) {
		PpeFlowSet |= (BIT_FUC_PREA);
#if defined (CONFIG_RA_HW_NAT_IPV6)
		if (ipv6_offload)
			PpeFlowSet |= (BIT_IPV6_PE_EN);
#endif
		RegModifyBits(PPE_POL_CFG, DFL_POL_AC_PRD, 16, 16);	//period
		RegModifyBits(PPE_POL_CFG, 1, 13, 1);	//enable Pre-account

	} else {
		PpeFlowSet &= ~(BIT_FUC_PREA | BIT_FMC_PREA | BIT_FBC_PREA);
#if defined (CONFIG_RA_HW_NAT_IPV6)
		PpeFlowSet &= ~(BIT_IPV6_PE_EN);
#endif
		/* We have to set period=0 first */
		RegModifyBits(PPE_POL_CFG, 0, 16, 16);	//period
		RegModifyBits(PPE_POL_CFG, 0, 13, 1);	//disable Pre-account 
		PpeRstPreAcPtr();

	}

	RegWrite(PPE_FLOW_SET, PpeFlowSet);
}

uint16_t PpeGetPreAcStr(void)
{
	uint32_t PpePreAc = 0;

	PpePreAc = RegRead(PPE_PRE_AC);
	return PpePreAc & 0x1FF;
}

void PpeSetPreAcStr(uint16_t PreAcStr)
{
	RegModifyBits(PPE_PRE_AC, PreAcStr, 0, 9);
}

uint16_t PpeGetPreAcEnd(void)
{
	uint32_t PpePreAc = 0;

	PpePreAc = RegRead(PPE_PRE_AC);
	return (PpePreAc >> 16) & 0x1FF;
}

void PpeSetPreAcEnd(uint16_t PreAcEnd)
{
	RegModifyBits(PPE_PRE_AC, PreAcEnd, 16, 9);
}

uint32_t PpeGetPostAcEbl(void)
{
	uint32_t PpeFlowSet = 0;

	PpeFlowSet = RegRead(PPE_FLOW_SET);

	if ((PpeFlowSet & ~BIT_FUC_POSA) ||
	    (PpeFlowSet & ~BIT_FMC_POSA) || (PpeFlowSet & ~BIT_FBC_POSA)) {
		return 1;
	} else {
		return 0;
	}

}

void PpeSetPostAcEbl(uint32_t PostAcEbl)
{
	uint32_t PpeFlowSet = 0;

	PpeFlowSet = RegRead(PPE_FLOW_SET);

	/* Post-Account engine for unicast/multicast/broadcast flow */
	if (PostAcEbl == 1) {
		PpeFlowSet |= (BIT_FUC_POSA);
#if defined (CONFIG_RA_HW_NAT_IPV6)
		if (ipv6_offload)
			PpeFlowSet |= (BIT_IPV6_PE_EN);
#endif
		RegModifyBits(PPE_POL_CFG, DFL_POL_AC_PRD, 16, 16);	//period
		RegModifyBits(PPE_POL_CFG, 1, 12, 1);	//enable Post-account

	} else {
		PpeFlowSet &= ~(BIT_FUC_POSA | BIT_FMC_POSA | BIT_FBC_POSA);
#if defined (CONFIG_RA_HW_NAT_IPV6)
		PpeFlowSet &= ~(BIT_IPV6_PE_EN);
#endif
		/* We have to set period=0 first */
		RegModifyBits(PPE_POL_CFG, 0, 16, 16);	//period
		RegModifyBits(PPE_POL_CFG, 0, 12, 1);	//disable Post-account
		PpeRstPostAcPtr();

	}

	RegWrite(PPE_FLOW_SET, PpeFlowSet);

}

uint16_t PpeGetPostAcStr(void)
{
	uint32_t PpePostAc = 0;

	PpePostAc = RegRead(PPE_POST_AC);
	return PpePostAc & 0x1FF;
}

void PpeSetPostAcStr(uint16_t PostAcStr)
{
	RegModifyBits(PPE_POST_AC, PostAcStr, 0, 9);
}

uint16_t PpeGetPostAcEnd(void)
{
	uint32_t PpePostAc = 0;

	PpePostAc = RegRead(PPE_POST_AC);
	return (PpePostAc >> 16) & 0x1FF;
}

void PpeSetPostAcEnd(uint16_t PostAcEnd)
{
	RegModifyBits(PPE_POST_AC, PostAcEnd, 16, 9);
}

void inline PpeInsAcEntry(void *Rule, enum AcType Type)
{
	uint32_t Index = 0;
	uint32_t *p = (uint32_t *) Rule;

	if (Type == PRE_AC) {
		Index = PpeGetPreAcEnd();
	} else {
		Index = PpeGetPostAcEnd();
	}

	NAT_DEBUG("Policy Table Base=%08X Offset=%d\n", POLICY_TBL_BASE, Index * 8);
	NAT_DEBUG("%08X: %08X\n", POLICY_TBL_BASE + Index * 8, *p);
	NAT_DEBUG("%08X: %08X\n", POLICY_TBL_BASE + Index * 8 + 4, *(p + 1));

	RegWrite(POLICY_TBL_BASE + Index * 8, *p);	/* Low bytes */
	RegWrite(POLICY_TBL_BASE + Index * 8 + 4, *(p + 1));	/* High bytes */

	/* Update AC_END */
	if (Type == PRE_AC) {
		PpeSetPreAcEnd(Index + 1);
	} else {
		PpeSetPostAcEnd(Index + 1);
	}
}

/*
 * MAC
 */
uint32_t AcInsMac(AcPlcyNode * node)
{
	struct l2_rule L2Rule;

	memset(&L2Rule, 0, sizeof(L2Rule));
	memcpy(&L2Rule.mac, node->Mac, ETH_ALEN);

	L2Rule.com.rt = L2_RULE;
	L2Rule.com.pn = PN_DONT_CARE;
	L2Rule.com.match = 1;

	L2Rule.com.ac.ee = 1;
	L2Rule.com.ac.ag = node->AgIdx;

	if (node->Type == PRE_AC) {
		L2Rule.com.dir = SMAC;
		PpeInsAcEntry(&L2Rule, PRE_AC);
	} else {		/* Post AC */
		L2Rule.com.dir = DMAC;
		PpeInsAcEntry(&L2Rule, POST_AC);
	}

	return 1;
}

/*
 * IP 
 */
uint32_t AcInsIp(AcPlcyNode * node)
{
	struct l3_rule L3Rule;
	uint8_t E, M;

	memset(&L3Rule, 0, sizeof(L3Rule));

	CalIpRange(node->IpS, node->IpE, &M, &E);
	L3Rule.com.ee_0.ee = 1;
	L3Rule.com.ac.ag = node->AgIdx;

	L3Rule.com.match = 1;
	L3Rule.com.pn = PN_DONT_CARE;
	L3Rule.com.rt = L3_RULE;
	L3Rule.ip.ip = node->IpS;
	L3Rule.ip.ip_rng_m = M;
	L3Rule.ip.ip_rng_e = E;
	L3Rule.ip.v4 = 1;

	if (node->Type == PRE_AC) {
		L3Rule.com.dir = SIP;
		PpeInsAcEntry(&L3Rule, PRE_AC);
	} else {		/* Post AC */
		L3Rule.com.dir = DIP;
		PpeInsAcEntry(&L3Rule, POST_AC);
	}

	return 1;
}

/*
 * VLAN
 */
uint32_t AcInsVlan(AcPlcyNode * node)
{
	struct l2_rule L2Rule;

	memset(&L2Rule, 0, sizeof(L2Rule));

	L2Rule.others.vid = node->VLAN;
	L2Rule.others.v = 1;

	L2Rule.com.rt = L2_RULE;
	L2Rule.com.pn = PN_DONT_CARE;
	L2Rule.com.match = 1;

	L2Rule.com.ac.ee = 1;
	L2Rule.com.ac.ag = node->AgIdx;

	if (node->Type == PRE_AC) {
		L2Rule.com.dir = OTHERS;
		PpeInsAcEntry(&L2Rule, PRE_AC);
	} else {		/* Post AC */
		L2Rule.com.dir = OTHERS;
		PpeInsAcEntry(&L2Rule, POST_AC);
	}

	return 1;
}

/* Remove all AC entries */
uint32_t AcCleanTbl(void)
{
	struct list_head *pos = NULL, *tmp;
	AcPlcyNode *node;

	list_for_each_safe(pos, tmp, &AcPlcyList.List) {
		node = list_entry(pos, AcPlcyNode, List);
		list_del(pos);
		kfree(node);
	}

	PpeSetPreAcEbl(0);	// Disable PreAc Table
	PpeSetPostAcEbl(0);	// Disable PostAc Table

	return 1;

}

/*     Low            High
 * +-------------+-------------+
 * |Byte_cnt(32) | Pkt_cnt(32) |
 * +-------------+-------------+
 */

uint32_t AcGetCnt(AcPlcyNode * SearchNode, enum AcCntType AcCntType)
{
	struct list_head *pos = NULL, *tmp;
	AcPlcyNode *node;
	long long result;

	list_for_each_safe(pos, tmp, &AcPlcyList.List) {
		node = list_entry(pos, AcPlcyNode, List);

		switch (SearchNode->RuleType) {
		case AC_MAC_GROUP:
			if ((memcmp(node->Mac, SearchNode->Mac, ETH_ALEN)
			     == 0) && node->Type == SearchNode->Type) {
				goto found;
			}
			break;
		case AC_IP_GROUP:
			if ((node->IpS == SearchNode->IpS) &&
			    (node->IpE == SearchNode->IpE) &&
			    (node->Type == SearchNode->Type)) {
				goto found;
			}
			break;
		case AC_VLAN_GROUP:
			if ((node->VLAN == SearchNode->VLAN) &&
			    (node->Type == SearchNode->Type)) {
				goto found;
			}
			break;

		}
	}

	return 0;

found:
	if (AcCntType == AC_BYTE_CNT) {
		result = RegRead(AC_BASE + node->AgIdx * 8);
		return result;
	} else {		/* Packet Count */
		result = RegRead(AC_BASE + node->AgIdx * 8 + 4);
		return result;
	}
}

int PpeGetFreeAcGrp(void)
{
	int i = 0;
	int j = 0;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			if ((AcFreeList[i] & (1 << j)) == 0) {
				AcFreeList[i] |= (1 << j);
				return (i * 8 + j);
			}
		}
	}

	return -1;
}

void PpeSetFreeAcGrp(uint32_t ag_idx)
{
	AcFreeList[ag_idx / 8] &= ~(1 << (ag_idx % 8));
}
