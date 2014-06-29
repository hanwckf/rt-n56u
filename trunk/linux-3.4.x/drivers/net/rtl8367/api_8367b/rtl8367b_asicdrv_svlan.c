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
 * Feature : SVLAN related functions
 *
 */
#include "rtl8367b_asicdrv_svlan.h"

void _rtl8367b_svlanConfStUser2Smi(rtl8367b_svlan_memconf_t *pUserSt, rtl8367b_svlan_memconf_smi_t *pSmiSt);
void _rtl8367b_svlanConfStSmi2User(rtl8367b_svlan_memconf_t *pUserSt, rtl8367b_svlan_memconf_smi_t *pSmiSt);
void _rtl8367b_svlanMc2sStUser2Smi(rtl8367b_svlan_mc2s_t *pUserSt, rtl8367b_svlan_mc2s_smi_t *pSmiSt);
void _rtl8367b_svlanMc2sStSmi2User(rtl8367b_svlan_mc2s_t *pUserSt, rtl8367b_svlan_mc2s_smi_t *pSmiSt);
void _rtl8367b_svlanSp2cStUser2Smi(rtl8367b_svlan_s2c_t *pUserSt, rtl8367b_svlan_s2c_smi_t *pSmiSt);
void _rtl8367b_svlanSp2cStSmi2User(rtl8367b_svlan_s2c_t *pUserSt, rtl8367b_svlan_s2c_smi_t *pSmiSt);
/* Function Name:
 *      rtl8367b_setAsicSvlanUplinkPortMask
 * Description:
 *      Set uplink ports mask
 * Input:
 *      portMask 	- Uplink port mask setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicSvlanUplinkPortMask(rtk_uint32 portMask)
{
    return rtl8367b_setAsicReg(RTL8367B_REG_SVLAN_UPLINK_PORTMASK, portMask);
}
/* Function Name:
 *      rtl8367b_getAsicSvlanUplinkPortMask
 * Description:
 *      Get uplink ports mask
 * Input:
 *      pPortmask 	- Uplink port mask setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicSvlanUplinkPortMask(rtk_uint32* pPortmask)
{
    return rtl8367b_getAsicReg(RTL8367B_REG_SVLAN_UPLINK_PORTMASK, pPortmask);
}
/* Function Name:
 *      rtl8367b_setAsicSvlanTpid
 * Description:
 *      Set accepted S-VLAN ether type. The default ether type of S-VLAN is 0x88a8
 * Input:
 *      protocolType 	- Ether type of S-tag frame parsing in uplink ports
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 and 0x9200
 * 		for Q-in-Q SLAN design. User can set mathced ether type as service provider supported protocol
 */
ret_t rtl8367b_setAsicSvlanTpid(rtk_uint32 protocolType)
{
    return rtl8367b_setAsicReg(RTL8367B_REG_VS_TPID, protocolType);
}
/* Function Name:
 *      rtl8367b_getAsicReg
 * Description:
 *      Get accepted S-VLAN ether type. The default ether type of S-VLAN is 0x88a8
 * Input:
 *      pProtocolType 	- Ether type of S-tag frame parsing in uplink ports
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicSvlanTpid(rtk_uint32* pProtocolType)
{
    return rtl8367b_getAsicReg(RTL8367B_REG_VS_TPID, pProtocolType);
}
/* Function Name:
 *      rtl8367b_setAsicSvlanPrioritySel
 * Description:
 *      Set SVLAN priority field setting
 * Input:
 *      priSel 	- S-priority assignment method, 0:internal priority 1:C-tag priority 2:using Svlan member configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_INPUT  	- Invalid input parameter
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicSvlanPrioritySel(rtk_uint32 priSel)
{
    if(priSel >= SPRISEL_END)
        return RT_ERR_INPUT;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_SVLAN_CFG, RTL8367B_VS_SPRISEL_MASK, priSel);
}
/* Function Name:
 *      rtl8367b_getAsicSvlanPrioritySel
 * Description:
 *      Get SVLAN priority field setting
 * Input:
 *      pPriSel 	- S-priority assignment method, 0:internal priority 1:C-tag priority 2:using Svlan member configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicSvlanPrioritySel(rtk_uint32* pPriSel)
{
   	return rtl8367b_getAsicRegBits(RTL8367B_REG_SVLAN_CFG, RTL8367B_VS_SPRISEL_MASK, pPriSel);
}
/* Function Name:
 *      rtl8367b_setAsicSvlanTrapPriority
 * Description:
 *      Set trap to CPU priority assignment
 * Input:
 *      priority 	- Priority assignment
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicSvlanTrapPriority(rtk_uint32 priority)
{
    if(priority > RTL8367B_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_QOS_TRAP_PRIORITY0, RTL8367B_SVLAN_PRIOIRTY_MASK, priority);
}
/* Function Name:
 *      rtl8367b_getAsicSvlanTrapPriority
 * Description:
 *      Get trap to CPU priority assignment
 * Input:
 *      pPriority 	- Priority assignment
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicSvlanTrapPriority(rtk_uint32* pPriority)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_QOS_TRAP_PRIORITY0, RTL8367B_SVLAN_PRIOIRTY_MASK, pPriority);
}
/* Function Name:
 *      rtl8367b_setAsicSvlanDefaultVlan
 * Description:
 *      Set default egress SVLAN
 * Input:
 *      port 	- Physical port number (0~7)
 *      index 	- index SVLAN member configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 *      RT_ERR_PORT_ID  			- Invalid port number
 *      RT_ERR_SVLAN_ENTRY_INDEX  	- Invalid SVLAN index parameter
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicSvlanDefaultVlan(rtk_uint32 port, rtk_uint32 index)
{
    ret_t retVal;

	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    if(index > RTL8367B_SVIDXMAX)
        return RT_ERR_SVLAN_ENTRY_INDEX;

	if(port & 1)
    	retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_SVLAN_PORTBASED_SVIDX_CTRL0 + (port >> 1), RTL8367B_VS_PORT1_SVIDX_MASK,index);
	else
    	retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_SVLAN_PORTBASED_SVIDX_CTRL0 + (port >> 1), RTL8367B_VS_PORT0_SVIDX_MASK,index);

    return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicSvlanDefaultVlan
 * Description:
 *      Get default egress SVLAN
 * Input:
 *      port 	- Physical port number (0~7)
 *      pIndex 	- index SVLAN member configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicSvlanDefaultVlan(rtk_uint32 port, rtk_uint32* pIndex)
{
    ret_t retVal;

	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

	if(port & 1)
    	retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_SVLAN_PORTBASED_SVIDX_CTRL0 + (port >> 1), RTL8367B_VS_PORT1_SVIDX_MASK,pIndex);
	else
    	retVal = rtl8367b_getAsicRegBits(RTL8367B_REG_SVLAN_PORTBASED_SVIDX_CTRL0 + (port >> 1), RTL8367B_VS_PORT0_SVIDX_MASK,pIndex);

    return retVal;
}
/* Function Name:
 *      rtl8367b_setAsicSvlanIngressUntag
 * Description:
 *      Set action received un-Stag frame from unplink port
 * Input:
 *      mode 		- 0:Drop 1:Trap 2:Assign SVLAN
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicSvlanIngressUntag(rtk_uint32 mode)
{
    return rtl8367b_setAsicRegBits(RTL8367B_REG_SVLAN_CFG, RTL8367B_VS_UNTAG_MASK, mode);
}
/* Function Name:
 *      rtl8367b_getAsicSvlanIngressUntag
 * Description:
 *      Get action received un-Stag frame from unplink port
 * Input:
 *      pMode 		- 0:Drop 1:Trap 2:Assign SVLAN
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicSvlanIngressUntag(rtk_uint32* pMode)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_SVLAN_CFG, RTL8367B_VS_UNTAG_MASK, pMode);
}
/* Function Name:
 *      rtl8367b_setAsicSvlanIngressUnmatch
 * Description:
 *      Set action received unmatched Stag frame from unplink port
 * Input:
 *      mode 		- 0:Drop 1:Trap 2:Assign SVLAN
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicSvlanIngressUnmatch(rtk_uint32 mode)
{
    return rtl8367b_setAsicRegBits(RTL8367B_REG_SVLAN_CFG, RTL8367B_VS_UNMAT_MASK, mode);
}
/* Function Name:
 *      rtl8367b_getAsicSvlanIngressUnmatch
 * Description:
 *      Get action received unmatched Stag frame from unplink port
 * Input:
 *      pMode 		- 0:Drop 1:Trap 2:Assign SVLAN
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicSvlanIngressUnmatch(rtk_uint32* pMode)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_SVLAN_CFG, RTL8367B_VS_UNMAT_MASK, pMode);

}
/* Function Name:
 *      rtl8367b_setAsicSvlanEgressUnassign
 * Description:
 *      Set unplink stream without egress SVID action
 * Input:
 *      enabled 	- 1:Trap egress unassigned frames to CPU, 0: Use SVLAN setup in VS_CPSVIDX as egress SVID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicSvlanEgressUnassign(rtk_uint32 enabled)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_SVLAN_CFG, RTL8367B_VS_UIFSEG_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicSvlanEgressUnassign
 * Description:
 *      Get unplink stream without egress SVID action
 * Input:
 *      pEnabled 	- 1:Trap egress unassigned frames to CPU, 0: Use SVLAN setup in VS_CPSVIDX as egress SVID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicSvlanEgressUnassign(rtk_uint32* pEnabled)
{
   	return rtl8367b_getAsicRegBit(RTL8367B_REG_SVLAN_CFG, RTL8367B_VS_UIFSEG_OFFSET, pEnabled);
}

void _rtl8367b_svlanConfStUser2Smi( rtl8367b_svlan_memconf_t *pUserSt, rtl8367b_svlan_memconf_smi_t *pSmiSt)
{

    pSmiSt->vs_member 		= pUserSt->vs_member;
    pSmiSt->vs_untag		= pUserSt->vs_untag;

    pSmiSt->vs_fid_msti		= pUserSt->vs_fid_msti;

    pSmiSt->vs_priority 	= pUserSt->vs_priority;

    pSmiSt->vs_svid			= pUserSt->vs_svid;
    pSmiSt->vs_efiden		= pUserSt->vs_efiden;
    pSmiSt->vs_efid			= pUserSt->vs_efid;

    pSmiSt->vs_force_fid	= pUserSt->vs_force_fid;

}
void _rtl8367b_svlanConfStSmi2User( rtl8367b_svlan_memconf_t *pUserSt, rtl8367b_svlan_memconf_smi_t *pSmiSt)
{
    pUserSt->vs_member 		= pSmiSt->vs_member;
    pUserSt->vs_untag 		= pSmiSt->vs_untag;

    pUserSt->vs_fid_msti	= pSmiSt->vs_fid_msti;

    pUserSt->vs_priority	= pSmiSt->vs_priority;

    pUserSt->vs_svid 		= pSmiSt->vs_svid;

    pUserSt->vs_efiden 		= pSmiSt->vs_efiden;
    pUserSt->vs_efid 		= pSmiSt->vs_efid;

    pUserSt->vs_force_fid 	= pSmiSt->vs_force_fid;

}
/* Function Name:
 *      rtl8367b_setAsicSvlanMemberConfiguration
 * Description:
 *      Set system 64 S-tag content
 * Input:
 *      index 			- index of 64 s-tag configuration
 *      pSvlanMemCfg 	- SVLAN member configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 *      RT_ERR_SVLAN_ENTRY_INDEX  	- Invalid SVLAN index parameter
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicSvlanMemberConfiguration(rtk_uint32 index, rtl8367b_svlan_memconf_t* pSvlanMemCfg)
{
    ret_t retVal;
    rtk_uint32 regData;
    rtk_uint16 *accessPtr;
    rtk_uint32 i;

    rtl8367b_svlan_memconf_smi_t smiSvlanMemConf;

    if(index > RTL8367B_SVIDXMAX)
        return RT_ERR_SVLAN_ENTRY_INDEX;

    memset(&smiSvlanMemConf, 0x00, sizeof(smiSvlanMemConf));
    _rtl8367b_svlanConfStUser2Smi(pSvlanMemCfg, &smiSvlanMemConf);

    accessPtr = (rtk_uint16*)&smiSvlanMemConf;


    regData = *accessPtr;
    for(i = 0; i < 3; i++)
    {
        retVal = rtl8367b_setAsicReg(RTL8367B_SVLAN_MEMBERCFG_BASE_REG(index) + i, regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        accessPtr ++;
        regData = *accessPtr;
    }


    return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicSvlanMemberConfiguration
 * Description:
 *      Get system 64 S-tag content
 * Input:
 *      index 			- index of 64 s-tag configuration
 *      pSvlanMemCfg 	- SVLAN member configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 *      RT_ERR_SVLAN_ENTRY_INDEX  	- Invalid SVLAN index parameter
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicSvlanMemberConfiguration(rtk_uint32 index,rtl8367b_svlan_memconf_t* pSvlanMemCfg)
{
    ret_t retVal;
    rtk_uint32 regData;
    rtk_uint16 *accessPtr;
    rtk_uint32 i;

    rtl8367b_svlan_memconf_smi_t smiSvlanMemConf;

    if(index > RTL8367B_SVIDXMAX)
        return RT_ERR_SVLAN_ENTRY_INDEX;

    memset(&smiSvlanMemConf, 0x00, sizeof(smiSvlanMemConf));

    accessPtr = (rtk_uint16*)&smiSvlanMemConf;

    for(i = 0; i < 3; i++)
    {
        retVal = rtl8367b_getAsicReg(RTL8367B_SVLAN_MEMBERCFG_BASE_REG(index) + i, &regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        *accessPtr = regData;

        accessPtr ++;
    }


    _rtl8367b_svlanConfStSmi2User(pSvlanMemCfg, &smiSvlanMemConf);

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicSvlanC2SConf
 * Description:
 *      Set SVLAN C2S table
 * Input:
 *      index 	- index of 128 Svlan C2S configuration
 *      evid 	- Enhanced VID
 *      portmask 	- available c2s port mask
 *      svidx 	- index of 64 Svlan member configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_ENTRY_INDEX  - Invalid entry index
 * Note:
 *      ASIC will check upstream's VID and assign related SVID to mathed packet
 */
ret_t rtl8367b_setAsicSvlanC2SConf(rtk_uint32 index, rtk_uint32 evid, rtk_uint32 portmask, rtk_uint32 svidx)
{
    ret_t retVal;
    rtk_uint32 regData;
    rtk_uint16 *accessPtr;
    rtk_uint32 i;

    rtl8367b_svlan_c2s_smi_t smiSvlanC2SConf;

    if(index > RTL8367B_C2SIDXMAX)
        return RT_ERR_ENTRY_INDEX;

    memset(&smiSvlanC2SConf, 0x00, sizeof(smiSvlanC2SConf));

    smiSvlanC2SConf.svidx = svidx;

    smiSvlanC2SConf.c2senPmsk= portmask;

    smiSvlanC2SConf.evid = evid;


    accessPtr =  (rtk_uint16*)&smiSvlanC2SConf;


    regData = *accessPtr;
    for(i = 0; i < 3; i++)
    {
        retVal = rtl8367b_setAsicReg(RTL8367B_SVLAN_C2SCFG_BASE_REG(index) + i, regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        accessPtr ++;
        regData = *accessPtr;
    }


    return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicSvlanC2SConf
 * Description:
 *      Get SVLAN C2S table
 * Input:
 *      index 	- index of 128 Svlan C2S configuration
 *      pEvid 	- Enhanced VID
 *      pPortmask 	- available c2s port mask
 *      pSvidx 	- index of 64 Svlan member configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_ENTRY_INDEX  - Invalid entry index
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicSvlanC2SConf(rtk_uint32 index, rtk_uint32* pEvid, rtk_uint32* pPortmask, rtk_uint32* pSvidx)
{
    ret_t retVal;
    rtk_uint32 regData;
    rtk_uint16 *accessPtr;
    rtk_uint32 i;

    rtl8367b_svlan_c2s_smi_t smiSvlanC2SConf;

    if(index > RTL8367B_C2SIDXMAX)
        return RT_ERR_ENTRY_INDEX;

    memset(&smiSvlanC2SConf, 0x00, sizeof(smiSvlanC2SConf));


    accessPtr = (rtk_uint16*)&smiSvlanC2SConf;

    for(i = 0; i < 3; i++)
    {
        retVal = rtl8367b_getAsicReg(RTL8367B_SVLAN_C2SCFG_BASE_REG(index) + i, &regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        *accessPtr = (rtk_uint16)regData;

        accessPtr ++;
    }

    *pSvidx = smiSvlanC2SConf.svidx;

    *pPortmask= smiSvlanC2SConf.c2senPmsk;

    *pEvid = smiSvlanC2SConf.evid;

    return retVal;
}

void _rtl8367b_svlanMc2sStUser2Smi(rtl8367b_svlan_mc2s_t *pUserSt, rtl8367b_svlan_mc2s_smi_t *pSmiSt)
{
    pSmiSt->svidx= pUserSt->svidx;

    pSmiSt->mask0 = pUserSt->smask & 0x000000FF;
    pSmiSt->mask1 = (pUserSt->smask & 0x0000FF00) >> 8;
    pSmiSt->mask2 = (pUserSt->smask & 0x00FF0000) >> 16;
    pSmiSt->mask3 = (pUserSt->smask & 0xFF000000) >> 24;

    pSmiSt->data0 = pUserSt->sdata & 0x000000FF;
    pSmiSt->data1 = (pUserSt->sdata & 0x0000FF00) >> 8;
    pSmiSt->data2 = (pUserSt->sdata & 0x00FF0000) >> 16;
    pSmiSt->data3 = (pUserSt->sdata & 0xFF000000) >> 24;

    pSmiSt->format = pUserSt->format;

    pSmiSt->valid = pUserSt->valid;
}
void _rtl8367b_svlanMc2sStSmi2User(rtl8367b_svlan_mc2s_t *pUserSt, rtl8367b_svlan_mc2s_smi_t *pSmiSt)
{
    pUserSt->svidx = pSmiSt->svidx;

    pUserSt->smask = (pSmiSt->mask3 << 24) | (pSmiSt->mask2 << 16) | (pSmiSt->mask1 << 8) | pSmiSt->mask0;

    pUserSt->sdata = (pSmiSt->data3 << 24) | (pSmiSt->data2 << 16) | (pSmiSt->data1 << 8) | pSmiSt->data0;

    pUserSt->format = pSmiSt->format;

    pUserSt->valid = pSmiSt->valid;
}
/* Function Name:
 *      rtl8367b_setAsicSvlanMC2SConf
 * Description:
 *      Set system MC2S content
 * Input:
 *      index 			- index of 32 SVLAN 32 MC2S configuration
 *      pSvlanMc2sCfg 	- SVLAN Multicast to SVLAN member configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_ENTRY_INDEX  - Invalid entry index
 * Note:
 *      If upstream packet is L2 multicast or IPv4 multicast packet and DMAC/DIP is matched MC2S
 * 		configuration, ASIC will assign egress SVID to the packet
 */
ret_t rtl8367b_setAsicSvlanMC2SConf(rtk_uint32 index,rtl8367b_svlan_mc2s_t* pSvlanMc2sCfg)
{
    ret_t retVal;
    rtk_uint32 regData;
    rtk_uint16 *accessPtr;
    rtk_uint32 i;

    rtl8367b_svlan_mc2s_smi_t smiSvlanMC2S;

    if(index > RTL8367B_MC2SIDXMAX)
        return RT_ERR_ENTRY_INDEX;

    memset(&smiSvlanMC2S, 0x00, sizeof(smiSvlanMC2S));
    _rtl8367b_svlanMc2sStUser2Smi(pSvlanMc2sCfg, &smiSvlanMC2S);

    accessPtr =  (rtk_uint16*)&smiSvlanMC2S;


    regData = *accessPtr;
    for(i = 0; i < 5; i++)
    {
        retVal = rtl8367b_setAsicReg(RTL8367BG_SVLAN_MCAST2S_ENTRY_BASE_REG(index) + i, regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        accessPtr ++;
        regData = *accessPtr;
    }


    return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicSvlanMC2SConf
 * Description:
 *      Get system MC2S content
 * Input:
 *      index 			- index of 32 SVLAN 32 MC2S configuration
 *      pSvlanMc2sCfg 	- SVLAN Multicast to SVLAN member configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_ENTRY_INDEX  - Invalid entry index
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicSvlanMC2SConf(rtk_uint32 index, rtl8367b_svlan_mc2s_t* pSvlanMc2sCfg)
{
    ret_t retVal;
    rtk_uint32 regData;
    rtk_uint16 *accessPtr;
    rtk_uint32 i;

    rtl8367b_svlan_mc2s_smi_t smiSvlanMC2S;

    if(index > RTL8367B_MC2SIDXMAX)
        return RT_ERR_ENTRY_INDEX;

    memset(&smiSvlanMC2S, 0x00, sizeof(smiSvlanMC2S));

    accessPtr = (rtk_uint16*)&smiSvlanMC2S;

    for(i = 0; i < 5; i++)
    {
        retVal = rtl8367b_getAsicReg(RTL8367BG_SVLAN_MCAST2S_ENTRY_BASE_REG(index) + i, &regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        *accessPtr = regData;

        accessPtr ++;
    }


    _rtl8367b_svlanMc2sStSmi2User(pSvlanMc2sCfg, &smiSvlanMC2S);

    return RT_ERR_OK;
}

void _rtl8367b_svlanSp2cStUser2Smi(rtl8367b_svlan_s2c_t *pUserSt, rtl8367b_svlan_s2c_smi_t *pSmiSt)
{
    pSmiSt->svidx 		= pUserSt->svidx;
    pSmiSt->dstport 	= pUserSt->dstport;
    pSmiSt->valid 		= pUserSt->valid;
    pSmiSt->vid  		= pUserSt->vid;
}
void _rtl8367b_svlanSp2cStSmi2User(rtl8367b_svlan_s2c_t *pUserSt, rtl8367b_svlan_s2c_smi_t *pSmiSt)
{
    pUserSt->svidx 		= pSmiSt->svidx;
    pUserSt->dstport 	= pSmiSt->dstport;
    pUserSt->valid 		= pSmiSt->valid;
    pUserSt->vid 		= pSmiSt->vid;
}
/* Function Name:
 *      rtl8367b_setAsicSvlanSP2CConf
 * Description:
 *      Set system 128 SP2C content
 * Input:
 *      index 			- index of 128 SVLAN & Port to CVLAN configuration
 *      pSvlanSp2cCfg 	- SVLAN & Port to CVLAN configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_ENTRY_INDEX  - Invalid entry index
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicSvlanSP2CConf(rtk_uint32 index, rtl8367b_svlan_s2c_t* pSvlanSp2cCfg)
{
    ret_t retVal;
    rtk_uint32 regData;
    rtk_uint16 *accessPtr;
    rtk_uint32 i;

    rtl8367b_svlan_s2c_smi_t smiSvlanSP2C;

    if(index > RTL8367B_SP2CMAX)
        return RT_ERR_ENTRY_INDEX;

    memset(&smiSvlanSP2C, 0x00, sizeof(smiSvlanSP2C));
    _rtl8367b_svlanSp2cStUser2Smi(pSvlanSp2cCfg,&smiSvlanSP2C);

    accessPtr = (rtk_uint16*)&smiSvlanSP2C;


    regData = *accessPtr;
    for(i = 0; i < 2; i++)
    {
        retVal = rtl8367b_setAsicReg(RTL8367B_SVLAN_S2C_ENTRY_BASE_REG(index) + i, regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        accessPtr ++;
        regData = *accessPtr;
    }


    return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicSvlanSP2CConf
 * Description:
 *      Get system 128 SP2C content
 * Input:
 *      index 			- index of 128 SVLAN & Port to CVLAN configuration
 *      pSvlanSp2cCfg 	- SVLAN & Port to CVLAN configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_ENTRY_INDEX  - Invalid entry index
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicSvlanSP2CConf(rtk_uint32 index,rtl8367b_svlan_s2c_t* pSvlanSp2cCfg)
{
    ret_t retVal;
    rtk_uint32 regData;
    rtk_uint16 *accessPtr;
    rtk_uint32 i;

    rtl8367b_svlan_s2c_smi_t smiSvlanSP2C;

    if(index > RTL8367B_SP2CMAX)
        return RT_ERR_ENTRY_INDEX;

    memset(&smiSvlanSP2C, 0x00, sizeof(smiSvlanSP2C));

    accessPtr = (rtk_uint16*)&smiSvlanSP2C;

    for(i = 0; i < 2; i++)
    {
        retVal = rtl8367b_getAsicReg(RTL8367B_SVLAN_S2C_ENTRY_BASE_REG(index) + i, &regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        *accessPtr = regData;

        accessPtr ++;
    }

    _rtl8367b_svlanSp2cStSmi2User(pSvlanSp2cCfg,&smiSvlanSP2C);

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicSvlanDmacCvidSel
 * Description:
 *      Set downstream CVID decision by DMAC
 * Input:
 *      port 		- Physical port number (0~7)
 *      enabled 	- 0:disabled, 1:enabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicSvlanDmacCvidSel(rtk_uint32 port, rtk_uint32 enabled)
{
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    return rtl8367b_setAsicRegBit(RTL8367B_REG_SVLAN_CFG, RTL8367B_VS_PORT0_DMACVIDSEL_OFFSET + port, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicSvlanDmacCvidSel
 * Description:
 *      Get downstream CVID decision by DMAC
 * Input:
 *      port 		- Physical port number (0~7)
 *      pEnabled 	- 0:disabled, 1:enabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicSvlanDmacCvidSel(rtk_uint32 port, rtk_uint32* pEnabled)
{
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBit(RTL8367B_REG_SVLAN_CFG, RTL8367B_VS_PORT0_DMACVIDSEL_OFFSET + port, pEnabled);;
}
/* Function Name:
 *      rtl8367b_setAsicSvlanUntagVlan
 * Description:
 *      Set default ingress untag SVLAN
 * Input:
 *      index 	- index SVLAN member configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 *      RT_ERR_SVLAN_ENTRY_INDEX  	- Invalid SVLAN index parameter
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicSvlanUntagVlan(rtk_uint32 index)
{
    if(index > RTL8367B_SVIDXMAX)
        return RT_ERR_SVLAN_ENTRY_INDEX;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_SVLAN_UNTAG_UNMAT_CFG, RTL8367B_VS_UNTAG_SVIDX_MASK, index);
}
/* Function Name:
 *      rtl8367b_getAsicSvlanUntagVlan
 * Description:
 *      Get default ingress untag SVLAN
 * Input:
 *      pIndex 	- index SVLAN member configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicSvlanUntagVlan(rtk_uint32* pIndex)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_SVLAN_UNTAG_UNMAT_CFG, RTL8367B_VS_UNTAG_SVIDX_MASK, pIndex);
}

/* Function Name:
 *      rtl8367b_setAsicSvlanUnmatchVlan
 * Description:
 *      Set default ingress unmatch SVLAN
 * Input:
 *      index 	- index SVLAN member configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 *      RT_ERR_SVLAN_ENTRY_INDEX  	- Invalid SVLAN index parameter
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicSvlanUnmatchVlan(rtk_uint32 index)
{
    if(index > RTL8367B_SVIDXMAX)
        return RT_ERR_SVLAN_ENTRY_INDEX;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_SVLAN_UNTAG_UNMAT_CFG, RTL8367B_VS_UNMAT_SVIDX_MASK, index);
}
/* Function Name:
 *      rtl8367b_getAsicSvlanUnmatchVlan
 * Description:
 *      Get default ingress unmatch SVLAN
 * Input:
 *      pIndex 	- index SVLAN member configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicSvlanUnmatchVlan(rtk_uint32* pIndex)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_SVLAN_UNTAG_UNMAT_CFG, RTL8367B_VS_UNMAT_SVIDX_MASK, pIndex);
}

