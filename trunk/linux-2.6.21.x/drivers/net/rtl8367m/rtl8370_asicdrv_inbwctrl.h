#ifndef _RTL8370_ASICDRV_INBWCTRL_H_
#define _RTL8370_ASICDRV_INBWCTRL_H_

#include "rtl8370_asicdrv.h"

#define RTL8370_INBWCTRL_DISABLE   0xFFFF8

extern ret_t rtl8370_setAsicPortIngressBandwidth( uint32 port, uint32 bandwidth, uint32 preifg, uint32 enableFC);
extern ret_t rtl8370_getAsicPortIngressBandwidth( uint32 port, uint32* pBandwidth, uint32* pPreifg, uint32* pEnableFC );

#endif /*_RTL8370_ASICDRV_INBWCTRL_H_*/

