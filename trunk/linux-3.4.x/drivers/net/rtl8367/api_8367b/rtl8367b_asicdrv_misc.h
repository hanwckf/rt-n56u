#ifndef _RTL8367B_ASICDRV_MISC_H_
#define _RTL8367B_ASICDRV_MISC_H_

#include "rtl8367b_asicdrv.h"

extern ret_t rtl8367b_setAsicMacAddress(ether_addr_t mac);
extern ret_t rtl8367b_getAsicMacAddress(ether_addr_t *pMac);
extern ret_t rtl8367b_getAsicDebugInfo(rtk_uint32 port, rtk_uint32 *pDebugifo);
extern ret_t rtl8367b_setAsicPortJamMode(rtk_uint32 mode);
extern ret_t rtl8367b_getAsicPortJamMode(rtk_uint32* pMode);
extern ret_t rtl8367b_setAsicMaxLengthInRx(rtk_uint32 maxLength);
extern ret_t rtl8367b_getAsicMaxLengthInRx(rtk_uint32* pMaxLength);
extern ret_t rtl8367b_setAsicMaxLengthAltTxRx(rtk_uint32 maxLength, rtk_uint32 pmskGiga, rtk_uint32 pmask100M);
extern ret_t rtl8367b_getAsicMaxLengthAltTxRx(rtk_uint32* pMaxLength, rtk_uint32* pPmskGiga, rtk_uint32* pPmask100M);

#endif /*_RTL8367B_ASICDRV_MISC_H_*/

