#ifndef _RTL8370_ASICDRV_RLDP_H_
#define _RTL8370_ASICDRV_RLDP_H_

#include "rtl8370_asicdrv.h"

extern ret_t rtl8370_setAsicRldp(uint32 enable);
extern ret_t rtl8370_getAsicRldp(uint32 *enable);
extern ret_t rtl8370_setAsicRldpEnable8051(uint32 enable);
extern ret_t rtl8370_getAsicRldpEnable8051(uint32 *enable);
extern ret_t rtl8370_setAsicRldpCompareRandomNumber(uint32 enable);
extern ret_t rtl8370_getAsicRldpCompareRandomNumber(uint32 *enable);
extern ret_t rtl8370_setAsicRldpIndicatorSource(uint32 src);
extern ret_t rtl8370_getAsicRldpIndicatorSource(uint32 *src);
extern ret_t rtl8370_setAsicRldpCheckingStatePara(uint32 retryCount, uint32 retryPeriod);
extern ret_t rtl8370_getAsicRldpCheckingStatePara(uint32 *retryCount, uint32 *retryPeriod);
extern ret_t rtl8370_setAsicRldpLoopStatePara(uint32 retryCount, uint32 retryPeriod);
extern ret_t rtl8370_getAsicRldpLoopStatePara(uint32 *retryCount, uint32 *retryPeriod);
extern ret_t rtl8370_setAsicRldpTxPortmask(uint32 pmsk);
extern ret_t rtl8370_getAsicRldpTxPortmask(uint32 *pmsk);
extern ret_t rtl8370_setAsicRldpRandomSeed(ether_addr_t seed);
extern ret_t rtl8370_getAsicRldpRandomSeed(ether_addr_t *seed);
extern ret_t rtl8370_setAsicRldpLoopedPortmask(uint32 pmsk);
extern ret_t rtl8370_getAsicRldpLoopedPortmask(uint32 *pmsk);

extern ret_t rtl8370_getAsicRldpRandomNumber(ether_addr_t *randNumber);
extern ret_t rtl8370_getAsicRldpLoopedPortPair(uint32 port, uint32 *loopedPair);
extern ret_t rtl8370_setAsicRldp_mode(uint32 mode);


#endif /*_RTL8370_ASICDRV_RLDP_H_*/

