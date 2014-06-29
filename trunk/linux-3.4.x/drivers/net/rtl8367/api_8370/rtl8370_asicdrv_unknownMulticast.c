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
 * $Date: 2010/12/02 04:34:27 $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367B
 * Feature : 
 *
 */

#include "rtl8370_asicdrv_unknownMulticast.h"

/*
@func ret_t | rtl8370_setAsicUnknownL2MulticastBehavior | Set behavior of L2 multicast
@parm uint32 | port | Physical port number (0~15).
@parm uint32 | behav | 0: flooding, 1: drop, 2: trap
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_NOT_ALLOWED | Invalid operation.
@comm
    When receives an unknown multicast packet, switch may trap, drop or flood this packet
*/
ret_t rtl8370_setAsicUnknownL2MulticastBehavior(uint32 port, uint32 behav)
{

    if(port >  RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(behav >= L2_BEHAV_MAX)
        return RT_ERR_NOT_ALLOWED;

    return rtl8370_setAsicRegBits(RTL8370_UNKNOWN_L2_MULTICAST_REG(port), RTL8370_UNKNOWN_L2_MULTICAST_MASK(port), behav);
}

/*
@func ret_t | rtl8370_getAsicUnknownL2MulticastBehavior | Get behavior of L2 multicast
@parm uint32 | port | Physical port number (0~15).
@parm uint32* | behav | 0: flooding, 1: drop, 2: trap
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    When receives an unknown multicast packet, switch may trap, drop or flood this packet
*/
ret_t rtl8370_getAsicUnknownL2MulticastBehavior(uint32 port, uint32 *behav)
{

    if(port >  RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBits(RTL8370_UNKNOWN_L2_MULTICAST_REG(port), RTL8370_UNKNOWN_L2_MULTICAST_MASK(port), behav);
}

/*
@func ret_t | rtl8370_setAsicUnknownIPv4MulticastBehavior | Set behavior of IPv4 multicast
@parm uint32 | port | Physical port number (0~15).
@parm uint32 | behav | 0: flooding, 1: drop, 2: trap
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_NOT_ALLOWED | Invalid operation.
@comm
    When receives an unknown multicast packet, switch may trap, drop or flood this packet
*/

ret_t rtl8370_setAsicUnknownIPv4MulticastBehavior(uint32 port, uint32 behav)
{

    if(port >  RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(behav >= L2_BEHAV_MAX)
        return RT_ERR_NOT_ALLOWED;

    return rtl8370_setAsicRegBits(RTL8370_UNKNOWN_IPV4_MULTICAST_REG(port), RTL8370_UNKNOWN_IPV4_MULTICAST_MASK(port), behav);
}

/*
@func ret_t | rtl8370_getAsicUnknownIPv4MulticastBehavior | Get behavior of L2 multicast
@parm uint32 | port | Physical port number (0~15).
@parm uint32* | behav | 0: flooding, 1: drop, 2: trap
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    When receives an unknown multicast packet, switch may trap, drop or flood this packet
*/
ret_t rtl8370_getAsicUnknownIPv4MulticastBehavior(uint32 port, uint32 *behav)
{
    if(port >  RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBits(RTL8370_UNKNOWN_IPV4_MULTICAST_REG(port), RTL8370_UNKNOWN_IPV4_MULTICAST_MASK(port), behav);
}

/*
@func ret_t | rtl8370_setAsicUnknownIPv6MulticastBehavior | Set behavior of IPv6 multicast
@parm uint32 | port | Physical port number (0~15).
@parm uint32 | behav | 0: flooding, 1: drop, 2: trap
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_NOT_ALLOWED | Invalid operation.
@comm
    When receives an unknown multicast packet, switch may trap, drop or flood this packet
*/
ret_t rtl8370_setAsicUnknownIPv6MulticastBehavior(uint32 port, uint32 behav)
{
    if(port >  RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(behav >= L2_BEHAV_MAX)
        return RT_ERR_NOT_ALLOWED;

    return rtl8370_setAsicRegBits(RTL8370_UNKNOWN_IPV6_MULTICAST_REG(port), RTL8370_UNKNOWN_IPV6_MULTICAST_MASK(port), behav);
}

/*
@func ret_t | rtl8370_getAsicUnknownIPv6MulticastBehavior | Get behavior of L2 multicast
@parm uint32 | port | Physical port number (0~15).
@parm uint32 | behav | 0: flooding, 1: drop, 2: trap
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    When receives an unknown multicast packet, switch may trap, drop or flood this packet
*/
ret_t rtl8370_getAsicUnknownIPv6MulticastBehavior(uint32 port, uint32 *behav)
{

    if(port >  RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBits(RTL8370_UNKNOWN_IPV6_MULTICAST_REG(port), RTL8370_UNKNOWN_IPV6_MULTICAST_MASK(port), behav);
}

/*
@func ret_t | rtl8370_setAsicUnknownMulticastTrapPriority | Set trap priority of unknown multicast frame
@parm uint32 | priority | priority (0~7)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_QOS_INT_PRIORITY | Invalid priority.
@comm
*/
ret_t rtl8370_setAsicUnknownMulticastTrapPriority(uint32 priority)
{
    if(priority > RTL8370_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    return rtl8370_setAsicRegBits(RTL8370SG_QOS_TRAP_PRIORITY_CTRL0_REG, RTL8370_UNKNOWN_MC_PRIORTY_MASK, priority);
}

/*
@func ret_t | rtl8370_getAsicUnknownMulticastTrapPriority | Get trap priority of unknown multicast frame
@parm uint32* | priority | priority (0~7)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@comm
*/
ret_t rtl8370_getAsicUnknownMulticastTrapPriority(uint32 *priority)
{
    return rtl8370_getAsicRegBits(RTL8370SG_QOS_TRAP_PRIORITY_CTRL0_REG, RTL8370_UNKNOWN_MC_PRIORTY_MASK, priority);
}

