/*
    Module Name:
    acl_policy.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Steven Liu  2007-01-23      Initial version
*/

#ifndef _ACL_POLICY_WANTED
#define _ACL_POLICY_WANTED

#include "policy.h"
#include "acl_ioctl.h"
#include "foe_fdb.h"

typedef struct {
	struct list_head List;
	enum AclRuleMethod Method;	/* Allow,Deny */
	enum AclRuleOpt Operate;	/* Add,Del */
	enum AclProtoType Proto;	/* Any,Tcp,Udp */
	uint16_t RuleType;	/* ioctl code */
	uint8_t Mac[6];
	uint8_t DMac[6];
	uint16_t Ethertype;	/* ethertype */
	uint16_t SpecialTag;	/* special tag to identify ESW port */
	uint16_t Protocol;	/* protocol of IP header */
	uint32_t SipS;		/* start of sip */
	uint32_t SipE;		/* end of sip */
	uint32_t DipS;		/* start of dip */
	uint32_t DipE;		/* end of dip */
	uint16_t SpS;		/* start of sport */
	uint16_t SpE;		/* end of sport */
	uint16_t DpS;		/* start of dport */
	uint16_t DpE;		/* end of dport */
	uint16_t FoeTb;		/* Foe Table Entry */
	unsigned int TosS:8;	/*start of TOS */
	unsigned int TosE:8;	/*end of TOS */
	unsigned int Vid:12;	/*Vlan ID */
	unsigned int up:3;	/*acl=>up */
	unsigned int pn:3;	/*Physical Port number */
} AclPlcyNode;

/*acl-up*/
typedef struct {
	enum AclProtoType Proto;	/* Any,Tcp,Udp */
	uint8_t Mac[6];
	uint8_t DMac[6];
	uint16_t Ethertype;	/* ethertype */
	uint16_t Protocol;	/* protocol of IP header */
	uint32_t Sip;		/* source ip */
	uint32_t Dip;		/* destination ip */
	uint16_t Sp;		/* source port */
	uint16_t Dp;		/* destination port */
	unsigned int Tos:8;
	 /*TOS*/ unsigned int Vid:12;	/*Vlan ID */
} AclClassifyKey;

/*
 * EXPORT FUNCTION
 */
uint32_t AclInsDflDeny(void);
uint32_t AclInsDflAllow(void);

uint32_t AclAddNode(AclPlcyNode * NewNode);
uint32_t AclDelNode(AclPlcyNode * NewNode);

uint32_t AclInsSDmac(AclPlcyNode * node);
uint32_t AclInsEthertype(AclPlcyNode * node);
uint32_t AclInsSmacDipDp(AclPlcyNode * node);
uint32_t AclInsSipDipDp(AclPlcyNode * node);
uint32_t AclInsSmacETypTOSSipSpDipDp(AclPlcyNode * node);

uint16_t PpeGetPreAclStr(void);
uint16_t PpeGetPreAclEnd(void);

uint32_t SyncAclTbl(void);
uint32_t AclCleanTbl(void);
uint32_t AclSetIpFragEntry(AclPlcyNode * node, enum FoeTblEE End);
uint32_t AclSetIpEntry(AclPlcyNode * node, enum L3RuleDir Dir,
		       enum FoeTblEE End);

void PpeSetPreAclEbl(uint32_t AclEbl);
AclPlcyNode *AclExistNode(AclPlcyNode * NewNode);
uint8_t AclClassify(AclClassifyKey * NewRateReach);
uint8_t AclGetAllEntries(struct acl_list_args *opt);

#endif
