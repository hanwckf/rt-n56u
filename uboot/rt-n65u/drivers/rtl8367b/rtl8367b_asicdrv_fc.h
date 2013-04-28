#ifndef _RTL8367B_ASICDRV_FC_H_
#define _RTL8367B_ASICDRV_FC_H_

#include "rtl8367b_asicdrv.h"

#define RTL8367B_PAGE_NUMBER    0x400


enum FLOW_CONTROL_TYPE
{
    FC_EGRESS = 0,
    FC_INGRESS, 
};

enum FC_JUMBO_SIZE
{
    FC_JUMBO_SIZE_3K = 0,
    FC_JUMBO_SIZE_4K,
    FC_JUMBO_SIZE_6K,
    FC_JUMBO_SIZE_9K,
    FC_JUMBO_SIZE_END,
    
};


extern ret_t rtl8367b_setAsicFlowControlSelect(rtk_uint32 select);
extern ret_t rtl8367b_getAsicFlowControlSelect(rtk_uint32 *pSelect);
extern ret_t rtl8367b_setAsicFlowControlJumboMode(rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicFlowControlJumboMode(rtk_uint32* pEnabled);
extern ret_t rtl8367b_setAsicFlowControlJumboModeSize(rtk_uint32 size);
extern ret_t rtl8367b_getAsicFlowControlJumboModeSize(rtk_uint32* pSize);
extern ret_t rtl8367b_setAsicFlowControlQueueEgressEnable(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicFlowControlQueueEgressEnable(rtk_uint32 port, rtk_uint32 qid, rtk_uint32* pEnabled);
extern ret_t rtl8367b_setAsicFlowControlDropAll(rtk_uint32 dropall);
extern ret_t rtl8367b_getAsicFlowControlDropAll(rtk_uint32* pDropall);
extern ret_t rtl8367b_setAsicFlowControlPauseAllThreshold(rtk_uint32 threshold);
extern ret_t rtl8367b_getAsicFlowControlPauseAllThreshold(rtk_uint32 *pThreshold);
extern ret_t rtl8367b_setAsicFlowControlSystemThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold);
extern ret_t rtl8367b_getAsicFlowControlSystemThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold);
extern ret_t rtl8367b_setAsicFlowControlSharedThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold);
extern ret_t rtl8367b_getAsicFlowControlSharedThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold);
extern ret_t rtl8367b_setAsicFlowControlPortThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold);
extern ret_t rtl8367b_getAsicFlowControlPortThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold);
extern ret_t rtl8367b_setAsicFlowControlPortPrivateThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold);
extern ret_t rtl8367b_getAsicFlowControlPortPrivateThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold);
extern ret_t rtl8367b_setAsicFlowControlSystemDropThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold);
extern ret_t rtl8367b_getAsicFlowControlSystemDropThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold);
extern ret_t rtl8367b_setAsicFlowControlSharedDropThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold);
extern ret_t rtl8367b_getAsicFlowControlSharedDropThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold);
extern ret_t rtl8367b_setAsicFlowControlPortDropThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold);
extern ret_t rtl8367b_getAsicFlowControlPortDropThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold);
extern ret_t rtl8367b_setAsicFlowControlPortPrivateDropThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold);
extern ret_t rtl8367b_getAsicFlowControlPortPrivateDropThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold);
extern ret_t rtl8367b_setAsicFlowControlSystemJumboThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold);
extern ret_t rtl8367b_getAsicFlowControlSystemJumboThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold);
extern ret_t rtl8367b_setAsicFlowControlSharedJumboThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold);
extern ret_t rtl8367b_getAsicFlowControlSharedJumboThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold);
extern ret_t rtl8367b_setAsicFlowControlPortJumboThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold);
extern ret_t rtl8367b_getAsicFlowControlPortJumboThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold);
extern ret_t rtl8367b_setAsicFlowControlPortPrivateJumboThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold);
extern ret_t rtl8367b_getAsicFlowControlPortPrivateJumboThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold);

extern ret_t rtl8367b_setAsicEgressFlowControlPortDropGap(rtk_uint32 gap);
extern ret_t rtl8367b_getAsicEgressFlowControlPortDropGap(rtk_uint32 *pGap);
extern ret_t rtl8367b_setAsicEgressFlowControlQueueDropGap(rtk_uint32 gap);
extern ret_t rtl8367b_getAsicEgressFlowControlQueueDropGap(rtk_uint32 *pGap);
extern ret_t rtl8367b_setAsicEgressFlowControlPortDropThreshold(rtk_uint32 port, rtk_uint32 threshold);
extern ret_t rtl8367b_getAsicEgressFlowControlPortDropThreshold(rtk_uint32 port, rtk_uint32 *pThreshold);
extern ret_t rtl8367b_setAsicEgressFlowControlQueueDropThreshold(rtk_uint32 qid, rtk_uint32 threshold);
extern ret_t rtl8367b_getAsicEgressFlowControlQueueDropThreshold(rtk_uint32 qid, rtk_uint32 *pThreshold);
extern ret_t rtl8367b_getAsicEgressQueueEmptyPortMask(rtk_uint32 *pPortmask);
extern ret_t rtl8367b_getAsicTotalPage(rtk_uint32 *pPageCount);
extern ret_t rtl8367b_getAsicPulbicPage(rtk_uint32 *pPageCount);
extern ret_t rtl8367b_getAsicMaxTotalPage(rtk_uint32 *pPageCount);
extern ret_t rtl8367b_getAsicMaxPulbicPage(rtk_uint32 *pPageCount);
extern ret_t rtl8367b_getAsicPortPage(rtk_uint32 port, rtk_uint32 *pPageCount);
extern ret_t rtl8367b_getAsicPortPageMax(rtk_uint32 port, rtk_uint32 *pPageCount);
extern ret_t rtl8367b_setAsicFlowControlEgressPortIndep(rtk_uint32 port, rtk_uint32 enable);
extern ret_t rtl8367b_getAsicFlowControlEgressPortIndep(rtk_uint32 port, rtk_uint32 *pEnable);

#endif /*_RTL8367B_ASICDRV_FC_H_*/

