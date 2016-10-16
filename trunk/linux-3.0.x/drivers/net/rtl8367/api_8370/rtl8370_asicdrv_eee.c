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

#include "rtl8370_asicdrv_eee.h"

/*
@func ret_t | rtl8370_setAsicEeeTxEnable | Set eee TX function enable/disable.
@parm uint32 | port | The port number.
@parm uint32 | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set the EEE TX enable function.

*/
ret_t rtl8370_setAsicEeeTxEnable(uint32 port, uint32 enable)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;
	
    if (enable > 1)
        return RT_ERR_INPUT;
    
    return rtl8370_setAsicRegBit(RTL8370_PORT_EEE_CFG_REG(port), RTL8370_PORT_EEE_TX_OFFSET, enable);
}

/*
@func ret_t | rtl8370_getAsicEeeTxEnable | Get eee TX  function enable/disable.
@parm uint32 | port | The port number.
@parm uint32* | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get the EEE TX enable function.
*/
ret_t rtl8370_getAsicEeeTxEnable(uint32 port, uint32 *enable)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;
	
    return rtl8370_getAsicRegBit(RTL8370_PORT_EEE_CFG_REG(port), RTL8370_PORT_EEE_TX_OFFSET, enable);
}

/*
@func ret_t | rtl8370_setAsicEeeRxEnable | Set eee TX function enable/disable.
@parm uint32 | port | The port number.
@parm uint32 | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set the EEE RX enable function.

*/
ret_t rtl8370_setAsicEeeRxEnable(uint32 port, uint32 enable)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;
	
    if (enable > 1)
        return RT_ERR_INPUT;
    
    return rtl8370_setAsicRegBit(RTL8370_PORT_EEE_CFG_REG(port), RTL8370_PORT_EEE_RX_OFFSET, enable);
}

/*
@func ret_t | rtl8370_getAsicEeeRxEnable | Get eee TX  function enable/disable.
@parm uint32 | port | The port number.
@parm uint32* | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get the EEE RX enable function.
*/
ret_t rtl8370_getAsicEeeRxEnable(uint32 port, uint32 *enable)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;
	
    return rtl8370_getAsicRegBit(RTL8370_PORT_EEE_CFG_REG(port), RTL8370_PORT_EEE_RX_OFFSET, enable);
}

/*
@func ret_t | rtl8370_setAsicEeeForceMode | Set eee force mode function enable/disable.
@parm uint32 | port | The port number.
@parm uint32 | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set the EEE force mode enable function.

*/
ret_t rtl8370_setAsicEeeForceMode(uint32 port, uint32 enable)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;
	
    if (enable > 1)
        return RT_ERR_INPUT;
    
    return rtl8370_setAsicRegBit(RTL8370_PORT_EEE_CFG_REG(port), RTL8370_PORT_EEE_FORCE_OFFSET, enable);
}

/*
@func ret_t | rtl8370_getAsicEeeForceMode | Get eee force mode function enable/disable.
@parm uint32 | port | The port number.
@parm uint32* | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get the EEE force mode function.
*/
ret_t rtl8370_getAsicEeeForceMode(uint32 port, uint32 *enable)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;
	
    return rtl8370_getAsicRegBit(RTL8370_PORT_EEE_CFG_REG(port), RTL8370_PORT_EEE_FORCE_OFFSET, enable);
}

/*
@func ret_t | rtl8370_setAsicEee100M | Set eee force mode function enable/disable.
@parm uint32 | port | The port number.
@parm uint32 | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set the 100M EEE enable function.

*/
ret_t rtl8370_setAsicEee100M(uint32 port, uint32 enable)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;
	
    if (enable > 1)
        return RT_ERR_INPUT;
    
    return rtl8370_setAsicRegBit(RTL8370_PORT_EEE_CFG_REG(port), RTL8370_PORT_EEE_100M_OFFSET, enable);
}

/*
@func ret_t | rtl8370_getAsicEee100M | Get 100M eee enable/disable.
@parm uint32 | port | The port number.
@parm uint32* | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get the 100M EEE function.
*/
ret_t rtl8370_getAsicEee100M(uint32 port, uint32 *enable)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;
	
    return rtl8370_getAsicRegBit(RTL8370_PORT_EEE_CFG_REG(port), RTL8370_PORT_EEE_100M_OFFSET, enable);
}

/*
@func ret_t | rtl8370_setAsicEeeGiga | Set eee force mode function enable/disable.
@parm uint32 | port | The port number.
@parm uint32 | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set the 100M EEE enable function.

*/
ret_t rtl8370_setAsicEeeGiga(uint32 port, uint32 enable)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;
	
    if (enable > 1)
        return RT_ERR_INPUT;
    
    return rtl8370_setAsicRegBit(RTL8370_PORT_EEE_CFG_REG(port), RTL8370_PORT_EEE_GIGA_OFFSET, enable);
}

/*
@func ret_t | rtl8370_getAsicEeeGiga | Get 100M eee enable/disable.
@parm uint32 | port | The port number.
@parm uint32* | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get the 100M EEE function.
*/
ret_t rtl8370_getAsicEeeGiga(uint32 port, uint32 *enable)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;
	
    return rtl8370_getAsicRegBit(RTL8370_PORT_EEE_CFG_REG(port), RTL8370_PORT_EEE_GIGA_OFFSET, enable);
}

/*
@func ret_t | rtl8370_getAsicEeeGiga | Get eee TX meter.
@parm uint32 | port | The port number.
@parm uint32* | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get the 100M EEE function.
*/
ret_t rtl8370_getAsicEeeTxMeter(uint32 port, uint32 *cnt)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;
	
    return rtl8370_getAsicReg(RTL8370_PORT_EEE_TX_METER_REG(port), cnt);
}

/*
@func ret_t | rtl8370_getAsicEeeGiga | Get eee RX meter.
@parm uint32 | port | The port number.
@parm uint32* | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get the 100M EEE function.
*/
ret_t rtl8370_getAsicEeeRxMeter(uint32 port, uint32 *cnt)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;
	
    return rtl8370_getAsicReg(RTL8370_PORT_EEE_RX_METER_REG(port), cnt);
}

/*
@func ret_t | rtl8370_getAsicEeeStatus | Get eee status.
@parm uint32 | port | The port number.
@parm uint32 | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get the EEE status.

*/
ret_t rtl8370_getAsicEeeStatus(uint32 port, rtl8370_eee_status_t *status)
{
    ret_t retVal;
    uint32 regData;
    uint16 *accessPtr;

	if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    accessPtr = (uint16*)status;

    retVal=rtl8370_getAsicReg(RTL8370_PORT_EEE_CFG_REG(port),&regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    *accessPtr = regData;

    return RT_ERR_OK;
}

