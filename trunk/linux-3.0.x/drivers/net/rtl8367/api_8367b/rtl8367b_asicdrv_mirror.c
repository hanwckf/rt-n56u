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
 * Feature : Port mirror related functions
 *
 */
#include "rtl8367b_asicdrv_mirror.h"
/* Function Name:
 *      rtl8367b_setAsicPortMirror
 * Description:
 *      Set port mirror function
 * Input:
 *      source 	- Source port
 *      monitor - Monitor (destination) port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPortMirror(rtk_uint32 source, rtk_uint32 monitor)
{
	ret_t retVal;

	if((source > RTL8367B_PORTIDMAX) || (monitor > RTL8367B_PORTIDMAX))
		return RT_ERR_PORT_ID;

	retVal = rtl8367b_setAsicRegBits(RTL8367B_MIRROR_CTRL_REG, RTL8367B_MIRROR_SOURCE_PORT_MASK, source);
	if(retVal != RT_ERR_OK)
		return retVal;

	
	return rtl8367b_setAsicRegBits(RTL8367B_MIRROR_CTRL_REG, RTL8367B_MIRROR_MONITOR_PORT_MASK, monitor);
}
/* Function Name:
 *      rtl8367b_getAsicPortMirror
 * Description:
 *      Get port mirror function
 * Input:
 *      pSource 	- Source port
 *      pMonitor - Monitor (destination) port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortMirror(rtk_uint32 *pSource, rtk_uint32 *pMonitor)
{
	ret_t retVal;
	
	retVal = rtl8367b_getAsicRegBits(RTL8367B_MIRROR_CTRL_REG, RTL8367B_MIRROR_SOURCE_PORT_MASK, pSource);
	if(retVal != RT_ERR_OK)
		return retVal;
	
	return rtl8367b_getAsicRegBits(RTL8367B_MIRROR_CTRL_REG, RTL8367B_MIRROR_MONITOR_PORT_MASK, pMonitor);
}
/* Function Name:
 *      rtl8367b_setAsicPortMirrorRxFunction
 * Description:
 *      Set the mirror function on RX of the mirrored
 * Input:
 *      enabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPortMirrorRxFunction(rtk_uint32 enabled)
{
	return rtl8367b_setAsicRegBit(RTL8367B_MIRROR_CTRL_REG, RTL8367B_MIRROR_RX_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicPortMirrorRxFunction
 * Description:
 *      Get the mirror function on RX of the mirrored
 * Input:
 *      pEnabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortMirrorRxFunction(rtk_uint32* pEnabled)
{
	return rtl8367b_getAsicRegBit(RTL8367B_MIRROR_CTRL_REG, RTL8367B_MIRROR_RX_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicPortMirrorTxFunction
 * Description:
 *      Set the mirror function on TX of the mirrored
 * Input:
 *      enabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPortMirrorTxFunction(rtk_uint32 enabled)
{
	return rtl8367b_setAsicRegBit(RTL8367B_MIRROR_CTRL_REG, RTL8367B_MIRROR_TX_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicPortMirrorTxFunction
 * Description:
 *      Get the mirror function on TX of the mirrored
 * Input:
 *      pEnabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortMirrorTxFunction(rtk_uint32* pEnabled)
{
	return rtl8367b_getAsicRegBit(RTL8367B_MIRROR_CTRL_REG, RTL8367B_MIRROR_TX_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicPortMirrorIsolation
 * Description:
 *      Set the traffic isolation on monitor port
 * Input:
 *      enabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPortMirrorIsolation(rtk_uint32 enabled)
{
	return rtl8367b_setAsicRegBit(RTL8367B_MIRROR_CTRL_REG, RTL8367B_MIRROR_ISO_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicPortMirrorIsolation
 * Description:
 *      Get the traffic isolation on monitor port
 * Input:
 *      pEnabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortMirrorIsolation(rtk_uint32* pEnabled)
{
	return rtl8367b_getAsicRegBit(RTL8367B_MIRROR_CTRL_REG, RTL8367B_MIRROR_ISO_OFFSET, pEnabled);
}

/* Function Name:
 *      rtl8367b_setAsicPortMirrorMask
 * Description:
 *      Set mirror source port mask
 * Input:
 *      SourcePortmask 	- Source Portmask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_MASK- Port Mask Error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPortMirrorMask(rtk_uint32 SourcePortmask)
{
    if( SourcePortmask > RTL8367B_PORTMASK)
        return RT_ERR_PORT_MASK;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_MIRROR_SRC_PMSK, RTL8367B_MIRROR_SRC_PMSK_MASK, SourcePortmask);
}

/* Function Name:
 *      rtl8367b_getAsicPortMirrorMask
 * Description:
 *      Get mirror source port mask
 * Input:
 *      None
 * Output:
 *      pSourcePortmask 	- Source Portmask
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_MASK- Port Mask Error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortMirrorMask(rtk_uint32 *pSourcePortmask)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_MIRROR_SRC_PMSK, RTL8367B_MIRROR_SRC_PMSK_MASK, pSourcePortmask);
}
