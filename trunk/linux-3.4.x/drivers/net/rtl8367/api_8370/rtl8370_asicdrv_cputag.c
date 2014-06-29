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
 * $Date: 2010/12/02 04:34:26 $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367B
 * Feature : 
 *
 */

#include "rtl8370_asicdrv_cputag.h"

/*
@func ret_t | rtl8370_setAsicCputagEnable | Set cpu tag function enable/disable.
@parm uint32 | enable | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_ENABLE | Invalid enable/disable input.
@comm
    The API can set CPU TAG enable function.

    If CPU tag function is disabled, CPU tag will not be added to frame
    forwarded to CPU port, and all ports cannot parse CPU tag.
*/
ret_t rtl8370_setAsicCputagEnable(uint32 enable)
{
    if(enable > 1)
        return RT_ERR_ENABLE;
    
    return rtl8370_setAsicRegBit(RTL8370_CPU_CTRL_REG, RTL8370_CPU_EN_OFFSET, enable);
}

/*
@func ret_t | rtl8370_getAsicCputagEnable | Get cpu tag function enable/disable.
@parm uint32* | enable | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@comm
    The API can get CPU TAG enable function.

    If CPU tag function is disabled, CPU tag will not be added to frame
    forwarded to CPU port, and all ports cannot parse CPU tag.

*/
ret_t rtl8370_getAsicCputagEnable(uint32 *enable)
{
    return rtl8370_getAsicRegBit(RTL8370_CPU_CTRL_REG, RTL8370_CPU_EN_OFFSET, enable);
}

/*
@func ret_t | rtl8370_setAsicCputagTrapPort | Set cpu tag trap port
@parm uint32 | port | port number (0~15)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID  Invalid port number.
@comm
    This API can set destination port of trapping frame
*/
ret_t rtl8370_setAsicCputagTrapPort(uint32 port)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8370_setAsicRegBits(RTL8370_CPU_CTRL_REG, RTL8370_CPU_TRAP_PORT_MASK, port);    
}

/*
@func ret_t | rtl8370_getAsicCputagTrapPort | Get cpu tag trap port
@parm uint32* | port | port number (0~15)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@comm
    This API can set destination port of trapping frame
*/
ret_t rtl8370_getAsicCputagTrapPort(uint32 *port)
{
    return rtl8370_getAsicRegBits(RTL8370_CPU_CTRL_REG, RTL8370_CPU_TRAP_PORT_MASK, port);    
}

/*
@func ret_t | rtl8370_setAsicCputagPortmask | Set ports that can parse CPU tag
@parm uint32 | pmsk | portmask (0~0xFFFF)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_MASK | Invalid portmask.
@comm
    This API can set ports that are able to parse CPU tag
*/
ret_t rtl8370_setAsicCputagPortmask(uint32 pmsk)
{
    if(pmsk > RTL8370_PORTMASK)
        return RT_ERR_PORT_MASK;

    return rtl8370_setAsicReg(RTL8370_CPU_PORT_MASK_REG, pmsk);
}

/*
@func ret_t | rtl8370_getAsicCputagPortmask | Get ports that can parse CPU tag
@parm uint32* | pmsk | portmask (0~0xFFFF)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@comm
    This API can set ports that are able to parse CPU tag
*/
ret_t rtl8370_getAsicCputagPortmask(uint32 *pmsk)
{
    return rtl8370_getAsicReg(RTL8370_CPU_PORT_MASK_REG, pmsk);
}

/*
@func ret_t | rtl8370_setAsicCputagInsertMode | Set ports that can parse CPU tag
@parm uint32 | mode | 0: insert to all packets; 1: insert to trapped packets; 2: don't insert
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_NOT_ALLOWED | Invalid action.
@comm
    This API can set the decision of which frames should be inserted
    with CPU tage. 
*/
ret_t rtl8370_setAsicCputagInsertMode(uint32 mode)
{
    if(mode >= CPUTAG_INSERT_MAX)
        return RT_ERR_NOT_ALLOWED;

    return rtl8370_setAsicRegBits(RTL8370_CPU_CTRL_REG, RTL8370_CPU_INSERTMODE_MASK, mode);
}

/*
@func ret_t | rtl8370_getAsicCputagInsertMode | Get ports that can parse CPU tag
@parm uint32 | mode | 0: insert to all packets; 1: insert to trapped packets; 2: don't insert
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@comm
    This API can get the decision of which frames should be inserted
    with CPU tage. 
*/
ret_t rtl8370_getAsicCputagInsertMode(uint32 *mode)
{
    return rtl8370_getAsicRegBits(RTL8370_CPU_CTRL_REG, RTL8370_CPU_INSERTMODE_MASK, mode);
}

/*
@func ret_t | rtl8370_setAsicCputagPriorityRemapping | Set queue assignment of CPU port
@parm uint32 | srcPri | internal priority (0~7)
@parm uint32 | newPri | internal priority after remapping (0~7)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_QOS_INT_PRIORITY | Invalid priority.
@comm
    This API can set the priority remapping of frames to CPU port
*/
ret_t rtl8370_setAsicCputagPriorityRemapping(uint32 srcPri, uint32 newPri)
{
    if(srcPri > RTL8370_PRIMAX || newPri > RTL8370_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    return rtl8370_setAsicRegBits(RTL8370_QOS_PRIPORITY_REMAPPING_IN_CPU_REG(srcPri), RTL8370_QOS_PRIPORITY_REMAPPING_IN_CPU_MASK(srcPri), newPri);
}

/*
@func ret_t | rtl8370_getAsicCputagPriorityRemapping | Get queue assignment of CPU port
@parm uint32 | srcPri | internal priority (0~7)
@parm uint32* | newPri | internal priority after remapping (0~7)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_QOS_INT_PRIORITY | Invalid priority.
@comm
    This API can set the priority remapping of frames to CPU port
*/
ret_t rtl8370_getAsicCputagPriorityRemapping(uint32 srcPri, uint32 *newPri)
{
    if(srcPri > RTL8370_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    return rtl8370_getAsicRegBits(RTL8370_QOS_PRIPORITY_REMAPPING_IN_CPU_REG(srcPri), RTL8370_QOS_PRIPORITY_REMAPPING_IN_CPU_MASK(srcPri), newPri);
}

