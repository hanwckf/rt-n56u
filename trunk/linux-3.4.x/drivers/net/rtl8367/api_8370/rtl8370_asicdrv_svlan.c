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
 * $Date: 2010/12/02 04:34:28 $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367B
 * Feature : 
 *
 */
#include "rtl8370_asicdrv_svlan.h"

void _rtl8370_svlanConfStUser2Smi( rtl8370_svlan_memconf_t *stUser, rtl8370_svlan_memconf_smi_t *stSmi);
void _rtl8370_svlanConfStSmi2User( rtl8370_svlan_memconf_t *stUser, rtl8370_svlan_memconf_smi_t *stSmi);
void _rtl8370_svlanMc2sStUser2Smi( rtl8370_svlan_mc2s_t *stUser, rtl8370_svlan_mc2s_smi_t *stSmi);
void _rtl8370_svlanMc2sStSmi2User( rtl8370_svlan_mc2s_t *stUser, rtl8370_svlan_mc2s_smi_t *stSmi);
void _rtl8370_svlanSp2cStUser2Smi( rtl8370_svlan_s2c_t *stUser, rtl8370_svlan_s2c_smi_t *stSmi);
void _rtl8370_svlanSp2cStSmi2User( rtl8370_svlan_s2c_t *stUser, rtl8370_svlan_s2c_smi_t *stSmi);

/*
@func ret_t | rtl8370_setAsicSvlanUplinkPortMask | Configure uplink ports mask.
@parm uint32 | portMask | Uplink port mask setting.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_MASK | Invalid portmask.
@comm
    Uplink port mask is setting which ports are connected to provider switch. If ports are belong uplink ports and all frames receiving from these port must 
    contain accept SVID in S-tag field.
*/
ret_t rtl8370_setAsicSvlanUplinkPortMask(uint32 portMask)
{
    if(portMask > RTL8370_PORTMASK)
        return RT_ERR_PORT_MASK;
    
    return rtl8370_setAsicReg(RTL8370_REG_SVLAN_UPLINK_PORTMASK,portMask);
}

/*
@func ret_t | rtl8370_getAsicSvlanUplinkPortMask | Get uplink ports mask configuration.
@parm uint32* | portMask | Uplink port mask setting.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    The API can get setting of  uplink port mask belong to service provider VLAN ports.
    
*/
ret_t rtl8370_getAsicSvlanUplinkPortMask(uint32* portMask)
{
    return rtl8370_getAsicReg(RTL8370_REG_SVLAN_UPLINK_PORTMASK,portMask);
}

/*
@func ret_t | rtl8370_setAsicSvlanTpid | Configure accepted S-VLAN ether type. The default ether type of S-VLAN is 0x88a8.
@parm uint32 | protocolType | Ether type of S-tag frame parsing in uplink ports.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 and 0x9200 for Q-in-Q SLAN design. User can set mathced ether
    type as service provider supported protocol. 
*/
ret_t rtl8370_setAsicSvlanTpid(uint32 protocolType)
{
    return rtl8370_setAsicReg(RTL8370_REG_VS_TPID,protocolType);
}

/*
@func ret_t | rtl8370_getAsicSvlanTpid | Get accepted S-VLAN ether type setting.
@parm uint32 | protocolType |  Ether type of S-tag frame parsing in uplink ports.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    The API can get ccepted ether type of service provider VLAN tag. The default ether type of service provider VLAN in 802.1ad is 0x88a8.
*/
ret_t rtl8370_getAsicSvlanTpid(uint32* protocolType)
{
    return rtl8370_getAsicReg(RTL8370_REG_VS_TPID, protocolType);
}
/*
@func ret_t | rtl8370_setAsicSvlanPrioritySel | SVLAN priority field setting
@parm uint32 | prisel | S-priority assignment method, 0:internal priority 1:C-tag priority 2:using Svlan member configuration
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    The API can set S-priority assignment method for S-tag frame output from uplink port 
*/
ret_t rtl8370_setAsicSvlanPrioritySel(uint32 prisel)
{
    if(prisel >= SPRISEL_MAX)
        return RT_ERR_INPUT;
    
    return rtl8370_setAsicRegBits(RTL8370_REG_SVLAN_CFG, RTL8370_VS_SPRISEL_MASK, prisel);
}
/*
@func ret_t | rtl8370_getAsicSvlanPrioritySel | SVLAN priority field setting
@parm uint32* | prisel | S-priority assignment method, 0:internal priority 1:C-tag priority 2:using Svlan member configuration
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get S-priority assignment method for S-tag frame output from uplink port 
*/
ret_t rtl8370_getAsicSvlanPrioritySel(uint32* prisel)
{
    return rtl8370_getAsicRegBits(RTL8370_REG_SVLAN_CFG, RTL8370_VS_SPRISEL_MASK, prisel);
}
/*
@func ret_t | rtl8370_setAsicSvlanTrapPriority | Trap to CPU priority assignment
@parm uint32 | priority | Priority assignment
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    The API can set trapping priority to packet from uplink port.

*/
ret_t rtl8370_setAsicSvlanTrapPriority(uint32 priority)
{
    if(priority > RTL8370_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY; 

    return rtl8370_setAsicRegBits(RTL8370_REG_QOS_TRAP_PRIORITY0, RTL8370_SVLAN_PRIOIRTY_MASK,priority);
}

/*
@func ret_t | rtl8370_getAsicSvlanTrapPriority | Get trapping to CPU priority assignment
@parm uint32* | priority | Priority assignment
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    The API can get trapping priority to packet from uplink port.

*/
ret_t rtl8370_getAsicSvlanTrapPriority(uint32* priority)
{
    return rtl8370_getAsicRegBits(RTL8370_REG_QOS_TRAP_PRIORITY0, RTL8370_SVLAN_PRIOIRTY_MASK,priority);
}
/*
@func ret_t | rtl8370_setAsicSvlanDefaultVlan | Configure default egress SVLAN.
@parm uint32 | index | index SVLAN member configuration
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_SVLAN_ENTRY_INDEX | Invalid input SVLAN index.
@comm
    The API can set port n S-tag format index while receiving frame from port n 
    is transmit through uplink port with s-tag field

*/
ret_t rtl8370_setAsicSvlanDefaultVlan(uint32 index)
{
    if(index > RTL8370_SVIDXMAX)
        return RT_ERR_SVLAN_ENTRY_INDEX;

    return rtl8370_setAsicRegBits(RTL8370_REG_SVLAN_CFG, RTL8370_VS_CPSVIDX_MASK,index);        
}

/*
@func ret_t | rtl8370_getAsicSvlanDefaultVlan | Get the configure default egress SVLAN.
@parm uint32* | index | index SVLAN member configuration
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    The API can get port n S-tag format index while receiving frame from port n 
    is transmit through uplink port with s-tag field

*/
ret_t rtl8370_getAsicSvlanDefaultVlan(uint32* index)
{
    return rtl8370_getAsicRegBits(RTL8370_REG_SVLAN_CFG, RTL8370_VS_CPSVIDX_MASK,index);        
}
/*
@func ret_t | rtl8370_setAsicSvlanIngressUntag | Configure enable trap received un-Stag frame to CPU from unplink port/
@parm uint32 | enabled | Trap received un-Stag frame to CPU 1: enabled, 0: disabled. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_ENABLE | Invalid enable parameter.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
     The API can set trapping function in uplink port while receiving un-Stagging frame.
*/
ret_t rtl8370_setAsicSvlanIngressUntag(uint32 enabled)
{
    if((enabled != 0) && (enabled != 1))
        return RT_ERR_ENABLE; 


    return rtl8370_setAsicRegBit(RTL8370_REG_SVLAN_CFG, RTL8370_VS_UNTAG_OFFSET, enabled);
}

/*
@func ret_t | rtl8370_getAsicSvlanIngressUntag | Configure enable trap received un-Stag frame to CPU from unplink port.
@parm uint32* | enabled | Trap received un-Stag frame to CPU 1: enabled, 0: disabled. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    The API can get configuration of trapping function in uplink port while receiving un-Stagging frame.
    
*/
ret_t rtl8370_getAsicSvlanIngressUntag(uint32* enabled)
{
    return rtl8370_getAsicRegBit(RTL8370_REG_SVLAN_CFG, RTL8370_VS_UNTAG_OFFSET, enabled);
}
/*
@func ret_t | rtl8370_setAsicSvlanIngressUnmatch | Configure enable trap received unmatched Stag frame to CPU from unplink port.
@parm uint32 | enabled | Trap received unmatched Stag frame to CPU 1: enabled, 0: disabled. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_ENABLE | Invalid enable parameter.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
     The API can set trapping function in uplink port while receiving unmatched Stag frame.
*/
ret_t rtl8370_setAsicSvlanIngressUnmatch(uint32 enabled)
{
    if((enabled != 0) && (enabled != 1))
        return RT_ERR_ENABLE; 

    return rtl8370_setAsicRegBit(RTL8370_REG_SVLAN_CFG, RTL8370_VS_UNMAT_OFFSET, enabled);
}

/*
@func ret_t | rtl8370_getAsicSvlanIngressUnmatch | Configure enable trap received unmatched Stag frame to CPU from unplink port.
@parm uint32* | enabled | Trap received unmatched Stag frame to CPU 1: enabled, 0: disabled. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    The API can get configuration of trapping function in uplink port while receiving unmatched Stag frame.
    
*/
ret_t rtl8370_getAsicSvlanIngressUnmatch(uint32* enabled)
{
    return rtl8370_getAsicRegBit(RTL8370_REG_SVLAN_CFG, RTL8370_VS_UNMAT_OFFSET, enabled);
}

/*
@func ret_t | rtl8370_setAsicSvlanEgressUnassign | Configure unplink stream without egress SVID action.
@parm uint32 | enabled | 1:Trap egress unassigned frames to CPU, 0: Use SVLAN setup in VS_CPSVIDX as egress SVID. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_ENABLE | Invalid enable parameter.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
     The API can set trapping function in uplink port while the egress frame is unassigned.
*/
ret_t rtl8370_setAsicSvlanEgressUnassign(uint32 enabled)
{
    if((enabled != 0) && (enabled != 1))
        return RT_ERR_ENABLE; 

    return rtl8370_setAsicRegBit(RTL8370_REG_SVLAN_CFG, RTL8370_VS_UIFSEG_OFFSET, enabled);
}

/*
@func ret_t | rtl8370_getAsicSvlanEgressUnassign | Configure unplink stream without egress SVID action.
@parm uint32 | enabled | 1:Trap egress unassigned frames to CPU, 0: Use SVLAN setup in VS_CPSVIDX as egress SVID. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
     The API can set trapping function in uplink port while the egress frame is unassigned.
     
*/
ret_t rtl8370_getAsicSvlanEgressUnassign(uint32* enabled)
{
    return rtl8370_getAsicRegBit(RTL8370_REG_SVLAN_CFG, RTL8370_VS_UIFSEG_OFFSET, enabled);
}

void _rtl8370_svlanConfStUser2Smi( rtl8370_svlan_memconf_t *stUser, rtl8370_svlan_memconf_smi_t *stSmi)
{
    stSmi->vs_relaysvid = stUser->vs_relaysvid;
    stSmi->vs_msti = stUser->vs_msti;
    
    stSmi->vs_member = stUser->vs_member;

    stSmi->vs_fid = stUser->vs_fid;

    stSmi->vs_priority = stUser->vs_priority;

    stSmi->vs_svid= stUser->vs_svid;
    stSmi->vs_efiden= stUser->vs_efiden;
    stSmi->vs_efid= stUser->vs_efid;
}
void _rtl8370_svlanConfStSmi2User( rtl8370_svlan_memconf_t *stUser, rtl8370_svlan_memconf_smi_t *stSmi)
{
    stUser->vs_relaysvid = stSmi->vs_relaysvid;
    stUser->vs_msti = stSmi->vs_msti;
    stUser->vs_fid = stSmi->vs_fid;

    stUser->vs_member = stSmi->vs_member;

    stUser->vs_priority = stSmi->vs_priority;

    stUser->vs_svid = stSmi->vs_svid;

    stUser->vs_efiden = stSmi->vs_efiden;
    stUser->vs_efid = stSmi->vs_efid;
}
/*
@func ret_t | rtl8370_setAsicSvlanMemberConfiguration| Configure system 64 S-tag content
@parm uint32 | index | index of 64 s-tag configuration
@parm rtl8370_svlan_memconf_t* | svlanMemConf | SVLAN member configuration
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_SVLAN_ENTRY_INDEX | Invalid input SVLAN index parameter.
@comm
    The API can set system 64 accepted s-tag frame format. Only 64 SVID S-tag frame will be accpeted
    to receiving from uplink ports. Other SVID S-tag frame or S-untagged frame will be droped.

*/
ret_t rtl8370_setAsicSvlanMemberConfiguration(uint32 index,rtl8370_svlan_memconf_t* svlanMemConf)
{
    ret_t retVal;
    uint32 regData;
    uint16 *accessPtr;
    uint32 i;

    rtl8370_svlan_memconf_smi_t smiSvlanMemConf;

    if(index > RTL8370_SVIDXMAX)
        return RT_ERR_SVLAN_ENTRY_INDEX;

    memset(&smiSvlanMemConf,0x00,sizeof(smiSvlanMemConf));
    _rtl8370_svlanConfStUser2Smi(svlanMemConf,&smiSvlanMemConf);

    accessPtr =  (uint16*)&smiSvlanMemConf;

    
    regData = *accessPtr;
    for(i = 0; i < 4; i++)
    {
        retVal = rtl8370_setAsicReg(RTL8370_SVLAN_MEMBERCFG_BASE_REG+(index<<2)+i,regData);
        if(retVal !=  RT_ERR_OK)
            return retVal;

        accessPtr ++;
        regData = *accessPtr;
    }
    
    
    return retVal;  
}   

/*
@func ret_t | rtl8370_getAsicSvlanMemberConfiguration| Get SVLAN member Configure.
@parm uint32 | index | index of 8 s-tag configuration
@parm rtl8370_svlan_memconf_t* | svlanMemConf | SVLAN member configuration
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_SVLAN_ENTRY_INDEX | Invalid SVLAN configuration index.
@comm
    The API can get system 64 accepted s-tag frame format. Only 64 SVID S-tag frame will be accpeted
    to receiving from uplink ports. Other SVID S-tag frame or S-untagged frame will be droped.

*/
ret_t rtl8370_getAsicSvlanMemberConfiguration(uint32 index,rtl8370_svlan_memconf_t* svlanMemConf)
{
    ret_t retVal;
    uint32 regData;
    uint16 *accessPtr;
    uint32 i;

    rtl8370_svlan_memconf_smi_t smiSvlanMemConf;

    if(index > RTL8370_SVIDXMAX)
        return RT_ERR_SVLAN_ENTRY_INDEX;

    memset(&smiSvlanMemConf,0x00,sizeof(smiSvlanMemConf));

    accessPtr = (uint16*)&smiSvlanMemConf;

    for(i = 0; i < 4; i++)
    {
        retVal = rtl8370_getAsicReg(RTL8370_SVLAN_MEMBERCFG_BASE_REG+(index<<2)+i,&regData);
        if(retVal !=  RT_ERR_OK)
            return retVal;

        *accessPtr = regData;

        accessPtr ++;
    }


    _rtl8370_svlanConfStSmi2User(svlanMemConf,&smiSvlanMemConf);

    return RT_ERR_OK;
}


/*
@func ret_t | rtl8370_setAsicSvlanC2SConf| Configure SVLAN C2S table 
@parm uint32 | index | index of 128 Svlan C2S configuration
@parm uint32 | evid | Enhanced VID
@parm uint32 | pmsk | available c2s port mask
@parm uint32 | svidx | index of 64 Svlan member configuration
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_VLAN_VID | Invalid vid
@rvalue RT_ERR_SVLAN_ENTRY_INDEX | Invalid SVLAN configuration index
@rvalue RT_ERR_PORT_MASK | Invalid portmask.
@rvalue RT_ERR_OUT_OF_RANGE | input out of range.
@comm
    The API can set system 128 C2S configuration. ASIC will check upstream's VID and assign related
    SVID to mathed packet.
    

*/
ret_t rtl8370_setAsicSvlanC2SConf(uint32 index,uint32 evid, uint32 pmsk, uint32 svidx)
{
    ret_t retVal;
    uint32 regData;
    uint16 *accessPtr;
    uint32 i;
    rtl8370_svlan_c2s_smi_t smiSvlanC2SConf;

    if(index > RTL8370_C2SIDXMAX)
        return RT_ERR_OUT_OF_RANGE;

    if(evid > RTL8370_EVIDMAX)
        return RT_ERR_VLAN_VID;

    if(pmsk > RTL8370_PORTMASK)
        return RT_ERR_PORT_MASK;

    if(svidx > RTL8370_SVIDXMAX)
        return RT_ERR_SVLAN_ENTRY_INDEX;    

    memset(&smiSvlanC2SConf, 0x00, sizeof(smiSvlanC2SConf));
    
    smiSvlanC2SConf.svidx = svidx;

    smiSvlanC2SConf.c2senPmsk= pmsk;

    smiSvlanC2SConf.evid = evid;

    
    accessPtr =  (uint16*)&smiSvlanC2SConf;

    
    regData = *accessPtr;
    for(i = 0; i < 3; i++)
    {
        retVal = rtl8370_setAsicReg(RTL8370SG_SVLAN_C2SCFG_BASE_REG+(index*3)+i,regData);
        if(retVal !=  RT_ERR_OK)
            return retVal;

        accessPtr ++;
        regData = *accessPtr;
    }
    
    
    return retVal;  
}   

/*
@func ret_t | rtl8370_getAsicSvlanC2SConf| Get configure SVLAN C2S table 
@parm uint32 | index | index of 128 Svlan C2S configuration
@parm uint32* | evid | Enhanced VID
@parm uint32* | pmsk | available c2s port mask
@parm uint32* | svidx | index of 64 Svlan member configuration
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_OUT_OF_RANGE | input out of range.
@comm
    The API can get system 128 C2S configuration. ASIC will check upstream's VID and assign related
    SVID to mathed packet.
    

*/
ret_t rtl8370_getAsicSvlanC2SConf(uint32 index,uint32* evid, uint32* pmsk, uint32* svidx)
{
    ret_t retVal;
    uint32 regData;
    uint16 *accessPtr;
    uint32 i;

    rtl8370_svlan_c2s_smi_t smiSvlanC2SConf;

    if(index > RTL8370_C2SIDXMAX)
        return RT_ERR_OUT_OF_RANGE;

    memset(&smiSvlanC2SConf,0x00,sizeof(smiSvlanC2SConf));


    accessPtr = (uint16*)&smiSvlanC2SConf;

    for(i = 0; i < 3; i++)
    {
        retVal = rtl8370_getAsicReg(RTL8370SG_SVLAN_C2SCFG_BASE_REG+(index*3)+i,&regData);
        if(retVal !=  RT_ERR_OK)
            return retVal;

        *accessPtr = regData;

        accessPtr ++;
    }

    *svidx = smiSvlanC2SConf.svidx;
    
    *pmsk= smiSvlanC2SConf.c2senPmsk;
    
    *evid = smiSvlanC2SConf.evid;
    
    return retVal;  
}   

void _rtl8370_svlanMc2sStUser2Smi( rtl8370_svlan_mc2s_t *stUser, rtl8370_svlan_mc2s_smi_t *stSmi)
{
   uint32 mask;
   uint32 regData;
    
    stSmi->svidx= stUser->svidx;
    
    mask = ntohl(stUser->mask);
    stSmi->mask0 = mask & 0x000000FF;
    stSmi->mask1 = (mask&0x0000FF00)>>8;
    stSmi->mask2 = (mask&0x00FF0000)>>16;
    stSmi->mask3 = (mask&0xFF000000)>>24;

    regData = ntohl(stUser->value);
    stSmi->data0 = regData & 0x000000FF;
    stSmi->data1 = (regData&0x0000FF00)>>8;
    stSmi->data2 = (regData&0x00FF0000)>>16;
    stSmi->data3 = (regData&0xFF000000)>>24;   
    
    stSmi->format = stUser->format;

    stSmi->valid = stUser->valid;
}
void _rtl8370_svlanMc2sStSmi2User( rtl8370_svlan_mc2s_t *stUser, rtl8370_svlan_mc2s_smi_t *stSmi)
{
    stUser->svidx = stSmi->svidx;

    stUser->mask = (stSmi->mask3 << 24) | (stSmi->mask2 << 16) | (stSmi->mask1 << 8) | stSmi->mask0;
    stUser->mask = htonl(stUser->mask);
    
    stUser->value = (stSmi->data3 << 24) | (stSmi->data2 << 16) | (stSmi->data1 << 8) | stSmi->data0;
    stUser->value = htonl(stUser->value);

    stUser->format = stSmi->format;
    
    stUser->valid = stSmi->valid;
}
/*
@func ret_t | rtl8370_setAsicSvlanMC2SConf| Configure system 32 S-tag content
@parm uint32 | index | index of 32 Multicast to SVLAN configuration
@parm rtl8370_svlan_mc2s_t* | svlanMC2SConf | SVLAN Multicast to SVLAN member configuration
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@rvalue RT_ERR_OUT_OF_RANGE | input out of range.
@comm
    The API can set system 32 Mutlicast to SVID configuration. If upstream packet is L2 multicast or IPv4 multicast
    packet and DMAC/DIP is matched MC2S configuration, ASIC will assign egress SVID to the packet.

*/
ret_t rtl8370_setAsicSvlanMC2SConf(uint32 index,rtl8370_svlan_mc2s_t* svlanMC2SConf)
{
    ret_t retVal;
    uint32 regData;
    uint16 *accessPtr;
    uint32 i;

    rtl8370_svlan_mc2s_smi_t smiSvlanMC2S;

    if(index > RTL8370_MC2SIDXMAX)
        return RT_ERR_OUT_OF_RANGE;

    memset(&smiSvlanMC2S,0x00,sizeof(smiSvlanMC2S));
    _rtl8370_svlanMc2sStUser2Smi(svlanMC2SConf,&smiSvlanMC2S);

    accessPtr =  (uint16*)&smiSvlanMC2S;

    
    regData = *accessPtr;
    for(i = 0; i < 5; i++)
    {
        retVal = rtl8370_setAsicReg(RTL8370SG_SVLAN_MCAST2S_ENTRY_BASE_REG+(index*5)+i,regData);
        if(retVal !=  RT_ERR_OK)
            return retVal;

        accessPtr ++;
        regData = *accessPtr;
    }
    
    
    return retVal;  
}   

/*
@func ret_t | rtl8370_getAsicSvlanMC2SConf| Get configure system 32 S-tag content
@parm uint32 | index | index of 32 Multicast to SVLAN configuration
@parm rtl8370_svlan_mc2s_t* | svlanMC2SConf | SVLAN Multicast to SVLAN member configuration
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    The API can get system 32 Mutlicast to SVID configuration. If upstream packet is L2 multicast or IPv4 multicast
    packet and DMAC/DIP is matched MC2S configuration, ASIC will assign egress SVID to the packet.

*/
ret_t rtl8370_getAsicSvlanMC2SConf(uint32 index,rtl8370_svlan_mc2s_t* svlanMC2SConf)
{
    ret_t retVal;
    uint32 regData;
    uint16 *accessPtr;
    uint32 i;

    rtl8370_svlan_mc2s_smi_t smiSvlanMC2S;

    if(index > RTL8370_MC2SIDXMAX)
        return RT_ERR_OUT_OF_RANGE;

    memset(&smiSvlanMC2S, 0x00, sizeof(smiSvlanMC2S));

    accessPtr = (uint16*)&smiSvlanMC2S;

    for(i = 0; i < 5; i++)
    {
        retVal = rtl8370_getAsicReg(RTL8370SG_SVLAN_MCAST2S_ENTRY_BASE_REG+(index*5)+i,&regData);
        if(retVal !=  RT_ERR_OK)
            return retVal;

        *accessPtr = regData;

        accessPtr ++;
    }


    _rtl8370_svlanMc2sStSmi2User(svlanMC2SConf,&smiSvlanMC2S);

    return RT_ERR_OK;
}

void _rtl8370_svlanSp2cStUser2Smi( rtl8370_svlan_s2c_t *stUser, rtl8370_svlan_s2c_smi_t *stSmi)
{
    stSmi->svid = stUser->svid;
    stSmi->dstport = stUser->dstport;       
    stSmi->evid = stUser->evid;
}
void _rtl8370_svlanSp2cStSmi2User( rtl8370_svlan_s2c_t *stUser, rtl8370_svlan_s2c_smi_t *stSmi)
{
    stUser->svid = stSmi->svid;
    stUser->dstport = stSmi->dstport;       
    stUser->evid = stSmi->evid;
}
/*
@func ret_t | rtl8370_setAsicSvlanSP2CConf| Configure system 128 SP2C content
@parm uint32 | index | index of 128 SVLAN & Port to CVLAN configuration
@parm rtl8370_svlan_s2c_t* | svlanSP2CConf | SVLAN & Port to CVLAN configuration
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@rvalue RT_ERR_OUT_OF_RANGE | input out of range.
@comm
    The API can set system 128 SVID & Destination Port to CVLAN configuration. 

*/
ret_t rtl8370_setAsicSvlanSP2CConf(uint32 index,rtl8370_svlan_s2c_t* svlanSP2CConf)
{
    ret_t retVal;
    uint32 regData;
    uint16 *accessPtr;
    uint32 i;

    rtl8370_svlan_s2c_smi_t smiSvlanSP2C;

    if(index > RTL8370_SP2CMAX)
        return RT_ERR_OUT_OF_RANGE;

    memset(&smiSvlanSP2C,0x00,sizeof(smiSvlanSP2C));
    _rtl8370_svlanSp2cStUser2Smi(svlanSP2CConf,&smiSvlanSP2C);

    accessPtr =  (uint16*)&smiSvlanSP2C;

    
    regData = *accessPtr;
    for(i=0;i<2;i++)
    {
        retVal = rtl8370_setAsicReg(RTL8370_SVLAN_S2C_ENTRY_BASE_REG +(index*2)+i,regData);
        if(retVal !=  RT_ERR_OK)
            return retVal;

        accessPtr ++;
        regData = *accessPtr;
    }
    
    
    return retVal;  
}   

/*
@func ret_t | rtl8370_getAsicSvlanSP2CConf| Get configure system 128 SP2C content
@parm uint32 | index | index of 128 SVLAN & Port to CVLAN configuration
@parm rtl8370_svlan_mc2s_t* | svlanSP2CConf | SVLAN & Port to CVLAN configuration
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
 The API can get system 128 SVID & Destination Port to CVLAN configuration. 

*/
ret_t rtl8370_getAsicSvlanSP2CConf(uint32 index,rtl8370_svlan_s2c_t* svlanSP2CConf)
{
    ret_t retVal;
    uint32 regData;
    uint16 *accessPtr;
    uint32 i;

    rtl8370_svlan_s2c_smi_t smiSvlanSP2C;

    if(index > RTL8370_SP2CMAX)
        return RT_ERR_OUT_OF_RANGE;

    memset(&smiSvlanSP2C,0x00,sizeof(smiSvlanSP2C));

    accessPtr = (uint16*)&smiSvlanSP2C;

    for(i=0;i<5;i++)
    {
        retVal = rtl8370_getAsicReg(RTL8370_SVLAN_S2C_ENTRY_BASE_REG+(index*2)+i,&regData);
        if(retVal !=  RT_ERR_OK)
            return retVal;

        *accessPtr = regData;

        accessPtr ++;
    }


    _rtl8370_svlanSp2cStSmi2User(svlanSP2CConf,&smiSvlanSP2C);

    return RT_ERR_OK;
}

