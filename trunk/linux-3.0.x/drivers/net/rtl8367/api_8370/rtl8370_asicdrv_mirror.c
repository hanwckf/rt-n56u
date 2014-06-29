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
 * $Date: 2010/12/02 04:34:39 $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367B
 * Feature : 
 *
 */
#include "rtl8370_asicdrv_mirror.h"

/*
@func ret_t | rtl8370_setAsicPortMirror | Configure port mirror function.
@parm uint32 | source | Source port no.
@parm uint32 | monitor | Monitor (destination) port. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    System supports one set of port mirror function. Mirrored port will be checked if mirror receiving frame or mirror transmitting frame to monitor port.   
*/
ret_t rtl8370_setAsicPortMirror(uint32 source, uint32 monitor)
{
    ret_t retVal;

    if((source > RTL8370_PORTIDMAX) || (monitor > RTL8370_PORTIDMAX))
        return RT_ERR_PORT_ID;

    retVal = rtl8370_setAsicRegBits(RTL8370_MIRROR_CTRL_REG, RTL8370_MIRROR_SOURCE_PORT_MASK, source);
    if(retVal !=  RT_ERR_OK)
        return retVal;

    return rtl8370_setAsicRegBits(RTL8370_MIRROR_CTRL_REG, RTL8370_MIRROR_MONITOR_PORT_MASK, monitor);
}

/*
@func ret_t | rtl8370_getAsicPortMirror | Get mirrored port and monitor port inforamtion.
@parm uint32* | source | Source port no.
@parm uint32* | monitor | Monitor (destination) port. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get the current setting mirrored port number and monitor port number 
    information.  
*/
ret_t rtl8370_getAsicPortMirror(uint32 *source, uint32 *monitor)
{
    ret_t retVal;
    
    retVal = rtl8370_getAsicRegBits(RTL8370_MIRROR_CTRL_REG, RTL8370_MIRROR_SOURCE_PORT_MASK, source);
    if(retVal !=  RT_ERR_OK)
        return retVal;

    return rtl8370_getAsicRegBits(RTL8370_MIRROR_CTRL_REG, RTL8370_MIRROR_MONITOR_PORT_MASK, monitor);
}

/*
@func ret_t | rtl8370_setAsicPortMirrorRxFunction | Enable the mirror function on RX of the mirrored port. 
@parm uint32 | enabled | 1: enabled, 0: disabled. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_ENABLE | Invalid enable input.  
@comm
    If the API is setted to enabled, the RX of mirrored port will be mirrorred to 
    the current monitor port.
*/
ret_t rtl8370_setAsicPortMirrorRxFunction(uint32 enabled)
{
    if(enabled > 1)
        return RT_ERR_ENABLE;
    
    return rtl8370_setAsicRegBit(RTL8370_MIRROR_CTRL_REG, RTL8370_MIRROR_RX_OFFSET, enabled);
}

/*
@func ret_t | rtl8370_getAsicPortMirrorRxFunction | Enable the mirror function on RX of the mirrored port. 
@parm uint32* | enabled | 1: enabled, 0: disabled. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    If the API is setted to enabled, the RX of mirrored port will be mirrorred to 
    the current monitor port.
*/
ret_t rtl8370_getAsicPortMirrorRxFunction(uint32* enabled)
{
    return rtl8370_getAsicRegBit(RTL8370_MIRROR_CTRL_REG, RTL8370_MIRROR_RX_OFFSET, enabled);
}


/*
@func ret_t | rtl8370_setAsicPortMirrorTxFunction | Enable the mirror function on TX of the mirrored port. 
@parm uint32 | enabled | 1: enabled, 0: disabled. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_ENABLE | Invalid enable input.  
@comm
    If the API is setted to be enabled, the TX of mirrored port will be mirrorred to 
    the current monitor port. 
*/
ret_t rtl8370_setAsicPortMirrorTxFunction(uint32 enabled)
{
    if(enabled > 1)
        return RT_ERR_ENABLE;
    
    return rtl8370_setAsicRegBit(RTL8370_MIRROR_CTRL_REG, RTL8370_MIRROR_TX_OFFSET, enabled);
}

/*
@func ret_t | rtl8370_getAsicPortMirrorTxFunction | Enable the mirror function on TX of the mirrored port. 
@parm uint32* | enabled | 1: enabled, 0: disabled. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    If the API is setted to be enabled, the TX of mirrored port will be mirrorred to 
    the current monitor port. 
*/
ret_t rtl8370_getAsicPortMirrorTxFunction(uint32* enabled)
{
    return rtl8370_getAsicRegBit(RTL8370_MIRROR_CTRL_REG, RTL8370_MIRROR_TX_OFFSET, enabled);
}

/*
@func ret_t | rtl8370_setAsicPortMirrorIsolation | Enable the traffic isolation on monitor port
@parm uint32 | enabled | 1: enabled, 0: disabled. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_ENABLE | Invalid enable input.
@comm
     If the API is setted to be enabled, the monitor port will accept frames from mirrored port only
*/
ret_t rtl8370_setAsicPortMirrorIsolation(uint32 enabled)
{
    if(enabled > 1)
        return RT_ERR_ENABLE;
    
    return rtl8370_setAsicRegBit(RTL8370_MIRROR_CTRL_REG, RTL8370_MIRROR_ISO_OFFSET, enabled);
}

/*
@func ret_t | rtl8370_getAsicPortMirrorIsolation | Enable the traffic isolation on monitor port 
@parm uint32* | enabled | 1: enabled, 0: disabled. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get the current mointor port isolation status.
*/
ret_t rtl8370_getAsicPortMirrorIsolation(uint32* enabled)
{
    return rtl8370_getAsicRegBit(RTL8370_MIRROR_CTRL_REG, RTL8370_MIRROR_ISO_OFFSET, enabled);
}

