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

#include "rtl8370_asicdrv_trunking.h"


/*
@func ret_t | rtl8370_setAsicTrunkingGroup | Set trunking group available port mask
@parm uint32 | group | Port trunking group (0~3).
@parm uint32 | portmask | Logic trunking enable port mask, max 4 ports
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_LA_TRUNK_ID | Invalid trunking group
@comm
    The API can set 4 port trunking group enabled port mask. Each port trunking group has max 4 ports.
    If enabled port mask has less than 2 ports available setting, then this trunking group function is disabled.    
 */
ret_t rtl8370_setAsicTrunkingGroup(uint32 group,uint32 portmask)
{
    if(group > RTL8370_TRUNKING_GROUPMAX)
        return RT_ERR_LA_TRUNK_ID;

    return rtl8370_setAsicRegBits(RTL8370_PORT_TRUNK_GROUP_MASK_REG, RTL8370_PORT_TRUNK_GROUP_MASK_MASK(group), portmask);
}
/*
@func ret_t | rtl8370_getAsicTrunkingGroup | Get trunking group available port mask
@parm uint32 | group | Port trunking group (0~3).
@parm uint32* | portmask | Logic trunking enable port mask, max 4 ports
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_LA_TRUNK_ID | Invalid trunking group
@comm
    The API can get 4 port trunking group enabled port mask. Each port trunking group has max 4 ports.
    If enabled port mask has less than 2 ports available setting, then this trunking group function is disabled.    
 */
ret_t rtl8370_getAsicTrunkingGroup(uint32 group,uint32* portmask)
{
    if(group > RTL8370_TRUNKING_GROUPMAX)
        return RT_ERR_LA_TRUNK_ID;

    return rtl8370_getAsicRegBits(RTL8370_PORT_TRUNK_GROUP_MASK_REG, RTL8370_PORT_TRUNK_GROUP_MASK_MASK(group), portmask);
}
/*
@func ret_t | rtl8370_setAsicTrunkingFlood | Set port trunking flood function
@parm uint32 | enabled | Port trunking flooding function 0:disable 1:enable
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_ENABLE | Invalid enable parameter.
@comm
    The API can set port trunking unknown flooding function. While this function was be set, then ASIC will send flooding packets to trunking 
    group logic 1st port and known packets will be send to left available trunking ports. This function will be enabled while 4 ports of trunking
    group are all aggregatted.
 */
ret_t rtl8370_setAsicTrunkingFlood(uint32 enabled)
{
    if ((enabled != 0) && (enabled != 1))
        return RT_ERR_ENABLE; 

    return rtl8370_setAsicRegBit(RTL8370_REG_PORT_TRUNK_CTRL, RTL8370_PORT_TRUNK_FLOOD_OFFSET, enabled);
}
/*
@func ret_t | rtl8370_getAsicTrunkingFlood | Get port trunking flood function
@parm uint32 | enabled | Port trunking flooding function 0:disable 1:enable
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get port trunking unknown flooding function. 
 */
ret_t rtl8370_getAsicTrunkingFlood(uint32* enabled)
{
    return rtl8370_getAsicRegBit(RTL8370_REG_PORT_TRUNK_CTRL, RTL8370_PORT_TRUNK_FLOOD_OFFSET, enabled);
}
/*
@func ret_t | rtl8370_setAsicTrunkingHashSelect | Set port trunking hash select sources
@parm uint32 | hashsel | hash sources mask
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_LA_HASHMASK | Hash algorithm selection error.
@comm
    The API can set port trunking hash algorithm sources.
    7 bits mask for link aggregation group0 hash parameter selection {DIP, SIP, DMAC, SMAC, SPA}
    0b0000001: SPA
    0b0000010: SMAC
    0b0000100: DMAC
    0b0001000: SIP
    0b0010000: DIP
    0b0100000: TCP/UDP Source Port
    0b1000000: TCP/UDP Destination Port
    Example:
    0b0000011: SMAC & SPA
    Note that it could be an arbitrary combination or independent set
 */
ret_t rtl8370_setAsicTrunkingHashSelect(uint32 hashsel)
{
    if (hashsel > RTL8370_PORT_TRUNK_HASH_MASK)
        return RT_ERR_LA_HASHMASK;

    return rtl8370_setAsicRegBits(RTL8370_REG_PORT_TRUNK_CTRL, RTL8370_PORT_TRUNK_HASH_MASK, hashsel);
}
/*
@func ret_t | rtl8370_getAsicTrunkingHashSelect | Get port trunking hash select sources
@parm uint32* | hashsel | hash sources mask
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get port trunking hash algorithm sources.
 */
ret_t rtl8370_getAsicTrunkingHashSelect(uint32* hashsel)
{
    return rtl8370_getAsicRegBits(RTL8370_REG_PORT_TRUNK_CTRL, RTL8370_PORT_TRUNK_HASH_MASK, hashsel);
}
    
/*
@func ret_t | rtl8370_getAsicQeueuEmptyStatus | get current output queue if empty status
@parm uint32* | portmask | queue empty port mask
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get queues are empty port mask
 */
ret_t rtl8370_getAsicQeueuEmptyStatus(uint32* portmask)
{
    return rtl8370_getAsicReg(RTL8370_REG_PORT_QEMPTY, portmask);
}



