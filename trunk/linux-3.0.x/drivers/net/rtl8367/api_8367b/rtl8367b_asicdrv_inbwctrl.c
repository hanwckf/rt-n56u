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
 * Feature : Ingress bandwidth control related functions
 *
 */
#include "rtl8367b_asicdrv_inbwctrl.h"
/* Function Name:
 *      rtl8367b_setAsicPortIngressBandwidth
 * Description:
 *      Set per-port total ingress bandwidth
 * Input:
 *      port 		- Physical port number (0~7)
 *      bandwidth 	- The total ingress bandwidth (unit: 8Kbps), 0x1FFFF:disable
 *      preifg 		- Include preamble and IFG, 0:Exclude, 1:Include
 *      enableFC 	- Action when input rate exceeds. 0: Drop	1: Flow Control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID  	- Invalid port number
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPortIngressBandwidth(rtk_uint32 port, rtk_uint32 bandwidth, rtk_uint32 preifg, rtk_uint32 enableFC)
{
	ret_t retVal;
	rtk_uint32 regData;
	rtk_uint32 regAddr;

	/* Invalid input parameter */
	if(port >= RTL8367B_PORTNO)
		return RT_ERR_PORT_ID;

	if(bandwidth > RTL8367B_QOS_GRANULARTY_MAX)
		return RT_ERR_OUT_OF_RANGE;

	regAddr = RTL8367B_INGRESSBW_PORT_RATE_LSB_REG(port);
	regData = bandwidth & RTL8367B_QOS_GRANULARTY_LSB_MASK;
	retVal = rtl8367b_setAsicReg(regAddr, regData);
	if(retVal != RT_ERR_OK) 
		return retVal;

	regAddr += 1;
	regData = (bandwidth & RTL8367B_QOS_GRANULARTY_MSB_MASK) >> RTL8367B_QOS_GRANULARTY_MSB_OFFSET;
	retVal = rtl8367b_setAsicRegBit(regAddr, 0, regData);
	if(retVal != RT_ERR_OK) 
		return retVal;

	regAddr = RTL8367B_PORT_MISC_CFG_REG(port);
	retVal = rtl8367b_setAsicRegBit(regAddr, RTL8367B_PORT0_MISC_CFG_INGRESSBW_IFG_OFFSET, preifg);
	if(retVal != RT_ERR_OK) 
		return retVal;

	regAddr = RTL8367B_PORT_MISC_CFG_REG(port);
	retVal = rtl8367b_setAsicRegBit(regAddr, RTL8367B_PORT0_MISC_CFG_INGRESSBW_FLOWCTRL_OFFSET, enableFC);
	if(retVal != RT_ERR_OK) 
		return retVal;
	
	return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_getAsicPortIngressBandwidth
 * Description:
 *      Get per-port total ingress bandwidth
 * Input:
 *      port 		- Physical port number (0~7)
 *      pBandwidth 	- The total ingress bandwidth (unit: 8Kbps), 0x1FFFF:disable
 *      pPreifg 		- Include preamble and IFG, 0:Exclude, 1:Include
 *      pEnableFC 	- Action when input rate exceeds. 0: Drop	1: Flow Control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID  	- Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortIngressBandwidth(rtk_uint32 port, rtk_uint32* pBandwidth, rtk_uint32* pPreifg, rtk_uint32* pEnableFC)
{
	ret_t retVal;
	rtk_uint32 regData;
	rtk_uint32 regAddr;

	/* Invalid input parameter */
	if(port >= RTL8367B_PORTNO)
		return RT_ERR_PORT_ID;

	regAddr = RTL8367B_INGRESSBW_PORT_RATE_LSB_REG(port);
	retVal = rtl8367b_getAsicReg(regAddr, &regData);
	if(retVal != RT_ERR_OK)
		return retVal;

	*pBandwidth = regData;
	
	regAddr += 1;
	retVal = rtl8367b_getAsicRegBit(regAddr, 0, &regData);
	if(retVal != RT_ERR_OK)
		return retVal;
	
	*pBandwidth |= (regData << RTL8367B_QOS_GRANULARTY_MSB_OFFSET);

	regAddr = RTL8367B_PORT_MISC_CFG_REG(port);
	retVal = rtl8367b_getAsicRegBit(regAddr, RTL8367B_PORT0_MISC_CFG_INGRESSBW_IFG_OFFSET, pPreifg);
	if(retVal != RT_ERR_OK)
		return retVal;
		
	regAddr = RTL8367B_PORT_MISC_CFG_REG(port);
	retVal = rtl8367b_getAsicRegBit(regAddr, RTL8367B_PORT0_MISC_CFG_INGRESSBW_FLOWCTRL_OFFSET, pEnableFC);
	if(retVal != RT_ERR_OK)
		return retVal;

	return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicPortIngressBandwidthBypass
 * Description:
 *      Set ingress bandwidth control bypasss 8899, RMA 01-80-C2-00-00-xx and IGMP
 * Input:
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPortIngressBandwidthBypass(rtk_uint32 enabled)
{
	return rtl8367b_setAsicRegBit(RTL8367B_REG_SW_DUMMY0, RTL8367B_INGRESSBW_BYPASS_EN_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicPortIngressBandwidthBypass
 * Description:
 *      Set ingress bandwidth control bypasss 8899, RMA 01-80-C2-00-00-xx and IGMP
 * Input:
 *      pEnabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortIngressBandwidthBypass(rtk_uint32* pEnabled)
{
	return rtl8367b_getAsicRegBit(RTL8367B_REG_SW_DUMMY0, RTL8367B_INGRESSBW_BYPASS_EN_OFFSET, pEnabled);
}

