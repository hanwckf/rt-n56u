/*
    Module Name:
    
    acl_policy.c

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
#include <linux/if_ether.h>

#include "util.h"
#include "acl_policy.h"
#include "frame_engine.h"

#if defined (CONFIG_RA_HW_NAT_IPV6)
extern int ipv6_offload;
#endif

static AclPlcyNode AclPlcyList = {.List = LIST_HEAD_INIT(AclPlcyList.List) };
extern uint32_t DebugLevel;

uint32_t SyncAclTbl(void)
{
	struct list_head *pos = NULL, *tmp;
	AclPlcyNode *node = NULL;

	PpeSetPreAclEbl(0);
	list_for_each_safe(pos, tmp, &AclPlcyList.List) {
		node = list_entry(pos, AclPlcyNode, List);

		switch (node->RuleType) {
		case ACL_ADD_SDMAC_ANY:
			AclInsSDmac(node);
			break;
		case ACL_ADD_ETYPE_ANY:
			AclInsEthertype(node);
			break;
		case ACL_ADD_SMAC_DIP_ANY:
		case ACL_ADD_SMAC_DIP_TCP:
		case ACL_ADD_SMAC_DIP_UDP:
			AclInsSmacDipDp(node);
			break;
		case ACL_ADD_SIP_DIP_ANY:
		case ACL_ADD_SIP_DIP_TCP:
		case ACL_ADD_SIP_DIP_UDP:
			AclInsSipDipDp(node);
			break;

		case ACL_ADD_SMAC_DMAC_ETYPE_VID_SIP_DIP_TOS_PORT:
			AclInsSmacETypTOSSipSpDipDp(node);
			break;
		}
	}

	/* Empty Rule */
	if (node == NULL) {
		NAT_PRINT("ACL Table All Empty!\n");
		return ACL_SUCCESS;
	}

	if (node->Method == ACL_ALLOW_RULE) {
		AclInsDflAllow();	/* insert my/broadcast mac */
		AclInsDflDeny();	/* if there is no entry matched, drop the packet */
	}

	PpeSetPreAclEbl(1);
	return ACL_SUCCESS;
}

AclPlcyNode *AclExistNode(AclPlcyNode * NewNode)
{
	struct list_head *pos = NULL, *tmp;
	AclPlcyNode *node = NULL;

	list_for_each_safe(pos, tmp, &AclPlcyList.List) {
		node = list_entry(pos, AclPlcyNode, List);

		switch (NewNode->RuleType) {
		case ACL_ADD_SDMAC_ANY:
			if (memcmp(node->Mac, NewNode->Mac, ETH_ALEN) == 0) {
				return node;
			}
			break;
		case ACL_ADD_ETYPE_ANY:
			if (memcmp(&node->Ethertype, &NewNode->Ethertype, 2) ==
			    0) {
				return node;
			}
			break;
		case ACL_ADD_SMAC_DIP_ANY:	/* primary key = smac + dip + protocol */
			if (memcmp(node->Mac, NewNode->Mac, ETH_ALEN) == 0
			    && node->DipS == NewNode->DipS
			    && node->DipE == NewNode->DipE
			    && node->Proto == NewNode->Proto) {
				return node;
			}
			break;
		case ACL_ADD_SMAC_DIP_TCP:
		case ACL_ADD_SMAC_DIP_UDP:	/* primary key = smac + dip +dp + protocol */
			if (memcmp(node->Mac, NewNode->Mac, ETH_ALEN) == 0
			    && (node->DipS == NewNode->DipS)
			    && (node->DipE == NewNode->DipE)
			    && (node->DpS == NewNode->DpS)
			    && (node->Proto == NewNode->Proto)) {
				return node;
			}
			break;
		case ACL_ADD_SIP_DIP_ANY:	/* primary key = sip + dip + protocol */
			if (node->SipS == NewNode->SipS
			    && node->SipE == NewNode->SipE
			    && node->DipS == NewNode->DipS
			    && node->DipE == NewNode->DipE
			    && node->Proto == NewNode->Proto) {
				return node;
			}
			break;
		case ACL_ADD_SIP_DIP_TCP:
		case ACL_ADD_SIP_DIP_UDP:	/* primary key = sip + dip + dp + protocol */
			if (node->SipS == NewNode->SipS
			    && node->SipE == NewNode->SipE
			    && node->DipS == NewNode->DipS
			    && node->DipE == NewNode->DipE
			    && node->DpS == NewNode->DpS
			    && node->Proto == NewNode->Proto) {
				return node;
			}
			break;
		case ACL_ADD_SMAC_DMAC_ETYPE_VID_SIP_DIP_TOS_PORT:
			/* primary key = smac + dmac + etype + vid + sip + dip + protocol + tos + sp + dp + protocol(tcp/udp/any) */
			if (memcmp(node->Mac, NewNode->Mac, ETH_ALEN) == 0
			    && memcmp(node->DMac, NewNode->DMac,
				      ETH_ALEN) == 0
			    && node->Ethertype == NewNode->Ethertype
			    && node->Vid == NewNode->Vid
			    && node->SipS == NewNode->SipS
			    && node->Protocol == NewNode->Protocol
			    && node->SipE == NewNode->SipE
			    && node->TosS == NewNode->TosS
			    && node->TosE == NewNode->TosE
			    && (node->DipS == NewNode->DipS)
			    && (node->DipE == NewNode->DipE)
			    && (node->DpS == NewNode->DpS)
			    && (node->DpE == NewNode->DpE)
			    && (node->SpS == NewNode->SpS)
			    && (node->SpE == NewNode->SpE)
			    && (node->Proto == NewNode->Proto)) {
				return node;
			}
			break;
		}
	}

	return NULL;
}

/*use ACL table to make classification to UP*/
uint8_t AclClassify(AclClassifyKey * NewRateReach)
{
	struct list_head *pos = NULL, *tmp;
	AclPlcyNode *node = NULL;

	uint8_t ZeroMac[ETH_ALEN] = { 0, 0, 0, 0, 0, 0 };

	list_for_each_safe(pos, tmp, &AclPlcyList.List) {
		node = list_entry(pos, AclPlcyNode, List);

		switch (node->RuleType) {
		case ACL_ADD_SDMAC_ANY:
			/* 1. SDrc Mac
			 */
			if (memcmp(node->Mac, NewRateReach->Mac, ETH_ALEN) == 0) {
				return node->up;
			}
			break;
		case ACL_ADD_ETYPE_ANY:
			/* 1. ETHERTYPE
			 */
			if (memcmp
			    (&node->Ethertype, &NewRateReach->Ethertype,
			     2) == 0) {
				return node->up;
			}
			break;
		case ACL_ADD_SMAC_DIP_ANY:
			/* 1. Src Mac
			 * 2. Dest IP
			 * 3. Src Mac + Dest Ip
			 */
			if ((memcmp(node->Mac, NewRateReach->Mac, ETH_ALEN)
			     == 0 || memcmp(node->Mac, ZeroMac, ETH_ALEN) == 0)
			    &&
			    ((node->DipS <= NewRateReach->Dip
			      && NewRateReach->Dip <= node->DipE)
			     || node->DipE == 0)) {
				return node->up;
			}
			break;
		case ACL_ADD_SMAC_DIP_TCP:
		case ACL_ADD_SMAC_DIP_UDP:
			/* 1. Src Mac (TCP/UDP)
			 * 2. Dest Ip (TCP/UDP)
			 * 3. Src Mac + Dest Ip (TCP/UDP)
			 * 4. Dest Port ONLY (TCP/UDP)
			 * 5. Src Mac + Dest Port (TCP/UDP)
			 * 6. Dest Ip + Dest Port (TCP/UDP)
			 * 7. Src Mac + Dest Ip + Dest Port (TCP/UDP)
			 */
			if ((memcmp(node->Mac, NewRateReach->Mac, ETH_ALEN)
			     == 0 || memcmp(node->Mac, ZeroMac, ETH_ALEN) == 0)
			    &&
			    ((node->DipS <= NewRateReach->Dip
			      && NewRateReach->Dip <= node->DipE)
			     || node->DipE == 0)
			    &&
			    ((node->DpS <= NewRateReach->Dp
			      && NewRateReach->Dp <= node->DpE)
			     || node->DpE == 0)
			    && (node->Proto == NewRateReach->Proto)) {
				return node->up;
			}
			break;
		case ACL_ADD_SIP_DIP_ANY:
			/* 1. Src IP
			 * 2. Dest IP
			 * 3. Src IP + Dest IP
			 */
			if (((node->SipS <= NewRateReach->Sip &&
			      NewRateReach->Sip <= node->SipE)
			     || node->SipE == 0)
			    &&
			    ((node->DipS <= NewRateReach->Dip
			      && NewRateReach->Dip <= node->DipE)
			     || node->DipE == 0)) {
				return node->up;
			}
			break;
		case ACL_ADD_SIP_DIP_TCP:
		case ACL_ADD_SIP_DIP_UDP:
			/* 1. Dest IP (TCP/UDP)
			 * 2. Dest Port ONLY (TCP/UDP)
			 * 3. Dest IP + Dest Port (TCP/UDP)
			 */
			if (((node->SipS <= NewRateReach->Sip &&
			      NewRateReach->Sip <= node->SipE)
			     || node->SipE == 0)
			    &&
			    ((node->DipS <= NewRateReach->Dip
			      && NewRateReach->Dip <= node->DipE)
			     || node->DipE == 0)
			    &&
			    ((node->DpS <= NewRateReach->Dp
			      && NewRateReach->Dp <= node->DpE)
			     || node->DpE == 0)
			    && (node->Proto == NewRateReach->Proto)) {
				return node->up;
			}
			break;
		case ACL_ADD_SMAC_DMAC_ETYPE_VID_SIP_DIP_TOS_PORT:
			if ((memcmp(node->Mac, NewRateReach->Mac, ETH_ALEN)
			     == 0 || memcmp(node->Mac, ZeroMac, ETH_ALEN) == 0)
			    &&
			    (memcmp
			     (node->DMac, NewRateReach->DMac,
			      ETH_ALEN) == 0
			     || memcmp(node->DMac, ZeroMac, ETH_ALEN) == 0)
			    && ((node->Protocol == NewRateReach->Protocol)
				|| node->Protocol == 0)
			    &&
			    ((node->SipS <= NewRateReach->Sip
			      && NewRateReach->Sip <= node->SipE)
			     || node->SipE == 0)
			    &&
			    ((node->DipS <= NewRateReach->Dip
			      && NewRateReach->Dip <= node->DipE)
			     || node->DipE == 0)
			    &&
			    ((node->DpS <= NewRateReach->Dp
			      && NewRateReach->Dp <= node->DpE)
			     || node->DpE == 0)
			    &&
			    ((node->SpS <= NewRateReach->Sp
			      && NewRateReach->Sp <= node->SpE)
			     || node->SpE == 0)
			    &&
			    ((node->TosS <= NewRateReach->Tos
			      && NewRateReach->Tos <= node->TosE)
			     || node->TosE == 0)
			    && ((node->Vid == NewRateReach->Vid)
				|| node->TosE == 0)
			    && ((node->Ethertype == NewRateReach->Ethertype)
				|| node->Ethertype == 0)
			    && (node->Proto == NewRateReach->Proto)) {
				return node->up;
			}
			break;
		}
	}

	return 0;		/*default UP=0 */
}

uint32_t AclAddNode(AclPlcyNode * NewNode)
{
	AclPlcyNode *node = NULL;

	if ((node = AclExistNode(NewNode))) {
		return ACL_SUCCESS;
	}

	node = (AclPlcyNode *) kmalloc(sizeof(AclPlcyNode), GFP_ATOMIC);

	if (node == NULL) {
		return ACL_FAIL;
	}

	memcpy(node, NewNode, sizeof(AclPlcyNode));
	list_add_tail(&node->List, &AclPlcyList.List);

	return SyncAclTbl();
}

uint32_t AclDelNode(AclPlcyNode * DelNode)
{
	struct list_head *pos = NULL, *tmp;
	AclPlcyNode *node;

	list_for_each_safe(pos, tmp, &AclPlcyList.List) {
		node = list_entry(pos, AclPlcyNode, List);

		switch (DelNode->RuleType) {
		case ACL_DEL_SDMAC_ANY:
			if (memcmp(node->Mac, DelNode->Mac, ETH_ALEN) == 0) {
				goto found;
			}
			break;
		case ACL_DEL_ETYPE_ANY:
			if (node->Ethertype == DelNode->Ethertype) {
				goto found;
			}
			break;
		case ACL_DEL_SMAC_DIP_ANY:
			if ((memcmp(node->Mac, DelNode->Mac, ETH_ALEN) ==
			     0) && node->DipS == DelNode->DipS
			    && node->DipE == DelNode->DipE) {
				goto found;
			}
			break;
		case ACL_DEL_SMAC_DIP_TCP:
		case ACL_DEL_SMAC_DIP_UDP:
			if ((memcmp(node->Mac, DelNode->Mac, ETH_ALEN) ==
			     0) && node->DipS == DelNode->DipS
			    && node->DipE == DelNode->DipE
			    && node->DpS == DelNode->DpS
			    && node->DpE == DelNode->DpE
			    && node->Proto == DelNode->Proto) {
				goto found;
			}
			break;
		case ACL_DEL_SIP_DIP_ANY:
			if (node->SipS == DelNode->SipS
			    && node->SipE == DelNode->SipE
			    && node->DipS == DelNode->DipS
			    && node->DipE == DelNode->DipE) {
				goto found;
			}
			break;
		case ACL_DEL_SIP_DIP_TCP:
		case ACL_DEL_SIP_DIP_UDP:
			if (node->SipS == DelNode->SipS
			    && node->SipE == DelNode->SipE
			    && node->DipS == DelNode->DipS
			    && node->DipE == DelNode->DipE
			    && node->DpS == DelNode->DpS
			    && node->DpE == DelNode->DpE
			    && node->Proto == DelNode->Proto) {
				goto found;
			}
			break;
		case ACL_DEL_SMAC_DMAC_ETYPE_VID_SIP_DIP_TOS_PORT:
			if ((memcmp(node->Mac, DelNode->Mac, ETH_ALEN) == 0)
			    && (memcmp(node->DMac, DelNode->DMac, ETH_ALEN)
				== 0) && node->Vid == DelNode->Vid
			    && node->Protocol == DelNode->Protocol
			    && node->SipS == DelNode->SipS
			    && node->SipE == DelNode->SipE
			    && node->SpS == DelNode->SpS
			    && node->SpE == DelNode->SpE
			    && node->TosS == DelNode->TosS
			    && node->TosE == DelNode->TosE
			    && node->DipS == DelNode->DipS
			    && node->DipE == DelNode->DipE
			    && node->DpS == DelNode->DpS
			    && node->DpE == DelNode->DpE
			    && node->Ethertype == DelNode->Ethertype
			    && node->Proto == DelNode->Proto) {
				goto found;
			}
			break;
		}
	}

	return ACL_FAIL;

      found:
	list_del(pos);
	kfree(node);
	return SyncAclTbl();
}

/*
 * Pre ACL Function
 */
uint32_t PpeGetPreAclEbl(void)
{
	uint32_t PpeFlowSet = 0;

	PpeFlowSet = RegRead(PPE_FLOW_SET);

	if ((PpeFlowSet & ~BIT_FUC_ACL) ||
	    (PpeFlowSet & ~BIT_FMC_ACL) || (PpeFlowSet & ~BIT_FBC_ACL)) {
		return 1;
	} else {
		return 0;
	}
}

void PpeSetPreAclEbl(uint32_t AclEbl)
{
	uint32_t PpeFlowSet = 0;

	PpeFlowSet = RegRead(PPE_FLOW_SET);

	/* ACL engine for unicast/multicast/broadcast flow */
	if (AclEbl == 1) {
		PpeFlowSet |= (BIT_FUC_ACL);
#if defined (CONFIG_RA_HW_NAT_IPV6)
		if (ipv6_offload)
			PpeFlowSet |= (BIT_IPV6_PE_EN);
#endif
	} else {
		/* Set Pre ACL Table */
		PpeFlowSet &= ~(BIT_FUC_ACL | BIT_FMC_ACL | BIT_FBC_ACL);
#if defined (CONFIG_RA_HW_NAT_IPV6)
		PpeFlowSet &= ~(BIT_IPV6_PE_EN);
#endif
		PpeRstPreAclPtr();
	}

	RegWrite(PPE_FLOW_SET, PpeFlowSet);

}

uint16_t PpeGetPreAclStr(void)
{
	uint32_t PpePreAcl = 0;

	PpePreAcl = RegRead(PPE_PRE_ACL);
	return PpePreAcl & 0x1FF;
}

void PpeSetPreAclStr(uint16_t PreAclStr)
{
	RegModifyBits(PPE_PRE_ACL, PreAclStr, 0, 9);
}

uint16_t PpeGetPreAclEnd(void)
{
	uint32_t PpePreAcl = 0;

	PpePreAcl = RegRead(PPE_PRE_ACL);
	return (PpePreAcl >> 16) & 0x1FF;
}

void PpeSetPreAclEnd(uint16_t PreAclEnd)
{
	RegModifyBits(PPE_PRE_ACL, PreAclEnd, 16, 9);
}

void inline PpeInsAclEntry(void *Rule)
{
	uint32_t Index = 0;
	uint32_t *p = (uint32_t *) Rule;

	Index = PpeGetPreAclEnd();

	NAT_DEBUG("Policy Table Base=%08X Offset=%d\n", POLICY_TBL_BASE, Index * 8);
	NAT_DEBUG("%08X: %08X\n", POLICY_TBL_BASE + Index * 8, *p);
	NAT_DEBUG("%08X: %08X\n", POLICY_TBL_BASE + Index * 8 + 4, *(p + 1));

	RegWrite(POLICY_TBL_BASE + Index * 8, *p);	/* Low bytes */
	RegWrite(POLICY_TBL_BASE + Index * 8 + 4, *(p + 1));	/* High bytes */

	/* Update PRE_ACL_END */
	RegModifyBits(PPE_PRE_ACL, Index + 1, 16, 9);
}

uint32_t AclInsDflAllow(void)
{
	/* Allow ARP Packet */
	struct l2_rule L2Rule;

	memset(&L2Rule, 0, sizeof(L2Rule));
	L2Rule.com.fpp.ee = 1;
	L2Rule.com.fpp.fpp = 0;
	L2Rule.com.fpp.fpn = FPN_ALLOW;
	L2Rule.com.dir = OTHERS;
	L2Rule.com.rt = L2_RULE;
	L2Rule.com.pn = PN_DONT_CARE;
	L2Rule.com.match = 1;

	L2Rule.others.e = 1;	/* eth type */
	L2Rule.others.etyp_pprot = ETH_P_ARP;

	PpeInsAclEntry(&L2Rule);
	return ACL_SUCCESS;

}

uint32_t AclInsDflDeny(void)
{
	struct l2_rule L2Rule;

	memset(&L2Rule, 0, sizeof(L2Rule));
	L2Rule.com.dir = OTHERS;
	L2Rule.com.rt = L2_RULE;
	L2Rule.com.pn = PN_DONT_CARE;
	L2Rule.com.match = 1;

	L2Rule.com.fpp.ee = 1;
	L2Rule.com.fpp.fpp = 0;
	L2Rule.com.fpp.fpn = FPN_DROP;

	L2Rule.others.v = 0;
	L2Rule.others.vid = 0;

	PpeInsAclEntry(&L2Rule);
	return ACL_SUCCESS;

}

/* Insert Layer2 Rule */
uint32_t
AclSetMacEntry(AclPlcyNode * node, enum L2RuleDir Dir, enum FoeTblEE End)
{
	struct l2_rule L2Rule;

	memset(&L2Rule, 0, sizeof(L2Rule));

	if (Dir == OTHERS) {
		if (node->Ethertype != 0) {
			L2Rule.others.etyp_pprot = node->Ethertype;
			L2Rule.others.e = 1;
		} else if (node->SpecialTag != 0) {
			L2Rule.others.etyp_pprot = node->SpecialTag;
			L2Rule.others.s = 1;
		}

		if (node->Vid != 0) {
			L2Rule.others.vid = node->Vid;
			L2Rule.others.v = 1;
		}
	} else {
		if (Dir == DMAC)
			memcpy(&L2Rule.mac, node->DMac, ETH_ALEN);
		else
			memcpy(&L2Rule.mac, node->Mac, ETH_ALEN);

		MacReverse(L2Rule.mac);
	}
	L2Rule.com.rt = L2_RULE;
	L2Rule.com.dir = Dir;
#if defined (CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_RT6855A)
	L2Rule.com.pn = PN_DONT_CARE;
#else
	L2Rule.com.pn = node->pn;
#endif
	L2Rule.com.match = 1;

	switch (End) {
	case ENTRY_END_FOE:
		L2Rule.com.foe.ee = 1;
		L2Rule.com.foe.foe = 1;
		L2Rule.com.foe.foe_tb = node->FoeTb;
		break;
	case ENTRY_END_FP:
		L2Rule.com.fpp.ee = 1;
		L2Rule.com.fpp.fpp = 0;

		if (node->Method == ACL_ALLOW_RULE) {
			L2Rule.com.fpp.up = node->up;
			L2Rule.com.fpp.fpn = FPN_ALLOW;
		} else if (node->Method == ACL_PRIORITY_RULE) {
			L2Rule.com.fpp.up = node->up;
			L2Rule.com.fpp.fpn = FPN_FRC_PRI_ONLY;
		} else {
			L2Rule.com.fpp.fpn = FPN_DROP;
		}
		break;
	case NOT_ENTRY_END:
		L2Rule.com.ee_0.ee = 0;
		L2Rule.com.ee_0.logic = AND;
		break;
	default:
		return ACL_FAIL;
	}

	PpeInsAclEntry(&L2Rule);

	return ACL_SUCCESS;
}

uint32_t AclSetIpFragEntry(AclPlcyNode * node, enum FoeTblEE End)
{
	struct l3_rule L3Rule;

	memset(&L3Rule, 0, sizeof(L3Rule));

	/* 
	 * MFV  MF  FOV  FOZ
	 * ---+---+----+----
	 *  1   1    1    1   MoreFrag=1 & Offset=0 ->First Frag Pkt
	 *  1   1    1    0   MoreFrag=1 & Offset!=0 ->Middle Frag Pkts
	 *  1   0    1    0   MoreFrag=0 & Offset!=0 ->Last Frag Pkts
	 *  1   0    1    1   MoreFrag=0 & Offset=0 -> Normal Pkt
	 *
	 *  HINT: Fragment Pkt = "NOT Normal Pkt"
	 */
	L3Rule.com.dir = IP_QOS;
	L3Rule.com.match = 0;	/* NOT Equal */
#if defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT6855A)
	L3Rule.com.pn = PN_DONT_CARE;
#else
	L3Rule.com.pn = node->pn;
#endif
	L3Rule.com.rt = L3_RULE;
	L3Rule.qos.tos_s = 0;
	L3Rule.qos.tos_e = 255;
	L3Rule.qos.mfv = 1;
	L3Rule.qos.mf = 0;
	L3Rule.qos.fov = 1;
	L3Rule.qos.foz = 1;
	L3Rule.qos.v4 = 1;

	switch (End) {
	case ENTRY_END_FOE:
		L3Rule.com.foe.ee = 1;
		L3Rule.com.foe.foe = 1;
		L3Rule.com.foe.foe_tb = node->FoeTb;
		break;
	case ENTRY_END_FP:
		L3Rule.com.fpp.ee = 1;
		L3Rule.com.fpp.fpp = 0;

		if (node->Method == ACL_ALLOW_RULE) {
			L3Rule.com.fpp.up = node->up;
			L3Rule.com.fpp.fpn = FPN_ALLOW;
		} else if (node->Method == ACL_PRIORITY_RULE) {
			L3Rule.com.fpp.up = node->up;
			L3Rule.com.fpp.fpn = FPN_FRC_PRI_ONLY;
		} else {
			L3Rule.com.fpp.fpn = FPN_DROP;
		}
		break;
	case NOT_ENTRY_END:
		L3Rule.com.ee_0.ee = 0;
		L3Rule.com.ee_0.logic = AND;
		break;
	default:
		return ACL_FAIL;
	}

	PpeInsAclEntry(&L3Rule);

	return ACL_SUCCESS;
}

/* Insert Layer3 Rule */
uint32_t
AclSetIpEntry(AclPlcyNode * node, enum L3RuleDir Dir, enum FoeTblEE End)
{
	struct l3_rule L3Rule;
	uint8_t E, M;

	memset(&L3Rule, 0, sizeof(L3Rule));

	switch (Dir) {
	case SIP:
		CalIpRange(node->SipS, node->SipE, &M, &E);
		L3Rule.ip.ip = node->SipS;
		break;
	case DIP:
		CalIpRange(node->DipS, node->DipE, &M, &E);
		L3Rule.ip.ip = node->DipS;
		break;
	case IP_QOS:
		L3Rule.qos.tos_s = node->TosS;
		L3Rule.qos.tos_e = node->TosE;
		L3Rule.qos.v4 = 1;
		break;
	default:
		return ACL_FAIL;
	}

	L3Rule.com.dir = Dir;
	L3Rule.com.match = 1;
#if defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT6855A)
	L3Rule.com.pn = PN_DONT_CARE;
#else
	L3Rule.com.pn = node->pn;
#endif
	L3Rule.com.rt = L3_RULE;
	if (Dir != IP_QOS) {
		L3Rule.ip.ip_rng_m = M;
		L3Rule.ip.ip_rng_e = E;
		L3Rule.ip.v4 = 1;
	}

	switch (End) {
	case ENTRY_END_FOE:
		L3Rule.com.foe.ee = 1;
		L3Rule.com.foe.foe = 1;
		L3Rule.com.foe.foe_tb = node->FoeTb;
		break;
	case ENTRY_END_FP:
		L3Rule.com.fpp.ee = 1;
		L3Rule.com.fpp.fpp = 0;

		if (node->Method == ACL_ALLOW_RULE) {
			L3Rule.com.fpp.up = node->up;
			L3Rule.com.fpp.fpn = FPN_ALLOW;
		} else if (node->Method == ACL_PRIORITY_RULE) {
			L3Rule.com.fpp.up = node->up;
			L3Rule.com.fpp.fpn = FPN_FRC_PRI_ONLY;
		} else {
			L3Rule.com.fpp.fpn = FPN_DROP;
		}

		break;
	case NOT_ENTRY_END:
		L3Rule.com.ee_0.ee = 0;
		L3Rule.com.ee_0.logic = AND;
		break;
	default:
		return ACL_FAIL;
	}

	PpeInsAclEntry(&L3Rule);

	return ACL_SUCCESS;

}

uint32_t
AclSetProtoEntry(AclPlcyNode * node, enum FoeTblTcpUdp Proto, enum FoeTblEE End)
{
	struct l4_rule L4Rule;

	memset(&L4Rule, 0, sizeof(L4Rule));

	L4Rule.com.match = 1;
#if defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT6855A)
	L4Rule.com.pn = PN_DONT_CARE;
#else
	L4Rule.com.pn = node->pn;
#endif
	L4Rule.com.rt = L4_RULE;
	L4Rule.ip.prot = FLT_IP_PROT;

	if (Proto == TCP) {
		L4Rule.ip.prot = 6;	//TCP
	} else {
		L4Rule.ip.prot = 17;	//UDP
	}

	switch (End) {
	case ENTRY_END_FOE:
		L4Rule.com.foe.ee = 1;
		L4Rule.com.foe.foe = 1;
		L4Rule.com.foe.foe_tb = node->FoeTb;
		break;
	case ENTRY_END_FP:
		L4Rule.com.fpp.ee = 1;
		L4Rule.com.fpp.fpp = 0;
		if (node->Method == ACL_ALLOW_RULE) {
			L4Rule.com.fpp.up = node->up;
			L4Rule.com.fpp.fpn = FPN_ALLOW;
		} else if (node->Method == ACL_PRIORITY_RULE) {
			L4Rule.com.fpp.up = node->up;
			L4Rule.com.fpp.fpn = FPN_FRC_PRI_ONLY;
		} else {
			L4Rule.com.fpp.fpn = FPN_DROP;
		}
		break;
	case NOT_ENTRY_END:
		L4Rule.com.ee_0.ee = 0;
		L4Rule.com.ee_0.logic = AND;
		break;
	}

	PpeInsAclEntry(&L4Rule);

	return ACL_SUCCESS;

}

/* Insert Layer4 Rule */
uint32_t
AclSetPortEntry(AclPlcyNode * node, enum L4RuleDir Dir,
		enum FoeTblTcpUdp Proto, enum FoeTblEE End)
{
	struct l4_rule L4Rule;

	memset(&L4Rule, 0, sizeof(L4Rule));
	L4Rule.com.dir = Dir;
	L4Rule.com.match = 1;
#if defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT6855A)
	L4Rule.com.pn = PN_DONT_CARE;
#else
	L4Rule.com.pn = node->pn;
#endif
	L4Rule.com.rt = L4_RULE;

	switch (Dir) {
	case SPORT:
		L4Rule.p_start = node->SpS;
		L4Rule.p_end = node->SpE;
		break;
	case DPORT:
		L4Rule.p_start = node->DpS;
		L4Rule.p_end = node->DpE;
		break;
	case DONT_CARE:
		break;
	default:		//invalid Dir for PortEntry
		return ACL_FAIL;
	}
	if (Dir == DONT_CARE) {

		L4Rule.ip.tu = FLT_IP_PROT;
		L4Rule.ip.prot = node->Protocol;
		NAT_PRINT("Protocol is 0x%2x!\n", node->Protocol);
	} else {

		if (Proto == TCP) {
			L4Rule.tcp.tu = FLT_TCP;
			L4Rule.tcp.tcp_fop = EQUAL;
			L4Rule.tcp.tcp_fm = 0x3F;
		} else if (Proto == UDP) {
			L4Rule.udp.tu = FLT_UDP;
		} else {
			L4Rule.tcp.tu = FLT_TCP_UDP;
			L4Rule.tcp.tcp_fop = EQUAL;
			L4Rule.tcp.tcp_fm = 0x3F;
		}
	}
	switch (End) {
	case ENTRY_END_FOE:
		L4Rule.com.foe.ee = 1;
		L4Rule.com.foe.foe = 1;
		L4Rule.com.foe.foe_tb = node->FoeTb;
		break;
	case ENTRY_END_FP:
		L4Rule.com.fpp.ee = 1;
		L4Rule.com.fpp.fpp = 0;
		if (node->Method == ACL_ALLOW_RULE) {
			L4Rule.com.fpp.up = node->up;
			L4Rule.com.fpp.fpn = FPN_ALLOW;
		} else if (node->Method == ACL_PRIORITY_RULE) {
			L4Rule.com.fpp.up = node->up;
			L4Rule.com.fpp.fpn = FPN_FRC_PRI_ONLY;
		} else {
			L4Rule.com.fpp.fpn = FPN_DROP;
		}
		break;
	case NOT_ENTRY_END:
		L4Rule.com.ee_0.ee = 0;
		L4Rule.com.ee_0.logic = AND;
		break;
	default:
		return ACL_FAIL;
	}

	PpeInsAclEntry(&L4Rule);

	return ACL_SUCCESS;

}

uint32_t AclInsSDmac(AclPlcyNode * node)
{
	//Insert SDMAC Entry 
	AclSetMacEntry(node, SDMAC, ENTRY_END_FP);

	return ACL_SUCCESS;
}

uint32_t AclInsEthertype(AclPlcyNode * node)
{
	//Insert Ethertype Entry 
	AclSetMacEntry(node, OTHERS, ENTRY_END_FP);

	return ACL_SUCCESS;
}

/*print ACL table*/
uint8_t AclGetAllEntries(struct acl_list_args * opt)
{
	struct list_head *pos = NULL, *tmp;
	AclPlcyNode *node = NULL;
	int count = 0;		/* valid entry count */

	list_for_each_safe(pos, tmp, &AclPlcyList.List) {
		node = list_entry(pos, AclPlcyNode, List);

		memcpy(opt->entries[count].mac, node->Mac, ETH_ALEN);
		memcpy(opt->entries[count].dmac, node->DMac, ETH_ALEN);
		opt->entries[count].ethertype = node->Ethertype;
		opt->entries[count].protocol = node->Protocol;
		opt->entries[count].sip_s = node->SipS;
		opt->entries[count].sip_e = node->SipE;
		opt->entries[count].dip_s = node->DipS;
		opt->entries[count].dip_e = node->DipE;
		opt->entries[count].dp_s = node->DpS;
		opt->entries[count].dp_e = node->DpE;
		opt->entries[count].sp_s = node->SpS;
		opt->entries[count].sp_e = node->SpE;
		opt->entries[count].tos_s = node->TosS;
		opt->entries[count].tos_e = node->TosE;
		opt->entries[count].vid = node->Vid;
		opt->entries[count].up = node->up;
		opt->entries[count].L4 = node->Proto;
		opt->entries[count].method = node->Method;
		opt->entries[count].pn = node->pn;

		count++;
	}
	opt->num_of_entries = count;
	return ACL_SUCCESS;
}

uint32_t AclInsSmacETypTOSSipSpDipDp(AclPlcyNode * node)
{
	uint8_t IgnoreMac[6] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
	uint16_t IgnoreEthertype = 0x0;
	uint32_t IgnoreSipS = 0x0;
	uint32_t IgnoreDipS = 0x0;
	uint16_t IgnoreSpS = 0x0;
	uint16_t IgnoreDpS = 0x0;
	uint16_t IgnoreVid = 0x0;
	uint16_t IgnoreTOS = 0x0;
	uint16_t IgnoreProt = 0x0;
	uint16_t IgnoreSpecialTag = 0x0;

#if defined (CONFIG_RALINK_RT6855) || defined (CONFIG_RALINK_RT6855A)
	if (node->pn != PN_DONT_CARE)
		node->SpecialTag = (0x8100 | node->pn);
#endif

	//Insert SMAC Entry 
	if (memcmp(node->Mac, IgnoreMac, ETH_ALEN) != 0) {
		if ((memcmp(node->DMac, IgnoreMac, ETH_ALEN) != 0) ||
		    (node->Protocol != IgnoreProt) ||
		    (node->Ethertype != IgnoreEthertype) ||
		    (node->Vid != IgnoreVid) ||
		    (node->SipS != IgnoreSipS) ||
		    (node->DipS != IgnoreDipS) ||
		    (node->TosS != IgnoreTOS) ||
		    (node->SpS != IgnoreSpS) || (node->DpS != IgnoreDpS))
			AclSetMacEntry(node, SMAC, NOT_ENTRY_END);
		else {
			AclSetMacEntry(node, SMAC, ENTRY_END_FP);
			return ACL_SUCCESS;
		}
	}
	//Insert DMAC Entry 
	if (memcmp(node->DMac, IgnoreMac, ETH_ALEN) != 0) {
		if ((node->Ethertype != IgnoreEthertype) ||
		    (node->Protocol != IgnoreProt) ||
		    (node->Vid != IgnoreVid) ||
		    (node->SipS != IgnoreSipS) ||
		    (node->DipS != IgnoreDipS) ||
		    (node->TosS != IgnoreTOS) ||
		    (node->SpS != IgnoreSpS) || (node->DpS != IgnoreDpS))
			AclSetMacEntry(node, DMAC, NOT_ENTRY_END);
		else {
			AclSetMacEntry(node, DMAC, ENTRY_END_FP);
			return ACL_SUCCESS;
		}
	}
	//Insert Ethertype/VID Entry 
	if ((node->Ethertype != IgnoreEthertype)
	    || (node->Vid != IgnoreVid) || node->SpecialTag != IgnoreSpecialTag) {
		if ((node->Protocol != IgnoreProt) ||
		    (node->SipS != IgnoreSipS) ||
		    (node->DipS != IgnoreDipS) ||
		    (node->TosS != IgnoreTOS) ||
		    (node->SpS != IgnoreSpS) || (node->DpS != IgnoreDpS))
			AclSetMacEntry(node, OTHERS, NOT_ENTRY_END);
		else {
			AclSetMacEntry(node, OTHERS, ENTRY_END_FP);
			return ACL_SUCCESS;
		}
	}

	if (node->TosS != IgnoreTOS) {
		if ((node->Protocol != IgnoreProt) ||
		    (node->SipS != IgnoreSipS) ||
		    (node->DipS != IgnoreDipS) ||
		    (node->SpS != IgnoreSpS) || (node->DpS != IgnoreDpS))
			AclSetIpEntry(node, IP_QOS, NOT_ENTRY_END);
		else {
			AclSetIpEntry(node, IP_QOS, ENTRY_END_FP);
			return ACL_SUCCESS;
		}
	}

	if (node->SipS != IgnoreSipS) {
		if ((node->Protocol != IgnoreProt) ||
		    (node->DipS != IgnoreDipS) ||
		    (node->SpS != IgnoreSpS) || (node->DpS != IgnoreDpS))
			AclSetIpEntry(node, SIP, NOT_ENTRY_END);
		else {
			AclSetIpEntry(node, SIP, ENTRY_END_FP);
			return ACL_SUCCESS;
		}
	}
	//Insert DIP Entry
	if (node->DipS != IgnoreDipS) {
		if ((node->Protocol != IgnoreProt) ||
		    (node->SpS != IgnoreSpS) || (node->DpS != IgnoreDpS))
			AclSetIpEntry(node, DIP, NOT_ENTRY_END);
		else {
			AclSetIpEntry(node, DIP, ENTRY_END_FP);
			return ACL_SUCCESS;
		}
	}
	//Insert IP Protocol Entry 
	if (node->Protocol != IgnoreProt) {
		if ((node->SpS != IgnoreSpS) || (node->DpS != IgnoreDpS))
			AclSetPortEntry(node, DONT_CARE, ANY, NOT_ENTRY_END);
		else {
			AclSetPortEntry(node, DONT_CARE, ANY, ENTRY_END_FP);
			return ACL_SUCCESS;
		}
	}
	//Insert SPort Entry
	if (node->SpS != IgnoreSpS) {
		if (node->DpS != IgnoreDpS) {
			if (node->Proto == ACL_PROTO_TCP)
				AclSetPortEntry(node, SPORT, TCP,
						NOT_ENTRY_END);
			else if (node->Proto == ACL_PROTO_UDP)
				AclSetPortEntry(node, SPORT, UDP,
						NOT_ENTRY_END);
			else if (node->Proto == ACL_PROTO_ANY)
				AclSetPortEntry(node, SPORT, ANY,
						NOT_ENTRY_END);
		} else {
			if (node->Proto == ACL_PROTO_TCP)
				AclSetPortEntry(node, SPORT, TCP, ENTRY_END_FP);
			else if (node->Proto == ACL_PROTO_UDP)
				AclSetPortEntry(node, SPORT, UDP, ENTRY_END_FP);
			else if (node->Proto == ACL_PROTO_ANY)
				AclSetPortEntry(node, SPORT, ANY, ENTRY_END_FP);
			return ACL_SUCCESS;
		}
	}
	//Insert DPort Entry
	if (node->Proto == ACL_PROTO_TCP) {
		AclSetPortEntry(node, DPORT, TCP, ENTRY_END_FP);
	} else if (node->Proto == ACL_PROTO_UDP) {
		AclSetPortEntry(node, DPORT, UDP, ENTRY_END_FP);
	} else if (node->Proto == ACL_PROTO_ANY) {
		AclSetPortEntry(node, DPORT, ANY, ENTRY_END_FP);
	}

	return ACL_SUCCESS;
}

uint32_t AclInsSmacDipDp(AclPlcyNode * node)
{
	//Insert SMAC Entry 
	AclSetMacEntry(node, SMAC, NOT_ENTRY_END);

	//Insert DIP Entry 
	if (node->RuleType == ACL_ADD_SMAC_DIP_ANY
	    || node->RuleType == ACL_ADD_SIP_DIP_ANY) {
		//Insert DP Entry
		AclSetIpEntry(node, DIP, ENTRY_END_FP);
	} else {
		//Insert DP Entry
		AclSetIpEntry(node, DIP, NOT_ENTRY_END);

		if (node->RuleType == ACL_ADD_SMAC_DIP_TCP
		    || node->RuleType == ACL_ADD_SIP_DIP_TCP) {
			AclSetPortEntry(node, DPORT, TCP, ENTRY_END_FP);
		} else if (node->RuleType == ACL_ADD_SMAC_DIP_UDP
			   || node->RuleType == ACL_ADD_SIP_DIP_UDP) {
			AclSetPortEntry(node, DPORT, UDP, ENTRY_END_FP);
		}
	}

	return ACL_SUCCESS;
}

uint32_t AclInsSipDipDp(AclPlcyNode * node)
{

	//Insert SIP Entry 
	AclSetIpEntry(node, SIP, NOT_ENTRY_END);

	if (node->RuleType == ACL_ADD_SMAC_DIP_ANY
	    || node->RuleType == ACL_ADD_SIP_DIP_ANY) {
		//Insert DIP Entry 
		AclSetIpEntry(node, DIP, ENTRY_END_FP);
	} else {
		//Insert DIP Entry 
		AclSetIpEntry(node, DIP, NOT_ENTRY_END);

		//Insert DP Entry
		if (node->RuleType == ACL_ADD_SMAC_DIP_TCP
		    || node->RuleType == ACL_ADD_SIP_DIP_TCP) {
			AclSetPortEntry(node, DPORT, TCP, ENTRY_END_FP);
		} else if (node->RuleType == ACL_ADD_SMAC_DIP_UDP
			   || node->RuleType == ACL_ADD_SIP_DIP_UDP) {
			AclSetPortEntry(node, DPORT, UDP, ENTRY_END_FP);
		}
	}

	return ACL_SUCCESS;
}

/* Remove all ACL entries */
uint32_t AclCleanTbl(void)
{
	struct list_head *pos = NULL, *tmp;
	AclPlcyNode *node;

	list_for_each_safe(pos, tmp, &AclPlcyList.List) {
		node = list_entry(pos, AclPlcyNode, List);
		list_del(pos);
		kfree(node);
	}

	PpeSetPreAclEbl(0);	// Disable PreAcl Table

	return ACL_SUCCESS;

}
