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
 * $Date: 2010/12/02 04:34:28 $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367B
 * Feature : 
 *
 */

#include "rtl8370_asicdrv_portIsolation.h"

/*
@func ret_t | rtl8370_setAsicPortIsolationPermittedPortmask | Set permitted port isolation portmask
@parm uint32 | port | Physical port number (0~15).
@parm uint32 | permitPortmask | portmask (0~0xFFFF)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_PORT_MASK | Invalid portmask.
@comm
    This API set the port mask that a port can trasmit packet to of each port
    A port can only transmit packet to ports included in permitted portmask
*/
ret_t rtl8370_setAsicPortIsolationPermittedPortmask(uint32 port, uint32 permitPortmask)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    if(permitPortmask > RTL8370_PORTMASK)
        return RT_ERR_PORT_MASK;
    
    return rtl8370_setAsicReg(RTL8370_PORT_ISOLATION_PORT_MASK_REG(port), permitPortmask);
}

/*
@func ret_t | rtl8370_getAsicPortIsolationPermittedPortmask | Get permitted port isolation portmask
@parm uint32 | port | Physical port number (0~15).
@parm uint32* | permitPortmask | portmask (0~0xFFFF)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    This API get the port mask that a port can trasmit packet to of each port
    A port can only transmit packet to ports included in permitted portmask
*/
ret_t rtl8370_getAsicPortIsolationPermittedPortmask(uint32 port, uint32 *permitPortmask)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;
    
    return rtl8370_getAsicReg(RTL8370_PORT_ISOLATION_PORT_MASK_REG(port), permitPortmask);
}

/*
@func ret_t | rtl8370_setAsicPortIsolationEfid | Set port isolation EFID
@parm uint32 | port | Physical port number (0~15).
@parm uint32 | efid | EFID (0~7)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_OUT_OF_RANGE | input out of range.
@comm
    This API set the EFID of each port.
    EFID is used in individual learning in filtering database
*/
ret_t rtl8370_setAsicPortIsolationEfid(uint32 port, uint32 efid)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    if(efid > RTL8370_EFIDMAX)
        return RT_ERR_OUT_OF_RANGE;

    return rtl8370_setAsicRegBits(RTL8370_PORT_EFID_REG(port), RTL8370_PORT_EFID_MASK(port), efid);
}

/*
@func ret_t | rtl8370_getAsicPortIsolationEfid | Get port isolation EFID
@parm uint32 | port | Physical port number (0~15).
@parm uint32* | efid | EFID (0~7)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    This API get the EFID of each port.
    EFID is used in individual learning in filtering database
*/

ret_t rtl8370_getAsicPortIsolationEfid(uint32 port, uint32 *efid)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBits(RTL8370_PORT_EFID_REG(port), RTL8370_PORT_EFID_MASK(port), efid);
}

