#ifndef _RTL8367B_ASICDRV_GREEN_H_
#define _RTL8367B_ASICDRV_GREEN_H_

#include "rtl8367b_asicdrv.h"
#include "rtl8367b_asicdrv_phy.h"

#define PHY_POWERSAVING_REG                         21
#define PHY_POWERSAVING_OFFSET                      12
#define PHY_POWERSAVING_MASK                        0x1000

extern ret_t rtl8367b_setAsicGreenTrafficType(rtk_uint32 priority, rtk_uint32 traffictype);
extern ret_t rtl8367b_getAsicGreenTrafficType(rtk_uint32 priority, rtk_uint32* pTraffictype);
extern ret_t rtl8367b_getAsicGreenPortPage(rtk_uint32 port, rtk_uint32* pPage);
extern ret_t rtl8367b_getAsicGreenHighPriorityTraffic(rtk_uint32 port, rtk_uint32* pIndicator);
extern ret_t rtl8367b_setAsicGreenHighPriorityTraffic(rtk_uint32 port);
extern ret_t rtl8367b_setAsicGreenEthernet(rtk_uint32 green);
extern ret_t rtl8367b_getAsicGreenEthernet(rtk_uint32* green);
extern ret_t rtl8367b_setAsicPowerSaving(rtk_uint32 phy, rtk_uint32 enable);
extern ret_t rtl8367b_getAsicPowerSaving(rtk_uint32 phy, rtk_uint32* enable);
#endif /*#ifndef _RTL8367B_ASICDRV_GREEN_H_*/

