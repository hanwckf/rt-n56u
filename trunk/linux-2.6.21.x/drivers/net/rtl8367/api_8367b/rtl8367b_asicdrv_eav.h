#ifndef _RTL8367B_ASICDRV_EAV_H_
#define _RTL8367B_ASICDRV_EAV_H_

#include "rtl8367b_asicdrv.h"


extern ret_t rtl8367b_setAsicEavEnable(rtk_uint32 port, rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicEavEnable(rtk_uint32 port, rtk_uint32 *pEnabled);
extern ret_t rtl8367b_setAsicEavPriRemapping(rtk_uint32 srcpriority, rtk_uint32 priority);
extern ret_t rtl8367b_getAsicEavPriRemapping(rtk_uint32 srcpriority, rtk_uint32 *pPriority);
extern ret_t rtl8367b_setAsicEavTimeFreq(rtk_uint32 frequence);
extern ret_t rtl8367b_getAsicEavTimeFreq(rtk_uint32* pFrequence);
extern ret_t rtl8367b_setAsicEavTimeOffsetSeccond(rtk_uint32 second);
extern ret_t rtl8367b_getAsicEavTimeOffsetSeccond(rtk_uint32* pSecond);
extern ret_t rtl8367b_setAsicEavTimeOffset512ns(rtk_uint32 ns);
extern ret_t rtl8367b_getAsicEavTimeOffset512ns(rtk_uint32* pNs);
extern ret_t rtl8367b_setAsicEavOffsetTune(rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicEavSystemTimeTransmit(rtk_uint32* pTransmit);
extern ret_t rtl8367b_getAsicEavSystemTimeSeccond(rtk_uint32* pSecond);
extern ret_t rtl8367b_getAsicEavSystemTime512ns(rtk_uint32* pNs);
extern ret_t rtl8367b_setAsicEavTimeSyncEn(rtk_uint32 port, rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicEavTimeSyncEn(rtk_uint32 port, rtk_uint32 *pEnabled);
extern ret_t rtl8367b_setAsicEavTimeStampFillEn(rtk_uint32 port, rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicEavTimeStampFillEn(rtk_uint32 port, rtk_uint32 *pEnabled);
extern ret_t rtl8367b_getAsicEavTimeSyncValid(rtk_uint32 port, rtk_uint32 *pValid);
extern ret_t rtl8367b_getAsicEavEgressTimestampSeccond(rtk_uint32 port, rtk_uint32* pSecond);
extern ret_t rtl8367b_getAsicEavEgressTimestamp512ns(rtk_uint32 port, rtk_uint32* pNs);

#endif /*#ifndef _RTL8367B_ASICDRV_EAV_H_*/

