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
 * $Revision: 28599 $
 * $Date: 2012-05-07 09:41:37 +0800 (星期一, 07 五月 2012) $
 *
 * Purpose : RTL8367B switch high-level API for RTL8367B
 * Feature : Interrupt related functions
 *
 */
#include "rtl8367b_asicdrv_interrupt.h"
/* Function Name:
 *      rtl8367b_setAsicInterruptPolarity
 * Description:
 *      Set interrupt trigger polarity
 * Input:
 *      polarity 	- 0:pull high 1: pull low
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicInterruptPolarity(rtk_uint32 polarity)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_INTR_CTRL, RTL8367B_INTR_CTRL_OFFSET, polarity);
}
/* Function Name:
 *      rtl8367b_getAsicInterruptPolarity
 * Description:
 *      Get interrupt trigger polarity
 * Input:
 *      pPolarity 	- 0:pull high 1: pull low
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicInterruptPolarity(rtk_uint32* pPolarity)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_INTR_CTRL, RTL8367B_INTR_CTRL_OFFSET, pPolarity);
}
/* Function Name:
 *      rtl8367b_setAsicInterruptMask
 * Description:
 *      Set interrupt enable mask
 * Input:
 *      imr 	- Interrupt mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicInterruptMask(rtk_uint32 imr)
{
    return rtl8367b_setAsicReg(RTL8367B_REG_INTR_IMR, imr);
}
/* Function Name:
 *      rtl8367b_getAsicInterruptMask
 * Description:
 *      Get interrupt enable mask
 * Input:
 *      pImr 	- Interrupt mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicInterruptMask(rtk_uint32* pImr)
{
    return rtl8367b_getAsicReg(RTL8367B_REG_INTR_IMR, pImr);
}
/* Function Name:
 *      rtl8367b_setAsicInterruptMask
 * Description:
 *      Clear interrupt enable mask
 * Input:
 *      ims 	- Interrupt status mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *   	This API can be used to clear ASIC interrupt status and register will be cleared by writting 1.
 *    	[0]:Link change,
 *    	[1]:Share meter exceed,
 *    	[2]:Learn number overed, 
 *    	[3]:Speed Change, 
 *    	[4]:Tx special congestion
 *    	[5]:1 second green feature
 *    	[6]:loop detection
 *    	[7]:interrupt from 8051
 *    	[8]:Cable diagnostic finish
 *    	[9]:ACL action interrupt trigger
 *    	[10]:UPS event
 */
ret_t rtl8367b_setAsicInterruptStatus(rtk_uint32 ims)
{
    return rtl8367b_setAsicReg(RTL8367B_REG_INTR_IMS, ims);
}
/* Function Name:
 *      rtl8367b_getAsicInterruptStatus
 * Description:
 *      Get interrupt enable mask
 * Input:
 *      pIms 	- Interrupt status mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *   	None
 */
ret_t rtl8367b_getAsicInterruptStatus(rtk_uint32* pIms)
{
    return rtl8367b_getAsicReg(RTL8367B_REG_INTR_IMS, pIms);
}
/* Function Name:
 *      rtl8367b_setAsicInterruptRelatedStatus
 * Description:
 *      Clear interrupt status
 * Input:
 *      type 	- per port Learn over, per-port speed change, per-port special congest, share meter exceed status
 *      status 	- exceed status, write 1 to clear
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *   	None
 */
ret_t rtl8367b_setAsicInterruptRelatedStatus(rtk_uint32 type, rtk_uint32 status)
{
    CONST rtk_uint32 indicatorAddress[INTRST_END] = {RTL8367B_REG_LEARN_OVER_INDICATOR,
													RTL8367B_REG_SPEED_CHANGE_INDICATOR,
													RTL8367B_REG_SPECIAL_CONGEST_INDICATOR,
													RTL8367B_REG_PORT_LINKDOWN_INDICATOR,
													RTL8367B_REG_PORT_LINKUP_INDICATOR,
													RTL8367B_REG_METER_OVERRATE_INDICATOR0,
													RTL8367B_REG_METER_OVERRATE_INDICATOR1,
													RTL8367B_REG_RLDP_LOOPED_INDICATOR,
													RTL8367B_REG_RLDP_RELEASED_INDICATOR};
	
    if(type >= INTRST_END )
        return RT_ERR_OUT_OF_RANGE;

    return rtl8367b_setAsicReg(indicatorAddress[type], status);
}
/* Function Name:
 *      rtl8367b_getAsicInterruptRelatedStatus
 * Description:
 *      Get interrupt status
 * Input:
 *      type 	- per port Learn over, per-port speed change, per-port special congest, share meter exceed status
 *      pStatus 	- exceed status, write 1 to clear
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *   	None
 */
ret_t rtl8367b_getAsicInterruptRelatedStatus(rtk_uint32 type, rtk_uint32* pStatus)
{
    CONST rtk_uint32 indicatorAddress[INTRST_END] = {RTL8367B_REG_LEARN_OVER_INDICATOR,
													RTL8367B_REG_SPEED_CHANGE_INDICATOR,
													RTL8367B_REG_SPECIAL_CONGEST_INDICATOR,
													RTL8367B_REG_PORT_LINKDOWN_INDICATOR,
													RTL8367B_REG_PORT_LINKUP_INDICATOR,
													RTL8367B_REG_METER_OVERRATE_INDICATOR0,
													RTL8367B_REG_METER_OVERRATE_INDICATOR1,
													RTL8367B_REG_RLDP_LOOPED_INDICATOR,
													RTL8367B_REG_RLDP_RELEASED_INDICATOR};

    if(type >= INTRST_END )
        return RT_ERR_OUT_OF_RANGE;

    return rtl8367b_getAsicReg(indicatorAddress[type], pStatus);
}

