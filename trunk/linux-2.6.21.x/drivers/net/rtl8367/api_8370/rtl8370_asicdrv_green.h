#ifndef _RTL8370_ASICDRV_GREEN_H_
#define _RTL8370_ASICDRV_GREEN_H_

#include "rtl8370_asicdrv.h"
#include "rtl8370_asicdrv_phy.h"
#include "rtk_api.h"

extern ret_t rtl8370_setAsicGreenTrafficType(uint32 priority, uint32 traffictype);
extern ret_t rtl8370_getAsicGreenTrafficType(uint32 priority, uint32* traffictype);
extern ret_t rtl8370_getAsicGreenPortPage(uint32 port, uint32* page);
extern ret_t rtl8370_getAsicGreenHighPriorityTraffic(uint32 port, uint32* indicator);
extern ret_t rtl8370_setAsicGreenEthernet(uint32 green);
extern ret_t rtl8370_getAsicGreenEthernet(uint32* green);
extern ret_t rtl8370_setAsicPowerSaving(uint32 phy, uint32 enable);
extern ret_t rtl8370_getAsicPowerSaving(uint32 phy, uint32* enable);
#endif /*#ifndef _RTL8370_ASICDRV_GREEN_H_*/

