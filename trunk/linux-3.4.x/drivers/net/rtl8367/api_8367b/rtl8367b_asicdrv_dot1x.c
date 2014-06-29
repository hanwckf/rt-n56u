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
 * Feature : 802.1X related functions
 *
 */
#include "rtl8367b_asicdrv_dot1x.h"
/* Function Name:
 *      rtl8367b_setAsic1xPBEnConfig
 * Description:
 *      Set 802.1x port-based port enable configuration
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
ret_t rtl8367b_setAsic1xPBEnConfig(rtk_uint32 port, rtk_uint32 enabled)
{
	if(port >= RTL8367B_PORTNO)
		return RT_ERR_PORT_ID;
    
	return rtl8367b_setAsicRegBit(RTL8367B_DOT1X_PORT_ENABLE_REG, port, enabled);
}
/* Function Name:
 *      rtl8367b_getAsic1xPBEnConfig
 * Description:
 *      Get 802.1x port-based port enable configuration
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
ret_t rtl8367b_getAsic1xPBEnConfig(rtk_uint32 port, rtk_uint32 *pEnabled)
{
	if(port >= RTL8367B_PORTNO)
		return RT_ERR_PORT_ID;

	return rtl8367b_getAsicRegBit(RTL8367B_DOT1X_PORT_ENABLE_REG, port, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsic1xPBAuthConfig
 * Description:
 *      Set 802.1x port-based authorised port configuration
 * Input:
 *      port 	- Physical port number (0~7)
 *      auth 	- 1: authorised, 0: non-authorised
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_setAsic1xPBAuthConfig(rtk_uint32 port, rtk_uint32 auth)
{
	if(port >= RTL8367B_PORTNO)
		return RT_ERR_PORT_ID;
	
	return rtl8367b_setAsicRegBit(RTL8367B_DOT1X_PORT_AUTH_REG, port, auth);
}
/* Function Name:
 *      rtl8367b_getAsic1xPBAuthConfig
 * Description:
 *      Get 802.1x port-based authorised port configuration
 * Input:
 *      port 	- Physical port number (0~7)
 *      pAuth 	- 1: authorised, 0: non-authorised
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsic1xPBAuthConfig(rtk_uint32 port, rtk_uint32 *pAuth)
{
	if(port >= RTL8367B_PORTNO)
		return RT_ERR_PORT_ID;

	return rtl8367b_getAsicRegBit(RTL8367B_DOT1X_PORT_AUTH_REG, port, pAuth);
}
/* Function Name:
 *      rtl8367b_setAsic1xPBOpdirConfig
 * Description:
 *      Set 802.1x port-based operational direction
 * Input:
 *      port 	- Physical port number (0~7)
 *      opdir 	- Operation direction 1: IN, 0:BOTH
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_setAsic1xPBOpdirConfig(rtk_uint32 port, rtk_uint32 opdir)
{
	if(port >= RTL8367B_PORTNO)
		return RT_ERR_PORT_ID;

	return rtl8367b_setAsicRegBit(RTL8367B_DOT1X_PORT_OPDIR_REG, port, opdir);
}
/* Function Name:
 *      rtl8367b_getAsic1xPBOpdirConfig
 * Description:
 *      Get 802.1x port-based operational direction
 * Input:
 *      port 	- Physical port number (0~7)
 *      pOpdir 	- Operation direction 1: IN, 0:BOTH
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsic1xPBOpdirConfig(rtk_uint32 port, rtk_uint32* pOpdir)
{
	if(port >= RTL8367B_PORTNO)
		return RT_ERR_PORT_ID;

	return rtl8367b_getAsicRegBit(RTL8367B_DOT1X_PORT_OPDIR_REG, port, pOpdir);
}
/* Function Name:
 *      rtl8367b_setAsic1xMBEnConfig
 * Description:
 *      Set 802.1x mac-based port enable configuration
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
ret_t rtl8367b_setAsic1xMBEnConfig(rtk_uint32 port, rtk_uint32 enabled)
{
	if(port >= RTL8367B_PORTNO)
		return RT_ERR_PORT_ID;

	return rtl8367b_setAsicRegBit(RTL8367B_DOT1X_MAC_ENABLE_REG, port, enabled);
}
/* Function Name:
 *      rtl8367b_getAsic1xMBEnConfig
 * Description:
 *      Get 802.1x mac-based port enable configuration
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
ret_t rtl8367b_getAsic1xMBEnConfig(rtk_uint32 port, rtk_uint32 *pEnabled)
{
	if(port >= RTL8367B_PORTNO)
		return RT_ERR_PORT_ID;

	return rtl8367b_getAsicRegBit(RTL8367B_DOT1X_MAC_ENABLE_REG, port, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsic1xMBOpdirConfig
 * Description:
 *      Set 802.1x mac-based operational direction
 * Input:
 *      opdir 		- Operation direction 1: IN, 0:BOTH
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsic1xMBOpdirConfig(rtk_uint32 opdir)
{
	return rtl8367b_setAsicRegBit(RTL8367B_DOT1X_CFG_REG, RTL8367B_DOT1X_MAC_OPDIR_OFFSET, opdir);
}
/* Function Name:
 *      rtl8367b_getAsic1xMBOpdirConfig
 * Description:
 *      Get 802.1x mac-based operational direction
 * Input:
 *      pOpdir 		- Operation direction 1: IN, 0:BOTH
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsic1xMBOpdirConfig(rtk_uint32 *pOpdir)
{
	return rtl8367b_getAsicRegBit(RTL8367B_DOT1X_CFG_REG, RTL8367B_DOT1X_MAC_OPDIR_OFFSET, pOpdir);
}
/* Function Name:
 *      rtl8367b_setAsic1xProcConfig
 * Description:
 *      Set 802.1x unauth. behavior configuration
 * Input:
 *      port 	- Physical port number (0~7)
 *      proc 	- 802.1x unauth. behavior configuration 0:drop 1:trap to CPU 2:Guest VLAN
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID  	- Invalid port number
 *      RT_ERR_DOT1X_PROC  	- Unauthorized behavior error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsic1xProcConfig(rtk_uint32 port, rtk_uint32 proc)
{
	if(port >= RTL8367B_PORTNO)
		return RT_ERR_PORT_ID;

	if(proc >= DOT1X_UNAUTH_END)
		return RT_ERR_DOT1X_PROC;

	return rtl8367b_setAsicRegBits(RTL8367B_DOT1X_UNAUTH_ACT_BASE, RTL8367B_DOT1X_UNAUTH_ACT_MASK(port),proc);
}
/* Function Name:
 *      rtl8367b_getAsic1xProcConfig
 * Description:
 *      Get 802.1x unauth. behavior configuration
 * Input:
 *      port	- Physical port number (0~7)
 *      pProc 	- 802.1x unauth. behavior configuration 0:drop 1:trap to CPU 2:Guest VLAN
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID  	- Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsic1xProcConfig(rtk_uint32 port, rtk_uint32* pProc)
{
	if(port >= RTL8367B_PORTNO)
		return RT_ERR_PORT_ID;

	return rtl8367b_getAsicRegBits(RTL8367B_DOT1X_UNAUTH_ACT_BASE, RTL8367B_DOT1X_UNAUTH_ACT_MASK(port),pProc);
}
/* Function Name:
 *      rtl8367b_setAsic1xGuestVidx
 * Description:
 *      Set 802.1x guest vlan index
 * Input:
 *      index	- 802.1x guest vlan index (0~31)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_DOT1X_GVLANIDX  	- Invalid cvid index
 * Note:
 *      None
 */
ret_t rtl8367b_setAsic1xGuestVidx(rtk_uint32 index)
{
	if(index >= RTL8367B_CVIDXNO)
		return RT_ERR_DOT1X_GVLANIDX;

	return rtl8367b_setAsicRegBits(RTL8367B_DOT1X_CFG_REG, RTL8367B_DOT1X_GVIDX_MASK, index);
}
/* Function Name:
 *      rtl8367b_getAsic1xGuestVidx
 * Description:
 *      Get 802.1x guest vlan index
 * Input:
 *      pIndex 	- 802.1x guest vlan index (0~31)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsic1xGuestVidx(rtk_uint32 *pIndex)
{
	return rtl8367b_getAsicRegBits(RTL8367B_DOT1X_CFG_REG, RTL8367B_DOT1X_GVIDX_MASK, pIndex);
}
/* Function Name:
 *      rtl8367b_setAsic1xGVOpdir
 * Description:
 *      Set 802.1x guest vlan talk to auth. DA
 * Input:
 *      enabled		- 0:disable 1:enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsic1xGVOpdir(rtk_uint32 enabled)
{
	return rtl8367b_setAsicRegBit(RTL8367B_DOT1X_CFG_REG, RTL8367B_DOT1X_GVOPDIR_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsic1xGVOpdir
 * Description:
 *      Get 802.1x guest vlan talk to auth. DA
 * Input:
 *      pEnabled		- 0:disable 1:enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsic1xGVOpdir(rtk_uint32 *pEnabled)
{
	return rtl8367b_getAsicRegBit(RTL8367B_DOT1X_CFG_REG, RTL8367B_DOT1X_GVOPDIR_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsic1xTrapPriority
 * Description:
 *      Set 802.1x Trap priority
 * Input:
 *      priority	- priority (0~7)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_QOS_INT_PRIORITY	- Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367b_setAsic1xTrapPriority(rtk_uint32 priority)
{
    if(priority > RTL8367B_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;
    
	return rtl8367b_setAsicRegBits(RTL8367B_REG_QOS_TRAP_PRIORITY0, RTL8367B_DOT1X_PRIORTY_MASK,priority);
}
/* Function Name:
 *      rtl8367b_getAsic1xTrapPriority
 * Description:
 *      Get 802.1x Trap priority
 * Input:
 *      pPriority	- priority (0~7)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsic1xTrapPriority(rtk_uint32 *pPriority)
{
	return rtl8367b_getAsicRegBits(RTL8367B_REG_QOS_TRAP_PRIORITY0, RTL8367B_DOT1X_PRIORTY_MASK, pPriority);
}

