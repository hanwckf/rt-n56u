#ifndef _RTL8370_ASICDRV_SCHEDULING_H_
#define _RTL8370_ASICDRV_SCHEDULING_H_

#include "rtl8370_asicdrv.h"

#define RTL8370_QWEIGHTMAX    0x7F
#define RTL8370_PORT_QUEUE_METER_INDEX_MAX    7

/* enum for queue type */
enum QUEUETYPE
{
	QTYPE_STRICT = 0,
	QTYPE_WFQ,
};
extern ret_t rtl8370_setAsicLeakyBucketParameter(uint32 tick, uint32 token);
extern ret_t rtl8370_getAsicLeakyBucketParameter(uint32 *tick, uint32 *token);
extern ret_t rtl8370_setAsicAprMeter(uint32 port, uint32 qid, uint32 apridx);
extern ret_t rtl8370_getAsicAprMeter(uint32 port, uint32 qid, uint32 *apridx);
extern ret_t rtl8370_setAsicPprMeter(uint32 port, uint32 qid, uint32 ppridx);
extern ret_t rtl8370_getAsicPprMeter(uint32 port, uint32 qid, uint32 *ppridx);
extern ret_t rtl8370_setAsicAprEnable(uint32 port, uint32 aprEnable);
extern ret_t rtl8370_getAsicAprEnable(uint32 port, uint32 *aprEnable);
extern ret_t rtl8370_setAsicPprEnable(uint32 port, uint32 pprEnable);
extern ret_t rtl8370_getAsicPprEnable(uint32 port, uint32 *pprEnable);

extern ret_t rtl8370_setAsicWFQWeight( uint32, uint32 queueid, uint32 weight );
extern ret_t rtl8370_getAsicWFQWeight( uint32, uint32 queueid, uint32 *weight );
extern ret_t rtl8370_setAsicWFQBurstSize(uint32 burstsize);
extern ret_t rtl8370_getAsicWFQBurstSize(uint32 *burstsize);

extern ret_t rtl8370_setAsicQueueType(uint32 port, uint32 qid, uint32 queueType);
extern ret_t rtl8370_getAsicQueueType(uint32 port, uint32 qid, uint32 *queueType);
extern ret_t rtl8370_setAsicQueueRate( uint32 port, uint32 qid, uint32 ppridx,uint32 apridx );
extern ret_t rtl8370_getAsicQueueRate( uint32 port, uint32 qid, uint32* ppridx,uint32* apridx );
extern ret_t rtl8370_setAsicPortEgressRate(uint32 port, uint32 rate);
extern ret_t rtl8370_getAsicPortEgressRate(uint32 port, uint32 *rate);
extern ret_t rtl8370_setAsicPortEgressRateIfg(uint32 ifg);
extern ret_t rtl8370_getAsicPortEgressRateIfg(uint32 *ifg);

#endif /*_RTL8370_ASICDRV_SCHEDULING_H_*/

