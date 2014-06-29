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
#include "rtl8370_asicdrv_rma.h"

/*=======================================================================
 *  Reserved Multicast Address APIs
 *========================================================================*/
/*
@func ret_t | rtl8370_setAsicRma | Set reserved multicast address for CPU trapping.
@parm uint32 | index | reserved multicast LSB byte, 0x00~0x2F is available value.
@parm rtl8370_rma_t* | rmacfg | type of RMA for trapping frame type setting.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_RMA_ADDR | Invalid RMA address index. 
@comm
    System support Resserved Multicast Address 01-80-C2-00-00-00~2F aware function. User can configure features relatived 
    packet be trapped to CPU or directly dropped. 
*/
ret_t rtl8370_setAsicRma(uint32 index, rtl8370_rma_t* rmacfg)
{
    uint32 regData;

    if(index > RTL8370_RMAMAX)
        return RT_ERR_RMA_ADDR;

    regData = *(uint16*)rmacfg;

    return rtl8370_setAsicReg(RTL8370_RMA_CTRL_BASE + index,regData);     
}

/*
@func ret_t | rtl8370_getAsicRma | Set reserved multicast address for CPU trapping.
@parm uint32 | index | reserved multicast LSB byte, 0x00~0x2F is available value.
@parm rtl8370_rma_t* | rmacfg | type of RMA for trapping frame type setting.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_RMA_ADDR | Invalid RMA address index. 
@comm
    System support Resserved Multicast Address 01-80-C2-00-00-00~2F aware function. User can configure features relatived 
    packet be trapped to CPU or directly dropped. 
*/
ret_t rtl8370_getAsicRma(uint32 index, rtl8370_rma_t* rmacfg)
{
    ret_t retVal;
    uint32 regData;
    uint16 regData16;

    if(index > RTL8370_RMAMAX)
        return RT_ERR_RMA_ADDR;

    retVal = rtl8370_getAsicReg(RTL8370_RMA_CTRL_BASE + index, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    regData16 = (uint16)(regData&0xFFFF);
    *rmacfg = *(rtl8370_rma_t *)(&regData16);

    return RT_ERR_OK;
}

