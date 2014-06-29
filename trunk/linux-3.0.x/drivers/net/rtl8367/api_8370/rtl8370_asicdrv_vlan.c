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
 * $Revision: 1.1.1.1 $
 * $Date: 2010/12/02 04:34:21 $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367B
 * Feature : 
 *
 */
#include "rtl8370_asicdrv_vlan.h"

#if defined(CONFIG_RTL8370_ASICDRV_TEST)
rtl8370_vlan4kentrysmi Rtl8370sVirtualVlanTable[RTL8370_VIDMAX + 1];
#endif

#if !defined(DISABLE_VLAN_SHADOW)
rtl8370_user_vlan4kentry  user_4kvlan[RTL8370_VIDMAX + 1];
#endif

void _rtl8370_VlanMCStUser2Smi(rtl8370_vlanconfiguser *ptr_vlancfg, rtl8370_vlanconfigsmi *ptr_smi_vlancfg);
void _rtl8370_VlanMCStSmi2User(rtl8370_vlanconfigsmi *ptr_smi_vlancfg, rtl8370_vlanconfiguser *ptr_vlancfg);
void _rtl8370_Vlan4kStUser2Smi(rtl8370_user_vlan4kentry *ptr_user_vlan4kEntry, rtl8370_vlan4kentrysmi *ptr_smi_vlan4kEntry);
void _rtl8370_Vlan4kStSmi2User(rtl8370_vlan4kentrysmi *ptr_smi_vlan4kEntry, rtl8370_user_vlan4kentry *ptr_user_vlan4kEntry);

void _rtl8370_VlanMCStUser2Smi(rtl8370_vlanconfiguser *ptr_vlancfg, rtl8370_vlanconfigsmi *ptr_smi_vlancfg)
{
    ptr_smi_vlancfg->mbr         	= ptr_vlancfg->mbr;
    ptr_smi_vlancfg->msti         	= ptr_vlancfg->msti;
    ptr_smi_vlancfg->fid         	= ptr_vlancfg->fid;
    ptr_smi_vlancfg->evid         	= ptr_vlancfg->evid;
    ptr_smi_vlancfg->meteridx     	= ptr_vlancfg->meteridx;
    ptr_smi_vlancfg->envlanpol     	= ptr_vlancfg->envlanpol;
    ptr_smi_vlancfg->vbpri         	= ptr_vlancfg->vbpri;
    ptr_smi_vlancfg->vbpen         	= ptr_vlancfg->vbpen;
    ptr_smi_vlancfg->lurep         	= ptr_vlancfg->lurep;
}

void _rtl8370_VlanMCStSmi2User(rtl8370_vlanconfigsmi *ptr_smi_vlancfg, rtl8370_vlanconfiguser *ptr_vlancfg)
{
    ptr_vlancfg->mbr            	= ptr_smi_vlancfg->mbr;
    ptr_vlancfg->msti            	= ptr_smi_vlancfg->msti;
    ptr_vlancfg->fid            	= ptr_smi_vlancfg->fid;
    ptr_vlancfg->evid            	= ptr_smi_vlancfg->evid;
    ptr_vlancfg->meteridx        	= ptr_smi_vlancfg->meteridx;
    ptr_vlancfg->envlanpol        	= ptr_smi_vlancfg->envlanpol;
    ptr_vlancfg->vbpri            	= ptr_smi_vlancfg->vbpri;
    ptr_vlancfg->vbpen            	= ptr_smi_vlancfg->vbpen;
    ptr_vlancfg->lurep            	= ptr_smi_vlancfg->lurep;
}

void _rtl8370_Vlan4kStUser2Smi(rtl8370_user_vlan4kentry *ptr_user_vlan4kEntry, rtl8370_vlan4kentrysmi *ptr_smi_vlan4kEntry)
{
    ptr_smi_vlan4kEntry->mbr        = ptr_user_vlan4kEntry->mbr;
     ptr_smi_vlan4kEntry->fid       = ptr_user_vlan4kEntry->fid;
    ptr_smi_vlan4kEntry->msti       = ptr_user_vlan4kEntry->msti;
     ptr_smi_vlan4kEntry->lurep     = ptr_user_vlan4kEntry->lurep;
     ptr_smi_vlan4kEntry->vbpen     = ptr_user_vlan4kEntry->vbpen;
    ptr_smi_vlan4kEntry->vbpri      = ptr_user_vlan4kEntry->vbpri;
    ptr_smi_vlan4kEntry->envlanpol  = ptr_user_vlan4kEntry->envlanpol;
    ptr_smi_vlan4kEntry->meteridx   = ptr_user_vlan4kEntry->meteridx;
    ptr_smi_vlan4kEntry->untag1     = ptr_user_vlan4kEntry->untag & 0x0003;
    ptr_smi_vlan4kEntry->untag2     = (ptr_user_vlan4kEntry->untag >> 2) & 0x3FFF;
}

void _rtl8370_Vlan4kStSmi2User(rtl8370_vlan4kentrysmi *ptr_smi_vlan4kEntry, rtl8370_user_vlan4kentry *ptr_user_vlan4kEntry)
{
    ptr_user_vlan4kEntry->mbr    	= ptr_smi_vlan4kEntry->mbr;
     ptr_user_vlan4kEntry->fid      = ptr_smi_vlan4kEntry->fid;
    ptr_user_vlan4kEntry->msti      = ptr_smi_vlan4kEntry->msti;
     ptr_user_vlan4kEntry->lurep    = ptr_smi_vlan4kEntry->lurep;
     ptr_user_vlan4kEntry->vbpen    = ptr_smi_vlan4kEntry->vbpen;
    ptr_user_vlan4kEntry->vbpri     = ptr_smi_vlan4kEntry->vbpri;
    ptr_user_vlan4kEntry->envlanpol = ptr_smi_vlan4kEntry->envlanpol;
    ptr_user_vlan4kEntry->meteridx  = ptr_smi_vlan4kEntry->meteridx;
    ptr_user_vlan4kEntry->untag     = (ptr_smi_vlan4kEntry->untag1 & 0x0003) | (ptr_smi_vlan4kEntry->untag2 << 2);
}


/*
@func ret_t | rtl8370_setAsicVlanMemberConfig | Set 32 VLAN member configurations.
@parm uint32 | index | VLAN member configuration index (0~31).
@parm rtl8370_vlanconfiguser* | ptr_vlancfg | VLAN member configuration. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@rvalue RT_ERR_L2_FID | Invalid FID (0~4095).
@rvalue RT_ERR_VLAN_PRIORITY | Invalid VLAN priority (0~7).
@rvalue RT_ERR_PORT_MASK | Invalid port mask (0x00~0x3F).
@rvalue RT_ERR_VLAN_VID | Invalid VID parameter (0~4095).
@rvalue RT_ERR_MSTI | Invalid msti parameter
@rvalue RT_ERR_VLAN_ENTRY_NOT_FOUND | Invalid VLAN member configuration index (0~15).
@comm
    VLAN ingress and egress will reference these 32 VLAN configurations 
    Port based, Protocol-and-Port based VLAN and 802.1x guest VLAN functions 
    retrieved VLAN information from these 32 member configurations too.
*/
ret_t rtl8370_setAsicVlanMemberConfig(uint32 index, rtl8370_vlanconfiguser *ptr_vlancfg)
{
    ret_t  retVal;
    uint32 regAddr;
    uint32 regData;
    uint16 *tableAddr;
    uint32 page_idx;
    rtl8370_vlanconfigsmi  smi_vlancfg;

    /* Error Checking  */
    if(index > RTL8370_CVIDXMAX)
        return RT_ERR_VLAN_ENTRY_NOT_FOUND;

    if(NULL == ptr_vlancfg)
        return RT_ERR_INPUT;

    if(ptr_vlancfg->evid > RTL8370_EVIDMAX) 
        return RT_ERR_VLAN_VID;

    if( (ptr_vlancfg->lurep != TRUE) && (ptr_vlancfg->lurep != FALSE) )
        return RT_ERR_INPUT;

    if(ptr_vlancfg->fid > RTL8370_FIDMAX)
        return RT_ERR_L2_FID;

    if(ptr_vlancfg->msti > RTL8370_MSTIMAX)
        return RT_ERR_MSTI;

    if( (ptr_vlancfg->envlanpol != TRUE) && (ptr_vlancfg->envlanpol != FALSE) )
        return RT_ERR_INPUT;

    if(ptr_vlancfg->meteridx > RTL8370_METERIDXMAX)
        return RT_ERR_FILTER_METER_ID;

    if( (ptr_vlancfg->vbpen != TRUE) && (ptr_vlancfg->vbpen != FALSE) )
        return RT_ERR_INPUT;

    if(ptr_vlancfg->vbpri > RTL8370_PRIMAX)
        return RT_ERR_VLAN_PRIORITY;

    memset(&smi_vlancfg, 0x00, sizeof(rtl8370_vlanconfigsmi));
    _rtl8370_VlanMCStUser2Smi(ptr_vlancfg, &smi_vlancfg);
    tableAddr = (uint16*)&smi_vlancfg;

    for(page_idx = 0; page_idx < 4; page_idx++)  /* 4 pages per VLAN Member Config */
    {
        regAddr = RTL8370_VLAN_MEMBER_CONFIGURATION_BASE + (index*4) + page_idx;
        regData = *tableAddr;
    
        retVal = rtl8370_setAsicReg(regAddr,regData);
        if(retVal !=  RT_ERR_OK)
            return retVal;

        tableAddr++;
    }
    
    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_getAsicVlanMemberConfig | Get 32 VLAN member configurations.
@parm uint32 | index | VLAN member configuration index (0~31).
@parm rtl8370_vlanconfiguser* | ptr_vlancfg | VLAN member configuration. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@rvalue RT_ERR_VLAN_ENTRY_NOT_FOUND | Invalid VLAN member configuration index (0~31).
@comm
    The API can get 32 VLAN member configuration.
    
*/
ret_t rtl8370_getAsicVlanMemberConfig(uint32 index, rtl8370_vlanconfiguser *ptr_vlancfg)
{
    ret_t  retVal;
    uint32 page_idx;
    uint32 regAddr;
    uint32 regData;
    uint16 *tableAddr;
    rtl8370_vlanconfigsmi  smi_vlancfg;

    if(index > RTL8370_CVIDXMAX)
        return RT_ERR_VLAN_ENTRY_NOT_FOUND;

    if(NULL == ptr_vlancfg)
        return RT_ERR_INPUT;

    memset(&smi_vlancfg, 0x00, sizeof(rtl8370_vlanconfigsmi));
    tableAddr  = (uint16*)&smi_vlancfg;

    for(page_idx = 0; page_idx < 4; page_idx++)  /* 4 pages per VLAN Member Config */
    {
        regAddr = RTL8370_VLAN_MEMBER_CONFIGURATION_BASE + (index*4) + page_idx;

        retVal = rtl8370_getAsicReg(regAddr, &regData);
        if(retVal != RT_ERR_OK)
            return retVal;
        
        *tableAddr = (uint16)regData;
        tableAddr++;
    }

    _rtl8370_VlanMCStSmi2User(&smi_vlancfg, ptr_vlancfg);
    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_setAsicVlan4kEntry | Set VID mapped entry to 4K VLAN table.
@parm rtl8370_user_vlan4kentry* | ptr_vlan4kEntry | VLAN entry seting for 4K table. There is VID field in entry structure and  entry is directly mapping to 4K table location (1 to 1).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@rvalue RT_ERR_L2_FID | Invalid FID (0~4095).
@rvalue RT_ERR_PORT_MASK | Invalid port mask (0x00~0x3F).
@rvalue RT_ERR_VLAN_VID | Invalid VID parameter (0~4095).
@comm
    VID field of C-tag is 12-bits and available VID range is 0~4095. In 802.1q spec. , null VID (0x000) means tag header contain priority information
    only and VID 0xFFF is reserved for implementtation usage. But ASIC still retrieved these VID entries in 4K VLAN table if VID is decided from 16
    member configurations. It has no available VID 0x000 and 0xFFF from C-tag. ASIC will retrieve these two non-standard VIDs (0x000 and 0xFFF) from 
    member configuration indirectly referenced by Port based, Protocol-and-Port based VLAN and 802.1x functions.
    
*/
ret_t rtl8370_setAsicVlan4kEntry(rtl8370_user_vlan4kentry *ptr_vlan4kEntry )
{
    rtl8370_vlan4kentrysmi vlan_4k_entry;
    uint32                    page_idx;
    uint16                    *tableAddr;
    ret_t                     retVal;
    uint32                     regData;

    if(NULL == ptr_vlan4kEntry)     
        return RT_ERR_INPUT;

    if(ptr_vlan4kEntry->vid > RTL8370_VIDMAX) 
        return RT_ERR_VLAN_VID;

    if((ptr_vlan4kEntry->lurep != TRUE) && (ptr_vlan4kEntry->lurep != FALSE) )
        return RT_ERR_INPUT;

    if(ptr_vlan4kEntry->fid > RTL8370_FIDMAX)
        return RT_ERR_L2_FID;

    if(ptr_vlan4kEntry->msti > RTL8370_MSTIMAX)
        return RT_ERR_MSTI;

    if( (ptr_vlan4kEntry->envlanpol != TRUE) && (ptr_vlan4kEntry->envlanpol != FALSE) )
        return RT_ERR_INPUT;

    if(ptr_vlan4kEntry->meteridx > RTL8370_METERIDXMAX)
        return RT_ERR_FILTER_METER_ID;

    if( (ptr_vlan4kEntry->vbpen != TRUE) && (ptr_vlan4kEntry->vbpen != FALSE) )
        return RT_ERR_INPUT;

    if(ptr_vlan4kEntry->vbpri > RTL8370_PRIMAX)
        return RT_ERR_VLAN_PRIORITY;

    memset(&vlan_4k_entry, 0x00, sizeof(rtl8370_vlan4kentrysmi));
    _rtl8370_Vlan4kStUser2Smi(ptr_vlan4kEntry, &vlan_4k_entry);

    /* Prepare Data */
    tableAddr = (uint16 *)&vlan_4k_entry;
    for(page_idx = 0; page_idx < (sizeof(rtl8370_vlan4kentrysmi) / 2); page_idx++)
    {
        regData = *tableAddr;
        retVal = rtl8370_setAsicReg(RTL8370_TABLE_ACCESS_DATA_BASE + page_idx, regData);
        if(retVal !=  RT_ERR_OK)
            return retVal;

        tableAddr++;
    }

    /* Write Address (VLAN_ID) */
    regData = ptr_vlan4kEntry->vid;
    retVal = rtl8370_setAsicReg(RTL8370_TABLE_ACCESS_ADDR_REG, regData);
    if(retVal !=  RT_ERR_OK)
        return retVal;

    /* Write Command */
    retVal = rtl8370_setAsicReg(RTL8370_TABLE_ACCESS_CTRL_REG, RTL8370_TABLE_ACCESS_REG_DATA(TB_OP_WRITE,TB_TARGET_CVLAN));
    if(retVal !=  RT_ERR_OK)
        return retVal;

#if defined(CONFIG_RTL8370_ASICDRV_TEST)
    memcpy(&Rtl8370sVirtualVlanTable[ptr_vlan4kEntry->vid], &vlan_4k_entry, sizeof(rtl8370_vlan4kentrysmi));
#endif

#if !defined(DISABLE_VLAN_SHADOW)
    memcpy(&user_4kvlan[ptr_vlan4kEntry->vid], ptr_vlan4kEntry, sizeof(rtl8370_user_vlan4kentry));
#endif

    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_getAsicVlan4kEntry | Get VID mapped entry to 4K VLAN table. 
@parm rtl8370_user_vlan4kentry* | ptr_vlan4kEntry | VLAN entry seting for 4K table. There is VID field in entry structure and  entry is directly mapping to 4K table location (1 to 1).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@rvalue RT_ERR_VLAN_VID | Invalid VID parameter (0~4095).
@comm
    The API can get entry of 4k VLAN table. Software must prepare the retrieving VID first at writing data and used control word to access desired VLAN entry.
    
*/
ret_t rtl8370_getAsicVlan4kEntry(rtl8370_user_vlan4kentry *ptr_vlan4kEntry )
{
#if defined(DISABLE_VLAN_SHADOW)
    rtl8370_vlan4kentrysmi vlan_4k_entry;
    uint32                    page_idx;
    uint16                    *tableAddr;
    ret_t                     retVal;
    uint32                     regData;

    if(NULL == ptr_vlan4kEntry)     
        return RT_ERR_INPUT;

    /* Write Address (VLAN_ID) */
    regData = ptr_vlan4kEntry->vid;
    retVal = rtl8370_setAsicReg(RTL8370_TABLE_ACCESS_ADDR_REG, regData);
    if(retVal !=  RT_ERR_OK)
        return retVal;

    /* Read Command */
    retVal = rtl8370_setAsicReg(RTL8370_TABLE_ACCESS_CTRL_REG, RTL8370_TABLE_ACCESS_REG_DATA(TB_OP_READ,TB_TARGET_CVLAN));
    if(retVal !=  RT_ERR_OK)
        return retVal;

    /* Check ASIC Command */

    /* Read VLAN data from register */
    tableAddr = (uint16 *)&vlan_4k_entry;
    for(page_idx = 0; page_idx < (sizeof(rtl8370_vlan4kentrysmi) / 2); page_idx++)
    {
        retVal = rtl8370_getAsicReg(RTL8370_TABLE_ACCESS_DATA_BASE + page_idx, &regData);
        if(retVal !=  RT_ERR_OK)
            return retVal;

        *tableAddr = regData;
        tableAddr++;
    }

    _rtl8370_Vlan4kStSmi2User(&vlan_4k_entry, ptr_vlan4kEntry);

#else

    uint16  vid;

    if(ptr_vlan4kEntry->vid > RTL8370_VIDMAX)
        return RT_ERR_VLAN_VID;

    vid = ptr_vlan4kEntry->vid;
    memcpy(ptr_vlan4kEntry, &user_4kvlan[ptr_vlan4kEntry->vid], sizeof(rtl8370_user_vlan4kentry));
    ptr_vlan4kEntry->vid = vid;

#endif

#if defined(CONFIG_RTL8370_ASICDRV_TEST)
    _rtl8370_Vlan4kStSmi2User(&Rtl8370sVirtualVlanTable[ptr_vlan4kEntry->vid], ptr_vlan4kEntry);
#endif

    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_setAsicVlanAccpetFrameType | Set per-port acceptable frame type
@parm uint32 | port | The port number
@parm rtl8370_accframetype | frame_type | The acceptable frame type
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_VLAN_ACCEPT_FRAME_TYPE | Invalid frame type.
@comm
    The API can set the acceptable frame type per-port.
    
*/
ret_t rtl8370_setAsicVlanAccpetFrameType(uint32 port, rtl8370_accframetype frame_type)
{
    uint32 regAddr, bit_mask;

    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(frame_type >= FRAME_TYPE_MAX_BOUND)
        return RT_ERR_VLAN_ACCEPT_FRAME_TYPE;

    regAddr = RTL8370_VLAN_ACCEPT_FRAME_TYPE_REG(port);
    bit_mask = RTL8370_VLAN_ACCEPT_FRAME_TYPE_MASK(port);
    return rtl8370_setAsicRegBits(regAddr, bit_mask, frame_type);
}

/*
@func ret_t | rtl8370_getAsicVlanAccpetFrameType | Get per-port acceptable frame type
@parm uint32 | port | The port number
@parm rtl8370_accframetype* | ptr_frame_type | The acceptable frame type
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can get the acceptable frame type per-port.
    
*/
ret_t rtl8370_getAsicVlanAccpetFrameType(uint32 port, rtl8370_accframetype *ptr_frame_type)
{
    uint32 regAddr,bit_mask;

    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(NULL == ptr_frame_type) 
        return RT_ERR_INPUT;

    regAddr = RTL8370_VLAN_ACCEPT_FRAME_TYPE_REG(port);
    bit_mask = RTL8370_VLAN_ACCEPT_FRAME_TYPE_MASK(port);
    return rtl8370_getAsicRegBits(regAddr, bit_mask, (uint32*)ptr_frame_type);
}

/*
@func ret_t | rtl8370_setAsicVlanIngressFilter | Set VLAN Ingress Filter
@parm uint32 | port | The port number
@parm uint32 | enabled | Enable or disable Ingress filter
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can set the VLAN ingress filter. When this function is enabled, if the receive port of a packet 
    is not the member of the VLAN which is the packet belongs to, it would be dropped.
*/
ret_t rtl8370_setAsicVlanIngressFilter(uint32 port, uint32 enabled)
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if((enabled != TRUE) && (enabled != FALSE) )
        return RT_ERR_INPUT;

    return rtl8370_setAsicRegBit(RTL8370SG_VLAN_INGRESS_REG, port, enabled);
}

/*
@func ret_t | rtl8370_getAsicVlanIngressFilter | Get VLAN Ingress Filter
@parm uint32 | port | The port number
@parm uint32* | ptr_enabled | Enable or disable Ingress filter
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can Get the VLAN ingress filter.
*/
ret_t rtl8370_getAsicVlanIngressFilter(uint32 port, uint32 *ptr_enabled)
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(NULL == ptr_enabled) 
        return RT_ERR_INPUT;

    return rtl8370_getAsicRegBit(RTL8370SG_VLAN_INGRESS_REG, port, ptr_enabled);
}
/*
@func ret_t | rtl8370_setAsicVlanEgressTagMode | Set CVLAN egress tag mode
@parm uint32 | port | The EGRESS port number
@parm rtl8370_egtagmode | tag_mode | The egress tag mode. Including Original mode, Keep tag mode and Priority tag mode.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can Set Egress tag mode. There are 3 mode for egress tag:
    Original mode : Output frame will follow VLAN untag setting.
    Keep tag mode : Output frame will keep VLAN original format.
    Priority tag mode : Output frame will be priority tag.
*/
ret_t rtl8370_setAsicVlanEgressTagMode( uint32 port, rtl8370_egtagmode tag_mode)
{
    ret_t  retVal;

    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(tag_mode >= EG_TAG_MODE_MAX_BOUND)
        return RT_ERR_INPUT;

    retVal = rtl8370_setAsicRegBits(RTL8370_PORT_MISC_CFG_REG(port), RTL8370_VLAN_EGRESS_MDOE_MASK, tag_mode);
    if(retVal != RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_getAsicVlanEgressTagMode | Get CVLAN egress tag mode
@parm uint32 | port | The EGRESS port number
@parm rtl8370_egtagmode* | ptr_tag_mode | The egress tag mode. Including Original mode, Keep tag mode and Priority tag mode.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can Get Egress tag mode. There are 3 mode for egress tag:
    Original mode : Output frame will follow VLAN untag setting.
    Keep tag mode : Output frame will keep VLAN original format.
    Priority tag mode : Output frame will be priority tag.
*/
ret_t rtl8370_getAsicVlanEgressTagMode( uint32 port, rtl8370_egtagmode *ptr_tag_mode)
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(NULL == ptr_tag_mode)
        return RT_ERR_INPUT;

    return rtl8370_getAsicRegBits(RTL8370_PORT_MISC_CFG_REG(port), RTL8370_VLAN_EGRESS_MDOE_MASK, (uint32*)ptr_tag_mode);
}

/*
@func ret_t | rtl8370_setAsicVlanPortBasedVID | Set port based VID which is indexed to 32 VLAN member configurations.
@parm uint32 | port | Physical port number (0~15).
@parm uint32 | index | Index to VLAN member configuration (0~31).
@parm uint32 | pri | 1Q Port based VLAN priority (0~7).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_VLAN_PRIORITY | Invalid priority
@rvalue RT_ERR_VLAN_ENTRY_NOT_FOUND | Invalid VLAN member configuration index (0~31).
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    This API can Set Port-Based VLAN setting
    In port based VLAN, untagged packets recieved by port N are forwarded to a VLAN according to the setting VID of port N. 
    Usage of VLAN 4k table is enabled and there are only VID and 802.1q priority retrieved from 32 member configurations . 
    Member set, untag set and FID of port based VLAN are be retrieved from 4K mapped VLAN entry.
    
*/
ret_t rtl8370_setAsicVlanPortBasedVID(uint32 port, uint32 index, uint32 pri)
{
    uint32 regAddr, bit_mask;
    ret_t  retVal;

    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(index > RTL8370_CVIDXMAX)
        return RT_ERR_VLAN_ENTRY_NOT_FOUND;

    if(pri > RTL8370_PRIMAX) 
        return RT_ERR_VLAN_PRIORITY;

    regAddr = RTL8370_VLAN_PVID_CTRL_REG(port);
    bit_mask = RTL8370_PORT_VIDX_MASK(port);
    retVal = rtl8370_setAsicRegBits(regAddr, bit_mask, index);
    if(retVal != RT_ERR_OK)
        return retVal;

    regAddr = RTL8370_VLAN_PORTBASED_PRIORITY_REG(port);
    bit_mask = RTL8370_VLAN_PORTBASED_PRIORITY_MASK(port);
    retVal = rtl8370_setAsicRegBits(regAddr, bit_mask, pri);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_getAsicVlanPortBasedVID | Set port based VID which is indexed to 32 VLAN member configurations.
@parm uint32 | port | Physical port number (0~15).
@parm uint32* | ptr_index | Index to VLAN member configuration (0~31).
@parm uint32* | ptr_pri | 1Q Port based VLAN priority (0~7).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_VLAN_ENTRY_NOT_FOUND | Invalid VLAN member configuration index (0~31).
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    This API can Get Port-Based VLAN setting
    In port based VLAN, untagged packets recieved by port N are forwarded to a VLAN according to the setting VID of port N. 
    Usage of VLAN 4k table is enabled and there are only VID and 802.1q priority retrieved from 32 member configurations. 
    Member set, untag set and FID of port based VLAN are be retrieved from 4K mapped VLAN entry.
    
*/
ret_t rtl8370_getAsicVlanPortBasedVID(uint32 port, uint32 *ptr_index, uint32 *ptr_pri)
{
    uint32 regAddr,bit_mask;
    ret_t  retVal;

    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(NULL == ptr_index)
        return RT_ERR_INPUT;

    if(NULL == ptr_pri) 
        return RT_ERR_INPUT;

    regAddr = RTL8370_VLAN_PVID_CTRL_REG(port);
    bit_mask = RTL8370_PORT_VIDX_MASK(port);
    retVal = rtl8370_getAsicRegBits(regAddr, bit_mask, ptr_index);
    if(retVal != RT_ERR_OK)
        return retVal;

    regAddr = RTL8370_VLAN_PORTBASED_PRIORITY_REG(port); 
    bit_mask = RTL8370_VLAN_PORTBASED_PRIORITY_MASK(port);
    retVal = rtl8370_getAsicRegBits(regAddr, bit_mask, ptr_pri);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_setAsicVlanProtocolBasedGroupData | Set protocol and port based group database.
@parm uint32 | index | Index of protocol and port based database index (0~7).
@parm rtl8370_protocolgdatacfg* | ptr_pbcfg | Protocol and port based group database entry.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_OUT_OF_RANGE | input out of range.
@comm
    This API can set protocol and port based group database.
    System supported only 4 entries and 3 types of frame format. Supported frame types are defined 
    as Ethernet (frame type = 0b00, Ether type > 0x05FF), RFC 1042 (frame type = 0b01,6 bytes after 
    Type/Length = AA-AA-03-00-00-00) and LLC other(frame type = 0b10). ASIC has available setting of
    each frame type per port and available system setting each defined frame type. If per system 
    frame type is set to invalid, then per port frame setting is take no effect. There is contained 
    valid bit setting in each group database.    
*/
ret_t rtl8370_setAsicVlanProtocolBasedGroupData(uint32 index, rtl8370_protocolgdatacfg *ptr_pbcfg)
{
    uint32  frame_type;
    uint32  ether_type;
    ret_t    retVal;

    /* Error Checking */
    if(index > RTL8370_PROTOVLAN_GIDX_MAX)
        return RT_ERR_OUT_OF_RANGE;

    if(NULL == ptr_pbcfg)
        return RT_ERR_INPUT;

    if(ptr_pbcfg->frame_type >= PPVLAN_FRAME_TYPE_MAX_BOUND )
        return RT_ERR_INPUT;

    if((ptr_pbcfg->frame_type==PPVLAN_FRAME_TYPE_ETHERNET)&&(ptr_pbcfg->ether_type<0x0600)&&(ptr_pbcfg->ether_type!=0))
        return RT_ERR_INPUT;    

    frame_type = ptr_pbcfg->frame_type;
    ether_type = ptr_pbcfg->ether_type;

    /* Frame type */
    retVal = rtl8370_setAsicRegBits(RTL8370SG_VLAN_PPB_FRAMETYPE_REG(index), RTL8370SG_VLAN_PPB_FRAMETYPE_MASK, frame_type);
    if(retVal != RT_ERR_OK)
        return retVal;
    
    /* Ether type */
    retVal = rtl8370_setAsicReg(RTL8370_VLAN_PPB_ETHERTYPR_REG(index), ether_type);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_getAsicVlanProtocolBasedGroupData | Get protocol and port based group database.
@parm uint32 | index | Index of protocol and port based database index (0~7).
@parm rtl8370_protocolgdatacfg* | ptr_pbcfg | Protocol and port based group database entry.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_OUT_OF_RANGE | input out of range.
@comm
    This API can get protocol and port based group database.
    System supported only 4 entries and 3 types of frame format. Supported frame types are defined 
    as Ethernet (frame type = 0b00, Ether type > 0x05FF), RFC 1042 (frame type = 0b01,6 bytes after 
    Type/Length = AA-AA-03-00-00-00) and LLC other(frame type = 0b10). ASIC has available setting of
    each frame type per port and available system setting each defined frame type. If per system 
    frame type is set to invalid, then per port frame setting is take no effect. There is contained 
    valid bit setting in each group database.    
*/
ret_t rtl8370_getAsicVlanProtocolBasedGroupData(uint32 index, rtl8370_protocolgdatacfg *ptr_pbcfg)
{
    uint32  frame_type;
    uint32  ether_type;
    ret_t    retVal;

    /* Error Checking */
    if(index > RTL8370_PROTOVLAN_GIDX_MAX)
        return RT_ERR_OUT_OF_RANGE;

    if(NULL == ptr_pbcfg)
        return RT_ERR_INPUT;

    /* Read Frame type */
    retVal = rtl8370_getAsicRegBits(RTL8370SG_VLAN_PPB_FRAMETYPE_REG(index), RTL8370SG_VLAN_PPB_FRAMETYPE_MASK, &frame_type);
    if(retVal != RT_ERR_OK)
        return retVal;

    /* Read Ether type */
    retVal = rtl8370_getAsicReg(RTL8370_VLAN_PPB_ETHERTYPR_REG(index), &ether_type);
    if(retVal != RT_ERR_OK)
        return retVal;


    ptr_pbcfg->frame_type = frame_type;
    ptr_pbcfg->ether_type = ether_type;
    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_setAsicVlanPortAndProtocolBased | Set protocol and port based VLAN configuration. 
@parm uint32 | port | Physical port number (0~15).
@parm uint32 | index | Index of protocol and port based database index.
@parm rtl8370_protocolvlancfg* | ptr_ppbcfg | Protocol and port based VLAN configuration.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_OUT_OF_RANGE | input out of range.
@comm
    Each port has 4 VLAN configurations for each protocol and port based group database. 
    Protocol and port based VLAN configuration contained 1 valid bit setting for each 
    group database entry. There is 802.1q priority field setting for each group database 
    entry. Different with port based VLAN information retrieving, ASIC decided 802.1q priority 
    of reveiving frame from dedicated port based VLAN configuration and didn't decide from 
    priority field of system VLAN 16 member configurations.
    
*/
ret_t rtl8370_setAsicVlanPortAndProtocolBased(uint32 port, uint32 index, rtl8370_protocolvlancfg *ptr_ppbcfg)
{
    uint32  reg_addr, bit_mask, bit_value;
    ret_t   retVal;

    /* Error Checking */
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(index > RTL8370_PROTOVLAN_GIDX_MAX)
        return RT_ERR_OUT_OF_RANGE;

    if(NULL == ptr_ppbcfg)
        return RT_ERR_INPUT;

    if( (ptr_ppbcfg->valid != FALSE) && (ptr_ppbcfg->valid != TRUE) )
        return RT_ERR_INPUT;

    if(ptr_ppbcfg->vlan_idx > RTL8370_CVIDXMAX)
        return RT_ERR_VLAN_ENTRY_NOT_FOUND;

    if(ptr_ppbcfg->priority > RTL8370_PRIMAX)
        return RT_ERR_VLAN_PRIORITY;

    /* Valid bit */
    reg_addr  = RTL8370_VLAN_PPB_VALID_REG(index);
    bit_mask  = 0x0001 << port;
    bit_value = ((ptr_ppbcfg->valid == TRUE) ? 0x1 : 0x0);
    retVal    = rtl8370_setAsicRegBits(reg_addr, bit_mask, bit_value);
    if(retVal != RT_ERR_OK)
        return retVal;

    /* Calculate the actual register address for CVLAN index*/
    reg_addr = RTL8370_VLAN_PPB_CTRL_REG(index,port);
    bit_mask = RTL8370_VLAN_PPB_CTRL_MASK(port);
    bit_value = ptr_ppbcfg->vlan_idx;
    
       retVal    = rtl8370_setAsicRegBits(reg_addr, bit_mask, bit_value);
    if(retVal != RT_ERR_OK)
        return retVal;

    /* write priority */
    reg_addr  = RTL8370_VLAN_PPB_PRIORITY_ITEM_REG(port,index);
    bit_mask  = RTL8370_VLAN_PPB_PRIORITY_ITEM_MASK(port);
    bit_value = ptr_ppbcfg->priority;
    retVal    = rtl8370_setAsicRegBits(reg_addr, bit_mask, bit_value);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_getAsicVlanPortAndProtocolBased | Get protocol and port based VLAN configuration. 
@parm uint32 | port | Physical port number (0~15).
@parm uint32 | index | Index of protocol and port based database index.
@parm rtl8370_protocolvlancfg* | ptr_ppbcfg | Protocol and port based VLAN configuration.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_OUT_OF_RANGE | input out of range.
@comm
    Each port has 4 VLAN configurations for each protocol and port based group database. 
    Protocol and port based VLAN configuration contained 1 valid bit setting for each 
    group database entry. There is 802.1q priority field setting for each group database 
    entry. Different with port based VLAN information retrieving, ASIC decided 802.1q priority 
    of reveiving frame from dedicated port based VLAN configuration and didn't decide from 
    priority field of system VLAN 16 member configurations.
    
*/
ret_t rtl8370_getAsicVlanPortAndProtocolBased(uint32 port, uint32 index, rtl8370_protocolvlancfg *ptr_ppbcfg)
{
    uint32  reg_addr, bit_mask, bit_value;
    ret_t   retVal;

    /* Error Checking */
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(index > RTL8370_PROTOVLAN_GIDX_MAX)
        return RT_ERR_OUT_OF_RANGE;

    if(NULL == ptr_ppbcfg)
        return RT_ERR_INPUT;

    /* Valid bit */
    reg_addr  = RTL8370_VLAN_PPB_VALID_REG(index);
    bit_mask  = 0x0001 << port;
    retVal    = rtl8370_getAsicRegBits(reg_addr, bit_mask, &bit_value);
    if(retVal != RT_ERR_OK)
        return retVal;

    ptr_ppbcfg->valid = bit_value;

    /* CVLAN index */
    reg_addr = RTL8370_VLAN_PPB_CTRL_REG(index,port);
    bit_mask = RTL8370_VLAN_PPB_CTRL_MASK(port);
    retVal = rtl8370_getAsicRegBits(reg_addr, bit_mask, &bit_value);
    if(retVal != RT_ERR_OK)
        return retVal;

    ptr_ppbcfg->vlan_idx = bit_value;
    

    /* priority */
    reg_addr = RTL8370_VLAN_PPB_PRIORITY_ITEM_REG(port,index);
    bit_mask = RTL8370_VLAN_PPB_PRIORITY_ITEM_MASK(port);
    retVal = rtl8370_getAsicRegBits(reg_addr, bit_mask, &bit_value);
    if(retVal != RT_ERR_OK)
        return retVal;

    ptr_ppbcfg->priority = bit_value;
    return RT_ERR_OK;
}
/*
@func ret_t | rtl8370_setAsicVlanFilter | Configure enable CVLAN filtering function
@parm uint32 | enabled | CVLAN filtering function 1: enabled, 0: disabled. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_ENABLE | Invalid enable parameter.
@comm
     The API can set CVLAN both egress/ingress filtering enable function.
*/
ret_t rtl8370_setAsicVlanFilter(uint32 enabled)
{
    if ((enabled != 0) && (enabled != 1))
        return RT_ERR_ENABLE; 
	
    return rtl8370_setAsicRegBit(RTL8370_VLAN_FILTERING_REG, RTL8370_VLAN_FILTERING_OFFSET, enabled);
}
/*
@func ret_t | rtl8370_getAsicVlanFilter | Configure enable CVLAN filtering function
@parm uint32* | enabled | CVLAN filtering function 1: enabled, 0: disabled. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
     The API can get CVLAN both egress/ingress filtering enable function.
*/
ret_t rtl8370_getAsicVlanFilter(uint32* enabled)
{
   	return rtl8370_getAsicRegBit(RTL8370_VLAN_FILTERING_REG, RTL8370_VLAN_FILTERING_OFFSET, enabled);
}

/*
@func ret_t | rtl8370_setAsicPortBasedFid | Set port based FID
@parm uint32 | port | Physical port number (0~15).
@parm uint32 | fid | Port based fid(0~0xFFF)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_L2_FID | Invalid FID (0~4095).
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    This API can set Port-Based FID.There are three FID sources of receiving packet.
    ASIC will follow Port-base FID > SVLAN FID > VLAN based FID to decide FID belong packet.
*/    
ret_t rtl8370_setAsicPortBasedFid(uint32 port, uint32 fid)
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(fid > RTL8370_FIDMAX)
        return RT_ERR_L2_FID;

    return rtl8370_setAsicReg(RTL8370_PORT_PBFID_REG(port),fid);
}

/*
@func ret_t | rtl8370_getAsicPortBasedFid | Get port based FID
@parm uint32 | port | Physical port number (0~15).
@parm uint32* | fid | Port based fid(0~0xFFF)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_L2_FID | Invalid FID (0~4095).
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    This API can get Port-Based FID.There are three FID sources of receiving packet.
*/    
ret_t rtl8370_getAsicPortBasedFid(uint32 port, uint32* fid)
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicReg(RTL8370_PORT_PBFID_REG(port),fid);
}

/*
@func ret_t | rtl8370_setAsicPortBasedFidEn | Set port based FID selection enable
@parm uint32 | port | Physical port number (0~15).
@parm uint32 | enabled | Port based fid selection enable setting 1:enable 0:disable
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_ENABLE | Invalid enable parameter.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
     This API can set Port-Based FID decision function.
*/    
ret_t rtl8370_setAsicPortBasedFidEn(uint32 port, uint32 enabled)
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;
	
    if((enabled != 0) && (enabled != 1))
        return RT_ERR_ENABLE; 
	
    return rtl8370_setAsicRegBit(RTL8370_REG_PORT_PBFIDEN,port, enabled);
}
/*
@func ret_t | rtl8370_getAsicPortBasedFidEn | Get port based FID selection enable
@parm uint32 | port | Physical port number (0~15).
@parm uint32* | enabled | Port based fid selection enable setting 1:enable 0:disable
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
     This API can get Port-Based FID decision function.
*/    
ret_t rtl8370_getAsicPortBasedFidEn(uint32 port, uint32* enabled)
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBit(RTL8370_REG_PORT_PBFIDEN,port, enabled);
}

/*
@func ret_t | rtl8370_setAsicSpanningTreeStatus | Configure spanning tree state per each port.
@parm uint32 | port | The port number
@parm uint32 | msti | Multiple spanning tree instance (0~15).
@parm uint32 | state | Spanning tree state for msti
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_MSTI | Invalid msti parameter
@rvalue RT_ERR_MSTP_STATE | Invalid STP state.
@common
    System supports per-port multiple spanning tree state for each msti. There are four states supported by ASIC.

    Disable state         ASIC did not receive and transmit packets at port with disable state.
    Blocking state        ASIC will receive BPDUs without L2 auto learning and does not transmit packet out of port in blocking state.
    Learning state        ASIC will receive packets with L2 auto learning and transmit out BPDUs only.
    Forwarding state    The port will receive and transmit packets normally.
*/
ret_t rtl8370_setAsicSpanningTreeStatus(uint32 port, uint32 msti, uint32 state)
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(msti > RTL8370_MSTIMAX)
        return RT_ERR_MSTI;

    if(state > STPST_FORWARDING)
        return RT_ERR_MSTP_STATE;

    return rtl8370_setAsicRegBits(RTL8370_VLAN_MSTI_REG(msti,port), RTL8370_VLAN_MSTI_MASK(port),state);
}

/*
@func ret_t | rtl8370_getAsicSpanningTreeStatus | Configure spanning tree state per each port.
@parm uint32 | port | The port number
@parm uint32 | msti | Multiple spanning tree instance (0~15).
@parm uint32* | state | Spanning tree state for msti
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_MSTI | Invalid msti parameter
@rvalue RT_ERR_MSTP_STATE | Invalid STP state.
@common
    System supports per-port multiple spanning tree state for each msti. There are four states supported by ASIC.
*/
ret_t rtl8370_getAsicSpanningTreeStatus(uint32 port, uint32 msti, uint32* state)
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(msti > RTL8370_MSTIMAX)
        return RT_ERR_MSTI;


    return rtl8370_getAsicRegBits(RTL8370_VLAN_MSTI_REG(msti,port), RTL8370_VLAN_MSTI_MASK(port),state);
}



