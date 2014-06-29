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
 * Feature : Unkown multicast related functions
 *
 */
#include "rtl8367b_asicdrv_unknownMulticast.h"
/* Function Name:
 *      rtl8367b_setAsicUnknownL2MulticastBehavior
 * Description:
 *      Set behavior of L2 multicast
 * Input:
 *      port 	- Physical port number (0~7)
 *      behave 	- 0: flooding, 1: drop, 2: trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID  	- Invalid port number
 *      RT_ERR_NOT_ALLOWED  - Invalid operation
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicUnknownL2MulticastBehavior(rtk_uint32 port, rtk_uint32 behave)
{
	if(port >  RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    if(behave >= L2_UNKOWN_MULTICAST_END)
        return RT_ERR_NOT_ALLOWED;

    return rtl8367b_setAsicRegBits(RTL8367B_UNKNOWN_L2_MULTICAST_REG(port), RTL8367B_UNKNOWN_L2_MULTICAST_MASK(port), behave);
}
/* Function Name:
 *      rtl8367b_getAsicUnknownL2MulticastBehavior
 * Description:
 *      Get behavior of L2 multicast
 * Input:
 *      port 	- Physical port number (0~7)
 *      pBehave 	- 0: flooding, 1: drop, 2: trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicUnknownL2MulticastBehavior(rtk_uint32 port, rtk_uint32 *pBehave)
{
	if(port >  RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBits(RTL8367B_UNKNOWN_L2_MULTICAST_REG(port), RTL8367B_UNKNOWN_L2_MULTICAST_MASK(port), pBehave);
}
/* Function Name:
 *      rtl8367b_setAsicUnknownIPv4MulticastBehavior
 * Description:
 *      Set behavior of IPv4 multicast
 * Input:
 *      port 	- Physical port number (0~7)
 *      behave 	- 0: flooding, 1: drop, 2: trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID  	- Invalid port number
 *      RT_ERR_NOT_ALLOWED  - Invalid operation
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicUnknownIPv4MulticastBehavior(rtk_uint32 port, rtk_uint32 behave)
{
	if(port >  RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    if(behave >= L3_UNKOWN_MULTICAST_END)
        return RT_ERR_NOT_ALLOWED;

    return rtl8367b_setAsicRegBits(RTL8367B_UNKNOWN_IPV4_MULTICAST_REG(port), RTL8367B_UNKNOWN_IPV4_MULTICAST_MASK(port), behave);
}
/* Function Name:
 *      rtl8367b_getAsicUnknownIPv4MulticastBehavior
 * Description:
 *      Get behavior of IPv4 multicast
 * Input:
 *      port 	- Physical port number (0~7)
 *      pBehave 	- 0: flooding, 1: drop, 2: trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicUnknownIPv4MulticastBehavior(rtk_uint32 port, rtk_uint32 *pBehave)
{
	if(port >  RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBits(RTL8367B_UNKNOWN_IPV4_MULTICAST_REG(port), RTL8367B_UNKNOWN_IPV4_MULTICAST_MASK(port), pBehave);
}
/* Function Name:
 *      rtl8367b_setAsicUnknownIPv6MulticastBehavior
 * Description:
 *      Set behavior of IPv6 multicast
 * Input:
 *      port 	- Physical port number (0~7)
 *      behave 	- 0: flooding, 1: drop, 2: trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID  	- Invalid port number
 *      RT_ERR_NOT_ALLOWED  - Invalid operation
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicUnknownIPv6MulticastBehavior(rtk_uint32 port, rtk_uint32 behave)
{
	if(port >  RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    if(behave >= L3_UNKOWN_MULTICAST_END)
        return RT_ERR_NOT_ALLOWED;

    return rtl8367b_setAsicRegBits(RTL8367B_UNKNOWN_IPV6_MULTICAST_REG(port), RTL8367B_UNKNOWN_IPV6_MULTICAST_MASK(port), behave);
}
/* Function Name:
 *      rtl8367b_getAsicUnknownIPv6MulticastBehavior
 * Description:
 *      Get behavior of IPv6 multicast
 * Input:
 *      port 	- Physical port number (0~7)
 *      pBehave 	- 0: flooding, 1: drop, 2: trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicUnknownIPv6MulticastBehavior(rtk_uint32 port, rtk_uint32 *pBehave)
{
	if(port >  RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBits(RTL8367B_UNKNOWN_IPV6_MULTICAST_REG(port), RTL8367B_UNKNOWN_IPV6_MULTICAST_MASK(port), pBehave);
}
/* Function Name:
 *      rtl8367b_setAsicUnknownMulticastTrapPriority
 * Description:
 *      Set trap priority of unknown multicast frame
 * Input:
 *      priority 	- priority (0~7)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicUnknownMulticastTrapPriority(rtk_uint32 priority)
{
    if(priority > RTL8367B_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    return rtl8367b_setAsicRegBits(RTL8367BG_QOS_TRAP_PRIORITY_CTRL0_REG, RTL8367B_UNKNOWN_MC_PRIORTY_MASK, priority);
}
/* Function Name:
 *      rtl8367b_getAsicUnknownMulticastTrapPriority
 * Description:
 *      Get trap priority of unknown multicast frame
 * Input:
 *      pPriority 	- priority (0~7)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicUnknownMulticastTrapPriority(rtk_uint32 *pPriority)
{
    return rtl8367b_getAsicRegBits(RTL8367BG_QOS_TRAP_PRIORITY_CTRL0_REG, RTL8367B_UNKNOWN_MC_PRIORTY_MASK, pPriority);
}
