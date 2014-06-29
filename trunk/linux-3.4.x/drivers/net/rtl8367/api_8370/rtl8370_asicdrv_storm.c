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
 * $Date: 2010/12/02 04:34:38 $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367B
 * Feature : 
 *
 */

#include "rtl8370_asicdrv_storm.h"

/*
@func ret_t | rtl8370_setAsicStormFilterBroadcastEnable | Set per-port broadcast storm filter enable/disable.
@parm uint32 | port | Physical port number (0~15).
@parm uint32 | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_ENABLE | Invalid input
@comm
    This API set per-port broadcast stomr filter enable/disable
    Broadcast storm filters of each port point to some meters. If 
    broadcast storm filter of port n is enabled, data length of
    broadcast packet will be included in the meter pointed by port
    n to elminate broadcast packet egress rate.
*/
ret_t rtl8370_setAsicStormFilterBroadcastEnable(uint32 port, uint32 enable)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    if(enable > 1)
        return RT_ERR_ENABLE;

    return rtl8370_setAsicRegBit(RTL8370_STORM_BCAST_REG, port, enable);
}

/*
@func ret_t | rtl8370_getAsicStormFilterBroadcastEnable | Get per-port broadcast storm filter enable/disable.
@parm uint32 | port | Physical port number (0~15).
@parm uint32* | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    This API get per-port broadcast stomr filter enable/disable
    Broadcast storm filters of each port point to some meters. If 
    broadcast storm filter of port n is enabled, data length of
    broadcast packet will be included in the meter pointed by port
    n to elminate broadcast packet egress rate.
*/
ret_t rtl8370_getAsicStormFilterBroadcastEnable(uint32 port, uint32 *enable)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBit(RTL8370_STORM_BCAST_REG, port, enable);
}

/*
@func ret_t | rtl8370_setAsicStormFilterBroadcastMeter | Set per-port broadcast storm filter meter.
@parm uint32 | port | port number (0~15).
@parm uint32 | meter | meter index (0~63).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_FILTER_METER_ID | Invalid meter index
@comm
    This API set per-port broadcast stomr filter meter
    Broadcast storm filters of each port point to some meters. If 
    broadcast storm filter of port n is enabled, data length of
    broadcast packet will be included in the meter pointed by port
    n to elminate broadcast packet egress rate.
*/
ret_t rtl8370_setAsicStormFilterBroadcastMeter(uint32 port, uint32 meter)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    if(meter > RTL8370_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    return rtl8370_setAsicRegBits(RTL8370_STORM_BCAST_METER_CRTL_REG(port), RTL8370_STORM_BCAST_METER_CRTL_MASK(port), meter);
}

/*
@func ret_t | rtl8370_getAsicStormFilterBroadcastMeter | Get per-port broadcast storm filter meter.
@parm uint32 | port | Physical port number (0~15).
@parm uint32* | meter | meter index (0~63).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    This API get per-port broadcast stomr filter meter
    Broadcast storm filters of each port point to some meters. If 
    broadcast storm filter of port n is enabled, data length of
    broadcast packet will be included in the meter pointed by port
    n to elminate broadcast packet egress rate.
*/
ret_t rtl8370_getAsicStormFilterBroadcastMeter(uint32 port, uint32 *meter)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBits(RTL8370_STORM_BCAST_METER_CRTL_REG(port), RTL8370_STORM_BCAST_METER_CRTL_MASK(port), meter);
}

/*
@func ret_t | rtl8370_setAsicStormFilterMulticastEnable | Set per-port multicast storm filter enable/disable.
@parm uint32 | port | Physical port number (0~15).
@parm uint32 | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_ENABLE | Invalid input
@comm
    This API set per-port multicast storm filter enable/disable
    Multicast storm filters of each port point to some meters. If 
    multicast storm filter of port n is enabled, data length of
    multicast packet will be included in the meter pointed by port
    n to elminate multicast packet egress rate.
*/
ret_t rtl8370_setAsicStormFilterMulticastEnable(uint32 port, uint32 enable)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    if(enable > 1)
        return RT_ERR_ENABLE;

    return rtl8370_setAsicRegBit(RTL8370_STORM_MCAST_REG, port, enable);
}

/*
@func ret_t | rtl8370_getAsicStormFilterMulticastEnable | Get per-port multicast storm filter enable/disable.
@parm uint32 | port | Physical port number (0~15).
@parm uint32* | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    This API get per-port multicast storm filter enable/disable
    Multicast storm filters of each port point to some meters. If 
    multicast storm filter of port n is enabled, data length of
    multicast packet will be included in the meter pointed by port
    n to elminate multicast packet egress rate.
*/
ret_t rtl8370_getAsicStormFilterMulticastEnable(uint32 port, uint32 *enable)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBit(RTL8370_STORM_MCAST_REG, port, enable);
}

/*
@func ret_t | rtl8370_setAsicStormFilterMulticastMeter | Set per-port multicast storm filter meter.
@parm uint32 | port | Physical port number (0~15).
@parm uint32 | meter | meter index (0~63).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_FILTER_METER_ID | Invalid meter index
@comm
    This API set per-port multicast storm filter meter
    Multicast storm filters of each port point to some meters. If 
    multicast storm filter of port n is enabled, data length of
    multicast packet will be included in the meter pointed by port
    n to elminate multicast packet egress rate.
*/
ret_t rtl8370_setAsicStormFilterMulticastMeter(uint32 port, uint32 meter)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    if(meter > RTL8370_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    return rtl8370_setAsicRegBits(RTL8370_STORM_MCAST_METER_CRTL_REG(port), RTL8370_STORM_MCAST_METER_CRTL_MASK(port), meter);
}

/*
@func ret_t | rtl8370_getAsicStormFilterMulticastMeter | Get per-port multicast storm filter meter.
@parm uint32 | port | Physical port number (0~15).
@parm uint32* | meter | meter index (0~63).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    This API get per-port multicast storm filter meter
    Multicast storm filters of each port point to some meters. If 
    multicast storm filter of port n is enabled, data length of
    multicast packet will be included in the meter pointed by port
    n to elminate multicast packet egress rate.
*/
ret_t rtl8370_getAsicStormFilterMulticastMeter(uint32 port, uint32 *meter)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBits(RTL8370_STORM_MCAST_METER_CRTL_REG(port), RTL8370_STORM_MCAST_METER_CRTL_MASK(port), meter);
}

/*
@func ret_t | rtl8370_setAsicStormFilterUnknownMulticastEnable | Set per-port unknown multicast storm filter enable/disable.
@parm uint32 | port | Physical port number (0~15).
@parm uint32 | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_ENABLE | Invalid input
@comm
    This API set per-port unknown multicast storm filter enable/disable
    Unknown multicast storm filters of each port point to some meters. If 
    unknown multicast storm filter of port n is enabled, data length of
    unknown multicast packet will be included in the meter pointed by port
    n to elminate unknown multicast packet egress rate.
*/
ret_t rtl8370_setAsicStormFilterUnknownMulticastEnable(uint32 port, uint32 enable)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    if(enable > 1)
        return RT_ERR_ENABLE;

    return rtl8370_setAsicRegBit(RTL8370_STORM_UNKNOWN_MCAST_REG, port, enable);
}

/*
@func ret_t | rtl8370_getAsicStormFilterUnknownMulticastEnable | Get per-port unknown multicast storm filter enable/disable.
@parm uint32 | port | Physical port number (0~15).
@parm uint32* | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    This API get per-port unknown multicast storm filter enable/disable
    Unknown multicast storm filters of each port point to some meters. If 
    unknown multicast storm filter of port n is enabled, data length of
    unknown multicast packet will be included in the meter pointed by port
    n to elminate unknown multicast packet egress rate.
*/
ret_t rtl8370_getAsicStormFilterUnknownMulticastEnable(uint32 port, uint32 *enable)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBit(RTL8370_STORM_UNKNOWN_MCAST_REG, port, enable);
}

/*
@func ret_t | rtl8370_setAsicStormFilterUnknownMulticastMeter | Set per-port unknown multicast storm filter meter.
@parm uint32 | port | Physical port number (0~15).
@parm uint32 | meter | meter index (0~63).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_FILTER_METER_ID | Invalid meter index
@comm
    This API set per-port unknown multicast storm filter meter
    Unknown multicast storm filters of each port point to some meters. If 
    unknown multicast storm filter of port n is enabled, data length of
    unknown multicast packet will be included in the meter pointed by port
    n to elminate unknown multicast packet egress rate.
*/
ret_t rtl8370_setAsicStormFilterUnknownMulticastMeter(uint32 port, uint32 meter)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    if(meter > RTL8370_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    return rtl8370_setAsicRegBits(RTL8370_STORM_UNMC_METER_CRTL_REG(port), RTL8370_STORM_UNMC_METER_CRTL_MASK(port), meter);
}

/*
@func ret_t | rtl8370_getAsicStormFilterUnknownMulticastMeter | Get per-port unknown multicast storm filter meter.
@parm uint32 | port | Physical port number (0~15).
@parm uint32* | meter | meter index (0~63).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    This API get per-port unknown multicast storm filter meter
    Unknown multicast storm filters of each port point to some meters. If 
    unknown multicast storm filter of port n is enabled, data length of
    unknown multicast packet will be included in the meter pointed by port
    n to elminate unknown multicast packet egress rate.
*/
ret_t rtl8370_getAsicStormFilterUnknownMulticastMeter(uint32 port, uint32 *meter)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBits(RTL8370_STORM_UNMC_METER_CRTL_REG(port), RTL8370_STORM_UNMC_METER_CRTL_MASK(port), meter);
}

/*
@func ret_t | rtl8370_setAsicStormFilterUnknownUnicastEnable | Set per-port unknown unicast storm filter enable/disable.
@parm uint32 | port | Physical port number (0~15).
@parm uint32 | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_ENABLE | Invalid input
@comm
    This API set per-port unknown unicast storm filter enable/disable
    Unknown unicast storm filters of each port point to some meters. If 
    unknown unicast storm filter of port n is enabled, data length of
    unknown unicast packet will be included in the meter pointed by port
    n to elminate unknown unicast packet egress rate.
*/
ret_t rtl8370_setAsicStormFilterUnknownUnicastEnable(uint32 port, uint32 enable)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    if(enable > 1)
        return RT_ERR_ENABLE;

    return rtl8370_setAsicRegBit(RTL8370_STORM_UNKNOWN_UCAST_REG, port, enable);
}

/*
@func ret_t | rtl8370_getAsicStormFilterUnknownUnicastEnable | Get per-port unknown unicast storm filter enable/disable.
@parm uint32 | port | Physical port number (0~15).
@parm uint32* | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    This API get per-port unknown unicast storm filter enable/disable
    Unknown unicast storm filters of each port point to some meters. If 
    unknown unicast storm filter of port n is enabled, data length of
    unknown unicast packet will be included in the meter pointed by port
    n to elminate unknown unicast packet egress rate.
*/
ret_t rtl8370_getAsicStormFilterUnknownUnicastEnable(uint32 port, uint32 *enable)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBit(RTL8370_STORM_UNKNOWN_UCAST_REG, port, enable);
}

/*
@func ret_t | rtl8370_setAsicStormFilterUnknownUnicastMeter | Set per-port unknown unicast storm filter meter.
@parm uint32 | port | Physical port number (0~15).
@parm uint32 | meter | meter index (0~63).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_FILTER_METER_ID | Invalid meter index
@comm
    This API set per-port unknown unicast storm filter meter
    Unknown unicast storm filters of each port point to some meters. If 
    unknown unicast storm filter of port n is enabled, data length of
    unknown unicast packet will be included in the meter pointed by port
    n to elminate unknown unicast packet egress rate.
*/
ret_t rtl8370_setAsicStormFilterUnknownUnicastMeter(uint32 port, uint32 meter)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    if(meter > RTL8370_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    return rtl8370_setAsicRegBits(RTL8370_STORM_UNDA_METER_CRTL_REG(port), RTL8370_STORM_UNDA_METER_CRTL_MASK(port), meter);
}

/*
@func ret_t | rtl8370_getAsicStormFilterUnknownUnicastMeter | Get per-port unknown unicast storm filter meter.
@parm uint32 | port | Physical port number (0~15).
@parm uint32* | meter | meter index (0~63).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    This API get per-port unknown unicast storm filter meter
    Unknown unicast storm filters of each port point to some meters. If 
    unknown unicast storm filter of port n is enabled, data length of
    unknown unicast packet will be included in the meter pointed by port
    n to elminate unknown unicast packet egress rate.
*/
ret_t rtl8370_getAsicStormFilterUnknownUnicastMeter(uint32 port, uint32 *meter)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBits(RTL8370_STORM_UNDA_METER_CRTL_REG(port), RTL8370_STORM_UNDA_METER_CRTL_MASK(port), meter);
}

