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

#include "rtl8370_asicdrv_misc.h"

/*
@func ret_t | rtl8370_setAsicMacAddress | Set switch MAC address
@parm ether_addr_t | mac | switch mac
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@comm
    The API can set Switch MAC
*/
ret_t rtl8370_setAsicMacAddress(ether_addr_t mac)
{
    ret_t retVal;
    uint32 regData;
    uint8 *accessPtr;
    uint32 i;

    accessPtr =  (uint8*)&mac;

    retVal = 0;
    regData = 0;
    for(i = 0; i <= 5; i++)
    {
        if (0 == (i%2))
        {
            regData = *accessPtr << 8;
        }
		else
		{
		    regData = regData | (*accessPtr);
            retVal = rtl8370_setAsicReg(RTL8370_SWITCH_MAC_BASE-(i/2),regData);
            if(retVal !=  RT_ERR_OK)
                return retVal;
			regData = 0;          		
		}
        accessPtr ++;
        
    }

    return retVal;
}

/*
@func ret_t | rtl8370_getAsicMacAddress | Get switch MAC address
@parm ether_addr_t* | mac | switch mac
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@comm
    The API can get Switch MAC
*/
ret_t rtl8370_getAsicMacAddress(ether_addr_t *mac)
{
    ret_t retVal;
    uint32 regData;
    uint8 *accessPtr;
    uint32 i;


    accessPtr = (uint8*)mac;

    retVal = 0;
    for(i = 0; i <= 5; i++)
    {
        if (0 == (i%2))
        {
            retVal = rtl8370_getAsicReg(RTL8370_SWITCH_MAC_BASE - (i/2),&regData);
            if(retVal !=  RT_ERR_OK)
                return retVal;
			
            *accessPtr = (regData & 0xFF00) >> 8;
        }
		else
		{
            *accessPtr = (regData & 0xFF);
        }
        accessPtr ++;
    }

    return retVal;
}

/*
@func ret_t | rtl8370_getAsicDebugInfo | Get per-port packet forward debugging information
@parm uint32 | phyNo | PHY number (0~15).
@parm uint32* | debugifo | per-port packet trap/drop/forward reason
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can get per-Port newest packet forwarding reason
*/
ret_t rtl8370_getAsicDebugInfo(uint32 port,uint32 *debugifo)
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBits(RTL8370_DEBUG_INFO_REG(port), RTL8370_DEBUG_INFO_MASK(port), debugifo);
}

/*
@func ret_t | rtl8370_setAsicPortJamMode | Enable half duplex flow control setting
@parm uint32 | mode | 0: Back-Pressure 1: DEFER 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    If the API can be used to set half duplex flow control with setting 0: BACK-PRESSURE 1:DEFER
*/
ret_t rtl8370_setAsicPortJamMode(uint32 mode)
{
    return rtl8370_setAsicRegBit(RTL8370_REG_CFG_BACKPRESSURE, RTL8370_LONGTXE_OFFSET, mode);
}

/*
@func ret_t | rtl8370_getAsicPortJamMode | Enable half duplex flow control setting
@parm uint32* | mode | 0: Back-Pressure 1: DEFER 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    If the API can be used to get half duplex flow control mode 
*/
ret_t rtl8370_getAsicPortJamMode(uint32* mode)
{
    return rtl8370_getAsicRegBit(RTL8370_REG_CFG_BACKPRESSURE, RTL8370_LONGTXE_OFFSET, mode);
}

/*
@func ret_t | rtl8370_setAsicMaxLengthInRx | Max receiving packet length.
@parm uint32 | maxLength | 0: 1522 bytes 1:1536 bytes 2:1552 bytes 3:16000bytes
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_INPUT | Invalid input length parameter.
@comm
    If the API can be used to set accepted max packet length.
*/
ret_t rtl8370_setAsicMaxLengthInRx(uint32 maxLength)
{    
    if (maxLength >= RTL8370_MAXPKTLEN_END)
        return RT_ERR_INPUT;
    
    return rtl8370_setAsicRegBits(RTL8370_REG_MAX_LENGTH_LIMINT_IPG, RTL8370_MAX_LENTH_CTRL_MASK, maxLength);
}

/*
@func ret_t | rtl8370_getAsicMaxLengthInRx | Max receiving packet length.
@parm uint32* | maxLength | 0: 1522 bytes 1:1536 bytes 2:1552 bytes 3:16000bytes
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    If the API can be used to get accepted max packet length.
*/
ret_t rtl8370_getAsicMaxLengthInRx(uint32* maxLength)
{
    return rtl8370_getAsicRegBits(RTL8370_REG_MAX_LENGTH_LIMINT_IPG, RTL8370_MAX_LENTH_CTRL_MASK, maxLength);
}

/*
@func ret_t | rtl8370_setAsicEthernetAV | Set ethernet AV trap to CPU
@parm uint32 | enable | 0: Disable 1: Enable 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    If the API can be used to set half duplex flow control with setting 0: BACK-PRESSURE 1:DEFER
*/
ret_t rtl8370_setAsicEthernetAv(uint32 enable)
{
    return rtl8370_setAsicRegBit(RTL8370_REG_EAV_CTRL, RTL8370_EAV_TRAP_CPU_OFFSET, enable);
}

/*
@func ret_t | rtl8370_getAsicEthernetAV | Get ethernet AV trap to CPU
@parm uint32* | enable |  0: Disable 1: Enable  
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    If the API can be used to get half duplex flow control mode 
*/
ret_t rtl8370_getAsicEthernetAv(uint32* enable)
{
    return rtl8370_getAsicRegBit(RTL8370_REG_EAV_CTRL, RTL8370_EAV_TRAP_CPU_OFFSET, enable);
}

