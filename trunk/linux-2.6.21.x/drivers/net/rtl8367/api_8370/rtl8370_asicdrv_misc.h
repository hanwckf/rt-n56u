#ifndef _RTL8370_ASICDRV_MISC_H_
#define _RTL8370_ASICDRV_MISC_H_

#include "rtl8370_asicdrv.h"


enum RTL8370_MAX_PKT_LENGTH
{
    RTL8370_MAXPKTLEN_1522B = 0,
    RTL8370_MAXPKTLEN_1536B,
    RTL8370_MAXPKTLEN_1552B,
    RTL8370_MAXPKTLEN_16000B,
    RTL8370_MAXPKTLEN_END   
};

extern ret_t rtl8370_setAsicMacAddress(ether_addr_t mac);
extern ret_t rtl8370_getAsicMacAddress(ether_addr_t *mac);
extern ret_t rtl8370_getAsicDebugInfo(uint32 port,uint32 *debugifo);
extern ret_t rtl8370_setAsicPortJamMode(uint32 mode);
extern ret_t rtl8370_getAsicPortJamMode(uint32* mode);
extern ret_t rtl8370_setAsicMaxLengthInRx(uint32 maxLength);
extern ret_t rtl8370_getAsicMaxLengthInRx(uint32* maxLength);
extern ret_t rtl8370_setAsicEthernetAv(uint32 enable);
extern ret_t rtl8370_getAsicEthernetAv(uint32* enable);

#endif /*_RTL8370_ASICDRV_MISC_H_*/

