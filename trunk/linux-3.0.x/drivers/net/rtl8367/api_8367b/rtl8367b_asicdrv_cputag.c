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
 * Feature : Proprietary CPU-tag related function drivers
 *
 */
#include "rtl8367b_asicdrv_cputag.h"
/* Function Name:
 *      rtl8367b_setAsicCputagEnable
 * Description:
 *      Set cpu tag function enable/disable
 * Input:
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_ENABLE  	- Invalid enable/disable input
 * Note:
 *      If CPU tag function is disabled, CPU tag will not be added to frame
 *    	forwarded to CPU port, and all ports cannot parse CPU tag.
 */
ret_t rtl8367b_setAsicCputagEnable(rtk_uint32 enabled)
{
    if(enabled > 1)
        return RT_ERR_ENABLE;
    
    return rtl8367b_setAsicRegBit(RTL8367B_REG_CPU_CTRL, RTL8367B_CPU_EN_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicCputagEnable
 * Description:
 *      Get cpu tag function enable/disable
 * Input:
 *      pEnabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicCputagEnable(rtk_uint32 *pEnabled)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_CPU_CTRL, RTL8367B_CPU_EN_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicCputagTrapPort
 * Description:
 *      Set cpu tag trap port
 * Input:
 *      port - port number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *     API can set destination port of trapping frame
 */
ret_t rtl8367b_setAsicCputagTrapPort(rtk_uint32 port)
{
    if(port >= RTL8367B_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_CPU_CTRL, RTL8367B_CPU_TRAP_PORT_MASK, port);    
}
/* Function Name:
 *      rtl8367b_getAsicCputagTrapPort
 * Description:
 *      Get cpu tag trap port
 * Input:
 *      pPort - port number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *     None
 */
ret_t rtl8367b_getAsicCputagTrapPort(rtk_uint32 *pPort)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_CPU_CTRL, RTL8367B_CPU_TRAP_PORT_MASK, pPort);    
}
/* Function Name:
 *      rtl8367b_setAsicCputagPortmask
 * Description:
 *      Set ports that can parse CPU tag
 * Input:
 *      portmask - port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_MASK  	- Invalid portmask
 * Note:
 *     None
 */
ret_t rtl8367b_setAsicCputagPortmask(rtk_uint32 portmask)
{
    if(portmask > RTL8367B_PORTMASK)
        return RT_ERR_PORT_MASK;

    return rtl8367b_setAsicReg(RTL8367B_CPU_PORT_MASK_REG, portmask);
}
/* Function Name:
 *      rtl8367b_getAsicCputagPortmask
 * Description:
 *      Get ports that can parse CPU tag
 * Input:
 *      pPortmask - port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 * Note:
 *     None
 */
ret_t rtl8367b_getAsicCputagPortmask(rtk_uint32 *pPortmask)
{
    return rtl8367b_getAsicReg(RTL8367B_CPU_PORT_MASK_REG, pPortmask);
}
/* Function Name:
 *      rtl8367b_setAsicCputagInsertMode
 * Description:
 *      Set CPU-tag insert mode
 * Input:
 *      mode - 0: insert to all packets; 1: insert to trapped packets; 2: don't insert
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_NOT_ALLOWED  - Actions not allowed by the function
 * Note:
 *     None
 */
ret_t rtl8367b_setAsicCputagInsertMode(rtk_uint32 mode)
{
    if(mode >= CPUTAG_INSERT_END)
        return RT_ERR_NOT_ALLOWED;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_CPU_CTRL, RTL8367B_CPU_INSERTMODE_MASK, mode);
}
/* Function Name:
 *      rtl8367b_getAsicCputagInsertMode
 * Description:
 *      Get CPU-tag insert mode
 * Input:
 *      pMode - 0: insert to all packets; 1: insert to trapped packets; 2: don't insert
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 * Note:
 *     None
 */
ret_t rtl8367b_getAsicCputagInsertMode(rtk_uint32 *pMode)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_CPU_CTRL, RTL8367B_CPU_INSERTMODE_MASK, pMode);
}
/* Function Name:
 *      rtl8367b_setAsicCputagPriorityRemapping
 * Description:
 *      Set queue assignment of CPU port
 * Input:
 *      srcPri - internal priority (0~7)
 *      newPri - internal priority after remapping (0~7)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 *      RT_ERR_QOS_INT_PRIORITY  	- Invalid priority
 * Note:
 *     None
 */
ret_t rtl8367b_setAsicCputagPriorityRemapping(rtk_uint32 srcPri, rtk_uint32 newPri)
{
    if((srcPri > RTL8367B_PRIMAX) || (newPri > RTL8367B_PRIMAX))
        return RT_ERR_QOS_INT_PRIORITY;

    return rtl8367b_setAsicRegBits(RTL8367B_QOS_PRIPORITY_REMAPPING_IN_CPU_REG(srcPri), RTL8367B_QOS_PRIPORITY_REMAPPING_IN_CPU_MASK(srcPri), newPri);
}
/* Function Name:
 *      rtl8367b_getAsicCputagPriorityRemapping
 * Description:
 *      Get queue assignment of CPU port
 * Input:
 *      srcPri - internal priority (0~7)
 *      pNewPri - internal priority after remapping (0~7)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 *      RT_ERR_QOS_INT_PRIORITY  	- Invalid priority
 * Note:
 *     None
 */
ret_t rtl8367b_getAsicCputagPriorityRemapping(rtk_uint32 srcPri, rtk_uint32 *pNewPri)
{
    if(srcPri > RTL8367B_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    return rtl8367b_getAsicRegBits(RTL8367B_QOS_PRIPORITY_REMAPPING_IN_CPU_REG(srcPri), RTL8367B_QOS_PRIPORITY_REMAPPING_IN_CPU_MASK(srcPri), pNewPri);
}
/* Function Name:
 *      rtl8367b_setAsicCputagPosition
 * Description:
 *      Set cpu tag insert position
 * Input:
 *      postion - 1: After entire packet(before CRC field), 0: After MAC_SA (Default)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *     None
 */
ret_t rtl8367b_setAsicCputagPosition(rtk_uint32 postion)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_CPU_CTRL, RTL8367B_CPU_TAG_POSITION_OFFSET, postion);
}
/* Function Name:
 *      rtl8367b_getAsicCputagPosition
 * Description:
 *      Get cpu tag insert position
 * Input:
 *      pPostion - 1: After entire packet(before CRC field), 0: After MAC_SA (Default)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *     None
 */
ret_t rtl8367b_getAsicCputagPosition(rtk_uint32* pPostion)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_CPU_CTRL, RTL8367B_CPU_TAG_POSITION_OFFSET, pPostion);
}

/* Function Name:
 *      rtl8367b_setAsicCputagMode
 * Description:
 *      Set cpu tag mode
 * Input:
 *      mode - 1: 4bytes mode, 0: 8bytes mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_INPUT 	- Invalid input parameters
 * Note:
 *      If CPU tag function is disabled, CPU tag will not be added to frame
 *    	forwarded to CPU port, and all ports cannot parse CPU tag.
 */
ret_t rtl8367b_setAsicCputagMode(rtk_uint32 mode)
{
    if(mode > 1)
        return RT_ERR_INPUT;
    
    return rtl8367b_setAsicRegBit(RTL8367B_REG_CPU_CTRL, RTL8367B_CPU_TAG_FORMAT_OFFSET, mode);
}
/* Function Name:
 *      rtl8367b_getAsicCputagMode
 * Description:
 *      Get cpu tag mode
 * Input:
 *      pMode - 1: 4bytes mode, 0: 8bytes mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicCputagMode(rtk_uint32 *pMode)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_CPU_CTRL, RTL8367B_CPU_TAG_FORMAT_OFFSET, pMode);
}
/* Function Name:
 *      rtl8367b_setAsicCputagRxMinLength
 * Description:
 *      Set cpu tag mode
 * Input:
 *      mode - 1: 64bytes, 0: 72bytes
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_INPUT 	- Invalid input parameters
 * Note:
 *      If CPU tag function is disabled, CPU tag will not be added to frame
 *    	forwarded to CPU port, and all ports cannot parse CPU tag.
 */
ret_t rtl8367b_setAsicCputagRxMinLength(rtk_uint32 mode)
{
    if(mode > 1)
        return RT_ERR_INPUT;
    
    return rtl8367b_setAsicRegBit(RTL8367B_REG_CPU_CTRL, RTL8367B_CPU_TAG_RXBYTECOUNT_OFFSET, mode);
}
/* Function Name:
 *      rtl8367b_getAsicCputagRxMinLength
 * Description:
 *      Get cpu tag mode
 * Input:
 *      pMode - 1: 64bytes, 0: 72bytes
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicCputagRxMinLength(rtk_uint32 *pMode)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_CPU_CTRL, RTL8367B_CPU_TAG_RXBYTECOUNT_OFFSET, pMode);
}



