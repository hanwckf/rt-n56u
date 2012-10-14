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
 * $Revision: 6657 $
 * $Date: 2009-10-30 14:48:32 +0800 (Fri, 30 Oct 2009) $
 *
 * Purpose : RTL8367B switch high-level API for RTL8367B
 * Feature : Green ethernet related functions
 *
 */
#include "rtl8367b_asicdrv_gpio.h"
/* Function Name:
 *      rtl8367b_getAsicGpioInput
 * Description:
 *      Get gpio input 
 * Input:
 *      gpioPin 		- GPIO pin number
 *      pInput 			- GPIO input
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE 	- input out of range.
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicGpioInput(rtk_uint32 gpioPin, rtk_uint32* pGpioInput)
{
	if(gpioPin >= RTL8367B_GPIOPINNO)
		return RT_ERR_OUT_OF_RANGE;
		
    return rtl8367b_getAsicRegBit(RTL8367B_REG_STATUS_GPIO, gpioPin, pGpioInput);
}
/* Function Name:
 *      rtl8367b_setAsicGpioOutput
 * Description:
 *      Set gpio output
 * Input:
 *      gpioPin 		- GPIO pin number
 *      gpioOutput 		- GPIO output
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE 	- input out of range.
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicGpioOutput(rtk_uint32 gpioPin, rtk_uint32 gpioOutput)
{
	if(gpioPin >= RTL8367B_GPIOPINNO)
		return RT_ERR_OUT_OF_RANGE;
	
    return rtl8367b_setAsicRegBit(RTL8367B_REG_CTRL_GPIO, gpioPin, gpioOutput);
}
/* Function Name:
 *      rtl8367b_getAsicGpioOutput
 * Description:
 *      Get gpio output
 * Input:
 *      gpioPin 		- GPIO pin number
 *      pGpioOutput 	- GPIO output
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE 	- input out of range.
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicGpioOutput(rtk_uint32 gpioPin, rtk_uint32* pGpioOutput)
{
	if(gpioPin >= RTL8367B_GPIOPINNO)
		return RT_ERR_OUT_OF_RANGE;
	
    return rtl8367b_getAsicRegBit(RTL8367B_REG_CTRL_GPIO, gpioPin, pGpioOutput);
}
/* Function Name:
 *      rtl8367b_setAsicGpioSelect
 * Description:
 *      Set gpio control 
 * Input:
 *      gpioPin 		- GPIO pin number
 *      gpioSelect		- GPIO select 0:by Asic 1:by 8051
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE 	- input out of range.
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicGpioSelect(rtk_uint32 gpioPin, rtk_uint32 gpioSelect)
{
	if(gpioPin >= RTL8367B_GPIOPINNO)
		return RT_ERR_OUT_OF_RANGE;
	
    return rtl8367b_setAsicRegBit(RTL8367B_REG_SEL_GPIO, gpioPin, gpioSelect);
}
/* Function Name:
 *      rtl8367b_getAsicGpioSelect
 * Description:
 *      Get gpio output
 * Input:
 *      gpioPin 		- GPIO pin number
 *      pGpioSelect		- GPIO select 0:by Asic 1:by 8051
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE 	- input out of range.
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicGpioSelect(rtk_uint32 gpioPin, rtk_uint32* pGpioSelect)
{
	if(gpioPin >= RTL8367B_GPIOPINNO)
		return RT_ERR_OUT_OF_RANGE;
	
    return rtl8367b_getAsicRegBit(RTL8367B_REG_SEL_GPIO, gpioPin, pGpioSelect);
}
/* Function Name:
 *      rtl8367b_setAsicGpioEn
 * Description:
 *      Set gpio control 
 * Input:
 *      gpioPin 		- GPIO pin number
 *      gpioEn			- GPIO enabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE 	- input out of range.
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicGpioEn(rtk_uint32 gpioPin, rtk_uint32 gpioEn)
{
	if(gpioPin >= RTL8367B_GPIOPINNO)
		return RT_ERR_OUT_OF_RANGE;
	
    return rtl8367b_setAsicRegBit(RTL8367B_REG_EN_GPIO, gpioPin, gpioEn);
}
/* Function Name:
 *      rtl8367b_getAsicGpioEn
 * Description:
 *      Get gpio output
 * Input:
 *      gpioPin 		- GPIO pin number
 *      pGpioEn			- GPIO enabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE 	- input out of range.
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicGpioEn(rtk_uint32 gpioPin, rtk_uint32* pGpioEn)
{
	if(gpioPin >= RTL8367B_GPIOPINNO)
		return RT_ERR_OUT_OF_RANGE;
	
    return rtl8367b_getAsicRegBit(RTL8367B_REG_EN_GPIO, gpioPin, pGpioEn);
}
/* Function Name:
 *      rtl8367b_setAsicGpioAclEnClear
 * Description:
 *      Set gpio control 
 * Input:
 *      gpioPin 		- GPIO pin number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE 	- input out of range.
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicGpioAclEnClear(rtk_uint32 gpioPin)
{
	if(gpioPin >= RTL8367B_GPIOPINNO)
		return RT_ERR_OUT_OF_RANGE;
	
    return rtl8367b_setAsicRegBit(RTL8367B_REG_ACL_GPIO, gpioPin, 1);
}
/* Function Name:
 *      rtl8367b_getAsicGpioAclEnClear
 * Description:
 *      Get gpio output
 * Input:
 *      gpioPin 		- GPIO pin number
 *      pGpioEn			- GPIO enabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE 	- input out of range.
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicGpioAclEnClear(rtk_uint32 gpioPin, rtk_uint32* pGpioEn)
{
	if(gpioPin >= RTL8367B_GPIOPINNO)
		return RT_ERR_OUT_OF_RANGE;
	
    return rtl8367b_getAsicRegBit(RTL8367B_REG_ACL_GPIO, gpioPin, pGpioEn);
}

