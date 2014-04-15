#ifndef _RTL8370_ASICDRV_QOS_H_
#define _RTL8370_ASICDRV_QOS_H_

#include "rtl8370_asicdrv.h"

#define RTL8370_DSCPMAX    63
#define RTL8370_DECISIONPRIMAX    0xFF

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
	PRIDEC_MAX,
};

extern ret_t rtl8370_setAsicRemarkingDot1pAbility(uint32 enabled);
extern ret_t rtl8370_getAsicRemarkingDot1pAbility(uint32* enabled);
extern ret_t rtl8370_setAsicRemarkingDot1pParameter( uint32 priority, uint32 newpriority );
extern ret_t rtl8370_getAsicRemarkingDot1pParameter( uint32 priority, uint32 *newpriority );
extern ret_t rtl8370_setAsicRemarkingDscpAbility(uint32 enabled);
extern ret_t rtl8370_getAsicRemarkingDscpAbility( uint32* enabled);
extern ret_t rtl8370_setAsicRemarkingDscpParameter( uint32 priority, uint32 newdscp );
extern ret_t rtl8370_getAsicRemarkingDscpParameter( uint32 priority, uint32* newdscp );

extern ret_t rtl8370_setAsicPriorityDot1qRemapping( uint32 srcpriority, uint32 priority );
extern ret_t rtl8370_getAsicPriorityDot1qRemapping( uint32 srcpriority, uint32 *priority );
extern ret_t rtl8370_setAsicPriorityDscpBased( uint32 dscp, uint32 priority );
extern ret_t rtl8370_getAsicPriorityDscpBased( uint32 dscp, uint32 *priority );
extern ret_t rtl8370_setAsicPriorityPortBased( uint32 port, uint32 priority );
extern ret_t rtl8370_getAsicPriorityPortBased( uint32 port, uint32 *priority );
extern ret_t rtl8370_setAsicPriorityDecision( enum PRIDECISION prisrc, uint32 decisionpri);
extern ret_t rtl8370_getAsicPriorityDecision( enum PRIDECISION prisrc, uint32* decisionpri);
extern ret_t rtl8370_setAsicPriorityToQIDMappingTable( uint32 qnum, uint32 priority, uint32 qid );
extern ret_t rtl8370_getAsicPriorityToQIDMappingTable(uint32 qnum, uint32 priority, uint32* qid);
extern ret_t rtl8370_setAsicOutputQueueMappingIndex( uint32 port, uint32 qnum );
extern ret_t rtl8370_getAsicOutputQueueMappingIndex( uint32 port, uint32 *qnum );

#endif /*#ifndef _RTL8370_ASICDRV_QOS_H_*/

