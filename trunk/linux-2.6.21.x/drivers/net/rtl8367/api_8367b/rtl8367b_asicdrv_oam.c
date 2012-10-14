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
 * $Revision: 14202 $
 * $Date: 2010-11-16 15:13:00 +0800 (星期二, 16 十一月 2010) $
 *
 * Purpose : RTL8367B switch high-level API for RTL8367B
 * Feature : OAM related functions
 *
 */

#include "rtl8367b_asicdrv_oam.h"
/* Function Name:
 *      rtl8367b_setAsicOamParser
 * Description:
 *      Set OAM parser state
 * Input:
 *      port 	- Physical port number (0~7)
 *      parser 	- Per-Port OAM parser state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID  	- Invalid port number
 *      RT_ERR_NOT_ALLOWED  - Invalid paser state
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicOamParser(rtk_uint32 port, rtk_uint32 parser)
{
    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;

    if(parser > OAM_PARFWDCPU)
        return RT_ERR_NOT_ALLOWED;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_OAM_PARSER_CTRL0, RTL8367B_OAM_PARSER_MASK(port), parser);
}
/* Function Name:
 *      rtl8367b_getAsicOamParser
 * Description:
 *      Get OAM parser state
 * Input:
 *      port 	- Physical port number (0~7)
 *      pParser 	- Per-Port OAM parser state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicOamParser(rtk_uint32 port, rtk_uint32* pParser)
{
    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBits(RTL8367B_REG_OAM_PARSER_CTRL0, RTL8367B_OAM_PARSER_MASK(port), pParser);
}
/* Function Name:
 *      rtl8367b_setAsicOamMultiplexer
 * Description:
 *      Set OAM multiplexer state
 * Input:
 *      port 		- Physical port number (0~7)
 *      multiplexer - Per-Port OAM multiplexer state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID  	- Invalid port number
 *      RT_ERR_NOT_ALLOWED  - Invalid multiplexer state
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicOamMultiplexer(rtk_uint32 port, rtk_uint32 multiplexer)
{
    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;

    if(multiplexer > OAM_MULCPU)
        return RT_ERR_NOT_ALLOWED;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_OAM_MULTIPLEXER_CTRL0, RTL8367B_OAM_MULTIPLEXER_MASK(port), multiplexer);
}
/* Function Name:
 *      rtl8367b_getAsicOamMultiplexer
 * Description:
 *      Get OAM multiplexer state
 * Input:
 *      port 		- Physical port number (0~7)
 *      pMultiplexer - Per-Port OAM multiplexer state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicOamMultiplexer(rtk_uint32 port, rtk_uint32* pMultiplexer)
{
    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBits(RTL8367B_REG_OAM_MULTIPLEXER_CTRL0, RTL8367B_OAM_MULTIPLEXER_MASK(port), pMultiplexer);
}
/* Function Name:
 *      rtl8367b_setAsicOamCpuPri
 * Description:
 *      Set trap priority for OAM packet
 * Input:
 *      priority 	- priority (0~7)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 *      RT_ERR_QOS_INT_PRIORITY  	- Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicOamCpuPri(rtk_uint32 priority)
{
    if(priority > RTL8367B_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_QOS_TRAP_PRIORITY0, RTL8367B_OAM_PRIOIRTY_MASK, priority);
}
/* Function Name:
 *      rtl8367b_getAsicOamCpuPri
 * Description:
 *      Get trap priority for OAM packet
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
ret_t rtl8367b_getAsicOamCpuPri(rtk_uint32 *pPriority)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_QOS_TRAP_PRIORITY0, RTL8367B_OAM_PRIOIRTY_MASK, pPriority);
}
/* Function Name:
 *      rtl8367b_setAsicOamEnable
 * Description:
 *      Set OAM function state
 * Input:
 *      enabled 	- OAM function usage 1:enable, 0:disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicOamEnable(rtk_uint32 enabled)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_OAM_CTRL, RTL8367B_OAM_CTRL_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicOamEnable
 * Description:
 *      Get OAM function state
 * Input:
 *      pEnabled 	- OAM function usage 1:enable, 0:disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicOamEnable(rtk_uint32 *pEnabled)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_OAM_CTRL, RTL8367B_OAM_CTRL_OFFSET, pEnabled);
}
