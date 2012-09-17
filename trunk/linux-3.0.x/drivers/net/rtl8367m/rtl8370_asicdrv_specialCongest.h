#ifndef _RTL8370_ASICDRV_SPECIALCONGEST_H_
#define _RTL8370_ASICDRV_SPECIALCONGEST_H_

#include "rtl8370_asicdrv.h"

#define RTL8370_SPECIALCONGEST_SUSTAIN_TIMERMAX 0xF

extern ret_t rtl8370_setAsicSpecialCongestModeConfig(uint32 port, uint32 sustain);
extern ret_t rtl8370_getAsicSpecialCongestModeConfig(uint32 port, uint32* sustain);

extern ret_t rtl8370_getAsicSpecialCongestModeTimer(uint32 port, uint32* timer);

#endif /*_RTL8370_ASICDRV_SPECIALCONGEST_H_*/

