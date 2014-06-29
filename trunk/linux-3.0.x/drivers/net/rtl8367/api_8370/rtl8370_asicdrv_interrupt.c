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
 * $Date: 2010/12/02 04:34:18 $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367B
 * Feature : 
 *
 */
#include "rtl8370_asicdrv_interrupt.h"

/*
@func ret_t | rtl8370_setAsicInterruptPolarity | set interrupt trigger polarity
@parm uint32 | polarity | 0:pull high 1: pull low
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API can be used to set I/O polarity while port linking status chnaged. Pull high GPIO
    while setting value is 0 and pull low while setting value 1
*/
ret_t rtl8370_setAsicInterruptPolarity(uint32 polarity)
{
    return rtl8370_setAsicRegBit(RTL8370_INTR_CTRL_REG, RTL8370_INTR_POLARITY_OFFSET, polarity);
}

/*
@func ret_t | rtl8370_getAsicInterruptPolarity | set interrupt trigger polarity
@parm uint32* | polarity | 0:pull high 1: pull low
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API can be used to set I/O polarity while port linking status chnaged. Pull high GPIO
    while setting value is 0 and pull low while setting value 1
*/
ret_t rtl8370_getAsicInterruptPolarity(uint32* polarity)
{
    return rtl8370_getAsicRegBit(RTL8370_INTR_CTRL_REG, RTL8370_INTR_POLARITY_OFFSET, polarity);
}

/*
@func ret_t | rtl8370_setAsicInterruptMask | set interrupt enable mask
@parm uint32 | imr | [0]:Link change, [1]:Share meter exceed, [2]:Learn number overed, [3]:Media change, [4]Speed Change, [5]:Tx special congestion
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API can be used to set ASIC interrupt enable condition as link down/link up or others.
*/
ret_t rtl8370_setAsicInterruptMask(uint32 imr)
{
    return rtl8370_setAsicReg(RTL8370_INTR_IMR_REG, imr);
}
    
/*
@func ret_t | rtl8370_getAsicInterruptMask | set interrupt enable mask
@parm uint32* | imr | [0]:Link change, [1]:Share meter exceed, [2]:Learn number overed, [3]:Media change, [4]Speed Change, [5]:Tx special congestion
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API can be used to set ASIC interrupt enable condition as link down/link up or others.
*/
ret_t rtl8370_getAsicInterruptMask(uint32* imr)
{
    return rtl8370_getAsicReg(RTL8370_INTR_IMR_REG, imr);
}
/*
@func ret_t | rtl8370_setAsicInterruptStatus | get interrupt status
@parm uint32 | ims | interrupt status mask
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API can be used to clear ASIC interrupt status and register will be cleared by writting 1.
    [0]:Link change,
    [1]:Share meter exceed,
    [2]:Learn number overed, 
    [4]Speed Change, 
    [5]:Tx special congestion
    [6]:1 second green feature
    [7]:loop detection
    [8]:interrupt from 8051
*/
ret_t rtl8370_setAsicInterruptStatus(uint32 ims)
{
    return rtl8370_setAsicReg(RTL8370_INTR_IMS_REG, ims);
}
/*
@func ret_t | rtl8370_getAsicInterruptStatus | get interrupt status
@parm uint32* | ims | interrupt status mask
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API can be used to get ASIC interrupt status and register will be cleared by writting 1.
*/
ret_t rtl8370_getAsicInterruptStatus(uint32* ims)
{
    return rtl8370_getAsicReg(RTL8370_INTR_IMS_REG, ims);
}

/*
@func ret_t | rtl8370_setAsicInterruptRelatedStatus | get interrupt status
@parm uint32 | type | per port Learn over, per-port speed change, per-port special congest, share meter exceed status
@parm uint32 | status | per port Learn over, per-port speed change, per-port special congest, share meter exceed status, write 1 to clear.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_INPUT | Invalid input value
@comm
    This API can be used to clear ASIC interrupt per port related status and share meter exceed status. All status will be clear by write 1.
*/
ret_t rtl8370_setAsicInterruptRelatedStatus(uint32 type, uint32 status)
{
    if(type >= INTRST_MAX )
        return RT_ERR_INPUT;

    return rtl8370_setAsicReg(RTL8370_INTR_INDICATOR_BASED + type, status);
}
/*
@func ret_t | rtl8370_getAsicInterruptRelatedStatus | get interrupt status
@parm uint32 | type | per port Learn over, per-port speed change, per-port special congest, share meter exceed status
@parm uint32* | status | per port Learn over, per-port speed change, per-port special congest, share meter exceed status
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_INPUT | Invalid input value
@comm
    This API can be used to get ASIC interrupt per port related status and share meter exceed status. All status will be clear by READ.
*/
ret_t rtl8370_getAsicInterruptRelatedStatus(uint32 type,uint32* status)
{
    if(type >= INTRST_MAX )
        return RT_ERR_INPUT;
    
    return rtl8370_getAsicReg(RTL8370_INTR_INDICATOR_BASED + type, status);
}

