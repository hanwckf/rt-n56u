#ifndef _RTL8370_ASICDRV_RRCP_H_
#define _RTL8370_ASICDRV_RRCP_H_

#include "rtl8370_asicdrv.h"

extern ret_t rtl8370_setAsicRrcp(uint32 vOneEnable, uint32 vTwoEnable);
extern ret_t rtl8370_getAsicRrcp(uint32 *vOneEnable, uint32 *vTwoEnable);
extern ret_t rtl8370_setAsicRrcpTrustPortmask(uint32 pmsk);
extern ret_t rtl8370_getAsicRrcpTrustPortmask(uint32 *pmsk);
extern ret_t rtl8370_setAsicRrcpAuthenticationKey(uint32 authKey);
extern ret_t rtl8370_getAsicRrcpAuthenticationKey(uint32 *authKey);
extern ret_t rtl8370_setAsicRrcpPrivateKey(uint32 privateKey);
extern ret_t rtl8370_getAsicRrcpPrivateKey(uint32 *privateKey);
extern ret_t rtl8370_setAsicRrcpV2Trap8051(uint32 trap);
extern ret_t rtl8370_getAsicRrcpV2Trap8051(uint32 *trap);

#endif /*_RTL8370_ASICDRV_RRCP_H_*/

