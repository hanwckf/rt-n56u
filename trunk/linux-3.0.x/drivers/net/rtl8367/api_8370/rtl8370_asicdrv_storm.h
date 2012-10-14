#ifndef _RTL8370_ASICDRV_STORM_H_
#define _RTL8370_ASICDRV_STORM_H_

#include "rtl8370_asicdrv.h"

extern ret_t rtl8370_setAsicStormFilterBroadcastEnable(uint32 port, uint32 enable);
extern ret_t rtl8370_getAsicStormFilterBroadcastEnable(uint32 port, uint32 *enable);
extern ret_t rtl8370_setAsicStormFilterBroadcastMeter(uint32 port, uint32 meter);
extern ret_t rtl8370_getAsicStormFilterBroadcastMeter(uint32 port, uint32 *meter);
extern ret_t rtl8370_setAsicStormFilterMulticastEnable(uint32 port, uint32 enable);
extern ret_t rtl8370_getAsicStormFilterMulticastEnable(uint32 port, uint32 *enable);
extern ret_t rtl8370_setAsicStormFilterMulticastMeter(uint32 port, uint32 meter);
extern ret_t rtl8370_getAsicStormFilterMulticastMeter(uint32 port, uint32 *meter);
extern ret_t rtl8370_setAsicStormFilterUnknownMulticastEnable(uint32 port, uint32 enable);
extern ret_t rtl8370_getAsicStormFilterUnknownMulticastEnable(uint32 port, uint32 *enable);
extern ret_t rtl8370_setAsicStormFilterUnknownMulticastMeter(uint32 port, uint32 meter);
extern ret_t rtl8370_getAsicStormFilterUnknownMulticastMeter(uint32 port, uint32 *meter);
extern ret_t rtl8370_setAsicStormFilterUnknownUnicastEnable(uint32 port, uint32 enable);
extern ret_t rtl8370_getAsicStormFilterUnknownUnicastEnable(uint32 port, uint32 *enable);
extern ret_t rtl8370_setAsicStormFilterUnknownUnicastMeter(uint32 port, uint32 meter);
extern ret_t rtl8370_getAsicStormFilterUnknownUnicastMeter(uint32 port, uint32 *meter);

#endif /*_RTL8370_ASICDRV_STORM_H_*/


