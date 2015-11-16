/*
    Module Name:
    
    mtr_policy.c

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
#include "mtr_policy.h"
#include "frame_engine.h"

#if defined (CONFIG_RA_HW_NAT_IPV6)
extern int ipv6_offload;
#endif

MtrPlcyNode MtrPlcyList = {.List = LIST_HEAD_INIT(MtrPlcyList.List) };

static char MtrFreeList[8];

void SyncMtrTbl(void)
{
	struct list_head *pos = NULL, *tmp;
	MtrPlcyNode *node = NULL;
	int PreRuleFound = 0, PostRuleFound = 0;

	PpeSetPreMtrEbl(0);	// Disable PreMtr Table
	PpeSetPostMtrEbl(0);	// Disable PostMtr Table

	list_for_each_safe(pos, tmp, &MtrPlcyList.List) {
		node = list_entry(pos, MtrPlcyNode, List);

		switch (node->RuleType) {
		case MTR_MAC_GROUP:
			MtrInsMac(node);
			break;
		case MTR_IP_GROUP:
			MtrInsIp(node);
			break;
		case MTR_SYN:
			MtrInsSYN(node);
			break;
		case MTR_FIN:
			MtrInsFIN(node);
			break;
		case MTR_PROTOCOL_UDP:
		case MTR_PROTOCOL_ICMP:
			MtrInsProtocol(node);
			break;
		default:
			break;
		}

		//Is there any Pre/Post Rule?
		if (node->Type == PRE_MTR) {
			PreRuleFound = 1;
		} else {
			PostRuleFound = 1;
		}
	}

	if (PreRuleFound) {
		PpeSetPreMtrEbl(1);
	} else if (PostRuleFound) {
		PpeSetPostMtrEbl(1);
	} else {
		NAT_PRINT("MTR Table All Empty!\n");
	}
}

MtrPlcyNode *MtrExistNode(MtrPlcyNode * NewNode)
{
	struct list_head *pos = NULL, *tmp;
	MtrPlcyNode *node = NULL;

	list_for_each_safe(pos, tmp, &MtrPlcyList.List) {
		node = list_entry(pos, MtrPlcyNode, List);

		switch (NewNode->RuleType) {
		case MTR_MAC_GROUP:
			if (memcmp(node->Mac, NewNode->Mac, 6) == 0 &&
			    node->Type == NewNode->Type) {
				return node;
			}
			break;
		case MTR_IP_GROUP:
			if (node->IpS == NewNode->IpS
			    && node->IpE == NewNode->IpE
			    && node->Type == NewNode->Type) {
				return node;
			}
			break;
		case MTR_SYN:
			if (node->RuleType == MTR_SYN) {
				return node;
			}
			break;
		case MTR_FIN:
			if (node->RuleType == MTR_FIN) {
				return node;
			}
			break;
		case MTR_PROTOCOL_UDP:
			if (node->RuleType == MTR_PROTOCOL_UDP) {
				return node;
			}
			break;
		case MTR_PROTOCOL_ICMP:
			if (node->RuleType == MTR_PROTOCOL_ICMP) {
				return node;
			}
			break;
		}
	}

	return NULL;
}

uint32_t MtrAddNode(MtrPlcyNode * NewNode)
{
	MtrPlcyNode *node = NULL;

	if ((node = MtrExistNode(NewNode))) {
		node->mtr_info = NewNode->mtr_info;

	} else {
		node = (MtrPlcyNode *) kmalloc(sizeof(MtrPlcyNode), GFP_ATOMIC);

		if (node == NULL) {
			return MTR_FAIL;
		}

		memcpy(node, NewNode, sizeof(MtrPlcyNode));
		node->MgNum = PpeGetFreeMtrGrp();
		list_add_tail(&node->List, &MtrPlcyList.List);
	}

	SyncMtrTbl();
	return MTR_SUCCESS;
}

uint32_t MtrDelNode(MtrPlcyNode * DelNode)
{
	struct list_head *pos = NULL, *tmp;
	MtrPlcyNode *node;

	list_for_each_safe(pos, tmp, &MtrPlcyList.List) {
		node = list_entry(pos, MtrPlcyNode, List);

		switch (DelNode->RuleType) {
		case MTR_MAC_GROUP:
			if (memcmp(node->Mac, DelNode->Mac, ETH_ALEN) == 0) {
				goto found;
			}
			break;
		case MTR_IP_GROUP:
			if (node->IpS == DelNode->IpS
			    && node->IpE == DelNode->IpE) {
				goto found;
			}
			break;
		case MTR_SYN:
			if (node->RuleType == MTR_SYN)
				goto found;
			break;
		case MTR_FIN:
			if (node->RuleType == MTR_FIN)
				goto found;
			break;
		case MTR_PROTOCOL_UDP:
			if (node->RuleType == MTR_PROTOCOL_UDP)
				goto found;
			break;
		case MTR_PROTOCOL_ICMP:
			if (node->RuleType == MTR_PROTOCOL_ICMP)
				goto found;
			break;
		}
	}

	return MTR_FAIL;

      found:
	PpeSetFreeMtrGrp(node->MgNum);
	list_del(pos);
	kfree(node);
	SyncMtrTbl();
	return MTR_SUCCESS;
}

/*
 * Pre Mtr Function
 */
uint32_t PpeGetPreMtrEbl(void)
{
	uint32_t PpeFlowSet = 0;

	PpeFlowSet = RegRead(PPE_FLOW_SET);

	if ((PpeFlowSet & ~BIT_FUC_PREM) ||
	    (PpeFlowSet & ~BIT_FMC_PREM) || (PpeFlowSet & ~BIT_FBC_PREM)) {
		return 1;
	} else {
		return 0;
	}

}

void PpeSetPreMtrEbl(uint32_t PreMtrEbl)
{
	uint32_t PpeFlowSet = 0;

	PpeFlowSet = RegRead(PPE_FLOW_SET);

	/* Pre-Meter engine for unicast/multicast/broadcast flow */
	if (PreMtrEbl == 1) {
		PpeFlowSet |= (BIT_FUC_PREM);
#if defined (CONFIG_RA_HW_NAT_IPV6)
		if (ipv6_offload)
			PpeFlowSet |= (BIT_IPV6_PE_EN);
#endif
	} else {
		PpeFlowSet &= ~(BIT_FUC_PREM | BIT_FMC_PREM | BIT_FBC_PREM);
#if defined (CONFIG_RA_HW_NAT_IPV6)
		PpeFlowSet &= ~(BIT_IPV6_PE_EN);
#endif
		PpeRstPreMtrPtr();
	}

	RegWrite(PPE_FLOW_SET, PpeFlowSet);
}

uint16_t PpeGetPreMtrStr(void)
{
	uint32_t PpePreMtr = 0;

	PpePreMtr = RegRead(PPE_PRE_MTR);
	return PpePreMtr & 0x1FF;
}

void PpeSetPreMtrStr(uint16_t PreMtrStr)
{
	RegModifyBits(PPE_PRE_MTR, PreMtrStr, 0, 9);
}

uint16_t PpeGetPreMtrEnd(void)
{
	uint32_t PpePreMtr = 0;

	PpePreMtr = RegRead(PPE_PRE_MTR);
	return (PpePreMtr >> 16) & 0x1FF;
}

void PpeSetPreMtrEnd(uint16_t PreMtrEnd)
{
	RegModifyBits(PPE_PRE_MTR, PreMtrEnd, 16, 9);
}

uint32_t PpeGetPostMtrEbl(void)
{
	uint32_t PpeFlowSet = 0;

	PpeFlowSet = RegRead(PPE_FLOW_SET);

	if ((PpeFlowSet & ~BIT_FUC_POSM) ||
	    (PpeFlowSet & ~BIT_FMC_POSM) || (PpeFlowSet & ~BIT_FBC_POSM)) {
		return 1;
	} else {
		return 0;
	}

}

void PpeSetPostMtrEbl(uint32_t PostMtrEbl)
{
	uint32_t PpeFlowSet = 0;

	PpeFlowSet = RegRead(PPE_FLOW_SET);

	/* Post-Meter engine for unicast/multicast/broadcast flow */
	if (PostMtrEbl == 1) {
		PpeFlowSet |= (BIT_FUC_POSM);
#if defined (CONFIG_RA_HW_NAT_IPV6)
		if (ipv6_offload)
			PpeFlowSet |= (BIT_IPV6_PE_EN);
#endif
	} else {
		PpeFlowSet &= ~(BIT_FUC_POSM | BIT_FMC_POSM | BIT_FBC_POSM);
#if defined (CONFIG_RA_HW_NAT_IPV6)
		PpeFlowSet &= ~(BIT_IPV6_PE_EN);
#endif
		PpeRstPostMtrPtr();

	}

	RegWrite(PPE_FLOW_SET, PpeFlowSet);

}

uint16_t PpeGetPostMtrStr(void)
{
	uint32_t PpePostMtr = 0;

	PpePostMtr = RegRead(PPE_POST_MTR);
	return PpePostMtr & 0x1FF;
}

void PpeSetPostMtrStr(uint16_t PostMtrStr)
{
	RegModifyBits(PPE_POST_MTR, PostMtrStr, 0, 9);
}

uint16_t PpeGetPostMtrEnd(void)
{
	uint32_t PpePostMtr = 0;

	PpePostMtr = RegRead(PPE_POST_MTR);
	return (PpePostMtr >> 16) & 0x1FF;
}

void PpeSetPostMtrEnd(uint16_t PostMtrEnd)
{
	RegModifyBits(PPE_POST_MTR, PostMtrEnd, 16, 9);
}

/* 
 * RT2880_Shuttle/RT2880_MP
 * Insert Mtr Entry to Frame Engine Counter Memory 
 * +----------------+----------------+------------+
 * |cur_bk_size(15) | token_rate(14) | bk_size(3) |
 * +----------------+----------------+------------+
 *
 * RT2880_MP2 (Byte Base)
 * +----------------+----------------+---------------+------------+
 * |Cur_bk_size(15) | Token_rate(14) | Max_Bk_Size(2)| Mtr_mode(1)|
 * +----------------+----------------+---------------+------------+
 *
 * RT2880_MP2 (Packet Base)
 * +---------------+-------------------+----------------+--------------+-----------+
 * |Cur_bk_size(7) | Time_to_refill(14)| Mtr_interval(3)|Max_BK_Size(7)|Mtr_mode(1)|
 * +---------------+-------------------+----------------+--------------+-----------+
 *
 */
void PpeInsMtrTbl(MtrPlcyNode * node)
{
	uint32_t MtrEntry = 0;

	if ((node->ByteBase.MtrMode & 0x01) == 0) {	// Byte Base
		MtrEntry = (node->ByteBase.TokenRate << 3) |
		    (node->ByteBase.MaxBkSize << 1) | node->ByteBase.MtrMode;
	} else {		// Packet Base
		MtrEntry = (node->PktBase.MtrIntval << 8) |
		    (node->PktBase.MaxBkSize << 1) | node->PktBase.MtrMode;
	}

	RegWrite(METER_BASE + node->MgNum * 4, MtrEntry);

	NAT_DEBUG("Meter Table Base=%08X Offset=%d\n", METER_BASE, node->MgNum * 4);
	NAT_DEBUG("%08X: %08X\n", METER_BASE + node->MgNum * 4, MtrEntry);
}

int PpeGetFreeMtrGrp(void)
{
	int i = 0;
	int j = 0;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			if ((MtrFreeList[i] & (1 << j)) == 0) {
				MtrFreeList[i] |= (1 << j);
				return (i * 8 + j);
			}
		}
	}

	return -1;
}

void PpeSetFreeMtrGrp(uint32_t mg_num)
{
	MtrFreeList[mg_num / 8] &= ~(1 << (mg_num % 8));
}

void inline PpeInsMtrEntry(void *Rule, enum MtrType Type)
{
	uint32_t Index = 0;
	uint32_t *p = (uint32_t *) Rule;

	if (Type == PRE_MTR) {
		Index = PpeGetPreMtrEnd();
	} else {
		Index = PpeGetPostMtrEnd();
	}

	NAT_DEBUG("\nPolicy Table Base=0x%08X Offset=0x%04X\n", POLICY_TBL_BASE, Index * 8);
	NAT_DEBUG("%08X: %08X\n", POLICY_TBL_BASE + Index * 8, *p);
	NAT_DEBUG("%08X: %08X\n", POLICY_TBL_BASE + Index * 8 + 4, *(p + 1));

	RegWrite(POLICY_TBL_BASE + Index * 8, *p);	/* Low bytes */
	RegWrite(POLICY_TBL_BASE + Index * 8 + 4, *(p + 1));	/* High bytes */

	/* Update MTR_END */
	if (Type == PRE_MTR) {
		PpeSetPreMtrEnd(Index + 1);
	} else {
		PpeSetPostMtrEnd(Index + 1);
	}
}

uint32_t MtrInsMac(MtrPlcyNode * node)
{
	struct l2_rule L2Rule;

	memset(&L2Rule, 0, sizeof(L2Rule));
	memcpy(&L2Rule.mac, node->Mac, ETH_ALEN);

	L2Rule.com.rt = L2_RULE;
	L2Rule.com.pn = PN_DONT_CARE;
	L2Rule.com.match = 1;

	L2Rule.com.mtr.ee = 1;
	L2Rule.com.mtr.dop = 1;	/* drop out profile */
	L2Rule.com.mtr.mg = node->MgNum;

	if (node->Type == PRE_MTR) {
		L2Rule.com.dir = SMAC;
		PpeInsMtrEntry(&L2Rule, PRE_MTR);
	} else {		/* Post Mtr */
		L2Rule.com.dir = DMAC;
		PpeInsMtrEntry(&L2Rule, POST_MTR);
	}
	PpeInsMtrTbl(node);
	return 1;
}

/*
 * IP 
 */
uint32_t MtrInsIp(MtrPlcyNode * node)
{
	struct l3_rule L3Rule;
	uint8_t E, M;

	memset(&L3Rule, 0, sizeof(L3Rule));

	CalIpRange(node->IpS, node->IpE, &M, &E);
	L3Rule.com.mtr.ee = 1;
	L3Rule.com.mtr.dop = 1;	/* drop out profile */
	L3Rule.com.mtr.mg = node->MgNum;

	L3Rule.com.match = 1;
	L3Rule.com.pn = PN_DONT_CARE;
	L3Rule.com.rt = L3_RULE;
	L3Rule.ip.ip = node->IpS;
	L3Rule.ip.ip_rng_m = M;
	L3Rule.ip.ip_rng_e = E;
	L3Rule.ip.v4 = 1;

	if (node->Type == PRE_MTR) {
		L3Rule.com.dir = SIP;
		PpeInsMtrEntry(&L3Rule, PRE_MTR);
	} else {		/* Post Mtr */
		L3Rule.com.dir = DIP;
		PpeInsMtrEntry(&L3Rule, POST_MTR);
	}

	PpeInsMtrTbl(node);
	return 1;
}

uint32_t MtrInsSYN(MtrPlcyNode * node)
{
	struct l4_rule L4Rule;

	memset(&L4Rule, 0, sizeof(L4Rule));
	L4Rule.tcp.tcp_f = TCP_SYN;
	L4Rule.tcp.tcp_fop = IN_SET;
	L4Rule.tcp.tu = FLT_TCP;

	L4Rule.com.rt = L4_RULE;
	L4Rule.com.pn = PN_DONT_CARE;
	L4Rule.com.match = 1;

	L4Rule.com.mtr.ee = 1;
	L4Rule.com.mtr.dop = 1;	/* drop out profile */
	L4Rule.com.mtr.mg = node->MgNum;

	if (node->Type == PRE_MTR) {
		L4Rule.com.dir = DONT_CARE;
		PpeInsMtrEntry(&L4Rule, PRE_MTR);
	} else {		/* Post Mtr Doesnot Need */
	}

	PpeInsMtrTbl(node);
	return 1;
}

uint32_t MtrInsFIN(MtrPlcyNode * node)
{
	struct l4_rule L4Rule;

	memset(&L4Rule, 0, sizeof(L4Rule));
	L4Rule.tcp.tcp_f = TCP_FIN;
	L4Rule.tcp.tcp_fop = IN_SET;
	L4Rule.tcp.tu = FLT_TCP;

	L4Rule.com.rt = L4_RULE;
	L4Rule.com.pn = PN_DONT_CARE;
	L4Rule.com.match = 1;

	L4Rule.com.mtr.ee = 1;
	L4Rule.com.mtr.dop = 1;	/* drop out profile */
	L4Rule.com.mtr.mg = node->MgNum;

	if (node->Type == PRE_MTR) {
		L4Rule.com.dir = DONT_CARE;
		PpeInsMtrEntry(&L4Rule, PRE_MTR);
	} else {		/* Post Mtr Doesnot Need */
	}

	PpeInsMtrTbl(node);
	return 1;
}

uint32_t MtrInsProtocol(MtrPlcyNode * node)
{
	struct l4_rule L4Rule;

	memset(&L4Rule, 0, sizeof(L4Rule));

	L4Rule.tcp.tu = FLT_IP_PROT;

	if (node->RuleType == MTR_PROTOCOL_UDP)
		L4Rule.ip.prot = 0x11;
	 /*UDP*/
	else if (node->RuleType == MTR_PROTOCOL_ICMP)
		L4Rule.ip.prot = 1;
	 /*ICMP*/ L4Rule.com.rt = L4_RULE;
	L4Rule.com.pn = PN_DONT_CARE;
	L4Rule.com.match = 1;

	L4Rule.com.mtr.ee = 1;
	L4Rule.com.mtr.dop = 1;	/* drop out profile */
	L4Rule.com.mtr.mg = node->MgNum;

	if (node->Type == PRE_MTR) {
		L4Rule.com.dir = DONT_CARE;
		PpeInsMtrEntry(&L4Rule, PRE_MTR);
	} else {		/* Post Mtr Doesnot Need */
	}

	PpeInsMtrTbl(node);
	return 1;
}

/* Remove all MTR entries */
uint32_t MtrCleanTbl(void)
{
	struct list_head *pos = NULL, *tmp;
	MtrPlcyNode *node;

	list_for_each_safe(pos, tmp, &MtrPlcyList.List) {
		node = list_entry(pos, MtrPlcyNode, List);
		list_del(pos);
		kfree(node);
	}

	PpeSetPreMtrEbl(0);	// Disable PreMtr Table
	PpeSetPostMtrEbl(0);	// Disable PostMtr Table

	return 1;

}

/*print Mtr table*/
uint8_t MtrGetAllEntries(struct mtr_list_args * opt)
{
	struct list_head *pos = NULL, *tmp;
	MtrPlcyNode *node = NULL;
	int count = 0;		/* valid entry count */

	list_for_each_safe(pos, tmp, &MtrPlcyList.List) {
		node = list_entry(pos, MtrPlcyNode, List);

		memcpy(opt->entries[count].mac, node->Mac, ETH_ALEN);
		MacReverse(opt->entries[count].mac);
		opt->entries[count].ip_s = node->IpS;
		opt->entries[count].ip_e = node->IpE;
		opt->entries[count].mg_num = node->MgNum;

		if (node->PktBase.MtrMode == 1) {	//pkt mode
			opt->entries[count].mtr_mode = node->PktBase.MtrMode;
			opt->entries[count].bk_size = node->PktBase.MaxBkSize;
			opt->entries[count].mtr_intval =
			    node->PktBase.MtrIntval;
		} else {	//byte mode
			opt->entries[count].mtr_mode = node->ByteBase.MtrMode;
			opt->entries[count].bk_size = node->ByteBase.MaxBkSize;
			opt->entries[count].token_rate =
			    node->ByteBase.TokenRate;
		}

		count++;

	}

	opt->num_of_entries = count;
	return MTR_SUCCESS;
}
