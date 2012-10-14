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
 * Feature : RRCP related functions
 *
 */
#include "rtl8367b_asicdrv_rrcp.h"
/* Function Name:
 *      rtl8367b_setAsicRrcp
 * Description:
 *      Set RRCP function enable/disable
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
ret_t rtl8367b_setAsicRrcp(rtk_uint32 enabled)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_RRCP_CTRL0, RTL8367B_RRCP_ENABLE_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicRrcp
 * Description:
 *      Get RRCP function enable/disable
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
ret_t rtl8367b_getAsicRrcp(rtk_uint32 *pEnabled)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_RRCP_CTRL0, RTL8367B_RRCP_ENABLE_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicRrcpAuthPortmask
 * Description:
 *      Set authorized portmask of RRCP
 * Input:
 *      portmask 	- port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_MASK  	- Invalid portmask
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRrcpAuthPortmask(rtk_uint32 portmask)
{
    if(portmask > RTL8367B_PORTMASK)
        return RT_ERR_PORT_MASK;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_RRCP_CTRL1, RTL8367B_RRCP_AUTH_PMSK_MASK, portmask);
}
/* Function Name:
 *      rtl8367b_getAsicRrcpAuthPortmask
 * Description:
 *      Get authorized portmask of RRCP
 * Input:
 *      pPortmask 	- port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRrcpAuthPortmask(rtk_uint32* pPortmask)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_RRCP_CTRL1, RTL8367B_RRCP_AUTH_PMSK_MASK, pPortmask);
}
/* Function Name:
 *      rtl8367b_setAsicRrcpAdminPortmask
 * Description:
 *      Set administrable portmask of RRCP
 * Input:
 *      portmask 	- port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_MASK  	- Invalid portmask
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRrcpAdminPortmask(rtk_uint32 portmask)
{
    if(portmask > RTL8367B_PORTMASK)
        return RT_ERR_PORT_MASK;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_RRCP_CTRL1, RTL8367B_RRCP_ADMIN_PMSK_MASK, portmask);
}
/* Function Name:
 *      rtl8367b_getAsicRrcpAdminPortmask
 * Description:
 *      Get administrable portmask of RRCP
 * Input:
 *      pPortmask 	- port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRrcpAdminPortmask(rtk_uint32* pPortmask)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_RRCP_CTRL1, RTL8367B_RRCP_ADMIN_PMSK_MASK, pPortmask);
}
/* Function Name:
 *      rtl8367b_setAsicRrcpAuthenticationKey
 * Description:
 *      Set authentication key of RRCPv1
 * Input:
 *      authKey 	- (0~0xFFFF)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRrcpAuthenticationKey(rtk_uint32 authKey)
{
    if(authKey > RTL8367B_REGDATAMAX)
        return RT_ERR_OUT_OF_RANGE;

    return rtl8367b_setAsicReg(RTL8367B_REG_RRCP_CTRL4, authKey);    
}
/* Function Name:
 *      rtl8367b_getAsicRrcpAuthenticationKey
 * Description:
 *      Get authentication key of RRCPv1
 * Input:
 *      pAuthKey 	- (0~0xFFFF)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRrcpAuthenticationKey(rtk_uint32 *pAuthKey)
{
    return rtl8367b_getAsicReg(RTL8367B_REG_RRCP_CTRL4, pAuthKey);    
}
/* Function Name:
 *      rtl8367b_setAsicRrcpVendorId
 * Description:
 *      Set vendor ID of RRCPv1
 * Input:
 *      id 	- vendor ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRrcpVendorId(rtk_uint32 id)
{
    ret_t retVal;

	retVal = rtl8367b_setAsicReg(RTL8367B_REG_RRCP_CTRL5, id & 0xFFFF);
    if(retVal != RT_ERR_OK)
        return retVal;
	
    return rtl8367b_setAsicReg(RTL8367B_REG_RRCP_CTRL6, (id >> 16) & 0xFFFF);    
}
/* Function Name:
 *      rtl8367b_getAsicRrcpVendorId
 * Description:
 *      Get vendor ID of RRCPv1
 * Input:
 *      pId 	- vendor ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRrcpVendorId(rtk_uint32* pId)
{
    ret_t retVal;
    rtk_uint32 regData;
	
	retVal = rtl8367b_getAsicReg(RTL8367B_REG_RRCP_CTRL5, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

	*pId = regData;
	
	retVal = rtl8367b_getAsicReg(RTL8367B_REG_RRCP_CTRL6, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

	*pId = *pId | (regData << 16);

	return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicRrcpCustomerCode
 * Description:
 *      Set customer code of RRCP
 * Input:
 *      code 	- Customer code
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRrcpCustomerCode(rtk_uint32 code)
{
    ret_t retVal;

	retVal = rtl8367b_setAsicReg(RTL8367B_REG_RRCP_CTRL9, code & 0xFFFF);
    if(retVal != RT_ERR_OK)
        return retVal;
	
    return rtl8367b_setAsicReg(RTL8367B_REG_RRCP_CTRL10, (code >> 16) & 0xFFFF);    
}
/* Function Name:
 *      rtl8367b_getAsicRrcpCustomerCode
 * Description:
 *      Get customer code of RRCP
 * Input:
 *      pCode 	- Customer code
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRrcpCustomerCode(rtk_uint32* pCode)
{
    ret_t retVal;
    rtk_uint32 regData;
	
	retVal = rtl8367b_getAsicReg(RTL8367B_REG_RRCP_CTRL9, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

	*pCode = regData;
	
	retVal = rtl8367b_getAsicReg(RTL8367B_REG_RRCP_CTRL10, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

	*pCode = *pCode | (regData << 16);

	return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicRrcpPrivateKey
 * Description:
 *      Set private key of RRCP
 * Input:
 *      key 	- Private key
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRrcpPrivateKey(rtk_uint32 key)
{
    ret_t retVal;

	retVal = rtl8367b_setAsicReg(RTL8367B_REG_RRCP_CTRL7, key & 0xFFFF);
    if(retVal != RT_ERR_OK)
        return retVal;
	
    return rtl8367b_setAsicReg(RTL8367B_REG_RRCP_CTRL8, (key >> 16) & 0xFFFF);    
}
/* Function Name:
 *      rtl8367b_getAsicRrcpPrivateKey
 * Description:
 *      Get private key of RRCP
 * Input:
 *      pKey 	- Private key
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRrcpPrivateKey(rtk_uint32* pKey)
{
    ret_t retVal;
    rtk_uint32 regData;
	
	retVal = rtl8367b_getAsicReg(RTL8367B_REG_RRCP_CTRL7, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

	*pKey = regData;
	
	retVal = rtl8367b_getAsicReg(RTL8367B_REG_RRCP_CTRL8, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

	*pKey = *pKey | (regData << 16);

	return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicRrcpv3Handle
 * Description:
 *      Set RRCPv3 handle method
 * Input:
 *      handle 	- 0: unaware, 1:Trap DA is switch MAC 2:Trap all RRVPv3 packets
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_NOT_ALLOWED  - Actions not allowed by the function
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRrcpv3Handle(rtk_uint32 handle)
{
    if(handle >= RRCPHANDLE_END)
        return RT_ERR_NOT_ALLOWED;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_RRCP_CTRL0, RTL8367B_RRCPV3_HANDLE_MASK, handle);    
}
/* Function Name:
 *      rtl8367b_getAsicRrcpv3Handle
 * Description:
 *      Get RRCPv3 handle method
 * Input:
 *      pHandle 	- 0: unaware, 1:Trap DA is switch MAC 2:Trap all RRVPv3 packets
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRrcpv3Handle(rtk_uint32 *pHandle)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_RRCP_CTRL0, RTL8367B_RRCPV3_HANDLE_MASK, pHandle);    
}
/* Function Name:
 *      rtl8367b_setAsicRrcpv1Handle
 * Description:
 *      Set RRCPv1 handle method
 * Input:
 *      handle 	- 0: ASIC, 1:8051
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRrcpv1Handle(rtk_uint32 handle)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_RRCP_CTRL0, RTL8367B_RRCPV1_HANDLE_OFFSET, handle);    
}
/* Function Name:
 *      rtl8367b_getAsicRrcpv1Handle
 * Description:
 *      Get RRCPv1 handle method
 * Input:
 *      pHandle 	- 0: ASIC, 1:8051
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRrcpv1Handle(rtk_uint32 *pHandle)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_RRCP_CTRL0, RTL8367B_RRCPV1_HANDLE_OFFSET, pHandle);    
}
/* Function Name:
 *      rtl8367b_setAsicRrcpv1GetCrc
 * Description:
 *      Set security Crc checking for RRCPv1 GET packets
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
ret_t rtl8367b_setAsicRrcpv1GetCrc(rtk_uint32 enabled)
{
    
    return rtl8367b_setAsicRegBit(RTL8367B_REG_RRCP_CTRL0, RTL8367B_RRCPV1_SECURITY_CRC_GET_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicRrcpv1GetCrc
 * Description:
 *      Get security Crc checking for RRCPv1 GET packets
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
ret_t rtl8367b_getAsicRrcpv1GetCrc(rtk_uint32 *pEnabled)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_RRCP_CTRL0, RTL8367B_RRCPV1_SECURITY_CRC_GET_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicRrcpv1SetCrc
 * Description:
 *      Set security Crc checking for RRCPv1 SET packets
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
ret_t rtl8367b_setAsicRrcpv1SetCrc(rtk_uint32 enabled)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_RRCP_CTRL0, RTL8367B_RRCPV1_SECURITY_CRC_SET_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicRrcpv1SetCrc
 * Description:
 *      Get security Crc checking for RRCPv1 SET packets
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
ret_t rtl8367b_getAsicRrcpv1SetCrc(rtk_uint32 *pEnabled)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_RRCP_CTRL0, RTL8367B_RRCPV1_SECURITY_CRC_SET_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicRrcpv3Crc
 * Description:
 *      Set security Crc checking for RRCPv3 packets
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
ret_t rtl8367b_setAsicRrcpv3Crc(rtk_uint32 enabled)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_RRCP_CTRL0, RTL8367B_RRCPV3_SECURITY_CRC_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicRrcpv3Crc
 * Description:
 *      Get security Crc checking for RRCPv3 packets
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
ret_t rtl8367b_getAsicRrcpv3Crc(rtk_uint32 *pEnabled)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_RRCP_CTRL0, RTL8367B_RRCPV3_SECURITY_CRC_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicRrcpVlanLeaky
 * Description:
 *      Set RRCP VLAN leaky function
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
ret_t rtl8367b_setAsicRrcpVlanLeaky(rtk_uint32 enabled)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_RRCP_CTRL0, RTL8367B_RRCP_VLANLEAKY_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicRrcpVlanLeaky
 * Description:
 *      Get RRCP VLAN leaky function
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
ret_t rtl8367b_getAsicRrcpVlanLeaky(rtk_uint32 *pEnabled)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_RRCP_CTRL0, RTL8367B_RRCP_VLANLEAKY_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicRrcpPbVlan
 * Description:
 *      Set RRCP port-based VLAN function
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
ret_t rtl8367b_setAsicRrcpPbVlan(rtk_uint32 enabled)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_RRCP_CTRL0, RTL8367B_RRCP_PBVLAN_EN_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicRrcpPbVlan
 * Description:
 *      Get RRCP port-based VLAN function
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
ret_t rtl8367b_getAsicRrcpPbVlan(rtk_uint32 *pEnabled)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_RRCP_CTRL0, RTL8367B_RRCP_PBVLAN_EN_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicRrcpMalformAct
 * Description:
 *      Set malformed RRCPv1 packet action
 * Input:
 *      handle 	-  0: forward, 1:trap, 2:drop
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_NOT_ALLOWED  - Actions not allowed by the function
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRrcpMalformAct(rtk_uint32 handle)
{
    if(handle >= RRCPMALFORMED_END)
        return RT_ERR_NOT_ALLOWED;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_RRCP_CTRL0, RTL8367B_RRCPV1_MALFORMED_ACT_MASK, handle);    
}
/* Function Name:
 *      rtl8367b_getAsicRrcpMalformAct
 * Description:
 *      Get malformed RRCPv1 packet action
 * Input:
 *      pHandle 	-  0: forward, 1:trap, 2:drop
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRrcpMalformAct(rtk_uint32 *pHandle)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_RRCP_CTRL0, RTL8367B_RRCPV1_MALFORMED_ACT_MASK, pHandle);    
}
/* Function Name:
 *      rtl8367b_setAsicRrcpIndication
 * Description:
 *      Set RRCP self indication configuration
 * Input:
 *      period 	-  indication internal 2**period seconds
 *      time 	-  indication times, 0 means disable indication function
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_NOT_ALLOWED  - Actions not allowed by the function
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRrcpIndication(rtk_uint32 period, rtk_uint32 time)
{
    ret_t retVal;
	
    if(period > (RTL8367B_RRCPV1_HELLO_PEDIOD_MASK >> RTL8367B_RRCPV1_HELLO_PEDIOD_OFFSET))
        return RT_ERR_NOT_ALLOWED;

	if(time > (RTL8367B_RRCPV1_HELLO_COUNT_MASK >> RTL8367B_RRCPV1_HELLO_COUNT_OFFSET))
        return RT_ERR_NOT_ALLOWED;


	retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_RRCP_CTRL2, RTL8367B_RRCPV1_HELLO_PEDIOD_MASK, period);
    if(retVal != RT_ERR_OK)
        return retVal;


    return rtl8367b_setAsicRegBits(RTL8367B_REG_RRCP_CTRL2, RTL8367B_RRCPV1_HELLO_COUNT_MASK, time);    
}
/* Function Name:
 *      rtl8367b_getAsicRrcpIndication
 * Description:
 *      Get RRCP self indication configuration
 * Input:
 *      pPeriod 	-  indication internal 2**period seconds
 *      pTime 	-  indication times, 0 means disable indication function
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRrcpIndication(rtk_uint32* pPeriod, rtk_uint32* pTime)
{
    ret_t retVal;
	
	retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_RRCP_CTRL2, RTL8367B_RRCPV1_HELLO_PEDIOD_MASK, pPeriod);
    if(retVal != RT_ERR_OK)
        return retVal;


    return rtl8367b_getAsicRegBits(RTL8367B_REG_RRCP_CTRL2, RTL8367B_RRCPV1_HELLO_COUNT_MASK, pTime);    
}
/* Function Name:
 *      rtl8367b_setAsicRrcpHelloTag
 * Description:
 *      Set RRCPv1 hello egress tag format
 * Input:
 *      format 	-  0: Keep, 1:ALE 2:Fixed 3:untag
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_NOT_ALLOWED  - Actions not allowed by the function
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRrcpHelloTag(rtk_uint32 format)
{
    if(format >= RRCPTAG_END)
        return RT_ERR_NOT_ALLOWED;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_RRCP_CTRL2, RTL8367B_RRCPV1_HELLOFWD_TAG_MASK, format);    
}
/* Function Name:
 *      rtl8367b_getAsicRrcpHelloTag
 * Description:
 *      Get RRCPv1 hello egress tag format
 * Input:
 *      pFormat 	-  0: Keep, 1:ALE 2:Fixed 3:untag
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRrcpHelloTag(rtk_uint32 *pFormat)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_RRCP_CTRL2, RTL8367B_RRCPV1_HELLOFWD_TAG_MASK, pFormat);    
}
/* Function Name:
 *      rtl8367b_setAsicRrcpFwdTag
 * Description:
 *      Set RRCPv3 forwarding egress tag format
 * Input:
 *      format 	-  0: Keep, 1:ALE 2:Fixed 3:untag
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_NOT_ALLOWED  - Actions not allowed by the function
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRrcpFwdTag(rtk_uint32 format)
{
    if(format >= RRCPTAG_END)
        return RT_ERR_NOT_ALLOWED;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_RRCP_CTRL2, RTL8367B_RRCP_FWD_TAG_MASK, format);    
}
/* Function Name:
 *      rtl8367b_getAsicRrcpFwdTag
 * Description:
 *      Get RRCPv3 forwarding egress tag format
 * Input:
 *      pFormat 	-  0: Keep, 1:ALE 2:Fixed 3:untag
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_NOT_ALLOWED  - Actions not allowed by the function
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRrcpFwdTag(rtk_uint32 *pFormat)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_RRCP_CTRL2, RTL8367B_RRCP_FWD_TAG_MASK, pFormat);    
}
/* Function Name:
 *      rtl8367b_setAsicRrcpReplyTag
 * Description:
 *      Set RRCP replying egress tag format
 * Input:
 *      format 	-  0: Fixed, 1:ALE
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRrcpReplyTag(rtk_uint32 format)
{
  	return rtl8367b_setAsicRegBit(RTL8367B_REG_RRCP_CTRL2, RTL8367B_RRCPV1_REPLY_TAG_OFFSET, format);    
}
/* Function Name:
 *      rtl8367b_getAsicRrcpReplyTag
 * Description:
 *      Get RRCP replying egress tag format
 * Input:
 *      pFormat 	-  0: Fixed, 1:ALE
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRrcpReplyTag(rtk_uint32 *pFormat)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_RRCP_CTRL2, RTL8367B_RRCPV1_REPLY_TAG_OFFSET, pFormat);    
}
/* Function Name:
 *      rtl8367b_setAsicRrcpVidPri
 * Description:
 *      Set RRCP fixed tag VID and priority
 * Input:
 *      vid 		- VID
 *      priority 	- Priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_VLAN_VID  		- Invalid VID parameter (0~4095)
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority (0~7)
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRrcpVidPri(rtk_uint32 vid, rtk_uint32 priority)
{
    rtk_int32 regData;
	
    if(vid > RTL8367B_VIDMAX)
        return RT_ERR_VLAN_VID;

	if(priority > RTL8367B_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

	regData = ((vid << RTL8367B_RRCP_TAG_VID_OFFSET) & RTL8367B_RRCP_TAG_VID_MASK) |
				((priority << RTL8367B_RRCP_TAG_PRIORITY_OFFSET) & RTL8367B_RRCP_TAG_PRIORITY_MASK);

    return rtl8367b_setAsicReg(RTL8367B_REG_RRCP_CTRL3, regData);    
}
/* Function Name:
 *      rtl8367b_getAsicRrcpVidPri
 * Description:
 *      Get RRCP fixed tag VID and priority
 * Input:
 *      pVid 		- VID
 *      pPriority 	- Priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRrcpVidPri(rtk_uint32* pVid, rtk_uint32* pPriority)
{
    ret_t retVal,regData;
	
	retVal = rtl8367b_getAsicReg(RTL8367B_REG_RRCP_CTRL3, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

	*pVid = (regData & RTL8367B_RRCP_TAG_VID_MASK) >> RTL8367B_RRCP_TAG_VID_OFFSET;
	*pPriority = (regData & RTL8367B_RRCP_TAG_PRIORITY_MASK) >> RTL8367B_RRCP_TAG_PRIORITY_OFFSET;
	
    return RT_ERR_OK;    
}

