#ifndef _RTL8370_ASICDRV_METER_H_
#define _RTL8370_ASICDRV_METER_H_

#include "rtl8370_asicdrv.h"

#define RTL8370_PAGE_NUMBER    0x800
#define RTL8370_INGRESS_DROP_ALL_THREHSOLD_MAX    0x7FF

enum FLOW_CONTROL_TYPE
{
    FC_EGRESS = 0,
    FC_INGRESS, 
};

extern ret_t rtl8370_setAsicFlowControlSelect(uint32 select);
extern ret_t rtl8370_getAsicFlowControlSelect(uint32 *select);
extern ret_t rtl8370_setAsicFlowControlQueueEgressEnable(uint32 port,uint32 qid, uint32 enabled);
extern ret_t rtl8370_getAsicFlowControlQueueEgressEnable(uint32 port,uint32 qid, uint32* enabled);
extern ret_t rtl8370_setAsicFlowControlPortEgressEnable(uint32 port,uint32 enable);
extern ret_t rtl8370_getAsicFlowControlPortEgressEnable(uint32 port,uint32* enable);
extern ret_t rtl8370_setAsicFlowControlDropAll(uint32 dropall);
extern ret_t rtl8370_getAsicFlowControlDropAll(uint32* dropall);
extern ret_t rtl8370_setAsicFlowControlPauseAllThreshold(uint32 threshold);
extern ret_t rtl8370_getAsicFlowControlPauseAllThreshold(uint32 *threshold);
extern ret_t rtl8370_setAsicFlowControlSystemThreshold(uint32 onThreshold, uint32 offThreshold);
extern ret_t rtl8370_getAsicFlowControlSystemThreshold(uint32 *onThreshold, uint32 *offThreshold);
extern ret_t rtl8370_setAsicFlowControlSharedThreshold(uint32 onThreshold, uint32 offThreshold);
extern ret_t rtl8370_getAsicFlowControlSharedThreshold(uint32 *onThreshold, uint32 *offThreshold);
extern ret_t rtl8370_setAsicFlowControlPortThreshold(uint32 onThreshold, uint32 offThreshold);
extern ret_t rtl8370_getAsicFlowControlPortThreshold(uint32 *onThreshold, uint32 *offThreshold);
extern ret_t rtl8370_setAsicFlowControlPortPrivateThreshold(uint32 onThreshold, uint32 offThreshold);
extern ret_t rtl8370_getAsicFlowControlPortPrivateThreshold(uint32 *onThreshold, uint32 *offThreshold);
extern ret_t rtl8370_setAsicFlowControlSystemDropThreshold(uint32 onThreshold, uint32 offThreshold);
extern ret_t rtl8370_getAsicFlowControlSystemDropThreshold(uint32 *onThreshold, uint32 *offThreshold);
extern ret_t rtl8370_setAsicFlowControlSharedDropThreshold(uint32 onThreshold, uint32 offThreshold);
extern ret_t rtl8370_getAsicFlowControlSharedDropThreshold(uint32 *onThreshold, uint32 *offThreshold);
extern ret_t rtl8370_setAsicFlowControlPortDropThreshold(uint32 onThreshold, uint32 offThreshold);
extern ret_t rtl8370_getAsicFlowControlPortDropThreshold(uint32 *onThreshold, uint32 *offThreshold);
extern ret_t rtl8370_setAsicFlowControlPortPrivateDropThreshold(uint32 onThreshold, uint32 offThreshold);
extern ret_t rtl8370_getAsicFlowControlPortPrivateDropThreshold(uint32 *onThreshold, uint32 *offThreshold);

extern ret_t rtl8370_setAsicEgressFlowControlPortDropGap(uint32 gap);
extern ret_t rtl8370_getAsicEgressFlowControlPortDropGap(uint32 *gap);
extern ret_t rtl8370_setAsicEgressFlowControlQueueDropGap(uint32 gap);
extern ret_t rtl8370_getAsicEgressFlowControlQueueDropGap(uint32 *gap);
extern ret_t rtl8370_setAsicEgressFlowControlPortDropThreshold(uint32 port, uint32 threshold);
extern ret_t rtl8370_getAsicEgressFlowControlPortDropThreshold(uint32 port, uint32 *threshold);
extern ret_t rtl8370_setAsicEgressFlowControlQueueDropThreshold(uint32 qid, uint32 threshold);
extern ret_t rtl8370_getAsicEgressFlowControlQueueDropThreshold(uint32 qid, uint32 *threshold);
extern ret_t rtl8370_getAsicEgressQueueEmptyPortMask(uint32 *pmsk);
extern ret_t rtl8370_getAsicTotalPage(uint32 *pageCount);
extern ret_t rtl8370_getAsicPulbicPage(uint32 *pageCount);
extern ret_t rtl8370_getAsicMaxTotalPage(uint32 *pageCount);
extern ret_t rtl8370_getAsicMaxPulbicPage(uint32 *pageCount);
extern ret_t rtl8370_getAsicPortPage(uint32 port, uint32 *pageCount);
extern ret_t rtl8370_getAsicPortPageMax(uint32 port, uint32 *pageCount);

#endif /*_RTL8370_ASICDRV_METER_H_*/

