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
 * Feature : Miscellaneous functions
 *
 */

#include "rtl8367b_asicdrv_misc.h"
/* Function Name:
 *      rtl8367b_setAsicMacAddress
 * Description:
 *      Set switch MAC address
 * Input:
 *      mac 	- switch mac
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicMacAddress(ether_addr_t mac)
{
	ret_t retVal;
	rtk_uint32 regData;
	rtk_uint8 *accessPtr;
	rtk_uint32 i;

	accessPtr =  (rtk_uint8*)&mac;

	regData = *accessPtr;
	accessPtr ++;
	regData = (regData << 8) | *accessPtr;
	accessPtr ++;
	for(i = 0; i <=2; i++)
	{
		retVal = rtl8367b_setAsicReg(RTL8367B_REG_SWITCH_MAC2 - i, regData);
		if(retVal != RT_ERR_OK)
			return retVal;

		regData = *accessPtr;
		accessPtr ++;
		regData = (regData << 8) | *accessPtr;
		accessPtr ++;
	}

	return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicMacAddress
 * Description:
 *      Get switch MAC address
 * Input:
 *      pMac 	- switch mac
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicMacAddress(ether_addr_t *pMac)
{
	ret_t retVal;
	rtk_uint32 regData;
	rtk_uint8 *accessPtr;
	rtk_uint32 i;


	accessPtr = (rtk_uint8*)pMac;

	for(i = 0; i <= 2; i++)
	{
		retVal = rtl8367b_getAsicReg(RTL8367B_REG_SWITCH_MAC2 - i, &regData);
		if(retVal != RT_ERR_OK)
			return retVal;

		*accessPtr = (regData & 0xFF00) >> 8;
		accessPtr ++;
		*accessPtr = regData & 0xFF;
		accessPtr ++;
	}

	return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicDebugInfo
 * Description:
 *      Get per-port packet forward debugging information
 * Input:
 *      port 		- Physical port number (0~7)
 *      pDebugifo 	- per-port packet trap/drop/forward reason
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicDebugInfo(rtk_uint32 port, rtk_uint32 *pDebugifo)
{
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBits(RTL8367B_DEBUG_INFO_REG(port), RTL8367B_DEBUG_INFO_MASK(port), pDebugifo);
}
/* Function Name:
 *      rtl8367b_setAsicPortJamMode
 * Description:
 *      Set half duplex flow control setting
 * Input:
 *      mode 	- 0: Back-Pressure 1: DEFER
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPortJamMode(rtk_uint32 mode)
{
	return rtl8367b_setAsicRegBit(RTL8367B_REG_CFG_BACKPRESSURE, RTL8367B_LONGTXE_OFFSET,mode);
}
/* Function Name:
 *      rtl8367b_getAsicPortJamMode
 * Description:
 *      Get half duplex flow control setting
 * Input:
 *      pMode 	- 0: Back-Pressure 1: DEFER
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortJamMode(rtk_uint32* pMode)
{
	return rtl8367b_getAsicRegBit(RTL8367B_REG_CFG_BACKPRESSURE, RTL8367B_LONGTXE_OFFSET, pMode);
}
/* Function Name:
 *      rtl8367b_setAsicMaxLengthInRx
 * Description:
 *      Set Max receiving packet length
 * Input:
 *      maxLength 	- 0: 1522 bytes 1:1536 bytes 2:1552 bytes 3:16000bytes
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicMaxLengthInRx(rtk_uint32 maxLength)
{	
	return rtl8367b_setAsicRegBits(RTL8367B_REG_MAX_LENGTH_LIMINT_IPG, RTL8367B_MAX_LENTH_CTRL_MASK, maxLength);
}
/* Function Name:
 *      rtl8367b_getAsicMaxLengthInRx
 * Description:
 *      Get Max receiving packet length
 * Input:
 *      pMaxLength 	- 0: 1522 bytes 1:1536 bytes 2:1552 bytes 3:16000bytes
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicMaxLengthInRx(rtk_uint32* pMaxLength)
{
	return rtl8367b_getAsicRegBits(RTL8367B_REG_MAX_LENGTH_LIMINT_IPG, RTL8367B_MAX_LENTH_CTRL_MASK, pMaxLength);
}
/* Function Name:
 *      rtl8367b_setAsicMaxLengthAltTxRx
 * Description:
 *      Set per-port Max receiving/transmit packet length in different speed
 * Input:
 *      maxLength 	- 0: 1522 bytes 1:1536 bytes 2:1552 bytes 3:16000bytes
 *      pmskGiga 	- enable port mask in 100Mpbs
 *      pmask100M 	- enable port mask in Giga
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicMaxLengthAltTxRx(rtk_uint32 maxLength, rtk_uint32 pmskGiga, rtk_uint32 pmask100M)
{	
	ret_t retVal;

	retVal = rtl8367b_setAsicReg(RTL8367B_REG_MAX_LENGTH_CFG, ((pmskGiga << RTL8367B_MAX_LENGTH_GIGA_OFFSET) & RTL8367B_MAX_LENGTH_GIGA_MASK) | (pmask100M & RTL8367B_MAX_LENGTH_10_100M_MASK));
	if(retVal != RT_ERR_OK)
		return retVal;
    
	return rtl8367b_setAsicRegBits(RTL8367B_REG_MAX_LEN_RX_TX, RTL8367B_MAX_LEN_RX_TX_MASK, maxLength);
}

/* Function Name:
 *      rtl8367b_getAsicMaxLengthAltTxRx
 * Description:
 *      Get per-port Max receiving/transmit packet length in different speed
 * Input:
 *      pMaxLength 	- 0: 1522 bytes 1:1536 bytes 2:1552 bytes 3:16000bytes
 *      pPmskGiga 	- enable port mask in 100Mpbs
 *      pPmask100M 	- enable port mask in Giga
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicMaxLengthAltTxRx(rtk_uint32* pMaxLength, rtk_uint32* pPmskGiga, rtk_uint32* pPmask100M)
{	
	ret_t retVal;
	rtk_uint32 regData;

	retVal = rtl8367b_getAsicReg(RTL8367B_REG_MAX_LENGTH_CFG, &regData);
	if(retVal != RT_ERR_OK)
		return retVal;

    *pPmskGiga = (regData & RTL8367B_MAX_LENGTH_GIGA_MASK) >> RTL8367B_MAX_LENGTH_GIGA_OFFSET;
    *pPmask100M = regData & RTL8367B_MAX_LENGTH_10_100M_MASK;

    return rtl8367b_getAsicRegBits(RTL8367B_REG_MAX_LEN_RX_TX, RTL8367B_MAX_LEN_RX_TX_MASK, pMaxLength);
}


