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
 * Feature : Packet generator related functions
 *
 */

#include "rtl8367b_asicdrv_pkg.h"

/* Function Name:
 *      rtl8367b_setAsicPkgCfg
 * Description:
 *      Set per-port packet generation enable/disable
 * Input:
 *      port 		- Physical port number (0~7)
 *      cfg 		- configuration type
 *      enabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPkgCfg(rtk_uint32 port, rtk_uint32 cfg, rtk_uint32 enabled)
{
	ret_t   retVal;
	rtk_uint32 regData;
    
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    if(cfg == PKG_CONFIG_START)
    {
        retVal = rtl8367b_getAsicRegBit(RTL8367B_PKG_CFG_REG(port),RTL8367B_PKTGEN_PORT0_CTRL_STATUS_OFFSET,&regData);
        if(retVal != RT_ERR_OK)
    	    return retVal;
        if(regData > 0)
            return RT_ERR_BUSYWAIT_TIMEOUT;
    }
    
    return rtl8367b_setAsicRegBit(RTL8367B_PKG_CFG_REG(port), cfg, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicPkgCfg
 * Description:
 *      Get per-port packet generation enable/disable
 * Input:
 *      port 		- Physical port number (0~7)
 *      cfg 		- configuration type
 *      pEnabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPkgCfg(rtk_uint32 port, rtk_uint32 cfg, rtk_uint32 *pEnabled)
{
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;
	
    return rtl8367b_getAsicRegBit(RTL8367B_PKG_CFG_REG(port), cfg, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicPkgMac
 * Description:
 *      Set per-port packet generation MAC address
 * Input:
 *      port 	- Physical port number (0~7)
 *      type 	- 0:DA 1:SA
 *      pMac 	- mac address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPkgMac(rtk_uint32 port, rtk_uint32 type, smi_ether_addr_t* pMac)
{
	rtk_uint16* ptrTmp;
	ret_t   retVal;
	rtk_uint32 regData;
	rtk_uint32 regAddr;
	rtk_uint32 i;

	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

	ptrTmp = (rtk_uint16* )pMac;
	
	if(0 == type)
		regAddr = RTL8367B_PKG_DA_REG(port);
	else
		regAddr = RTL8367B_PKG_SA_REG(port);

	for(i = 0; i < 3; i++)
	{
		regData = *(ptrTmp + i);
		retVal = rtl8367b_setAsicReg(regAddr + i, regData);
	    if(retVal != RT_ERR_OK)
	        return retVal;
	}
    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_getAsicPkgMac
 * Description:
 *      Get per-port packet generation MAC address
 * Input:
 *      port 	- Physical port number (0~7)
 *      type 	- 0:DA 1:SA
 *      pMac 	- mac address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPkgMac(rtk_uint32 port, rtk_uint32 type, smi_ether_addr_t* pMac)
{
	rtk_uint16* ptrTmp;
	ret_t retVal;
	rtk_uint32 regData;
	rtk_uint32 regAddr;
	rtk_uint32 i;

	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

	ptrTmp = (rtk_uint16* )pMac;

	if( 0 == type )
		regAddr = RTL8367B_PKG_DA_REG(port);
	else
		regAddr = RTL8367B_PKG_SA_REG(port);

	for(i = 0; i < 3; i++)
	{
		retVal = rtl8367b_getAsicReg(regAddr + i, &regData);
	    if(retVal != RT_ERR_OK)
	        return retVal;

		*(ptrTmp + i) = regData;
	}

	return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicPkgNum
 * Description:
 *      Set per-port packet generation number
 * Input:
 *      port 	- Physical port number (0~7)
 *      number 	- number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPkgNum(rtk_uint32 port, rtk_uint32 number)
{
	ret_t   retVal;
	
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

	retVal = rtl8367b_setAsicReg(RTL8367B_PKG_NUM_REG(port), number&0xFFFF);
    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8367b_setAsicReg(RTL8367B_PKG_NUM_REG(port)+1, number>>16);
}
/* Function Name:
 *      rtl8367b_getAsicPkgNum
 * Description:
 *      Get per-port packet generation number
 * Input:
 *      port 	- Physical port number (0~7)
 *      pNumber 	- number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPkgNum(rtk_uint32 port, rtk_uint32* pNumber)
{
	ret_t   retVal;
	rtk_uint32 regData;
	
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

	retVal = rtl8367b_getAsicReg(RTL8367B_PKG_NUM_REG(port), &regData);
    if(retVal != RT_ERR_OK)
        return retVal;
	
	*pNumber = regData;
	
	retVal = rtl8367b_getAsicReg(RTL8367B_PKG_NUM_REG(port)+1, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

	*pNumber = (regData<<16) | *pNumber;
;

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicPkgLength
 * Description:
 *      Set per-port packet generation length
 * Input:
 *      port 	- Physical port number (0~7)
 *      length 	- Packet length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPkgLength(rtk_uint32 port, rtk_uint32 length)
{
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    return rtl8367b_setAsicReg(RTL8367B_PKG_LENGTH_REG(port), length);
}
/* Function Name:
 *      rtl8367b_getAsicPkgLength
 * Description:
 *      Get per-port packet generation length
 * Input:
 *      port 	- Physical port number (0~7)
 *      pLength 	- Packet length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPkgLength(rtk_uint32 port, rtk_uint32* pLength)
{
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    return rtl8367b_getAsicReg(RTL8367B_PKG_LENGTH_REG(port), pLength);
}
/* Function Name:
 *      rtl8367b_setAsicPkgMaxLength
 * Description:
 *      Set per-port packet generation maximum length
 * Input:
 *      port 	- Physical port number (0~7)
 *      length 	- Packet length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPkgMaxLength(rtk_uint32 port, rtk_uint32 length)
{
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    return rtl8367b_setAsicReg(RTL8367B_PKG_MAXLENGTH_REG(port), length);
}
/* Function Name:
 *      rtl8367b_getAsicPkgMaxLength
 * Description:
 *      Get per-port packet generation maximum length
 * Input:
 *      port 	- Physical port number (0~7)
 *      pLength 	- Packet length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPkgMaxLength(rtk_uint32 port, rtk_uint32* pLength)
{
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    return rtl8367b_getAsicReg(RTL8367B_PKG_MAXLENGTH_REG(port), pLength);
}
/* Function Name:
 *      rtl8367b_setAsicPkgBypassFC
 * Description:
 *      Set packet generation bypass flow control
 * Input:
 *      enabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPkgBypassFC(rtk_uint32 enabled)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_PKTGEN_COMMAND, RTL8367B_PKTGEN_BYPASS_FLOWCONTROL_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicPkgBypassFC
 * Description:
 *      Get packet generation bypass flow control
 * Input:
 *      pEnabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPkgBypassFC(rtk_uint32* pEnabled)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_PKTGEN_COMMAND, RTL8367B_PKTGEN_BYPASS_FLOWCONTROL_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicPkgPayload
 * Description:
 *      Set packet generation user defined payload
 * Input:
 *      index 	- 0~47
 *      payload - User defined payload byte
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - Invalid payload index
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPkgPayload(rtk_uint32 index, rtk_uint32 payload)
{
	rtk_uint32 regAddr;
	rtk_uint32 regMask;

	if(index >= PKG_PAYLOAD_SIZE)
		return RT_ERR_OUT_OF_RANGE;
	
	regAddr = RTL8367B_REG_PKTGEN_PAYLOAD_CTRL0 + (index >> 1);
	regMask = 0xFF << ((index & 0x1) << 3);

    return rtl8367b_setAsicRegBits(regAddr, regMask, payload);
}
/* Function Name:
 *      rtl8367b_getAsicPkgPayload
 * Description:
 *      Get packet generation user defined payload
 * Input:
 *      index 	- 0~47
 *      pPayload - User defined payload byte
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - Invalid payload index
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPkgPayload(rtk_uint32 index, rtk_uint32* pPayload)
{
	rtk_uint32 regAddr;
	rtk_uint32 regMask;

	if(index >= PKG_PAYLOAD_SIZE)
		return RT_ERR_OUT_OF_RANGE;
	
	regAddr = RTL8367B_REG_PKTGEN_PAYLOAD_CTRL0 + (index >> 1);
	regMask = 0xFF << ((index & 0x1) << 3);

    return rtl8367b_getAsicRegBits(regAddr, regMask,pPayload);
}
