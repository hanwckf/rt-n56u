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
 * $Date: 2010/12/02 04:34:32 $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367B
 * Feature : 
 *
 */

#include "rtl8370_asicdrv_dot1x.h"

/*
@func ret_t  | rtl8370_setAsic1xPBEnConfig | Set 802.1x port-based port enable configuration
@parm uint32 | port | port number (0~15)
@parm uint32 | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_ENABLE | Invalid enable input.
@comm
    The API can update the port-based port enable register content.
*/
ret_t rtl8370_setAsic1xPBEnConfig(uint32 port,uint32 enabled)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;
    
    if(enabled > 1)
        return RT_ERR_ENABLE; 

    return rtl8370_setAsicRegBit(RTL8370_DOT1X_PORT_ENABLE_REG, port, enabled);
}

/*
@func ret_t  | rtl8370_getAsic1xPBEnConfig | Get 802.1x port-based port enable configuration
@parm uint32 | port | port number (0~15)
@parm uint32* | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can update the port-based port enable register content.
*/
ret_t rtl8370_getAsic1xPBEnConfig(uint32 port,uint32 *enabled)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBit(RTL8370_DOT1X_PORT_ENABLE_REG, port, enabled);
}

/*
@func ret_t  | rtl8370_setAsic1xPBAuthConfig | Set 802.1x port-based auth. port configuration
@parm uint32 | port | Physical port number.
@parm uint32 | auth | 1: authorised, 0: non-authorised.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_ENABLE | Invalid enable input.
@comm
    The API can update the port-based auth. port register content.
*/
ret_t rtl8370_setAsic1xPBAuthConfig(uint32 port,uint32 auth)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;
    if(auth > 1)
        return RT_ERR_ENABLE;  

    return rtl8370_setAsicRegBit(RTL8370_DOT1X_PORT_AUTH_REG, port, auth);
}

/*
@func ret_t | rtl8370_getAsic1xPBAuthConfig | get 802.1x port-based auth. port configuration
@parm uint32 | port | Physical port number.
@parm uint32* | auth | 1: authorised, 0: non-authorised.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can get 802.1x port-based auth. port information.
    
*/
ret_t rtl8370_getAsic1xPBAuthConfig(uint32 port,uint32 *auth)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBit(RTL8370_DOT1X_PORT_AUTH_REG, port, auth);
}

/*
@func ret_t  | rtl8370_setAsic1xPBOpdirConfig | Set 802.1x port-based operational direction configuration
@parm uint32 | port | Physical port number.
@parm uint32 | opdir | Operation direction 1: IN, 0:BOTH
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_ENABLE | Invalid enable input.
@comm
    The API can update the port-based operational direction register content.
*/
ret_t rtl8370_setAsic1xPBOpdirConfig(uint32 port,uint32 opdir)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;
    if(opdir > 1)
        return RT_ERR_ENABLE;

    return rtl8370_setAsicRegBit(RTL8370_DOT1X_PORT_OPDIR_REG, port, opdir);
}

/*
@func ret_t | rtl8370_getAsic1xPBOpdirConfig | get 802.1x port-based operational direction configuration
@parm uint32 | port | Physical port number.
@parm uint32* | opdir | Operation direction 1: IN, 0:BOTH
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can get 802.1x port-based operational direction information.
    
*/
ret_t rtl8370_getAsic1xPBOpdirConfig(uint32 port,uint32* opdir)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBit(RTL8370_DOT1X_PORT_OPDIR_REG, port, opdir);
}

/*
@func ret_t  | rtl8370_setAsic1xMBEnConfig | Set 802.1x mac-based port enable configuration
@parm uint32 | port | Physical port number.
@parm uint32 | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_ENABLE | Invalid enable input.
@comm
    The API can update the mac-based port enable register content.
*/
ret_t rtl8370_setAsic1xMBEnConfig(uint32 port,uint32 enabled)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;
    if(enabled > 1)
        return RT_ERR_ENABLE;

    return rtl8370_setAsicRegBit(RTL8370_DOT1X_MAC_ENABLE_REG, port, enabled);
}

/*
@func ret_t | rtl8370_getAsic1xMBEnConfig | get 802.1x mac-based port enable configuration
@parm uint32 | port | Physical port number.
@parm uint32* | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_OUT_OF_RANGE | Register undefine.
@comm
    The API can get 802.1x mac-based port enable information.
    
*/
ret_t rtl8370_getAsic1xMBEnConfig(uint32 port,uint32 *enabled)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBit(RTL8370_DOT1X_MAC_ENABLE_REG, port, enabled);
}

/*
@func ret_t  | rtl8370_setAsic1xMBOpdirConfig | Set 802.1x mac-based operational direction configuration
@parm uint32 | opdir | Operation direction 1: IN, 0:BOTH
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can update the mac-based operational direction register content.
*/
ret_t rtl8370_setAsic1xMBOpdirConfig(uint32 opdir)
{
    if(opdir > 1)
        return RT_ERR_ENABLE;

    return rtl8370_setAsicRegBit(RTL8370_DOT1X_CFG_REG, RTL8370_DOT1X_MAC_OPDIR_OFFSET, opdir);
}

/*
@func ret_t | rtl8370_getAsic1xMBOpdirConfig | get 802.1x mac-based operational direction configuration
@parm uint32* | opdir | Operation direction 1: IN, 0:BOTH
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get 802.1x mac-based operational direction information.
    
*/
ret_t rtl8370_getAsic1xMBOpdirConfig(uint32 *opdir)
{
    return rtl8370_getAsicRegBit(RTL8370_DOT1X_CFG_REG, RTL8370_DOT1X_MAC_OPDIR_OFFSET, opdir);
}

/*
@func ret_t  | rtl8370_setAsic1xProcConfig | Set 802.1x unauth. behavior configuration
@parm uint32 | port | Physical port number.
@parm uint32 | proc | 802.1x unauth. behavior configuration 0:drop 1:trap to CPU 2:Guest VLAN
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_DOT1X_PROC | Invalid input parameter.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can update the 802.1x unauth. behavior content.
*/
ret_t rtl8370_setAsic1xProcConfig(uint32 port, uint32 proc)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    if(proc >= DOT1X_UNAUTH_MAX)
        return RT_ERR_DOT1X_PROC;

    return rtl8370_setAsicRegBits(RTL8370_DOT1X_UNAUTH_ACT_REG(port), RTL8370_DOT1X_UNAUTH_ACT_MASK(port),proc);
}

/*
@func ret_t | rtl8370_getAsic1xProcConfig | get 802.1x unauth. behavior configuration
@parm uint32 | port | Physical port number.
@parm uint32* | proc | 802.1x unauth. behavior configuration 0:drop 1:trap to CPU 2:Guest VLAN
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can get 802.1x unauth. behavior configuration.
*/
ret_t rtl8370_getAsic1xProcConfig(uint32 port, uint32* proc)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBits(RTL8370_DOT1X_UNAUTH_ACT_REG(port), RTL8370_DOT1X_UNAUTH_ACT_MASK(port),proc);
}

/*
@func ret_t  | rtl8370_setAsicGVIndexConfig | Set 802.1x guest vlan index
@parm uint32 | index | 802.1x guest vlan index (0~31)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_DOT1X_GVLANIDX | Invalid guest vlan index
@comm
    The API can update the 802.1x guest vlan index content.
*/
ret_t rtl8370_setAsic1xGuestVidx(uint32 index)
{
    if(index >= RTL8370_CVLANMCNO)
        return RT_ERR_DOT1X_GVLANIDX;

    return rtl8370_setAsicRegBits(RTL8370_DOT1X_CFG_REG, RTL8370_DOT1X_GVIDX_MASK, index);
}

/*
@func ret_t | rtl8370_getAsicGVIndexConfig | get 802.1x guest vlan index configuration
@parm uint32* | index | 802.1x guest vlan index
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get 802.1x guest vlan index configuration.
    
*/
ret_t rtl8370_getAsic1xGuestVidx(uint32 *index)
{
    return rtl8370_getAsicRegBits(RTL8370_DOT1X_CFG_REG, RTL8370_DOT1X_GVIDX_MASK, index);
}

/*
@func ret_t  | rtl8370_setAsic1xGVOpdir | Set 802.1x guest vlan talk to auth. DA
@parm uint32 | enabled | 0:disable 1:enable
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_DOT1X_GVLANTALK | Invalid operation direction input.
@comm
    The API can update the 802.1x guest vlan talk to auth. DA content.
*/
ret_t rtl8370_setAsic1xGVOpdir(uint32 enabled)
{
    if(enabled > 1)
        return RT_ERR_DOT1X_GVLANTALK;
    
    return rtl8370_setAsicRegBit(RTL8370_DOT1X_CFG_REG, RTL8370_DOT1X_GVOPDIR_OFFSET, enabled);
}

/*
@func ret_t | rtl8370_getAsic1xGVOpdir | get 802.1x guest vlan talk to unauth. DA configuration
@parm uint32* | enabled | 0:disable 1:enable
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get 802.1x guest vlan talk to unauth. DA configuration.
    
*/
ret_t rtl8370_getAsic1xGVOpdir(uint32 *enabled)
{
    return rtl8370_getAsicRegBit(RTL8370_DOT1X_CFG_REG, RTL8370_DOT1X_GVOPDIR_OFFSET, enabled);
}

/*
@func ret_t  | rtl8370_setAsic1xTrapPriority | Set 802.1x Trap priority
@parm uint32 | priority | priority (0~7)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_QOS_INT_PRIORITY | Invalid priority.
@comm
    The API can update the 802.1x trap priority.
*/
ret_t rtl8370_setAsic1xTrapPriority(uint32 priority)
{
    if(priority > RTL8370_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;
    
    return rtl8370_setAsicRegBits(RTL8370SG_QOS_TRAP_PRIORITY_CTRL0_REG, RTL8370_DOT1X_PRIORTY_MASK,priority);
}

/*
@func ret_t  | rtl8370_getAsic1xTrapPriority | Get 802.1x Trap priority
@parm uint32* | priority | priority (0~7)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can update the 802.1x trap priority.
*/
ret_t rtl8370_getAsic1xTrapPriority(uint32 *priority)
{
    return rtl8370_getAsicRegBits(RTL8370SG_QOS_TRAP_PRIORITY_CTRL0_REG, RTL8370_DOT1X_PRIORTY_MASK,priority);
}

