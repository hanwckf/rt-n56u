#ifndef _RTL8367B_ASICDRV_RLDP_H_
#define _RTL8367B_ASICDRV_RLDP_H_

#include "rtl8367b_asicdrv.h"

extern ret_t rtl8367b_setAsicRldp(rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicRldp(rtk_uint32 *pEnabled);
extern ret_t rtl8367b_setAsicRldpEnable8051(rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicRldpEnable8051(rtk_uint32 *pEnabled);
extern ret_t rtl8367b_setAsicRldpCompareRandomNumber(rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicRldpCompareRandomNumber(rtk_uint32 *pEnabled);
extern ret_t rtl8367b_setAsicRldpIndicatorSource(rtk_uint32 src);
extern ret_t rtl8367b_getAsicRldpIndicatorSource(rtk_uint32 *pSrc);
extern ret_t rtl8367b_setAsicRldpCheckingStatePara(rtk_uint32 retryCount, rtk_uint32 retryPeriod);
extern ret_t rtl8367b_getAsicRldpCheckingStatePara(rtk_uint32 *pRetryCount, rtk_uint32 *pRetryPeriod);
extern ret_t rtl8367b_setAsicRldpLoopStatePara(rtk_uint32 retryCount, rtk_uint32 retryPeriod);
extern ret_t rtl8367b_getAsicRldpLoopStatePara(rtk_uint32 *pRetryCount, rtk_uint32 *pRetryPeriod);
extern ret_t rtl8367b_setAsicRldpTxPortmask(rtk_uint32 portmask);
extern ret_t rtl8367b_getAsicRldpTxPortmask(rtk_uint32 *pPortmask);
extern ret_t rtl8367b_setAsicRldpMagicNum(ether_addr_t seed);
extern ret_t rtl8367b_getAsicRldpMagicNum(ether_addr_t *pSeed);
extern ret_t rtl8367b_setAsicRldpLoopedPortmask(rtk_uint32 portmask);
extern ret_t rtl8367b_getAsicRldpLoopedPortmask(rtk_uint32 *pPortmask);
extern ret_t rtl8367b_setAsicRldp8051Portmask(rtk_uint32 portmask);
extern ret_t rtl8367b_getAsicRldp8051Portmask(rtk_uint32 *pPortmask);


extern ret_t rtl8367b_getAsicRldpRandomNumber(ether_addr_t *pRandNumber);
extern ret_t rtl8367b_getAsicRldpLoopedPortPair(rtk_uint32 port, rtk_uint32 *pLoopedPair);
extern ret_t rtl8367b_setAsicRlppTrap8051(rtk_uint32 enabled);
extern ret_t rtl8367b_getAsicRlppTrap8051(rtk_uint32 *pEnabled);

ret_t rtl8367b_setAsicRldpLeaveLoopedPortmask(rtk_uint32 portmask);
ret_t rtl8367b_getAsicRldpLeaveLoopedPortmask(rtk_uint32 *pPortmask);
ret_t rtl8367b_setAsicRldpConfiguredLoopedPortmask(rtk_uint32 portmask);
ret_t rtl8367b_getAsicRldpConfiguredLoopedPortmask(rtk_uint32 *pPortmask);
ret_t rtl8367b_setAsicRldpTriggerMode(rtk_uint32 enabled);
ret_t rtl8367b_getAsicRldpTriggerMode(rtk_uint32 *pEnabled);

ret_t rtl8367b_setAsicRldp_mode(rtk_uint32 mode);

#endif /*_RTL8367B_ASICDRV_RLDP_H_*/

