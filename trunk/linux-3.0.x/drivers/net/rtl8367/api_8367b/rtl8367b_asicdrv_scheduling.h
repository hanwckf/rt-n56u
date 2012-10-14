#ifndef _RTL8367B_ASICDRV_SCHEDULING_H_
#define _RTL8367B_ASICDRV_SCHEDULING_H_

#include "rtl8367b_asicdrv.h"

#define RTL8367B_QWEIGHTMAX    0x7F
#define RTL8367B_PORT_QUEUE_METER_INDEX_MAX    7

/* enum for queue type */
enum QUEUETYPE
{
	QTYPE_STRICT = 0,
	QTYPE_WFQ,
};
extern ret_t rtl8367b_setAsicLeakyBucketParameter(rtk_uint32 tick, rtk_uint32 token);
extern ret_t rtl8367b_getAsicLeakyBucketParameter(rtk_uint32 *tick, rtk_uint32 *token);
extern ret_t rtl8367b_setAsicAprMeter(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 apridx);
extern ret_t rtl8367b_getAsicAprMeter(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 *apridx);
extern ret_t rtl8367b_setAsicPprMeter(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 ppridx);
extern ret_t rtl8367b_getAsicPprMeter(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 *ppridx);
extern ret_t rtl8367b_setAsicAprEnable(rtk_uint32 port, rtk_uint32 aprEnable);
extern ret_t rtl8367b_getAsicAprEnable(rtk_uint32 port, rtk_uint32 *aprEnable);
extern ret_t rtl8367b_setAsicPprEnable(rtk_uint32 port, rtk_uint32 pprEnable);
extern ret_t rtl8367b_getAsicPprEnable(rtk_uint32 port, rtk_uint32 *pprEnable);

extern ret_t rtl8367b_setAsicWFQWeight(rtk_uint32, rtk_uint32 queueid, rtk_uint32 weight );
extern ret_t rtl8367b_getAsicWFQWeight(rtk_uint32, rtk_uint32 queueid, rtk_uint32 *weight );
extern ret_t rtl8367b_setAsicWFQBurstSize(rtk_uint32 burstsize);
extern ret_t rtl8367b_getAsicWFQBurstSize(rtk_uint32 *burstsize);

extern ret_t rtl8367b_setAsicQueueType(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 queueType);
extern ret_t rtl8367b_getAsicQueueType(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 *queueType);
extern ret_t rtl8367b_setAsicQueueRate(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 ppridx, rtk_uint32 apridx );
extern ret_t rtl8367b_getAsicQueueRate(rtk_uint32 port, rtk_uint32 qid, rtk_uint32* ppridx, rtk_uint32* apridx );
extern ret_t rtl8367b_setAsicPortEgressRate(rtk_uint32 port, rtk_uint32 rate);
extern ret_t rtl8367b_getAsicPortEgressRate(rtk_uint32 port, rtk_uint32 *rate);
extern ret_t rtl8367b_setAsicPortEgressRateIfg(rtk_uint32 ifg);
extern ret_t rtl8367b_getAsicPortEgressRateIfg(rtk_uint32 *ifg);

#endif /*_RTL8367B_ASICDRV_SCHEDULING_H_*/

