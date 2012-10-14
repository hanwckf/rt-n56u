#ifndef _RTL8367B_ASICDRV_SPECIALCONGEST_H_
#define _RTL8367B_ASICDRV_SPECIALCONGEST_H_

#include "rtl8367b_asicdrv.h"

#define RTL8367B_SPECIALCONGEST_SUSTAIN_TIMERMAX 0xF

extern ret_t rtl8367b_setAsicSpecialCongestModeConfig(rtk_uint32 port, rtk_uint32 sustain);
extern ret_t rtl8367b_getAsicSpecialCongestModeConfig(rtk_uint32 port, rtk_uint32* pSustain);

extern ret_t rtl8367b_getAsicSpecialCongestModeTimer(rtk_uint32 port, rtk_uint32* pTimer);

#endif /*_RTL8367B_ASICDRV_SPECIALCONGEST_H_*/

