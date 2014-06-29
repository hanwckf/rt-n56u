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
 * $Date: 2010/12/02 04:34:19 $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367B
 * Feature : 
 *
 */
#include "rtl8370_asicdrv_igmp.h"

/*
@func ret_t | rtl8370_setAsicIpMulticastVlanLeaky | Set IP multicast VLAN Leaky function
@parm uint32 | port | The port number
@parm uint32 | enabled | Enable or disable IP multicast function
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_ENABLE | Invalid enable input.
@comm
    The API can Set the IP multicast Leaky function. When enabling this function, 
    if the lookup result(forwarding portmap) of IP Multicast packet is over VLAN boundary, 
    the packet can be forwarded across VLAN.
*/
ret_t rtl8370_setAsicIpMulticastVlanLeaky( uint32 port, uint32 enabled )
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;
    
    if(enabled > 1)
        return RT_ERR_ENABLE; 

    return rtl8370_setAsicRegBit(RTL8370_REG_IPMCAST_VLAN_LEAKY, port, enabled);
}

/*
@func ret_t | rtl8370_getAsicIpMulticastVlanLeaky | Get IP multicast VLAN Leaky function
@parm uint32 | port | The port number
@parm uint32* | ptr_enabled | Enable or disable IP multicast function
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can Get the IP multicast Leaky function.
*/
ret_t rtl8370_getAsicIpMulticastVlanLeaky( uint32 port, uint32 *ptr_enabled )
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBit(RTL8370_REG_IPMCAST_VLAN_LEAKY, port, ptr_enabled);
}
/*
@func ret_t | rtl8370_setAsicIpMulticastPortIsoLeaky | Set IP multicast port isolation leaky function
@parm uint32 | port | The port number
@parm uint32 | enabled | Enable or disable IP multicast function
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_ENABLE | Invalid enable input.
@comm
    The API can set the IP multicast port isolation function. When enabling this function, 
    if the lookup result(forwarding portmap) of IP Multicast packet is over port isolation boundary, 
    the packet can be forwarded across isolated ports.
*/
ret_t rtl8370_setAsicIpMulticastPortIsoLeaky( uint32 port, uint32 enabled )
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(enabled > 1)
        return RT_ERR_ENABLE;
    
    return rtl8370_setAsicRegBit(RTL8370_REG_IPMCAST_PORTISO_LEAKY, port, enabled);
}
/*
@func ret_t | rtl8370_getAsicIpMulticastPortIsoLeaky | Get IP multicast port isolation leaky function
@parm uint32 | port | The port number
@parm uint32* | enabled | Enable or disable IP multicast function
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can get the IP multicast port isolation function. When enabling this function, 
    if the lookup result(forwarding portmap) of IP Multicast packet is over port isolation boundary, 
    the packet can be forwarded across isolated ports.
*/
ret_t rtl8370_getAsicIpMulticastPortIsoLeaky( uint32 port, uint32* enabled )
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBit(RTL8370_REG_IPMCAST_PORTISO_LEAKY, port, enabled);
}
/*
@func ret_t | rtl8370_setAsicIgmp | Set IGMP/MLD trap function
@parm enum rtl8370_igmp_t* | igmpcfg | IGMP configuration for trapping frame type setting.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@comm
    This API can set both IPv4 IGMP/IPv6 MLD with/without PPPoE header trapping function.
    All 4 kinds of IGMP/MLD function can be set seperately.
*/
ret_t rtl8370_setAsicIgmp( rtl8370_igmp_t* igmpcfg)
{
    uint32 regData;

    regData = *(uint16*)igmpcfg;

    return rtl8370_setAsicReg(RTL8370_IGMP_CTRL_REG, regData);     
}
/*
@func ret_t | rtl8370_getAsicIgmp | Set IGMP/MLD trap function
@parm enum rtl8370_igmp_t* | igmpcfg | IGMP configuration for trapping frame type setting.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@comm
    This API can set both IPv4 IGMP/IPv6 MLD with/without PPPoE header trapping function.
    All 4 kinds of IGMP/MLD function can be set seperately.
*/
ret_t rtl8370_getAsicIgmp( rtl8370_igmp_t* igmpcfg)
{
    ret_t retVal;
    uint32 regData;
    uint16 regData16;

    retVal = rtl8370_getAsicReg(RTL8370_IGMP_CTRL_REG,&regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    regData16 = (uint16)(regData&0xFFFF);
    *igmpcfg = *(rtl8370_igmp_t*)(&regData16);

    return RT_ERR_OK;
}

