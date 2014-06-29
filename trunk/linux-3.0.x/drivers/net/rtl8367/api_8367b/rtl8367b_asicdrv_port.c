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
 * Feature : Port security related functions
 *
 */

#include "rtl8367b_asicdrv_port.h"

/* Function Name:
 *      rtl8367b_setAsicPortUnknownDaBehavior
 * Description:
 *      Set UNDA behavior
 * Input:
 *      behavior 	- 0: flooding; 1: drop; 2:trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_NOT_ALLOWED  - Invalid behavior
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPortUnknownDaBehavior(rtk_uint32 behavior)
{
    if(behavior >= L2_BEHAVE_END)
		return RT_ERR_NOT_ALLOWED;

	return rtl8367b_setAsicRegBits(RTL8367B_PORT_SECURIT_CTRL_REG, RTL8367B_UNKNOWN_UNICAST_DA_BEHAVE_MASK, behavior);
}
/* Function Name:
 *      rtl8367b_getAsicPortUnknownDaBehavior
 * Description:
 *      Get UNDA behavior
 * Input:
 *      pBehavior 	- 0: flooding; 1: drop; 2:trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortUnknownDaBehavior(rtk_uint32 *pBehavior)
{
	return rtl8367b_getAsicRegBits(RTL8367B_PORT_SECURIT_CTRL_REG, RTL8367B_UNKNOWN_UNICAST_DA_BEHAVE_MASK, pBehavior);
}
/* Function Name:
 *      rtl8367b_setAsicPortUnknownSaBehavior
 * Description:
 *      Set UNSA behavior
 * Input:
 *      behavior 	- 0: flooding; 1: drop; 2:trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_NOT_ALLOWED  - Invalid behavior
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPortUnknownSaBehavior(rtk_uint32 behavior)
{
    if(behavior >= L2_BEHAVE_SA_END)
		return RT_ERR_NOT_ALLOWED;

	return rtl8367b_setAsicRegBits(RTL8367B_PORT_SECURIT_CTRL_REG, RTL8367B_UNKNOWN_SA_BEHAVE_MASK, behavior);
}
/* Function Name:
 *      rtl8367b_getAsicPortUnknownSaBehavior
 * Description:
 *      Get UNSA behavior
 * Input:
 *      pBehavior 	- 0: flooding; 1: drop; 2:trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortUnknownSaBehavior(rtk_uint32 *pBehavior)
{
	return rtl8367b_getAsicRegBits(RTL8367B_PORT_SECURIT_CTRL_REG, RTL8367B_UNKNOWN_SA_BEHAVE_MASK, pBehavior);
}
/* Function Name:
 *      rtl8367b_setAsicPortUnmatchedSaBehavior
 * Description:
 *      Set Unmatched SA behavior
 * Input:
 *      behavior 	- 0: flooding; 1: drop; 2:trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_NOT_ALLOWED  - Invalid behavior
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPortUnmatchedSaBehavior(rtk_uint32 behavior)
{
    if(behavior >= L2_BEHAVE_SA_END)
		return RT_ERR_NOT_ALLOWED;

	return rtl8367b_setAsicRegBits(RTL8367B_PORT_SECURIT_CTRL_REG, RTL8367B_UNMATCHED_SA_BEHAVE_MASK, behavior);
}
/* Function Name:
 *      rtl8367b_getAsicPortUnmatchedSaBehavior
 * Description:
 *      Get Unmatched SA behavior
 * Input:
 *      pBehavior 	- 0: flooding; 1: drop; 2:trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortUnmatchedSaBehavior(rtk_uint32 *pBehavior)
{
	return rtl8367b_getAsicRegBits(RTL8367B_PORT_SECURIT_CTRL_REG, RTL8367B_UNMATCHED_SA_BEHAVE_MASK, pBehavior);
}
/* Function Name:
 *      rtl8367b_setAsicPortUnknownDaFloodingPortmask
 * Description:
 *      Set UNDA flooding portmask
 * Input:
 *      portmask 	- portmask(0~0xFF)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_MASK 	- Invalid portmask
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPortUnknownDaFloodingPortmask(rtk_uint32 portmask)
{
	if(portmask > RTL8367B_PORTMASK)
		return RT_ERR_PORT_MASK;

	return rtl8367b_setAsicReg(RTL8367B_UNUCAST_FLOADING_PMSK_REG, portmask);
}
/* Function Name:
 *      rtl8367b_getAsicPortUnknownDaFloodingPortmask
 * Description:
 *      Get UNDA flooding portmask
 * Input:
 *      pPortmask 	- portmask(0~0xFF)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortUnknownDaFloodingPortmask(rtk_uint32 *pPortmask)
{
	return rtl8367b_getAsicReg(RTL8367B_UNUCAST_FLOADING_PMSK_REG, pPortmask);
}
/* Function Name:
 *      rtl8367b_setAsicPortUnknownMulticastFloodingPortmask
 * Description:
 *      Set UNMC flooding portmask
 * Input:
 *      portmask 	- portmask(0~0xFF)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_MASK 	- Invalid portmask
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPortUnknownMulticastFloodingPortmask(rtk_uint32 portmask)
{
	if(portmask > RTL8367B_PORTMASK)
		return RT_ERR_PORT_MASK;

	return rtl8367b_setAsicReg(RTL8367B_UNMCAST_FLOADING_PMSK_REG, portmask);
}
/* Function Name:
 *      rtl8367b_getAsicPortUnknownMulticastFloodingPortmask
 * Description:
 *      Get UNMC flooding portmask
 * Input:
 *      pPortmask 	- portmask(0~0xFF)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortUnknownMulticastFloodingPortmask(rtk_uint32 *pPortmask)
{
	return rtl8367b_getAsicReg(RTL8367B_UNMCAST_FLOADING_PMSK_REG, pPortmask);
}
/* Function Name:
 *      rtl8367b_setAsicPortBcastFloodingPortmask
 * Description:
 *      Set Bcast flooding portmask
 * Input:
 *      portmask 	- portmask(0~0xFF)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_MASK 	- Invalid portmask
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPortBcastFloodingPortmask(rtk_uint32 portmask)
{
	if(portmask > RTL8367B_PORTMASK)
		return RT_ERR_PORT_MASK;

	return rtl8367b_setAsicReg(RTL8367B_BCAST_FLOADING_PMSK_REG, portmask);
}
/* Function Name:
 *      rtl8367b_getAsicPortBcastFloodingPortmask
 * Description:
 *      Get Bcast flooding portmask
 * Input:
 *      pPortmask 	- portmask(0~0xFF)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortBcastFloodingPortmask(rtk_uint32 *pPortmask)
{
	return rtl8367b_getAsicReg(RTL8367B_BCAST_FLOADING_PMSK_REG, pPortmask);
}
/* Function Name:
 *      rtl8367b_setAsicPortBlockSpa
 * Description:
 *      Set disabling blocking frame if source port and destination port are the same
 * Input:
 *      port 	- Physical port number (0~7)
 *      permit 	- 0: block; 1: permit
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPortBlockSpa(rtk_uint32 port, rtk_uint32 permit)
{
	if(port >= RTL8367B_PORTNO)
		return RT_ERR_PORT_ID;

	return rtl8367b_setAsicRegBit(RTL8367B_SOURCE_PORT_BLOCK_REG, port, permit);
}
/* Function Name:
 *      rtl8367b_getAsicPortBlockSpa
 * Description:
 *      Get disabling blocking frame if source port and destination port are the same
 * Input:
 *      port 	- Physical port number (0~7)
 *      pPermit 	- 0: block; 1: permit
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortBlockSpa(rtk_uint32 port, rtk_uint32* pPermit)
{
	if(port >= RTL8367B_PORTNO)
		return RT_ERR_PORT_ID;

	return rtl8367b_getAsicRegBit(RTL8367B_SOURCE_PORT_BLOCK_REG, port, pPermit);
}
/* Function Name:
 *      rtl8367b_setAsicPortDos
 * Description:
 *      Set DOS function
 * Input:
 *      type 	- DOS type
 *      drop 	- 0: permit; 1: drop
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - Invalid payload index
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPortDos(rtk_uint32 type, rtk_uint32 drop)
{
	if(type >= DOS_END)
		return RT_ERR_OUT_OF_RANGE;

	return rtl8367b_setAsicRegBit(RTL8367B_REG_DOS_CFG, RTL8367B_DROP_DAEQSA_OFFSET + type, drop);
}
/* Function Name:
 *      rtl8367b_getAsicPortDos
 * Description:
 *      Get DOS function
 * Input:
 *      type 	- DOS type
 *      pDrop 	- 0: permit; 1: drop
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - Invalid payload index
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortDos(rtk_uint32 type, rtk_uint32* pDrop)
{
	if(type >= DOS_END)
		return RT_ERR_OUT_OF_RANGE;

	return rtl8367b_getAsicRegBit(RTL8367B_REG_DOS_CFG, RTL8367B_DROP_DAEQSA_OFFSET + type,pDrop);
}
/* Function Name:
 *      rtl8367b_setAsicPortForceLink
 * Description:
 *      Set port force linking configuration
 * Input:
 *      port 		- Physical port number (0~7)
 *      pPortAbility - port ability configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPortForceLink(rtk_uint32 port, rtl8367b_port_ability_t *pPortAbility)
{
    rtk_uint32 regData;
    rtk_uint16 *accessPtr;
	rtl8367b_port_ability_t ability;

    /* Invalid input parameter */
    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;

    memset(&ability, 0x00, sizeof(rtl8367b_port_ability_t));
    memcpy(&ability, pPortAbility, sizeof(rtl8367b_port_ability_t));

    accessPtr =  (rtk_uint16*)&ability;

    regData = *accessPtr;
    return rtl8367b_setAsicReg(RTL8367B_REG_MAC0_FORCE_SELECT+port, regData);
}
/* Function Name:
 *      rtl8367b_getAsicPortForceLink
 * Description:
 *      Get port force linking configuration
 * Input:
 *      port 		- Physical port number (0~7)
 *      pPortAbility - port ability configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortForceLink(rtk_uint32 port, rtl8367b_port_ability_t *pPortAbility)
{
    ret_t retVal;
    rtk_uint32 regData;
    rtk_uint16 *accessPtr;
    rtl8367b_port_ability_t ability;

    /* Invalid input parameter */
    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;

    memset(&ability, 0x00, sizeof(rtl8367b_port_ability_t));


    accessPtr = (rtk_uint16*)&ability;

    retVal = rtl8367b_getAsicReg(RTL8367B_REG_MAC0_FORCE_SELECT + port, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    *accessPtr = regData;

    memcpy(pPortAbility, &ability, sizeof(rtl8367b_port_ability_t));

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_getAsicPortStatus
 * Description:
 *      Get port link status
 * Input:
 *      port 		- Physical port number (0~7)
 *      pPortAbility - port ability configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortStatus(rtk_uint32 port, rtl8367b_port_status_t *pPortStatus)
{
    ret_t retVal;
    rtk_uint32 regData;
    rtk_uint16 *accessPtr;
    rtl8367b_port_status_t status;

    /* Invalid input parameter */
    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;

    memset(&status, 0x00, sizeof(rtl8367b_port_status_t));


    accessPtr =  (rtk_uint16*)&status;

    retVal = rtl8367b_getAsicReg(RTL8367B_REG_PORT0_STATUS+port,&regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    *accessPtr = regData;

    memcpy(pPortStatus, &status, sizeof(rtl8367b_port_status_t));

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicPortForceLinkExt
 * Description:
 *      Set external interface force linking configuration
 * Input:
 *      id 			- external interface id (0~2)
 *      portAbility - port ability configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPortForceLinkExt(rtk_uint32 id, rtl8367b_port_ability_t *pPortAbility)
{
    rtk_uint32  reg_data;

    /* Invalid input parameter */
    if(id >= RTL8367B_EXTNO)
        return RT_ERR_OUT_OF_RANGE;

    reg_data = (rtk_uint32)(*(rtk_uint16 *)pPortAbility);

	if(0 == id || 1 == id)
	    return rtl8367b_setAsicReg(RTL8367B_REG_DIGITAL_INTERFACE0_FORCE + id, reg_data);
	else
	    return rtl8367b_setAsicReg(RTL8367B_REG_DIGITAL_INTERFACE2_FORCE, reg_data);
}
/* Function Name:
 *      rtl8367b_getAsicPortForceLinkExt
 * Description:
 *      Get external interface force linking configuration
 * Input:
 *      id 			- external interface id (0~1)
 *      pPortAbility - port ability configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortForceLinkExt(rtk_uint32 id, rtl8367b_port_ability_t *pPortAbility)
{
    rtk_uint32  reg_data;
    rtk_uint16  ability_data;
    ret_t       retVal;

    /* Invalid input parameter */
    if(id >= RTL8367B_EXTNO)
        return RT_ERR_OUT_OF_RANGE;

	if(0 == id || 1 == id)
    	retVal = rtl8367b_getAsicReg(RTL8367B_REG_DIGITAL_INTERFACE0_FORCE+id, &reg_data);
	else
	    retVal = rtl8367b_getAsicReg(RTL8367B_REG_DIGITAL_INTERFACE2_FORCE, &reg_data);

    if(retVal != RT_ERR_OK)
        return retVal;

    ability_data = (rtk_uint16)reg_data;
    memcpy(pPortAbility, &ability_data, sizeof(rtl8367b_port_ability_t));
    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicPortExtMode
 * Description:
 *      Set external interface mode configuration
 * Input:
 *      id 		- external interface id (0~2)
 *      mode 	- external interface mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPortExtMode(rtk_uint32 id, rtk_uint32 mode)
{
    ret_t   retVal;

    if(id >= RTL8367B_EXTNO)
        return RT_ERR_OUT_OF_RANGE;

    if(mode >= EXT_END)
        return RT_ERR_OUT_OF_RANGE;

    if(mode == EXT_GMII)
    {
        if( (retVal = rtl8367b_setAsicRegBit(RTL8367B_REG_EXT0_RGMXF, RTL8367B_EXT0_RGTX_INV_OFFSET, 1)) != RT_ERR_OK)
            return retVal;

        if( (retVal = rtl8367b_setAsicRegBit(RTL8367B_REG_EXT1_RGMXF, RTL8367B_EXT1_RGTX_INV_OFFSET, 1)) != RT_ERR_OK)
            return retVal;

        if( (retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_EXT_TXC_DLY, RTL8367B_EXT1_GMII_TX_DELAY_MASK, 5)) != RT_ERR_OK)
            return retVal;

        if( (retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_EXT_TXC_DLY, RTL8367B_EXT0_GMII_TX_DELAY_MASK, 6)) != RT_ERR_OK)
            return retVal;
    }

    if( (mode == EXT_TMII_MAC) || (mode == EXT_TMII_PHY) )
    {
        if( (retVal = rtl8367b_setAsicRegBit(RTL8367B_REG_BYPASS_LINE_RATE, id, 1)) != RT_ERR_OK)
            return retVal;
    }
    else
    {
        if( (retVal = rtl8367b_setAsicRegBit(RTL8367B_REG_BYPASS_LINE_RATE, id, 0)) != RT_ERR_OK)
            return retVal;
    }

	if(0 == id || 1 == id)
   		return rtl8367b_setAsicRegBits(RTL8367B_REG_DIGITAL_INTERFACE_SELECT, RTL8367B_SELECT_GMII_0_MASK << (id * RTL8367B_SELECT_GMII_1_OFFSET), mode);
	else
   		return rtl8367b_setAsicRegBits(RTL8367B_REG_DIGITAL_INTERFACE_SELECT_1, RTL8367B_SELECT_RGMII_2_MASK, mode);
}
/* Function Name:
 *      rtl8367b_getAsicPortExtMode
 * Description:
 *      Get external interface mode configuration
 * Input:
 *      id 		- external interface id (0~1)
 *      pMode 	- external interface mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortExtMode(rtk_uint32 id, rtk_uint32 *pMode)
{
    if(id >= RTL8367B_EXTNO)
        return RT_ERR_OUT_OF_RANGE;

	if(0 == id || 1 == id)
  		return rtl8367b_getAsicRegBits(RTL8367B_REG_DIGITAL_INTERFACE_SELECT, RTL8367B_SELECT_GMII_0_MASK << (id * RTL8367B_SELECT_GMII_1_OFFSET), pMode);
	else
   		return rtl8367b_getAsicRegBits(RTL8367B_REG_DIGITAL_INTERFACE_SELECT_1, RTL8367B_SELECT_RGMII_2_MASK, pMode);
}

/* Function Name:
 *      rtl8367b_setAsicPortEnableAll
 * Description:
 *      Set ALL ports enable.
 * Input:
 *      enable - enable all ports.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPortEnableAll(rtk_uint32 enable)
{
    if(enable >= 2)
        return RT_ERR_INPUT;

    return rtl8367b_setAsicRegBit(RTL8367B_REG_PHY_AD, RTL8367B_PDNPHY_OFFSET, !enable);
}

/* Function Name:
 *      rtl8367B_getAsicPortEnableAll
 * Description:
 *      Set ALL ports enable.
 * Input:
 *      enable - enable all ports.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortEnableAll(rtk_uint32 *pEnable)
{
    ret_t retVal;
    rtk_uint32 regData;

    retVal = rtl8367b_getAsicRegBit(RTL8367B_REG_PHY_AD, RTL8367B_PDNPHY_OFFSET, &regData);
    if(retVal !=  RT_ERR_OK)
        return retVal;

    if (regData==0)
        *pEnable = 1;
    else
        *pEnable = 0;

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicPortSmallIpg
 * Description:
 *      Set small ipg egress mode
 * Input:
 *      port 	- Physical port number (0~7)
 *      enable 	- 0: normal, 1: small
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPortSmallIpg(rtk_uint32 port, rtk_uint32 enable)
{
	if(port >= RTL8367B_PORTNO)
		return RT_ERR_PORT_ID;

	return rtl8367b_setAsicRegBit(RTL8367B_PORT_SMALL_IPG_REG(port), RTL8367B_PORT0_MISC_CFG_SMALL_TAG_IPG_OFFSET, enable);
}

/* Function Name:
 *      rtl8367b_getAsicPortSmallIpg
 * Description:
 *      Get small ipg egress mode
 * Input:
 *      port 	- Physical port number (0~7)
 *      pEnable 	- 0: normal, 1: small
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortSmallIpg(rtk_uint32 port, rtk_uint32* pEnable)
{
	if(port >= RTL8367B_PORTNO)
		return RT_ERR_PORT_ID;

	return rtl8367b_getAsicRegBit(RTL8367B_PORT_SMALL_IPG_REG(port), RTL8367B_PORT0_MISC_CFG_SMALL_TAG_IPG_OFFSET, pEnable);
}

/* Function Name:
 *      rtl8367b_setAsicPortLoopback
 * Description:
 *      Set MAC loopback
 * Input:
 *      port 	- Physical port number (0~7)
 *      enable 	- 0: Disable, 1: enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPortLoopback(rtk_uint32 port, rtk_uint32 enable)
{
    if(port >= RTL8367B_PORTNO)
		return RT_ERR_PORT_ID;

    return rtl8367b_setAsicRegBit(RTL8367B_PORT_MISC_CFG_REG(port), RTL8367B_PORT0_MISC_CFG_MAC_LOOPBACK_OFFSET, enable);
}

/* Function Name:
 *      rtl8367b_getAsicPortLoopback
 * Description:
 *      Set MAC loopback
 * Input:
 *      port 	- Physical port number (0~7)
 * Output:
 *      pEnable - 0: Disable, 1: enable
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortLoopback(rtk_uint32 port, rtk_uint32 *pEnable)
{
    if(port >= RTL8367B_PORTNO)
		return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBit(RTL8367B_PORT_MISC_CFG_REG(port), RTL8367B_PORT0_MISC_CFG_MAC_LOOPBACK_OFFSET, pEnable);
}

/* Function Name:
 *      rtl8367b_setAsicPortRTCT
 * Description:
 *      Set RTCT
 * Input:
 *      portmask 	- Port mask of RTCT enabled (0-4)
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK 		    - Success
 *      RT_ERR_SMI  	    - SMI access error
 *      RT_ERR_PORT_MASK    - Invalid port mask
 * Note:
 *      RTCT test takes 4.8 seconds at most.
 */
ret_t rtl8367b_setAsicPortRTCT(rtk_uint32 portmask)
{
    ret_t       retVal;

    if(portmask > (0x0001 << RTL8367B_PHYNO))
		return RT_ERR_PORT_MASK;

    if((retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_RTCT_ENABLE, RTL8367B_RTCT_ENABLE_PORT_MASK_MASK, portmask)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367b_setAsicRegBit(RTL8367B_REG_SEL_RTCT_PARA, RTL8367B_EN_RTCT_TIMOUT_OFFSET, 1)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367b_setAsicRegBit(RTL8367B_REG_SEL_RTCT_PARA, RTL8367B_EN_ALL_RTCT_OFFSET, 0)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367b_setAsicRegBit(RTL8367B_REG_SEL_RTCT_PARA, RTL8367B_DO_RTCT_COMMAND_OFFSET, 0)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367b_setAsicRegBit(RTL8367B_REG_SEL_RTCT_PARA, RTL8367B_DO_RTCT_COMMAND_OFFSET, 1)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicPortRTCTResult
 * Description:
 *      Get RTCT result
 * Input:
 *      port 	- Port ID of RTCT result
 * Output:
 *      pResult - The result of port ID
 * Return:
 *      RT_ERR_OK 		            - Success
 *      RT_ERR_SMI  	            - SMI access error
 *      RT_ERR_PORT_MASK            - Invalid port mask
 *      RT_ERR_PHY_RTCT_NOT_FINISH  - RTCT test doesn't finish.
 * Note:
 *      RTCT test takes 4.8 seconds at most.
 *      If this API returns RT_ERR_PHY_RTCT_NOT_FINISH,
 *      users should wait a whole then read it again.
 */
ret_t rtl8367b_getAsicPortRTCTResult(rtk_uint32 port, rtl8367b_port_rtct_result_t *pResult)
{
    ret_t       retVal;
    rtk_uint32  regData, finish = 1;

    if(port >= RTL8367B_PHYNO)
		return RT_ERR_PORT_ID;

    if((retVal = rtl8367b_setAsicPHYReg(port, RTL8367B_PHY_PAGE_ADDRESS, RTL8367B_RTCT_PAGE)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367b_getAsicPHYReg(port, RTL8367B_RTCT_RESULT_A_REG, &regData)) != RT_ERR_OK)
        return retVal;

    if((regData & 0x4000) == 0x4000)
    {
        pResult->channelALen = regData & 0x1FFF;

        if((retVal = rtl8367b_getAsicPHYReg(port, RTL8367B_RTCT_RESULT_B_REG, &regData)) != RT_ERR_OK)
            return retVal;

        pResult->channelBLen = regData & 0x1FFF;

        if((retVal = rtl8367b_getAsicPHYReg(port, RTL8367B_RTCT_RESULT_C_REG, &regData)) != RT_ERR_OK)
            return retVal;

        pResult->channelCLen = regData & 0x1FFF;

        if((retVal = rtl8367b_getAsicPHYReg(port, RTL8367B_RTCT_RESULT_D_REG, &regData)) != RT_ERR_OK)
            return retVal;

        pResult->channelDLen = regData & 0x1FFF;

        if((retVal = rtl8367b_getAsicPHYReg(port, RTL8367B_RTCT_STATUS_REG, &regData)) != RT_ERR_OK)
            return retVal;

        pResult->channelALinedriver = (regData & 0x0001);
        pResult->channelBLinedriver = (regData & 0x0002);
        pResult->channelCLinedriver = (regData & 0x0004);
        pResult->channelDLinedriver = (regData & 0x0008);

        pResult->channelAMismatch   = (regData & 0x0010);
        pResult->channelBMismatch   = (regData & 0x0020);
        pResult->channelCMismatch   = (regData & 0x0040);
        pResult->channelDMismatch   = (regData & 0x0080);

        pResult->channelAOpen       = (regData & 0x0100);
        pResult->channelBOpen       = (regData & 0x0200);
        pResult->channelCOpen       = (regData & 0x0400);
        pResult->channelDOpen       = (regData & 0x0800);

        pResult->channelAShort      = (regData & 0x1000);
        pResult->channelBShort      = (regData & 0x2000);
        pResult->channelCShort      = (regData & 0x4000);
        pResult->channelDShort      = (regData & 0x8000);
    }
    else
        finish = 0;

    if((retVal = rtl8367b_setAsicPHYReg(port, RTL8367B_PHY_PAGE_ADDRESS, 0)) != RT_ERR_OK)
        return retVal;

    if(finish == 0)
        return RT_ERR_PHY_RTCT_NOT_FINISH;
    else
        return RT_ERR_OK;
}
