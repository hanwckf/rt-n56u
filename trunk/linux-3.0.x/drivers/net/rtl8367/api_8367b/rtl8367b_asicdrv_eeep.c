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
 * Feature : EEE plus related functions
 *
 */
#include "rtl8367b_asicdrv_eeep.h"
/* Function Name:
 *      rtl8367b_setAsicEeepTxEnable
 * Description:
 *      Set per-port EEEP Tx function enable/disable
 * Input:
 *      port 		- Physical port number (0~7)
 *      enabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      EEEP function is enabled, MAC will auto reducing power consumption of switch
 */
ret_t rtl8367b_setAsicEeepTxEnable(rtk_uint32 port, rtk_uint32 enabled)
{
	if(port >= RTL8367B_PORTNO)
		return RT_ERR_PORT_ID;

    return rtl8367b_setAsicRegBit(RTL8367B_EEEP_CFG_REG(port), RTL8367B_PORT0_EEECFG_EEEP_ENABLE_TX_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicEeepTxEnable
 * Description:
 *      Get per-port EEEP Tx function enable/disable
 * Input:
 *      port 		- Physical port number (0~7)
 *      pEnabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicEeepTxEnable(rtk_uint32 port, rtk_uint32 *pEnabled)
{
	if(port >= RTL8367B_PORTNO)
		return RT_ERR_PORT_ID;
	
    return rtl8367b_getAsicRegBit(RTL8367B_EEEP_CFG_REG(port), RTL8367B_PORT0_EEECFG_EEEP_ENABLE_TX_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicEeepRxEnable
 * Description:
 *      Set per-port EEEP Rx function enable/disable
 * Input:
 *      port 		- Physical port number (0~7)
 *      enabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicEeepRxEnable(rtk_uint32 port, rtk_uint32 enabled)
{
	if(port >= RTL8367B_PORTNO)
		return RT_ERR_PORT_ID;
	
    return rtl8367b_setAsicRegBit(RTL8367B_EEEP_CFG_REG(port), RTL8367B_PORT0_EEECFG_EEEP_ENABLE_RX_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicEeepRxEnable
 * Description:
 *      Get per-port EEEP Rx function enable/disable
 * Input:
 *      port 		- Physical port number (0~7)
 *      pEnabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicEeepRxEnable(rtk_uint32 port, rtk_uint32 *pEnabled)
{
	if(port >= RTL8367B_PORTNO)
		return RT_ERR_PORT_ID;
	
    return rtl8367b_getAsicRegBit(RTL8367B_EEEP_CFG_REG(port), RTL8367B_PORT0_EEECFG_EEEP_ENABLE_RX_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicEeepGigaTxRateEnable
 * Description:
 *      Set EEEP Giga Tx rate LPI enable/disable
 * Input:
 *      enabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicEeepGigaTxRateEnable(rtk_uint32 enabled)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_EEEP_GIGA_CTRL2, RTL8367B_EEEP_TXEN_GIGA_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicEeepGigaTxRateEnable
 * Description:
 *      Get EEEP Giga Tx rate LPI enable/disable
 * Input:
 *      pEnabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicEeepGigaTxRateEnable(rtk_uint32* pEnabled)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_EEEP_GIGA_CTRL2, RTL8367B_EEEP_TXEN_GIGA_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicEeep100mTxRateEnable
 * Description:
 *      Set EEEP 100Mpbs Tx rate LPI enable/disable
 * Input:
 *      enabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicEeep100mTxRateEnable(rtk_uint32 enabled)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_EEEP_GIGA_CTRL2, RTL8367B_EEEP_TXEN_GIGA_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicEeep100mTxRateEnable
 * Description:
 *      Get EEEP 100Mpbs Tx rate LPI enable/disable
 * Input:
 *      pEnabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicEeep100mTxRateEnable(rtk_uint32* pEnabled)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_EEEP_GIGA_CTRL2, RTL8367B_EEEP_TXEN_GIGA_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicEeepSleepStep
 * Description:
 *      Set EEEP maximum sleep step
 * Input:
 *      step 	- Sleep step
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      Step is as bigger as reducing more power consumption. But, it also
 *   	means the packet queue time in sleep mode is as longer as step value
 */
ret_t rtl8367b_setAsicEeepSleepStep(rtk_uint32 step)
{
    return rtl8367b_setAsicRegBits(RTL8367B_REG_EEEP_CTRL0, RTL8367B_EEEP_SLEEP_STEP_MASK, step);
}
/* Function Name:
 *      rtl8367b_getAsicEeepSleepStep
 * Description:
 *      Get EEEP maximum sleep step
 * Input:
 *      pStep 	- Sleep step
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicEeepSleepStep(rtk_uint32* pStep)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_EEEP_CTRL0, RTL8367B_EEEP_SLEEP_STEP_MASK, pStep);
}
/* Function Name:
 *      rtl8367b_setAsicEeep100mTimeUint
 * Description:
 *      Set EEEP time unit in 100Mpbs mode
 * Input:
 *      unit 	- Time unit 0:1us 1:16us 2:128us 3:1024 unit
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicEeep100mTimeUint(rtk_uint32 unit)
{
    return rtl8367b_setAsicRegBits(RTL8367B_REG_EEEP_100M_CTRL2, RTL8367B_EEEP_TU_100M_MASK, unit);
}
/* Function Name:
 *      rtl8367b_getAsicEeep100mTimeUint
 * Description:
 *      Get EEEP time unit in 100Mpbs mode
 * Input:
 *      pUnit 	- Time unit 0:1us 1:16us 2:128us 3:1024 unit
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicEeep100mTimeUint(rtk_uint32* pUnit)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_EEEP_100M_CTRL2, RTL8367B_EEEP_TU_100M_MASK, pUnit);
}
/* Function Name:
 *      rtl8367b_setAsicEeepGigaTimeUint
 * Description:
 *      Set EEEP time unit in Giga mode
 * Input:
 *      unit 	- Time unit 0:1us 1:16us 2:128us 3:1024 unit
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicEeepGigaTimeUint(rtk_uint32 unit)
{
    return rtl8367b_setAsicRegBits(RTL8367B_REG_EEEP_GIGA_CTRL2, RTL8367B_EEEP_TU_GIGA_MASK, unit);
}
/* Function Name:
 *      rtl8367b_getAsicEeepGigaTimeUint
 * Description:
 *      Get EEEP time unit in Giga mode
 * Input:
 *      pUnit 	- Time unit 0:1us 1:16us 2:128us 3:1024 unit
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicEeepGigaTimeUint(rtk_uint32* pUnit)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_EEEP_GIGA_CTRL2, RTL8367B_EEEP_TU_GIGA_MASK, pUnit);
}
/* Function Name:
 *      rtl8367b_setAsicEeep100mRxRateTimer
 * Description:
 *      Set EEEP detect Rx rate timer in 100Mpbs mode
 * Input:
 *      timer 	- Timer of detecting Rx rate
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicEeep100mRxRateTimer(rtk_uint32 timer)
{
    return rtl8367b_setAsicRegBits(RTL8367B_REG_EEEP_100M_CTRL0, RTL8367B_EEEP_TR_100M_MASK, timer);
}
/* Function Name:
 *      rtl8367b_getAsicEeep100mRxRateTimer
 * Description:
 *      Get EEEP detect Rx rate timer in 100Mpbs mode
 * Input:
 *      pTimer 	- Timer of detecting Rx rate
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicEeep100mRxRateTimer(rtk_uint32* pTimer)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_EEEP_100M_CTRL0, RTL8367B_EEEP_TR_100M_MASK, pTimer);
}
/* Function Name:
 *      rtl8367b_setAsicEeepGigaRxRateTimer
 * Description:
 *      Set EEEP detect Rx rate timer in Giga mode
 * Input:
 *      timer 	- Timer of detecting Rx rate
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicEeepGigaRxRateTimer(rtk_uint32 timer)
{
    return rtl8367b_setAsicRegBits(RTL8367B_REG_EEEP_GIGA_CTRL0, RTL8367B_EEEP_TR_GIGA_MASK, timer);
}
/* Function Name:
 *      rtl8367b_getAsicEeepGigaRxRateTimer
 * Description:
 *      Get EEEP detect Rx rate timer in Giga mode
 * Input:
 *      pTimer 	- Timer of detecting Rx rate
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicEeepGigaRxRateTimer(rtk_uint32* pTimer)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_EEEP_GIGA_CTRL0, RTL8367B_EEEP_TR_GIGA_MASK, pTimer);
}
/* Function Name:
 *      rtl8367b_setAsicEeep100mTxRateTimer
 * Description:
 *      Set EEEP detect Tx rate timer in 100Mpbs mode
 * Input:
 *      timer 	- Timer of detecting Rx rate
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicEeep100mTxRateTimer(rtk_uint32 timer)
{
    return rtl8367b_setAsicRegBits(RTL8367B_REG_EEEP_CTRL1, RTL8367B_EEEP_TXR_100M_MASK, timer);
}
/* Function Name:
 *      rtl8367b_getAsicEeep100mTxRateTimer
 * Description:
 *      Get EEEP detect Tx rate timer in 100Mpbs mode
 * Input:
 *      pTimer 	- Timer of detecting Rx rate
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicEeep100mTxRateTimer(rtk_uint32* pTimer)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_EEEP_CTRL1, RTL8367B_EEEP_TXR_100M_MASK, pTimer);
}
/* Function Name:
 *      rtl8367b_setAsicEeepGigaTxRateTimer
 * Description:
 *      Set EEEP detect Tx rate timer in Giga mode
 * Input:
 *      timer 	- Timer of detecting Rx rate
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicEeepGigaTxRateTimer(rtk_uint32 timer)
{
    return rtl8367b_setAsicRegBits(RTL8367B_REG_EEEP_CTRL1, RTL8367B_EEEP_TXR_GIGA_MASK, timer);
}
/* Function Name:
 *      rtl8367b_getAsicEeepGigaTxRateTimer
 * Description:
 *      Get EEEP detect Tx rate timer in Giga mode
 * Input:
 *      pTimer 	- Timer of detecting Rx rate
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicEeepGigaTxRateTimer(rtk_uint32* pTimer)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_EEEP_CTRL1, RTL8367B_EEEP_TXR_GIGA_MASK, pTimer);
}
/* Function Name:
 *      rtl8367b_setAsicEeep100mRxRateTh
 * Description:
 *      Set EEEP detect Rx rate threshold for sleeping in 100Mpbs mode
 * Input:
 *      threshold 	- Threshold of Rx rate for falling sleep
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */ 
ret_t rtl8367b_setAsicEeep100mRxRateTh(rtk_uint32 threshold)
{
    return rtl8367b_setAsicReg(RTL8367B_REG_EEEP_RX_RATE_100M, threshold);
}
/* Function Name:
 *      rtl8367b_getAsicEeep100mRxRateTh
 * Description:
 *      Get EEEP detect Rx rate threshold for sleeping in 100Mpbs mode
 * Input:
 *      pThreshold 	- Threshold of Rx rate for falling sleep
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */ 
ret_t rtl8367b_getAsicEeep100mRxRateTh(rtk_uint32* pThreshold)
{
    return rtl8367b_getAsicReg(RTL8367B_REG_EEEP_RX_RATE_100M, pThreshold);
}
/* Function Name:
 *      rtl8367b_setAsicEeepGigaRxRateTh
 * Description:
 *      Set EEEP detect Rx rate threshold for sleeping in GIGA mode
 * Input:
 *      threshold 	- Threshold of Rx rate for falling sleep
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */ 
ret_t rtl8367b_setAsicEeepGigaRxRateTh(rtk_uint32 threshold)
{
    return rtl8367b_setAsicReg(RTL8367B_REG_EEEP_RX_RATE_GIGA, threshold);
}
/* Function Name:
 *      rtl8367b_getAsicEeepGigaRxRateTh
 * Description:
 *      Get EEEP detect Rx rate threshold for sleeping in GIGA mode
 * Input:
 *      pThreshold 	- Threshold of Rx rate for falling sleep
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */ 
ret_t rtl8367b_getAsicEeepGigaRxRateTh(rtk_uint32* pThreshold)
{
    return rtl8367b_getAsicReg(RTL8367B_REG_EEEP_RX_RATE_GIGA, pThreshold);
}
/* Function Name:
 *      rtl8367b_setAsicEeep100mTxRateTh
 * Description:
 *      Set EEEP detect Tx rate threshold for sleeping in 100Mpbs mode
 * Input:
 *      threshold 	- Threshold of Rx rate for falling sleep
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */ 
ret_t rtl8367b_setAsicEeep100mTxRateTh(rtk_uint32 threshold)
{
    return rtl8367b_setAsicReg(RTL8367B_REG_EEEP_TX_RATE_100M, threshold);
}
/* Function Name:
 *      rtl8367b_getAsicEeep100mTxRateTh
 * Description:
 *      Get EEEP detect Tx rate threshold for sleeping in 100Mpbs mode
 * Input:
 *      pThreshold 	- Threshold of Rx rate for falling sleep
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */ 
ret_t rtl8367b_getAsicEeep100mTxRateTh(rtk_uint32* pThreshold)
{
    return rtl8367b_getAsicReg(RTL8367B_REG_EEEP_TX_RATE_100M, pThreshold);
}
/* Function Name:
 *      rtl8367b_setAsicEeepGigaTxRateTh
 * Description:
 *      Set EEEP detect Tx rate threshold for sleeping in GIGA mode
 * Input:
 *      threshold 	- Threshold of Rx rate for falling sleep
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */  
ret_t rtl8367b_setAsicEeepGigaTxRateTh(rtk_uint32 threshold)
{
    return rtl8367b_setAsicReg(RTL8367B_REG_EEEP_TX_RATE_GIGA, threshold);
}
/* Function Name:
 *      rtl8367b_getAsicEeepGigaTxRateTh
 * Description:
 *      Get EEEP detect Tx rate threshold for sleeping in GIGA mode
 * Input:
 *      pThreshold 	- Threshold of Rx rate for falling sleep
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */  
ret_t rtl8367b_getAsicEeepGigaTxRateTh(rtk_uint32* pThreshold)
{
    return rtl8367b_getAsicReg(RTL8367B_REG_EEEP_TX_RATE_GIGA, pThreshold);
}
/* Function Name:
 *      rtl8367b_setAsicEeep100mSleepTimer
 * Description:
 *      Set EEEP falling sleeping timer for 1 step in 100Mpbs mode
 * Input:
 *      timer 	- Timer in timer unit
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */  
ret_t rtl8367b_setAsicEeep100mSleepTimer(rtk_uint32 timer)
{
    return rtl8367b_setAsicRegBits(RTL8367B_REG_EEEP_100M_CTRL2, RTL8367B_EEEP_TS_100M_MASK, timer);
}
/* Function Name:
 *      rtl8367b_getAsicEeep100mSleepTimer
 * Description:
 *      Get EEEP falling sleeping timer for 1 step in 100Mpbs mode
 * Input:
 *      pTimer 	- Timer in timer unit
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */  
ret_t rtl8367b_getAsicEeep100mSleepTimer(rtk_uint32* pTimer)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_EEEP_100M_CTRL2, RTL8367B_EEEP_TS_100M_MASK, pTimer);
}
/* Function Name:
 *      rtl8367b_setAsicEeepGigaSleepTimer
 * Description:
 *      Set EEEP falling sleeping timer for 1 step in GIGA mode
 * Input:
 *      timer 	- Timer in timer unit
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */  
ret_t rtl8367b_setAsicEeepGigaSleepTimer(rtk_uint32 timer)
{
    return rtl8367b_setAsicRegBits(RTL8367B_REG_EEEP_GIGA_CTRL2, RTL8367B_EEEP_TS_GIGA_MASK, timer);
}
/* Function Name:
 *      rtl8367b_getAsicEeepGigaSleepTimer
 * Description:
 *      Get EEEP falling sleeping timer for 1 step in GIGA mode
 * Input:
 *      pTimer 	- Timer in timer unit
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */  
ret_t rtl8367b_getAsicEeepGigaSleepTimer(rtk_uint32* pTimer)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_EEEP_GIGA_CTRL2, RTL8367B_EEEP_TS_GIGA_MASK, pTimer);
}
/* Function Name:
 *      rtl8367b_setAsicEeep100mPauseOnTimer
 * Description:
 *      Set EEEP timer atfer sending pause on before falling sleep in 100Mpbs mode
 * Input:
 *      timer 	- Timer in timer unit
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */   
ret_t rtl8367b_setAsicEeep100mPauseOnTimer(rtk_uint32 timer)
{
    return rtl8367b_setAsicRegBits(RTL8367B_REG_EEEP_100M_CTRL1, RTL8367B_EEEP_TP_100M_MASK, timer);
}
/* Function Name:
 *      rtl8367b_getAsicEeep100mPauseOnTimer
 * Description:
 *      Get EEEP timer atfer sending pause on before falling sleep in 100Mpbs mode
 * Input:
 *      pTimer 	- Timer in timer unit
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */   
ret_t rtl8367b_getAsicEeep100mPauseOnTimer(rtk_uint32* pTimer)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_EEEP_100M_CTRL1, RTL8367B_EEEP_TP_100M_MASK, pTimer);
}
/* Function Name:
 *      rtl8367b_setAsicEeepGigaPauseOnTimer
 * Description:
 *      Set EEEP timer atfer sending pause on before falling sleep in GIGA mode
 * Input:
 *      timer 	- Timer in timer unit
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */   
ret_t rtl8367b_setAsicEeepGigaPauseOnTimer(rtk_uint32 timer)
{
    return rtl8367b_setAsicRegBits(RTL8367B_REG_EEEP_GIGA_CTRL1, RTL8367B_EEEP_TP_GIGA_MASK, timer);
}
/* Function Name:
 *      rtl8367b_getAsicEeepGigaPauseOnTimer
 * Description:
 *      Get EEEP timer atfer sending pause on before falling sleep in GIGA mode
 * Input:
 *      pTimer 	- Timer in timer unit
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */   
ret_t rtl8367b_getAsicEeepGigaPauseOnTimer(rtk_uint32* pTimer)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_EEEP_GIGA_CTRL1, RTL8367B_EEEP_TP_GIGA_MASK, pTimer);
}
/* Function Name:
 *      rtl8367b_setAsicEeep100mTxWakeupTimer
 * Description:
 *      Set EEEP timer before sending TX queue packets in 100Mpbs mode
 * Input:
 *      timer 	- Timer in timer unit
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */   
ret_t rtl8367b_setAsicEeep100mTxWakeupTimer(rtk_uint32 timer)
{
    return rtl8367b_setAsicRegBits(RTL8367B_REG_EEEP_100M_CTRL1, RTL8367B_EEEP_TW_100M_MASK, timer);
}
/* Function Name:
 *      rtl8367b_getAsicEeep100mTxWakeupTimer
 * Description:
 *      Get EEEP timer before sending TX queue packets in 100Mpbs mode
 * Input:
 *      pTimer 	- Timer in timer unit
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */   
ret_t rtl8367b_getAsicEeep100mTxWakeupTimer(rtk_uint32* pTimer)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_EEEP_100M_CTRL1, RTL8367B_EEEP_TW_100M_MASK, pTimer);
}
/* Function Name:
 *      rtl8367b_setAsicEeepGigaTxWakeupTimer
 * Description:
 *      Set EEEP timer before sending TX queue packets in GIGA mode
 * Input:
 *      timer 	- Timer in timer unit
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */   
ret_t rtl8367b_setAsicEeepGigaTxWakeupTimer(rtk_uint32 timer)
{
    return rtl8367b_setAsicRegBits(RTL8367B_REG_EEEP_GIGA_CTRL1, RTL8367B_EEEP_TW_GIGA_MASK, timer);
}
/* Function Name:
 *      rtl8367b_getAsicEeepGigaTxWakeupTimer
 * Description:
 *      Get EEEP timer before sending TX queue packets in GIGA mode
 * Input:
 *      pTimer 	- Timer in timer unit
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */   
ret_t rtl8367b_getAsicEeepGigaTxWakeupTimer(rtk_uint32* pTimer)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_EEEP_GIGA_CTRL1, RTL8367B_EEEP_TW_GIGA_MASK, pTimer);
}
/* Function Name:
 *      rtl8367b_setAsicEeep100mRxWakeupTimer
 * Description:
 *      Set EEEP timer for Rx wakeup in 100Mpbs mode
 * Input:
 *      timer 	- Timer in timer unit
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */  
ret_t rtl8367b_setAsicEeep100mRxWakeupTimer(rtk_uint32 timer)
{
    return rtl8367b_setAsicRegBits(RTL8367B_REG_EEEP_100M_CTRL0, RTL8367B_EEEP_RW_100M_MASK, timer);
}
/* Function Name:
 *      rtl8367b_getAsicEeep100mRxWakeupTimer
 * Description:
 *      Get EEEP timer for Rx wakeup in 100Mpbs mode
 * Input:
 *      pTimer 	- Timer in timer unit
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */  
ret_t rtl8367b_getAsicEeep100mRxWakeupTimer(rtk_uint32* pTimer)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_EEEP_100M_CTRL0, RTL8367B_EEEP_RW_100M_MASK, pTimer);
}
/* Function Name:
 *      rtl8367b_setAsicEeepGigaRxWakeupTimer
 * Description:
 *      Set EEEP timer for Rx wakeup in GIGA mode
 * Input:
 *      timer 	- Timer in timer unit
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */  
ret_t rtl8367b_setAsicEeepGigaRxWakeupTimer(rtk_uint32 timer)
{
    return rtl8367b_setAsicRegBits(RTL8367B_REG_EEEP_GIGA_CTRL0, RTL8367B_EEEP_RW_GIGA_MASK, timer);
}
/* Function Name:
 *      rtl8367b_getAsicEeepGigaRxWakeupTimer
 * Description:
 *      Get EEEP timer for Rx wakeup in GIGA mode
 * Input:
 *      pTimer 	- Timer in timer unit
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */  
ret_t rtl8367b_getAsicEeepGigaRxWakeupTimer(rtk_uint32* pTimer)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_EEEP_GIGA_CTRL0, RTL8367B_EEEP_RW_GIGA_MASK, pTimer);
}
