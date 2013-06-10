#ifndef _RTL8370_ASICDRV_TRUNKING_H_
#define _RTL8370_ASICDRV_TRUNKING_H_

#include "rtl8370_asicdrv.h"

#define RTL8370_TRUNKING_GROUPNO       4
#define RTL8370_TRUNKING_GROUPMAX      (RTL8370_TRUNKING_GROUPNO-1)

extern ret_t rtl8370_setAsicTrunkingGroup(uint32 group,uint32 portmask);
extern ret_t rtl8370_getAsicTrunkingGroup(uint32 group,uint32* portmask);
extern ret_t rtl8370_setAsicTrunkingFlood(uint32 enable);
extern ret_t rtl8370_getAsicTrunkingFlood(uint32* enable);
extern ret_t rtl8370_setAsicTrunkingHashSelect(uint32 hashsel);
extern ret_t rtl8370_getAsicTrunkingHashSelect(uint32* hashsel);

extern ret_t rtl8370_getAsicQeueuEmptyStatus(uint32* portmask);

#endif /*_RTL8370_ASICDRV_TRUNKING_H_*/

