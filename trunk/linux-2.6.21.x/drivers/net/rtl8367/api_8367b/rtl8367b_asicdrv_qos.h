#ifndef _RTL8367B_ASICDRV_QOS_H_
#define _RTL8367B_ASICDRV_QOS_H_

#include "rtl8367b_asicdrv.h"

#define RTL8367B_DECISIONPRIMAX    0xFF

/* enum Priority Selection Types */
enum PRIDECISION
{
	PRIDEC_PORT = 0,
	PRIDEC_ACL,
	PRIDEC_DSCP,
	PRIDEC_1Q,
	PRIDEC_1AD,
	PRIDEC_CVLAN,
	PRIDEC_DA,
	PRIDEC_SA,
	PRIDEC_END,
};

extern ret_t rtl8367b_setAsicRemarkingDot1pAbility(rtk_uint32 port, rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicRemarkingDot1pAbility(rtk_uint32 port, rtk_uint32* pEnabled);
extern ret_t rtl8367b_setAsicRemarkingDot1pParameter(rtk_uint32 priority, rtk_uint32 newPriority );
extern ret_t rtl8367b_getAsicRemarkingDot1pParameter(rtk_uint32 priority, rtk_uint32 *pNewPriority );
extern ret_t rtl8367b_setAsicRemarkingDscpAbility(rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicRemarkingDscpAbility(rtk_uint32* pEnabled);
extern ret_t rtl8367b_setAsicRemarkingDscpParameter(rtk_uint32 priority, rtk_uint32 newDscp );
extern ret_t rtl8367b_getAsicRemarkingDscpParameter(rtk_uint32 priority, rtk_uint32* pNewDscp );

extern ret_t rtl8367b_setAsicPriorityDot1qRemapping(rtk_uint32 srcpriority, rtk_uint32 priority );
extern ret_t rtl8367b_getAsicPriorityDot1qRemapping(rtk_uint32 srcpriority, rtk_uint32 *pPriority );
extern ret_t rtl8367b_setAsicPriorityDscpBased(rtk_uint32 dscp, rtk_uint32 priority );
extern ret_t rtl8367b_getAsicPriorityDscpBased(rtk_uint32 dscp, rtk_uint32 *pPriority );
extern ret_t rtl8367b_setAsicPriorityPortBased(rtk_uint32 port, rtk_uint32 priority );
extern ret_t rtl8367b_getAsicPriorityPortBased(rtk_uint32 port, rtk_uint32 *pPriority );
extern ret_t rtl8367b_setAsicPriorityDecision(rtk_uint32 prisrc, rtk_uint32 decisionPri);
extern ret_t rtl8367b_getAsicPriorityDecision(rtk_uint32 prisrc, rtk_uint32* pDecisionPri);
extern ret_t rtl8367b_setAsicPriorityToQIDMappingTable(rtk_uint32 qnum, rtk_uint32 priority, rtk_uint32 qid );
extern ret_t rtl8367b_getAsicPriorityToQIDMappingTable(rtk_uint32 qnum, rtk_uint32 priority, rtk_uint32* pQid);
extern ret_t rtl8367b_setAsicOutputQueueMappingIndex(rtk_uint32 port, rtk_uint32 qnum );
extern ret_t rtl8367b_getAsicOutputQueueMappingIndex(rtk_uint32 port, rtk_uint32 *pQnum );

#endif /*#ifndef _RTL8367B_ASICDRV_QOS_H_*/

