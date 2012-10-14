#ifndef _RTL8367B_ASICDRV_AUTOFALLBACK_H_
#define _RTL8367B_ASICDRV_AUTOFALLBACK_H_

#include "rtl8367b_asicdrv.h"
#include "rtl8367b_asicdrv_phy.h"

#define AUTOFALLBACK_MAX_TIMEOUT    (1024)

typedef enum
{
    MONITOR_MAX_8K = 0,
    MONITOR_MAX_16K,
    MONITOR_MAX_32K,
    MONITOR_MAX_64K,
    MONITOR_MAX_128K,
    MONITOR_MAX_256K,
    MONITOR_MAX_512K,
    MONITOR_MAX_1M,
    MONITOR_MAX_END
}RTL8367B_AUTOFALLBACK_MONITOR_MAX;

typedef enum
{
    ERR_RATIO_1 = 0,
    ERR_RATIO_2,
    ERR_RATIO_4,
    ERR_RATIO_8,
    ERR_RATIO_16,
    ERR_RATIO_32,
    ERR_RATIO_64,
    ERR_RATIO_128,
    ERR_RATIO_END,
}RTL8367B_AUTOFALLBACK_ERR_RATIO;

typedef enum
{
    LINE_LEVEL_0 = 0,
    LINE_LEVEL_1,
    LINE_LEVEL_2,
    LINE_LEVEL_END,
}RTL8367B_AUTOFALLBACK_LINE_LEVEL;

extern ret_t rtl8367b_setAsicAutoFallBackEnable(rtk_uint32 port, rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicAutoFallBackEnable(rtk_uint32 port, rtk_uint32 *pEnabled);
extern ret_t rtl8367b_setAsicAutoFallBackMaxCount(RTL8367B_AUTOFALLBACK_MONITOR_MAX monitor);
extern ret_t rtl8367b_getAsicAutoFallBackMaxCount(RTL8367B_AUTOFALLBACK_MONITOR_MAX *pMonitor);
extern ret_t rtl8367b_setAsicAutoFallBackErrorRate(RTL8367B_AUTOFALLBACK_ERR_RATIO ratio);
extern ret_t rtl8367b_getAsicAutoFallBackErrorRate(RTL8367B_AUTOFALLBACK_ERR_RATIO *pRatio);
extern ret_t rtl8367b_setAsicAutoFallBackTimeout(rtk_uint32 timeout_ms);
extern ret_t rtl8367b_getAsicAutoFallBackTimeout(rtk_uint32 *pTimeout_ms);
extern ret_t rtl8367b_setAsicAutoFallBackTimeoutIgnore(rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicAutoFallBackTimeoutIgnore(rtk_uint32 *pEnabled);
extern ret_t rtl8367b_getAsicAutoFallBackMonitorCNT(rtk_uint32 port, rtk_uint32 *pMcnt);
extern ret_t rtl8367b_getAsicAutoFallBackErrorCNT(rtk_uint32 port, rtk_uint32 *pEcnt);

#endif /*#ifndef _RTL8367B_ASICDRV_AUTOFALLBACK_H_*/

