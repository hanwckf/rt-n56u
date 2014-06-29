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
 * $Date: 2010/12/02 04:34:22 $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367B
 * Feature : 
 *
 */
#include "rtl8370_asicdrv_inbwctrl.h"

/*
@func ret_t | rtl8370_setAsicPortIngressBandwidth | Set per-port total ingress bandwidth.
@parm uint32 | port | The port number.
@parm uint32 | bandwidth | The total ingress bandwidth (unit: 8Kbps), 0x1FFFF:disable. 
@parm uint32 | preifg | Include preamble and IFG, 0:Exclude, 1:Include.
@parm uint32 | enableFC | Action when input rate exceeds. 0: Drop    1: Flow Control
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@rvalue RT_ERR_INBW_RATE | Invalid input rate parameter.
@comm
    The API can set port ingress bandwidth. Port ingress bandwidth = (bandwidth+1)*8Kbps.
    To disable port ingress bandwidth control, the parameter 'bandwidth' should be set as 0x1FFFF.
 */
ret_t rtl8370_setAsicPortIngressBandwidth( uint32 port, uint32 bandwidth, uint32 preifg, uint32 enableFC)
{
    uint32 retVal;
    uint32 regData;
    uint32 regAddr;

    /* Invalid input parameter */
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    if(bandwidth > RTL8370_QOS_GRANULARTY_MAX)
        return RT_ERR_INBW_RATE;
    
    if((enableFC > 1) || (preifg > 1))
        return RT_ERR_INPUT;
    
    regAddr = RTL8370_INGRESSBW_PORT_RATE_LSB_REG(port);
    regData = bandwidth & RTL8370_QOS_GRANULARTY_LSB_MASK;
    retVal = rtl8370_setAsicReg(regAddr, regData);
    if (retVal !=  RT_ERR_OK) 
        return retVal;

    regAddr += 1;
    regData = (bandwidth & RTL8370_QOS_GRANULARTY_MSB_MASK) >> RTL8370_QOS_GRANULARTY_MSB_OFFSET;
    retVal = rtl8370_setAsicRegBit(regAddr, 0, regData);
    if (retVal !=  RT_ERR_OK) 
        return retVal;

    regAddr = RTL8370_PORT_MISC_CFG_REG(port);
    retVal = rtl8370_setAsicRegBit(regAddr, RTL8370_INGRESSBW_PORT_IFG_OFFSET, preifg);
    if (retVal !=  RT_ERR_OK) 
        return retVal;

    regAddr = RTL8370_PORT_MISC_CFG_REG(port);

    return rtl8370_setAsicRegBit(regAddr, RTL8370_INGRESSBW_PORT_FLOWCRTL_ENABLE_OFFSET, enableFC);
}

/*
@func ret_t | rtl8370_getAsicPortIngressBandwidth | Get per-port total ingress bandwidth.
@parm uint32 | port | The port number.
@parm uint32* | bandwidth | The total ingress bandwidth (unit: 8Kbps), 0x1FFFF:disable. 
@parm uint32* | preifg | Include preamble and IFG, 0:Exclude, 1:Include.
@parm uint32* | enableFC | Action when input rate exceeds. 0: Drop    1: Flow Control
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    The API can set port ingress bandwidth. Port ingress bandwidth = (bandwidth+1)*8Kbps.
    To disable port ingress bandwidth control, the parameter 'bandwidth' should be set as 0x1FFFF.
 */
ret_t rtl8370_getAsicPortIngressBandwidth( uint32 port, uint32* pBandwidth, uint32* pPreifg, uint32* pEnableFC )
{
    uint32 retVal;
    uint32 regData;
    uint32 regAddr;

    /* Invalid input parameter */
    if(port >=RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    regAddr = RTL8370_INGRESSBW_PORT_RATE_LSB_REG(port);
    retVal = rtl8370_getAsicReg(regAddr, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    *pBandwidth = regData;
    
    regAddr += 1;
    retVal = rtl8370_getAsicRegBit(regAddr, 0, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;
    
    *pBandwidth |= (regData << RTL8370_QOS_GRANULARTY_MSB_OFFSET);

    regAddr = RTL8370_PORT_MISC_CFG_REG(port);
    retVal = rtl8370_getAsicRegBit(regAddr, RTL8370_INGRESSBW_PORT_IFG_OFFSET, pPreifg);
    if(retVal != RT_ERR_OK)
        return retVal;
        
    regAddr = RTL8370_PORT_MISC_CFG_REG(port);

    return rtl8370_getAsicRegBit(regAddr, RTL8370_INGRESSBW_PORT_FLOWCRTL_ENABLE_OFFSET, pEnableFC);
}

