/*
 * Copyright (C) 2009 Realtek Semiconductor Corp. 
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated, 
 * modified or distributed under the authorized license from Realtek. 
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER 
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED. 
 *
 * $Revision: 1.1.1.1 $
 * $Date: 2010/12/02 04:34:29 $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367B
 * Feature : 
 *
 */
#include "rtl8370_asicdrv_green.h"

/*
@func ret_t | rtl8370_getAsicGreenPortPage | Set per-Port ingress page usage per second
@parm uint32 | port | The port number
@parm uint32* | page | page number of ingress packet occuping per second
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can get ingress traffic occuping page number per second for high layer green feature usage
*/
ret_t rtl8370_getAsicGreenPortPage(uint32 port, uint32* page)
{
    ret_t retVal;
    uint32 regData;
    uint32 pageMeter;
    
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    retVal = rtl8370_getAsicReg(RTL8370_PAGEMETER_PORT_REG(port) , &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    pageMeter = regData;

    retVal = rtl8370_getAsicReg(RTL8370_PAGEMETER_PORT_REG(port) + 1 , &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    pageMeter = pageMeter + (regData <<16);

    *page = pageMeter;     
    return RT_ERR_OK;
}
/*
@func ret_t | rtl8370_setAsicGreenTrafficType | Set traffic type for each priority
@parm uint32 | priority | internal priority (0~7).
@parm uint32 | traffictype | high/low traffic type, 1:high priority traffic type, 0:low priority traffic type
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_QOS_INT_PRIORITY | Invalid priority. 
@comm
    The API can set internal priority to traffic type. ASIC will set indicator for receiving high-prioirty traffic in 1second by traffic type configuration.
*/
ret_t rtl8370_setAsicGreenTrafficType(uint32 priority, uint32 traffictype)
{

    if(priority > RTL8370_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    return rtl8370_setAsicRegBit(RTL8370_REG_HIGHPRI_CFG , priority, (traffictype?1:0));
}
/*
@func ret_t | rtl8370_getAsicGreenTrafficType | Get traffic type for each priority
@parm uint32 | priority | internal priority (0~7).
@parm uint32* | traffictype | high/low traffic type, 1:high priority traffic type, 0:low priority traffic type
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can get internal priority to traffic type. ASIC will set indicator for receiving high-prioirty traffic in 1second by traffic type configuration.
*/
ret_t rtl8370_getAsicGreenTrafficType(uint32 priority, uint32* traffictype)
{
    
    if(priority > RTL8370_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    return rtl8370_getAsicRegBit(RTL8370_REG_HIGHPRI_CFG, priority, traffictype);
}
/*
@func ret_t | rtl8370_getAsicGreenHighPriorityTraffic | Set traffic type for each priority
@parm uint32 | port | The port number
@parm uint32* | indicator | Have received high priority traffic indicator. If 1 means ASCI had received high priority in 1second checking priod.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can get indicator which ASIC had received high priority traffic or not.
*/
ret_t rtl8370_getAsicGreenHighPriorityTraffic(uint32 port, uint32* indicator)
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBit(RTL8370_REG_HIGHPRI_INDICATOR, port, indicator);
}

/*
@func int32 | rtl8370_setAsicGreenEthernet | Set green ethernet function.
@parm uint32 | green | Green feature function usage 1:enable 0:disable.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@comm
 	The API can set Green Ethernet function to reduce power consumption. While green feature is enabled, ASIC will automatic
 detect the cable length and then select different power mode for best performance with minimums power consumption. Link down
 ports will enter power savining mode in 10 seconds after the cable disconnected if power saving function is enabled.
*/
ret_t rtl8370_setAsicGreenEthernet(uint32 green)
{
    if (green > 1)
        return RT_ERR_INPUT;

	return rtl8370_setAsicRegBit(RTL8370_REG_PHY_AD,6,green);
}

/*
@func int32 | rtl8370_getAsicGreenEthernet | Get green ethernet function.
@parm uint32 | *green | Green feature function usage 1:enable 0:disable.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@comm
 	The API can set Green Ethernet function to reduce power consumption. While green feature is enabled, ASIC will automatic
 detect the cable length and then select different power mode for best performance with minimums power consumption. Link down
 ports will enter power savining mode in 10 seconds after the cable disconnected if power saving function is enabled.
*/
ret_t rtl8370_getAsicGreenEthernet(uint32* green)
{
	return rtl8370_getAsicRegBit(RTL8370_REG_PHY_AD,6,green);
}


/*
@func ret_t | rtl8370_setAsicPowerSaving | Set power saving mode
@parm uint32 | phy | phy number
@parm uint32 | enable | enable power saving mode.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can set power saving mode per phy.
*/
ret_t rtl8370_setAsicPowerSaving(uint32 phy, uint32 enable)
{
    rtk_api_ret_t retVal;
    uint32 phyData;

    if(phy > RTL8370_PHYIDMAX)
        return RT_ERR_PORT_ID;
    if (enable > 1)
        return RT_ERR_INPUT;

    if ((retVal = rtl8370_setAsicPHYReg(phy,RTL8370_PHY_PAGE_ADDRESS,0))!=RT_ERR_OK)
        return retVal;  
    
    if ((retVal = rtl8370_getAsicPHYReg(phy,PHY_POWERSAVING_REG,&phyData))!=RT_ERR_OK)
        return retVal;

    phyData = (phyData & (~PHY_POWERSAVING_MASK)) | (enable<<PHY_POWERSAVING_OFFSET) ;

    if ((retVal = rtl8370_setAsicPHYReg(phy,PHY_POWERSAVING_REG,phyData))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_getAsicPowerSaving | Get power saving mode
@parm uint32 | port | The port number
@parm uint32* | enable | enable power saving mode.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can get power saving mode per phy.
*/
ret_t rtl8370_getAsicPowerSaving(uint32 phy, uint32* enable)
{
    rtk_api_ret_t retVal;
    uint32 phyData;

    if(phy > RTL8370_PHYIDMAX)
        return RT_ERR_PORT_ID;

    if ((retVal = rtl8370_setAsicPHYReg(phy,RTL8370_PHY_PAGE_ADDRESS,0))!=RT_ERR_OK)
        return retVal;  
    
    if ((retVal = rtl8370_getAsicPHYReg(phy,PHY_POWERSAVING_REG,&phyData))!=RT_ERR_OK)
        return retVal;

    if ((phyData & PHY_POWERSAVING_MASK) > 0)
        *enable = 1;
    else 
        *enable = 0;
    
    return RT_ERR_OK;
}
