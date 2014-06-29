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
 * Feature : VLAN related functions
 *
 */
#include "rtl8367b_asicdrv_vlan.h"

#if defined(CONFIG_RTL8367B_ASICDRV_TEST)
rtl8367b_vlan4kentrysmi Rtl8370sVirtualVlanTable[RTL8367B_VIDMAX + 1];
#endif

#if !defined(DISABLE_VLAN_SHADOW)
rtl8367b_user_vlan4kentry   user_4kvlan[RTL8367B_VIDMAX + 1];
#endif

void _rtl8367b_VlanMCStUser2Smi(rtl8367b_vlanconfiguser *pVlanCg, rtl8367b_vlanconfigsmi *pSmiVlanCfg)
{
	pSmiVlanCfg->mbr 		= pVlanCg->mbr;
	pSmiVlanCfg->fid_msti 	= pVlanCg->fid_msti;
	pSmiVlanCfg->evid 		= pVlanCg->evid;
	pSmiVlanCfg->meteridx 	= pVlanCg->meteridx;
	pSmiVlanCfg->envlanpol 	= pVlanCg->envlanpol;
	pSmiVlanCfg->vbpri 		= pVlanCg->vbpri;
	pSmiVlanCfg->vbpen 		= pVlanCg->vbpen;
}

void _rtl8367b_VlanMCStSmi2User(rtl8367b_vlanconfigsmi *pSmiVlanCfg, rtl8367b_vlanconfiguser *pVlanCg)
{
	pVlanCg->mbr			= pSmiVlanCfg->mbr;
	pVlanCg->fid_msti		= pSmiVlanCfg->fid_msti;
	pVlanCg->evid			= pSmiVlanCfg->evid;
    pVlanCg->meteridx		= pSmiVlanCfg->meteridx;
	pVlanCg->envlanpol		= pSmiVlanCfg->envlanpol;
	pVlanCg->vbpri			= pSmiVlanCfg->vbpri;
	pVlanCg->vbpen			= pSmiVlanCfg->vbpen;
}

void _rtl8367b_Vlan4kStUser2Smi(rtl8367b_user_vlan4kentry *pUserVlan4kEntry, rtl8367b_vlan4kentrysmi *pSmiVlan4kEntry)
{
    pSmiVlan4kEntry->mbr        = pUserVlan4kEntry->mbr;
    pSmiVlan4kEntry->untag      = pUserVlan4kEntry->untag;
 	pSmiVlan4kEntry->fid_msti   = pUserVlan4kEntry->fid_msti;
 	pSmiVlan4kEntry->vbpen      = pUserVlan4kEntry->vbpen;
	pSmiVlan4kEntry->vbpri      = pUserVlan4kEntry->vbpri;
	pSmiVlan4kEntry->envlanpol  = pUserVlan4kEntry->envlanpol;
	pSmiVlan4kEntry->meteridx   = pUserVlan4kEntry->meteridx;
	pSmiVlan4kEntry->ivl_svl	= pUserVlan4kEntry->ivl_svl;

}

void _rtl8367b_Vlan4kStSmi2User(rtl8367b_vlan4kentrysmi *pSmiVlan4kEntry, rtl8367b_user_vlan4kentry *pUserVlan4kEntry)
{
    pUserVlan4kEntry->mbr    	= pSmiVlan4kEntry->mbr;
    pUserVlan4kEntry->untag    	= pSmiVlan4kEntry->untag;
 	pUserVlan4kEntry->fid_msti  = pSmiVlan4kEntry->fid_msti;
 	pUserVlan4kEntry->vbpen     = pSmiVlan4kEntry->vbpen;
	pUserVlan4kEntry->vbpri     = pSmiVlan4kEntry->vbpri;
	pUserVlan4kEntry->envlanpol = pSmiVlan4kEntry->envlanpol;
	pUserVlan4kEntry->meteridx  = pSmiVlan4kEntry->meteridx;
	pUserVlan4kEntry->ivl_svl  	= pSmiVlan4kEntry->ivl_svl;
}
/* Function Name:
 *      rtl8367b_setAsicVlanMemberConfig
 * Description:
 *      Set 32 VLAN member configurations
 * Input:
 *      index 		- VLAN member configuration index (0~31)
 *      pVlanCg - VLAN member configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 *      RT_ERR_INPUT  				- Invalid input parameter
 *      RT_ERR_L2_FID  				- Invalid FID
 *      RT_ERR_PORT_MASK  			- Invalid portmask
 *      RT_ERR_FILTER_METER_ID  	- Invalid meter
 *      RT_ERR_QOS_INT_PRIORITY  	- Invalid priority
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - Invalid VLAN member configuration index
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicVlanMemberConfig(rtk_uint32 index, rtl8367b_vlanconfiguser *pVlanCg)
{
	ret_t  retVal;
	rtk_uint32 regAddr;
	rtk_uint32 regData;
	rtk_uint16 *tableAddr;
    rtk_uint32 page_idx;
    rtl8367b_vlanconfigsmi  smi_vlancfg;

    /* Error Checking  */
	if(index > RTL8367B_CVIDXMAX)
        return RT_ERR_VLAN_ENTRY_NOT_FOUND;

    if(pVlanCg->evid > RTL8367B_EVIDMAX)
        return RT_ERR_INPUT;


    if(pVlanCg->mbr > RTL8367B_PORTMASK)
        return RT_ERR_PORT_MASK;

    if(pVlanCg->fid_msti > RTL8367B_FIDMAX)
        return RT_ERR_L2_FID;

    if(pVlanCg->meteridx > RTL8367B_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    if(pVlanCg->vbpri > RTL8367B_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    memset(&smi_vlancfg, 0x00, sizeof(rtl8367b_vlanconfigsmi));
    _rtl8367b_VlanMCStUser2Smi(pVlanCg, &smi_vlancfg);
    tableAddr = (rtk_uint16*)&smi_vlancfg;

    for(page_idx = 0; page_idx < 4; page_idx++)  /* 4 pages per VLAN Member Config */
    {
        regAddr = RTL8367B_VLAN_MEMBER_CONFIGURATION_BASE + (index * 4) + page_idx;
    	regData = *tableAddr;

    	retVal = rtl8367b_setAsicReg(regAddr, regData);
    	if(retVal != RT_ERR_OK)
            return retVal;

        tableAddr++;
    }

	return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_getAsicVlanMemberConfig
 * Description:
 *      Get 32 VLAN member configurations
 * Input:
 *      index 		- VLAN member configuration index (0~31)
 *      pVlanCg - VLAN member configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 *      RT_ERR_INPUT  				- Invalid input parameter
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - Invalid VLAN member configuration index
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicVlanMemberConfig(rtk_uint32 index, rtl8367b_vlanconfiguser *pVlanCg)
{
    ret_t  retVal;
    rtk_uint32 page_idx;
    rtk_uint32 regAddr;
    rtk_uint32 regData;
    rtk_uint16 *tableAddr;
    rtl8367b_vlanconfigsmi  smi_vlancfg;

    if(index > RTL8367B_CVIDXMAX)
		return RT_ERR_VLAN_ENTRY_NOT_FOUND;

    memset(&smi_vlancfg, 0x00, sizeof(rtl8367b_vlanconfigsmi));
    tableAddr  = (rtk_uint16*)&smi_vlancfg;

    for(page_idx = 0; page_idx < 4; page_idx++)  /* 4 pages per VLAN Member Config */
    {
        regAddr = RTL8367B_VLAN_MEMBER_CONFIGURATION_BASE + (index * 4) + page_idx;

        retVal = rtl8367b_getAsicReg(regAddr, &regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        *tableAddr = (rtk_uint16)regData;
        tableAddr++;
    }

    _rtl8367b_VlanMCStSmi2User(&smi_vlancfg, pVlanCg);
    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicVlan4kEntry
 * Description:
 *      Set VID mapped entry to 4K VLAN table
 * Input:
 *      pVlan4kEntry - 4K VLAN configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 *      RT_ERR_INPUT  				- Invalid input parameter
 *      RT_ERR_L2_FID  				- Invalid FID
 *      RT_ERR_VLAN_VID 			- Invalid VID parameter (0~4095)
 *      RT_ERR_PORT_MASK  			- Invalid portmask
 *      RT_ERR_FILTER_METER_ID  	- Invalid meter
 *      RT_ERR_QOS_INT_PRIORITY  	- Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicVlan4kEntry(rtl8367b_user_vlan4kentry *pVlan4kEntry )
{
    rtl8367b_vlan4kentrysmi vlan_4k_entry;
	rtk_uint32					page_idx;
	rtk_uint16					*tableAddr;
	ret_t 					retVal;
	rtk_uint32 					regData;

    if(pVlan4kEntry->vid > RTL8367B_VIDMAX)
        return RT_ERR_VLAN_VID;

    if(pVlan4kEntry->mbr > RTL8367B_PORTMASK)
        return RT_ERR_PORT_MASK;

    if(pVlan4kEntry->untag > RTL8367B_PORTMASK)
        return RT_ERR_PORT_MASK;

    if(pVlan4kEntry->fid_msti > RTL8367B_FIDMAX)
        return RT_ERR_L2_FID;

    if(pVlan4kEntry->meteridx > RTL8367B_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    if(pVlan4kEntry->vbpri > RTL8367B_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    memset(&vlan_4k_entry, 0x00, sizeof(rtl8367b_vlan4kentrysmi));
    _rtl8367b_Vlan4kStUser2Smi(pVlan4kEntry, &vlan_4k_entry);

	/* Prepare Data */
	tableAddr = (rtk_uint16 *)&vlan_4k_entry;
	for(page_idx = 0; page_idx < (sizeof(rtl8367b_vlan4kentrysmi) / 2); page_idx++)
	{
		regData = *tableAddr;
		retVal = rtl8367b_setAsicReg(RTL8367B_TABLE_ACCESS_WRDATA_BASE + page_idx, regData);
		if(retVal != RT_ERR_OK)
			return retVal;

		tableAddr++;
	}

	/* Write Address (VLAN_ID) */
	regData = pVlan4kEntry->vid;
	retVal = rtl8367b_setAsicReg(RTL8367B_TABLE_ACCESS_ADDR_REG, regData);
	if(retVal != RT_ERR_OK)
		return retVal;

	/* Write Command */
	retVal = rtl8367b_setAsicRegBits(RTL8367B_TABLE_ACCESS_CTRL_REG, RTL8367B_TABLE_TYPE_MASK | RTL8367B_COMMAND_TYPE_MASK,RTL8367B_TABLE_ACCESS_REG_DATA(TB_OP_WRITE,TB_TARGET_CVLAN));
	if(retVal != RT_ERR_OK)
		return retVal;

#if defined(CONFIG_RTL8367B_ASICDRV_TEST)
    memcpy(&Rtl8370sVirtualVlanTable[pVlan4kEntry->vid], &vlan_4k_entry, sizeof(rtl8367b_vlan4kentrysmi));
#endif

#if !defined(DISABLE_VLAN_SHADOW)
    memcpy(&user_4kvlan[pVlan4kEntry->vid], pVlan4kEntry, sizeof(rtl8367b_user_vlan4kentry));
#endif

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_getAsicVlan4kEntry
 * Description:
 *      Get VID mapped entry to 4K VLAN table
 * Input:
 *      pVlan4kEntry - 4K VLAN configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_VLAN_VID 		- Invalid VID parameter (0~4095)
 *      RT_ERR_BUSYWAIT_TIMEOUT - LUT is busy at retrieving
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicVlan4kEntry(rtl8367b_user_vlan4kentry *pVlan4kEntry )
{
#if defined(DISABLE_VLAN_SHADOW)
	rtl8367b_vlan4kentrysmi vlan_4k_entry;
	rtk_uint32					page_idx;
	rtk_uint16					*tableAddr;
	ret_t 					    retVal;
	rtk_uint32 					regData;

    if(pVlan4kEntry->vid > RTL8367B_VIDMAX)
        return RT_ERR_VLAN_VID;

	/* Write Address (VLAN_ID) */
	regData = pVlan4kEntry->vid;
	retVal = rtl8367b_setAsicReg(RTL8367B_TABLE_ACCESS_ADDR_REG, regData);
	if(retVal != RT_ERR_OK)
		return retVal;

	/* Read Command */
	retVal = rtl8367b_setAsicRegBits(RTL8367B_TABLE_ACCESS_CTRL_REG, RTL8367B_TABLE_TYPE_MASK | RTL8367B_COMMAND_TYPE_MASK, RTL8367B_TABLE_ACCESS_REG_DATA(TB_OP_READ,TB_TARGET_CVLAN));
	if(retVal != RT_ERR_OK)
		return retVal;

	/* Check ASIC Command */
	retVal = rtl8367b_getAsicRegBit(RTL8367B_TABLE_ACCESS_STATUS_REG, RTL8367B_TABLE_LUT_ADDR_BUSY_FLAG_OFFSET,&regData);
	if(retVal != RT_ERR_OK)
        return RT_ERR_BUSYWAIT_TIMEOUT;

	/* Read VLAN data from register */
	tableAddr = (rtk_uint16 *)&vlan_4k_entry;
	for(page_idx = 0; page_idx < (sizeof(rtl8367b_vlan4kentrysmi) / 2); page_idx++)
	{
		retVal = rtl8367b_getAsicReg(RTL8367B_TABLE_ACCESS_RDDATA_BASE + page_idx, &regData);
		if(retVal != RT_ERR_OK)
			return retVal;

		*tableAddr = regData;
		tableAddr++;
	}

	_rtl8367b_Vlan4kStSmi2User(&vlan_4k_entry, pVlan4kEntry);

#else

    rtk_uint16  vid;

    if(pVlan4kEntry->vid > RTL8367B_VIDMAX)
        return RT_ERR_VLAN_VID;

    vid = pVlan4kEntry->vid;
    memcpy(pVlan4kEntry, &user_4kvlan[pVlan4kEntry->vid], sizeof(rtl8367b_user_vlan4kentry));
    pVlan4kEntry->vid = vid;

#endif

#if defined(CONFIG_RTL8367B_ASICDRV_TEST)
    _rtl8367b_Vlan4kStSmi2User(&Rtl8370sVirtualVlanTable[pVlan4kEntry->vid], pVlan4kEntry);
#endif

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicVlanAccpetFrameType
 * Description:
 *      Set per-port acceptable frame type
 * Input:
 *      port 		- Physical port number (0~7)
 *      frameType 	- The acceptable frame type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 						- Success
 *      RT_ERR_SMI  					- SMI access error
 *      RT_ERR_PORT_ID  				- Invalid port number
 *      RT_ERR_VLAN_ACCEPT_FRAME_TYPE  	- Invalid frame type
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicVlanAccpetFrameType(rtk_uint32 port, rtl8367b_accframetype frameType)
{
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(frameType >= FRAME_TYPE_MAX_BOUND)
        return RT_ERR_VLAN_ACCEPT_FRAME_TYPE;

    return rtl8367b_setAsicRegBits(RTL8367B_VLAN_ACCEPT_FRAME_TYPE_REG(port), RTL8367B_VLAN_ACCEPT_FRAME_TYPE_MASK(port), frameType);
}
/* Function Name:
 *      rtl8367b_getAsicVlanAccpetFrameType
 * Description:
 *      Get per-port acceptable frame type
 * Input:
 *      port 		- Physical port number (0~7)
 *      pFrameType 	- The acceptable frame type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 						- Success
 *      RT_ERR_SMI  					- SMI access error
 *      RT_ERR_PORT_ID  				- Invalid port number
 *      RT_ERR_VLAN_ACCEPT_FRAME_TYPE  	- Invalid frame type
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicVlanAccpetFrameType(rtk_uint32 port, rtl8367b_accframetype *pFrameType)
{
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBits(RTL8367B_VLAN_ACCEPT_FRAME_TYPE_REG(port), RTL8367B_VLAN_ACCEPT_FRAME_TYPE_MASK(port), pFrameType);
}
/* Function Name:
 *      rtl8367b_setAsicVlanIngressFilter
 * Description:
 *      Set VLAN Ingress Filter
 * Input:
 *      port 		- Physical port number (0~7)
 *      enabled 	- Enable or disable Ingress filter
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicVlanIngressFilter(rtk_uint32 port, rtk_uint32 enabled)
{
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367b_setAsicRegBit(RTL8367BG_VLAN_INGRESS_REG, port, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicVlanIngressFilter
 * Description:
 *      Get VLAN Ingress Filter
 * Input:
 *      port 		- Physical port number (0~7)
 *      pEnable 	- Enable or disable Ingress filter
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicVlanIngressFilter(rtk_uint32 port, rtk_uint32 *pEnable)
{
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBit(RTL8367BG_VLAN_INGRESS_REG, port, pEnable);
}
/* Function Name:
 *      rtl8367b_setAsicVlanEgressTagMode
 * Description:
 *      Set CVLAN egress tag mode
 * Input:
 *      port 		- Physical port number (0~7)
 *      tagMode 	- The egress tag mode. Including Original mode, Keep tag mode and Priority tag mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_INPUT  	- Invalid input parameter
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicVlanEgressTagMode(rtk_uint32 port, rtl8367b_egtagmode tagMode)
{
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(tagMode >= EG_TAG_MODE_END)
        return RT_ERR_INPUT;

    return rtl8367b_setAsicRegBits(RTL8367B_PORT_MISC_CFG_REG(port), RTL8367B_VLAN_EGRESS_MDOE_MASK, tagMode);
}
/* Function Name:
 *      rtl8367b_getAsicVlanEgressTagMode
 * Description:
 *      Get CVLAN egress tag mode
 * Input:
 *      port 		- Physical port number (0~7)
 *      pTagMode 	- The egress tag mode. Including Original mode, Keep tag mode and Priority tag mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicVlanEgressTagMode(rtk_uint32 port, rtl8367b_egtagmode *pTagMode)
{
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBits(RTL8367B_PORT_MISC_CFG_REG(port), RTL8367B_VLAN_EGRESS_MDOE_MASK, pTagMode);
}
/* Function Name:
 *      rtl8367b_setAsicVlanPortBasedVID
 * Description:
 *      Set port based VID which is indexed to 32 VLAN member configurations
 * Input:
 *      port 	- Physical port number (0~7)
 *      index 	- Index to VLAN member configuration
 *      pri 	- 1Q Port based VLAN priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 *      RT_ERR_PORT_ID  			- Invalid port number
 *      RT_ERR_QOS_INT_PRIORITY  	- Invalid priority
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - Invalid VLAN member configuration index
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicVlanPortBasedVID(rtk_uint32 port, rtk_uint32 index, rtk_uint32 pri)
{
    rtk_uint32 regAddr, bit_mask;
    ret_t  retVal;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(index > RTL8367B_CVIDXMAX)
        return RT_ERR_VLAN_ENTRY_NOT_FOUND;

    if(pri > RTL8367B_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    regAddr = RTL8367B_VLAN_PVID_CTRL_REG(port);
    bit_mask = RTL8367B_PORT_VIDX_MASK(port);
    retVal = rtl8367b_setAsicRegBits(regAddr, bit_mask, index);
    if(retVal != RT_ERR_OK)
        return retVal;

    regAddr = RTL8367B_VLAN_PORTBASED_PRIORITY_REG(port);
    bit_mask = RTL8367B_VLAN_PORTBASED_PRIORITY_MASK(port);
    retVal = rtl8367b_setAsicRegBits(regAddr, bit_mask, pri);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_getAsicVlanPortBasedVID
 * Description:
 *      Get port based VID which is indexed to 32 VLAN member configurations
 * Input:
 *      port 	- Physical port number (0~7)
 *      pIndex 	- Index to VLAN member configuration
 *      pPri 	- 1Q Port based VLAN priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicVlanPortBasedVID(rtk_uint32 port, rtk_uint32 *pIndex, rtk_uint32 *pPri)
{
    rtk_uint32 regAddr,bit_mask;
    ret_t  retVal;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    regAddr = RTL8367B_VLAN_PVID_CTRL_REG(port);
    bit_mask = RTL8367B_PORT_VIDX_MASK(port);
    retVal = rtl8367b_getAsicRegBits(regAddr, bit_mask, pIndex);
    if(retVal != RT_ERR_OK)
        return retVal;

    regAddr = RTL8367B_VLAN_PORTBASED_PRIORITY_REG(port);
    bit_mask = RTL8367B_VLAN_PORTBASED_PRIORITY_MASK(port);
    retVal = rtl8367b_getAsicRegBits(regAddr, bit_mask, pPri);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicVlanProtocolBasedGroupData
 * Description:
 *      Set protocol and port based group database
 * Input:
 *      index 		- Index to VLAN member configuration
 *      pPbCfg 	- Protocol and port based group database entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 *      RT_ERR_INPUT  				- Invalid input parameter
 *      RT_ERR_VLAN_PROTO_AND_PORT  - Invalid protocol base group database index
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicVlanProtocolBasedGroupData(rtk_uint32 index, rtl8367b_protocolgdatacfg *pPbCfg)
{
	rtk_uint32  frameType;
    rtk_uint32  etherType;
	ret_t	retVal;

	/* Error Checking */
	if(index > RTL8367B_PROTOVLAN_GIDX_MAX)
		return RT_ERR_VLAN_PROTO_AND_PORT;

    if(pPbCfg->frameType >= PPVLAN_FRAME_TYPE_END )
		return RT_ERR_INPUT;

	frameType = pPbCfg->frameType;
	etherType = pPbCfg->etherType;

	/* Frame type */
    retVal = rtl8367b_setAsicRegBits(RTL8367BG_VLAN_PPB_FRAMETYPE_REG(index), RTL8367BG_VLAN_PPB_FRAMETYPE_MASK, frameType);
    if(retVal != RT_ERR_OK)
        return retVal;

	/* Ether type */
	retVal = rtl8367b_setAsicReg(RTL8367B_VLAN_PPB_ETHERTYPR_REG(index), etherType);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_getAsicVlanProtocolBasedGroupData
 * Description:
 *      Get protocol and port based group database
 * Input:
 *      index 		- Index to VLAN member configuration
 *      pPbCfg 	- Protocol and port based group database entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 *      RT_ERR_INPUT  				- Invalid input parameter
 *      RT_ERR_VLAN_PROTO_AND_PORT  - Invalid protocol base group database index
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicVlanProtocolBasedGroupData(rtk_uint32 index, rtl8367b_protocolgdatacfg *pPbCfg)
{
	rtk_uint32  frameType;
    rtk_uint32  etherType;
	ret_t	retVal;

	/* Error Checking */
	if(index > RTL8367B_PROTOVLAN_GIDX_MAX)
		return RT_ERR_VLAN_PROTO_AND_PORT;

	/* Read Frame type */
    retVal = rtl8367b_getAsicRegBits(RTL8367BG_VLAN_PPB_FRAMETYPE_REG(index), RTL8367BG_VLAN_PPB_FRAMETYPE_MASK, &frameType);
    if(retVal != RT_ERR_OK)
        return retVal;

	/* Read Ether type */
	retVal = rtl8367b_getAsicReg(RTL8367B_VLAN_PPB_ETHERTYPR_REG(index), &etherType);
    if(retVal != RT_ERR_OK)
        return retVal;


	pPbCfg->frameType = frameType;
	pPbCfg->etherType = etherType;
    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicVlanPortAndProtocolBased
 * Description:
 *      Set protocol and port based VLAN configuration
 * Input:
 *      port 		- Physical port number (0~7)
 *      index 		- Index of protocol and port based database index
 *      pPpbCfg 	- Protocol and port based VLAN configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 *      RT_ERR_INPUT  				- Invalid input parameter
 *      RT_ERR_PORT_ID  			- Invalid port number
 *      RT_ERR_QOS_INT_PRIORITY  	- Invalid priority
 *      RT_ERR_VLAN_PROTO_AND_PORT  - Invalid protocol base group database index
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - Invalid VLAN member configuration index
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicVlanPortAndProtocolBased(rtk_uint32 port, rtk_uint32 index, rtl8367b_protocolvlancfg *pPpbCfg)
{
	rtk_uint32  reg_addr, bit_mask, bit_value;
	ret_t   retVal;

	/* Error Checking */
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

	if(index > RTL8367B_PROTOVLAN_GIDX_MAX)
		return RT_ERR_VLAN_PROTO_AND_PORT;

	if( (pPpbCfg->valid != FALSE) && (pPpbCfg->valid != TRUE) )
        return RT_ERR_INPUT;

	if(pPpbCfg->vlan_idx > RTL8367B_CVIDXMAX)
        return RT_ERR_VLAN_ENTRY_NOT_FOUND;

	if(pPpbCfg->priority > RTL8367B_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

	/* Valid bit */
	reg_addr  = RTL8367B_VLAN_PPB_VALID_REG(index);
	bit_mask  = 0x0001 << port;
	bit_value = ((TRUE == pPpbCfg->valid) ? 0x1 : 0x0);
	retVal    = rtl8367b_setAsicRegBits(reg_addr, bit_mask, bit_value);
	if(retVal != RT_ERR_OK)
		return retVal;

	/* Calculate the actual register address for CVLAN index*/
	reg_addr = RTL8367B_VLAN_PPB_CTRL_REG(index, port);
	bit_mask = RTL8367B_VLAN_PPB_CTRL_MASK(port);
	bit_value = pPpbCfg->vlan_idx;

   	retVal	= rtl8367b_setAsicRegBits(reg_addr, bit_mask, bit_value);
	if(retVal != RT_ERR_OK)
		return retVal;

	/* write priority */
	reg_addr  = RTL8367B_VLAN_PPB_PRIORITY_ITEM_REG(port, index);
	bit_mask  = RTL8367B_VLAN_PPB_PRIORITY_ITEM_MASK(port);
	bit_value = pPpbCfg->priority;
	retVal    = rtl8367b_setAsicRegBits(reg_addr, bit_mask, bit_value);
	if(retVal != RT_ERR_OK)
		return retVal;

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_getAsicVlanPortAndProtocolBased
 * Description:
 *      Get protocol and port based VLAN configuration
 * Input:
 *      port 		- Physical port number (0~7)
 *      index 		- Index of protocol and port based database index
 *      pPpbCfg 	- Protocol and port based VLAN configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 *      RT_ERR_INPUT  				- Invalid input parameter
 *      RT_ERR_PORT_ID  			- Invalid port number
 *      RT_ERR_VLAN_PROTO_AND_PORT  - Invalid protocol base group database index
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicVlanPortAndProtocolBased(rtk_uint32 port, rtk_uint32 index, rtl8367b_protocolvlancfg *pPpbCfg)
{
	rtk_uint32  reg_addr, bit_mask, bit_value;
	ret_t   retVal;

	/* Error Checking */
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

	if(index > RTL8367B_PROTOVLAN_GIDX_MAX)
		return RT_ERR_VLAN_PROTO_AND_PORT;

	if(pPpbCfg == NULL)
		return RT_ERR_INPUT;

	/* Valid bit */
	reg_addr  = RTL8367B_VLAN_PPB_VALID_REG(index);
	bit_mask  = 0x0001 << port;
	retVal    = rtl8367b_getAsicRegBits(reg_addr, bit_mask, &bit_value);
	if(retVal != RT_ERR_OK)
		return retVal;

	pPpbCfg->valid = bit_value;

	/* CVLAN index */
	reg_addr = RTL8367B_VLAN_PPB_CTRL_REG(index,port);
	bit_mask = RTL8367B_VLAN_PPB_CTRL_MASK(port);
	retVal = rtl8367b_getAsicRegBits(reg_addr, bit_mask, &bit_value);
	if(retVal != RT_ERR_OK)
		return retVal;

	pPpbCfg->vlan_idx = bit_value;


	/* priority */
	reg_addr = RTL8367B_VLAN_PPB_PRIORITY_ITEM_REG(port,index);
	bit_mask = RTL8367B_VLAN_PPB_PRIORITY_ITEM_MASK(port);
	retVal = rtl8367b_getAsicRegBits(reg_addr, bit_mask, &bit_value);
	if(retVal != RT_ERR_OK)
		return retVal;

	pPpbCfg->priority = bit_value;
    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicVlanFilter
 * Description:
 *      Set enable CVLAN filtering function
 * Input:
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicVlanFilter(rtk_uint32 enabled)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_VLAN_CTRL, RTL8367B_VLAN_CTRL_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicVlanFilter
 * Description:
 *      Get enable CVLAN filtering function
 * Input:
 *      pEnabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicVlanFilter(rtk_uint32* pEnabled)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_VLAN_CTRL, RTL8367B_VLAN_CTRL_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicVlanUntagDscpPriorityEn
 * Description:
 *      Set enable Dscp to untag 1Q priority
 * Input:
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicVlanUntagDscpPriorityEn(rtk_uint32 enabled)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_UNTAG_DSCP_PRI_CFG, RTL8367B_UNTAG_DSCP_PRI_CFG_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicVlanUntagDscpPriorityEn
 * Description:
 *      Get enable Dscp to untag 1Q priority
 * Input:
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicVlanUntagDscpPriorityEn(rtk_uint32* enabled)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_UNTAG_DSCP_PRI_CFG, RTL8367B_UNTAG_DSCP_PRI_CFG_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_setAsicPortBasedFid
 * Description:
 *      Set port based FID
 * Input:
 *      port 	- Physical port number (0~7)
 *      fid 	- Port based fid
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_L2_FID  	- Invalid FID
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPortBasedFid(rtk_uint32 port, rtk_uint32 fid)
{
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(fid > RTL8367B_FIDMAX)
        return RT_ERR_L2_FID;

    return rtl8367b_setAsicReg(RTL8367B_PORT_PBFID_REG(port),fid);
}
/* Function Name:
 *      rtl8367b_getAsicPortBasedFid
 * Description:
 *      Get port based FID
 * Input:
 *      port 	- Physical port number (0~7)
 *      pFid 	- Port based fid
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortBasedFid(rtk_uint32 port, rtk_uint32* pFid)
{
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367b_getAsicReg(RTL8367B_PORT_PBFID_REG(port), pFid);
}
/* Function Name:
 *      rtl8367b_setAsicPortBasedFidEn
 * Description:
 *      Set port based FID selection enable
 * Input:
 *      port 	- Physical port number (0~7)
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPortBasedFidEn(rtk_uint32 port, rtk_uint32 enabled)
{
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367b_setAsicRegBit(RTL8367B_REG_PORT_PBFIDEN,port, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicPortBasedFidEn
 * Description:
 *      Get port based FID selection enable
 * Input:
 *      port 	- Physical port number (0~7)
 *      pEnabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortBasedFidEn(rtk_uint32 port, rtk_uint32* pEnabled)
{
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBit(RTL8367B_REG_PORT_PBFIDEN,port, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicSpanningTreeStatus
 * Description:
 *      Set spanning tree state per each port
 * Input:
 *      port 	- Physical port number (0~7)
 *      msti 	- Multiple spanning tree instance
 *      state 	- Spanning tree state for msti
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_MSTI  		- Invalid msti parameter
 *      RT_ERR_PORT_ID  	- Invalid port number
 *      RT_ERR_MSTP_STATE  	- Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicSpanningTreeStatus(rtk_uint32 port, rtk_uint32 msti, rtk_uint32 state)
{
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(msti > RTL8367B_MSTIMAX)
        return RT_ERR_MSTI;

	if(state > STPST_FORWARDING)
		return RT_ERR_MSTP_STATE;

	return rtl8367b_setAsicRegBits(RTL8367B_VLAN_MSTI_REG(msti,port), RTL8367B_VLAN_MSTI_MASK(port),state);
}
/* Function Name:
 *      rtl8367b_getAsicSpanningTreeStatus
 * Description:
 *      Set spanning tree state per each port
 * Input:
 *      port 	- Physical port number (0~7)
 *      msti 	- Multiple spanning tree instance
 *      pState 	- Spanning tree state for msti
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_MSTI  		- Invalid msti parameter
 *      RT_ERR_PORT_ID  	- Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicSpanningTreeStatus(rtk_uint32 port, rtk_uint32 msti, rtk_uint32* pState)
{
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(msti > RTL8367B_MSTIMAX)
        return RT_ERR_MSTI;

	return rtl8367b_getAsicRegBits(RTL8367B_VLAN_MSTI_REG(msti,port), RTL8367B_VLAN_MSTI_MASK(port), pState);
}
/* Function Name:
 *      rtl8367b_setAsicVlanTransparent
 * Description:
 *      Set enable CVLAN transparent
 * Input:
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicVlanTransparent(rtk_uint32 enabled)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_VLAN_TRANSPARENT_EN_CFG, RTL8367B_VLAN_TRANSPARENT_EN_CFG_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicVlanTransparent
 * Description:
 *      Get CVLAN transparent state
 * Input:
 *      pEnabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicVlanTransparent(rtk_uint32* pEnabled)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_VLAN_TRANSPARENT_EN_CFG, RTL8367B_VLAN_TRANSPARENT_EN_CFG_OFFSET, pEnabled);
}

/* Function Name:
 *      rtl8367b_setAsicVlanEgressKeep
 * Description:
 *      Set per egress port VLAN keep mode
 * Input:
 *      port 		- Physical port number (0~7)
 *      portmask 	- portmask(0~0xFF)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_MASK  	- Invalid portmask
 *      RT_ERR_PORT_ID  	- Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicVlanEgressKeep(rtk_uint32 port, rtk_uint32 portmask)
{
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(portmask > RTL8367B_PORTMASK)
        return RT_ERR_PORT_MASK;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_VLAN_EGRESS_KEEP_CTRL0 + (port>>1),RTL8367B_PORT0_VLAN_KEEP_MASK_MASK<<((port&1)*8),portmask);
}
/* Function Name:
 *      rtl8367b_getAsicVlanEgressKeep
 * Description:
 *      Get per egress port VLAN keep mode
 * Input:
 *      port 		- Physical port number (0~7)
 *      pPortmask 	- portmask(0~0xFF)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicVlanEgressKeep(rtk_uint32 port, rtk_uint32* pPortmask)
{
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBits(RTL8367B_REG_VLAN_EGRESS_KEEP_CTRL0 + (port>>1),RTL8367B_PORT0_VLAN_KEEP_MASK_MASK<<((port&1)*8), pPortmask);
}

