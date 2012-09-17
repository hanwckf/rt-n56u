/*
    Module Name:
    mtr_policy.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Steven Liu  2007-01-23      Initial version
*/

#ifndef _MTR_POLICY_WANTED
#define _MTR_POLICY_WANTED

#include "policy.h"
#include "mtr_ioctl.h"

enum MtrRuleType {
	MTR_MAC_GROUP = 0,
	MTR_IP_GROUP = 1,
	MTR_SYN = 2,
	MTR_FIN = 3,
	MTR_PROTOCOL_UDP = 4,
	MTR_PROTOCOL_ICMP = 5
};

enum MtrType {
	PRE_MTR = 0,
	POST_MTR = 1
};

typedef struct {
	struct list_head List;
	enum MtrType Type;	/* Pre / Post */
	enum MtrRuleType RuleType;
	uint8_t Mac[6];
	uint32_t IpS;		/* start of ip */
	uint32_t IpE;		/* end of ip */
	uint32_t MgNum;		/* meter group */
	uint32_t CurBkSize:15;
	uint32_t TokenRate:14;
	uint32_t BkSize:3;
	union {			//RT2880_MP2 new feature
		struct {
			uint32_t MtrMode:1;
			uint32_t MaxBkSize:2;
			uint32_t TokenRate:14;
			uint32_t CurrBkSize:15;
		} ByteBase;

		struct {
			uint32_t MtrMode:1;
			uint32_t MaxBkSize:7;
			enum MtrInterval MtrIntval:3;
			uint32_t TimeToReFill:14;
			uint32_t CurrBkSize:7;
		} PktBase;

		uint32_t mtr_info;
	};
} MtrPlcyNode;

/*
 * EXPORT FUNCTION
 */

uint32_t MtrInsMac(MtrPlcyNode * node);
uint32_t MtrInsIp(MtrPlcyNode * node);
uint32_t MtrInsSYN(MtrPlcyNode * node);
uint32_t MtrInsFIN(MtrPlcyNode * node);
uint32_t MtrInsProtocol(MtrPlcyNode * node);

uint32_t MtrAddNode(MtrPlcyNode * NewNode);
uint32_t MtrDelNode(MtrPlcyNode * NewNode);

void SyncMtrTbl(void);
uint32_t MtrCleanTbl(void);

void PpeSetPreMtrEbl(uint32_t PreMtrlEbl);
void PpeSetPostMtrEbl(uint32_t PortMtrEbl);

uint16_t PpeGetPreMtrStr(void);
uint16_t PpeGetPreMtrEnd(void);
uint16_t PpeGetPostMtrStr(void);
uint16_t PpeGetPostMtrEnd(void);

int PpeGetFreeMtrGrp(void);
void PpeSetFreeMtrGrp(uint32_t MgNum);

MtrPlcyNode *MtrExistNode(MtrPlcyNode * NewNode);
uint8_t MtrGetAllEntries(struct mtr_list_args *opt);

#endif
