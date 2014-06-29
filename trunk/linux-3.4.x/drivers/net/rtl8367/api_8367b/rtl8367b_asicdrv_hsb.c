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
 * Feature : Field selector related functions
 *
 */
#include "rtl8367b_asicdrv_hsb.h"
/* Function Name:
 *      rtl8367b_setAsicFieldSelector
 * Description:
 *      Set user defined field selectors in HSB
 * Input:
 *      index 		- index of field selector 0-15
 *      format 		- Format of field selector
 *      offset 		- Retrieving data offset
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      System support 16 user defined field selctors.
 * 		Each selector can be enabled or disable. User can defined retrieving 16-bits in many predefiend 
 * 		standard l2/l3/l4 payload.
 */
ret_t rtl8367b_setAsicFieldSelector(rtk_uint32 index, rtk_uint32 format, rtk_uint32 offset)
{
	rtk_uint32 regData;
	
	if(index > RTL8367B_FIELDSEL_FORMAT_NUMBER)
		return RT_ERR_OUT_OF_RANGE;
	
	if(format >= FIELDSEL_FORMAT_END)
		return RT_ERR_OUT_OF_RANGE;

	regData = (((format << RTL8367B_FIELD_SELECTOR_FORMAT_OFFSET) & RTL8367B_FIELD_SELECTOR_FORMAT_MASK ) |
			   ((offset << RTL8367B_FIELD_SELECTOR_OFFSET_OFFSET) & RTL8367B_FIELD_SELECTOR_OFFSET_MASK ));

    return rtl8367b_setAsicReg(RTL8367B_FIELD_SELECTOR_REG(index), regData);
}
/* Function Name:
 *      rtl8367b_getAsicFieldSelector
 * Description:
 *      Get user defined field selectors in HSB
 * Input:
 *      index 	    - index of field selector 0-15
 *      pFormat 	- Format of field selector
 *      pOffset 	- Retrieving data offset
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicFieldSelector(rtk_uint32 index, rtk_uint32* pFormat, rtk_uint32* pOffset)
{
    ret_t retVal;
	rtk_uint32 regData;

    retVal = rtl8367b_getAsicReg(RTL8367B_FIELD_SELECTOR_REG(index), &regData);
	if(retVal != RT_ERR_OK)
		return retVal;

	*pFormat    = ((regData & RTL8367B_FIELD_SELECTOR_FORMAT_MASK) >> RTL8367B_FIELD_SELECTOR_FORMAT_OFFSET);
	*pOffset    = ((regData & RTL8367B_FIELD_SELECTOR_OFFSET_MASK) >> RTL8367B_FIELD_SELECTOR_OFFSET_OFFSET);

    return RT_ERR_OK;
}
