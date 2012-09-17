/*
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
	AC_MAC_GROUP = 0,
	AC_IP_GROUP = 1,
	AC_VLAN_GROUP = 2
};

enum AcType {
	PRE_AC = 0,
	POST_AC = 1
};

enum AcCntType {
	AC_BYTE_CNT = 0,
	AC_PKT_CNT = 1
};

typedef struct {
	struct list_head List;
	enum AcType Type;
	enum AcRuleType RuleType;
	uint8_t Mac[6];
	uint16_t PortS;		/* start of port */
	uint16_t PortE;		/* end of port */
	uint32_t IpS;		/* start of ip */
	uint32_t IpE;		/* end of ip */
	uint32_t IpProto;	/* ip protocol */
	uint16_t VLAN:12;	/* VLAN ID */
	uint8_t AgIdx;		/* accounting group number */
} AcPlcyNode;

/*
 * EXPORT FUNCTION
 */
uint32_t AcInsMac(AcPlcyNode * node);
uint32_t AcInsIp(AcPlcyNode * node);
uint32_t AcInsVlan(AcPlcyNode * node);

uint32_t AcAddNode(AcPlcyNode * NewNode);
uint32_t AcDelNode(AcPlcyNode * NewNode);

void SyncAcTbl(void);
uint32_t AcCleanTbl(void);

void PpeSetPreAcEbl(uint32_t PreAclEbl);
void PpeSetPostAcEbl(uint32_t PostAclEbl);

uint16_t PpeGetPreAcStr(void);
uint16_t PpeGetPreAcEnd(void);
uint16_t PpeGetPostAcStr(void);
uint16_t PpeGetPostAcEnd(void);

int PpeGetFreeAcGrp(void);
void PpeSetFreeAcGrp(uint32_t AcNum);

AcPlcyNode *AcExistNode(AcPlcyNode * NewNode);
uint32_t AcGetCnt(AcPlcyNode * SearchNode, enum AcCntType AcCntType);

#endif
