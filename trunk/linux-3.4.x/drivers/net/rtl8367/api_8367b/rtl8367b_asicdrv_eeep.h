#ifndef _RTL8367B_ASICDRV_EEEP_H_
#define _RTL8367B_ASICDRV_EEEP_H_

#include "rtl8367b_asicdrv.h"

#define RTL8367B_EEEP_SLEEP_STEP_MAX   		0xFF

enum EEEP_TIMER_UNIT
{
    EEEP_TIMER_1US = 0,
    EEEP_TIMER_16US,
    EEEP_TIMER_128US,
    EEEP_TIMER_1024US,
    EEEP_TIMER_END
};

extern ret_t rtl8367b_setAsicEeepTxEnable(rtk_uint32 port, rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicEeepTxEnable(rtk_uint32 port, rtk_uint32 *pEnabled);
extern ret_t rtl8367b_setAsicEeepRxEnable(rtk_uint32 port, rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicEeepRxEnable(rtk_uint32 port, rtk_uint32 *pEnabled);
extern ret_t rtl8367b_setAsicEeepSleepStep(rtk_uint32 step);
extern ret_t rtl8367b_getAsicEeepSleepStep(rtk_uint32* pStep);
extern ret_t rtl8367b_setAsicEeep100mTimeUint(rtk_uint32 unit);
extern ret_t rtl8367b_getAsicEeep100mTimeUint(rtk_uint32* pUnit);
extern ret_t rtl8367b_setAsicEeepGigaTimeUint(rtk_uint32 unit);
extern ret_t rtl8367b_getAsicEeepGigaTimeUint(rtk_uint32* pUnit);
extern ret_t rtl8367b_setAsicEeep100mRxRateTh(rtk_uint32 threshold);
extern ret_t rtl8367b_getAsicEeep100mRxRateTh(rtk_uint32* pThreshold);
extern ret_t rtl8367b_setAsicEeepGigaRxRateTh(rtk_uint32 threshold);
extern ret_t rtl8367b_getAsicEeepGigaRxRateTh(rtk_uint32* pThreshold);
extern ret_t rtl8367b_setAsicEeep100mTxRateTh(rtk_uint32 threshold);
extern ret_t rtl8367b_getAsicEeep100mTxRateTh(rtk_uint32* pThreshold);
extern ret_t rtl8367b_setAsicEeepGigaTxRateTh(rtk_uint32 threshold);
extern ret_t rtl8367b_getAsicEeepGigaTxRateTh(rtk_uint32* pThreshold);
extern ret_t rtl8367b_setAsicEeep100mSleepTimer(rtk_uint32 timer);
extern ret_t rtl8367b_getAsicEeep100mSleepTimer(rtk_uint32* pTimer);
extern ret_t rtl8367b_setAsicEeepGigaSleepTimer(rtk_uint32 timer);
extern ret_t rtl8367b_getAsicEeepGigaSleepTimer(rtk_uint32* pTimer);
extern ret_t rtl8367b_setAsicEeep100mPauseOnTimer(rtk_uint32 timer);
extern ret_t rtl8367b_getAsicEeep100mPauseOnTimer(rtk_uint32* pTimer);
extern ret_t rtl8367b_setAsicEeepGigaPauseOnTimer(rtk_uint32 timer);
extern ret_t rtl8367b_getAsicEeepGigaPauseOnTimer(rtk_uint32* pTimer);
extern ret_t rtl8367b_setAsicEeep100mTxWakeupTimer(rtk_uint32 timer);
extern ret_t rtl8367b_getAsicEeep100mTxWakeupTimer(rtk_uint32* pTimer);
extern ret_t rtl8367b_setAsicEeepGigaTxWakeupTimer(rtk_uint32 timer);
extern ret_t rtl8367b_getAsicEeepGigaTxWakeupTimer(rtk_uint32* pTimer);
extern ret_t rtl8367b_setAsicEeep100mRxRateTimer(rtk_uint32 timer);
extern ret_t rtl8367b_getAsicEeep100mRxRateTimer(rtk_uint32* pTimer);
extern ret_t rtl8367b_setAsicEeepGigaRxRateTimer(rtk_uint32 timer);
extern ret_t rtl8367b_getAsicEeepGigaRxRateTimer(rtk_uint32* pTimer);
extern ret_t rtl8367b_setAsicEeepGigaRxWakeupTimer(rtk_uint32 timer);
extern ret_t rtl8367b_getAsicEeepGigaRxWakeupTimer(rtk_uint32* pTimer);
extern ret_t rtl8367b_setAsicEeep100mTxRateTimer(rtk_uint32 timer);
extern ret_t rtl8367b_getAsicEeep100mTxRateTimer(rtk_uint32* pTimer);
extern ret_t rtl8367b_setAsicEeepGigaTxRateTimer(rtk_uint32 timer);
extern ret_t rtl8367b_getAsicEeepGigaTxRateTimer(rtk_uint32* pTimer);
extern ret_t rtl8367b_setAsicEeep100mRxWakeupTimer(rtk_uint32 timer);
extern ret_t rtl8367b_getAsicEeep100mRxWakeupTimer(rtk_uint32* pTimer);
extern ret_t rtl8367b_setAsicEeepGigaTxRateEnable(rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicEeepGigaTxRateEnable(rtk_uint32* pEnabled);
extern ret_t rtl8367b_setAsicEeep100mTxRateEnable(rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicEeep100mTxRateEnable(rtk_uint32* pEnabled);




#endif /*#ifndef _RTL8367B_ASICDRV_EEEP_H_*/

