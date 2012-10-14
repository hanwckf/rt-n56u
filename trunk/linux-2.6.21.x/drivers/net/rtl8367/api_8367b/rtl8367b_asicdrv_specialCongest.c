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
 * $Revision: 14202 $
 * $Date: 2010-11-16 15:13:00 +0800 (星期二, 16 十一月 2010) $
 *
 * Purpose : RTL8367B switch high-level API for RTL8367B
 * Feature : Special congestion related functions
 *
 */
#include "rtl8367b_asicdrv_specialCongest.h"
/* Function Name:
 *      rtl8367b_setAsicSpecialCongestModeConfig
 * Description:
 *      Set ASIC special congest mode configuration
 * Input:
 *      port 	- Physical port number (0~7)
 *      sustain - sustain timer (0-15)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID  	- Invalid port number
 *      RT_ERR_OUT_OF_RANGE - Input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicSpecialCongestModeConfig(rtk_uint32 port, rtk_uint32 sustain)
{
	if(port >= RTL8367B_PORTNO)
		return RT_ERR_PORT_ID;
	
    if(sustain > RTL8367B_SPECIALCONGEST_SUSTAIN_TIMERMAX)
        return RT_ERR_OUT_OF_RANGE;

	return rtl8367b_setAsicRegBits(RTL8367B_PORT_MISC_CFG_REG(port), RTL8367B_SPECIALCONGEST_SUSTAIN_TIMER_MASK, sustain);
}
/* Function Name:
 *      rtl8367b_getAsicSpecialCongestModeConfig
 * Description:
 *      Get ASIC special congest mode configuration
 * Input:
 *      port 	- Physical port number (0~7)
 *      pSustain - sustain timer (0-15)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID  	- Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicSpecialCongestModeConfig(rtk_uint32 port, rtk_uint32 *pSustain)
{
	if(port >= RTL8367B_PORTNO)
		return RT_ERR_PORT_ID;

	return rtl8367b_getAsicRegBits(RTL8367B_PORT_MISC_CFG_REG(port), RTL8367B_SPECIALCONGEST_SUSTAIN_TIMER_MASK, pSustain); 
}
/* Function Name:
 *      rtl8367b_getAsicSpecialCongestModeTimer
 * Description:
 *      Get ASIC special congest mode timer
 * Input:
 *      port 	- Physical port number (0~7)
 *      pTimer 	- time (0-15)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID  	- Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicSpecialCongestModeTimer(rtk_uint32 port, rtk_uint32* pTimer)
{
    if(port > RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBits(RTL8367B_PORT_SPECIAL_CONGEST_MODE_TIMER_REG(port), RTL8367B_PKTGEN_PORT0_TIMER_TIMER_MASK, pTimer);
}
