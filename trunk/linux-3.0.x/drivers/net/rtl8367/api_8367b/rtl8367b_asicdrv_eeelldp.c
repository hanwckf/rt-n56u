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
 * Feature : EEE LLDP related functions
 *
 */

#include "rtl8367b_asicdrv_eeelldp.h"

ret_t _rtl8367b_getAsicEeelldpFrameDataReg(rtk_uint32 regAddr, rtk_uint32 dataLength, rtk_int8 *readDataPtr);

/* Function Name:
 *      rtl8367b_setAsicEeelldp
 * Description:
 *      Set eeelldp function enable/disable
 * Input:
 *      enabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      If EEELLDP function is enabled, upon receiving an eeelldp frame,
 *    	ASIC will signal 8051 (interrupt or trap) to handled the received
 *    	EEELLDP frame
 */
ret_t rtl8367b_setAsicEeelldp(rtk_uint32 enabled)
{
    
    return rtl8367b_setAsicRegBit(RTL8367B_REG_EEELLDP_CTRL0, RTL8367B_EEELLDP_ENABLE_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicEeelldp
 * Description:
 *      Get eeelldp function enable/disable
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
 ret_t rtl8367b_getAsicEeelldp(rtk_uint32 *pEnabled)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_EEELLDP_CTRL0, RTL8367B_EEELLDP_ENABLE_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicEeelldpTrapCpu
 * Description:
 *      Set trap eeelldp to CPU/not trap to CPU
 * Input:
 *      trap 	- 1: trap, 0: do not trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicEeelldpTrapCpu(rtk_uint32 trap)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_EEELLDP_CTRL0, RTL8367B_EEELLDP_TRAP_CPU_OFFSET, trap);
}
/* Function Name:
 *      rtl8367b_getAsicEeelldpTrapCpu
 * Description:
 *      Get trap eeelldp to CPU/not trap to CPU
 * Input:
 *      pTrap 	- 1: trap, 0: do not trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicEeelldpTrapCpu(rtk_uint32 *pTrap)
{    
    return rtl8367b_getAsicRegBit(RTL8367B_REG_EEELLDP_CTRL0, RTL8367B_EEELLDP_TRAP_CPU_OFFSET, pTrap);
}
/* Function Name:
 *      rtl8367b_setAsicEeelldpTrap8051
 * Description:
 *      Set trap eeelldp to 8051/not trap to 8051
 * Input:
 *      trap 	- 1: trap, 0: do not trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicEeelldpTrap8051(rtk_uint32 trap)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_EEELLDP_CTRL0, RTL8367B_EEELLDP_TRAP_8051_OFFSET, trap);
}
/* Function Name:
 *      rtl8367b_getAsicEeelldpTrap8051
 * Description:
 *      Get trap eeelldp to 8051/not trap to 8051
 * Input:
 *      pTrap 	- 1: trap, 0: do not trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicEeelldpTrap8051(rtk_uint32 *pTrap)
{    
    return rtl8367b_getAsicRegBit(RTL8367B_REG_EEELLDP_CTRL0, RTL8367B_EEELLDP_TRAP_8051_OFFSET, pTrap);
}
/* Function Name:
 *      rtl8367b_setAsicEeelldpTrapCpuPri
 * Description:
 *      Set trap eeelldp trap to CPU priority
 * Input:
 *      priority 	- trap to CPU priority (0~7)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 *      RT_ERR_QOS_INT_PRIORITY  	- Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicEeelldpTrapCpuPri(rtk_uint32 priority)
{
    if(priority > RTL8367B_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;
    
    return rtl8367b_setAsicRegBits(RTL8367B_REG_QOS_TRAP_PRIORITY1, RTL8367B_EEELLDP_TRAP_PRI_MASK, priority);
}
/* Function Name:
 *      rtl8367b_getAsicEeelldpTrapCpuPri
 * Description:
 *      Get trap eeelldp trap to CPU priority
 * Input:
 *      pPriority 	- trap to CPU priority (0~7)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicEeelldpTrapCpuPri(rtk_uint32 *pPriority)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_QOS_TRAP_PRIORITY1, RTL8367B_EEELLDP_TRAP_PRI_MASK, pPriority);
}
/* Function Name:
 *      rtl8367b_setAsicEeelldpInterrupt8051
 * Description:
 *      Set interrupt to 8051/not interrupt to 8051 while receiving a eeelldp
 * Input:
 *      interrupt 	- 1: interrupt, 0: do not interrupt
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicEeelldpInterrupt8051(rtk_uint32 interrupt)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_INTR_IMR_8051, RTL8367B_INTR_IMR_8051_EEELLDP_8051_OFFSET, interrupt);
}
/* Function Name:
 *      rtl8367b_getAsicEeelldpInterrupt8051
 * Description:
 *      Get interrupt to 8051/not interrupt to 8051 while receiving a eeelldp
 * Input:
 *      pInterrupt 	- 1: interrupt, 0: do not interrupt
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicEeelldpInterrupt8051(rtk_uint32 *pInterrupt)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_INTR_IMR_8051, RTL8367B_INTR_IMR_8051_EEELLDP_8051_OFFSET, pInterrupt);
}
/* Function Name:
 *      rtl8367b_setAsicEeelldpSubtype
 * Description:
 *      Set eeelldp frame subtype of ASIC
 * Input:
 *      subtype 	- (0~0xFF)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE	- input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicEeelldpSubtype(rtk_uint32 subtype)
{
    if(subtype > 0xFF)
        return RT_ERR_OUT_OF_RANGE;
    
    return rtl8367b_setAsicRegBits(RTL8367B_REG_EEELLDP_CTRL0, RTL8367B_EEELLDP_SUBTYPE_MASK, subtype);
}
/* Function Name:
 *      rtl8367b_getAsicEeelldpSubtype
 * Description:
 *      Get eeelldp frame subtype of ASIC
 * Input:
 *      pSubtype 	- (0~0xFF)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicEeelldpSubtype(rtk_uint32 *pSubtype)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_EEELLDP_CTRL0, RTL8367B_EEELLDP_SUBTYPE_MASK, pSubtype);
}
/* Function Name:
 *      rtl8367b_getAsicEeelldpSubtype
 * Description:
 *      Set portmask of ports that receive EEELLDP frame
 * Input:
 *      portmask 	- (0~0xFF)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicEeelldpRxPortmask(rtk_uint32 portmask)
{
    return rtl8367b_setAsicRegBits(RTL8367B_REG_EEELLDP_PMSK, RTL8367B_EEELLDP_PMSK_MASK, portmask);
}
/* Function Name:
 *      rtl8367b_getAsicEeelldpRxPortmask
 * Description:
 *      Get portmask of ports that receive EEELLDP frame
 * Input:
 *      pPortmask 	- (0~0xFF)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicEeelldpRxPortmask(rtk_uint32 *pPortmask)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_EEELLDP_PMSK, RTL8367B_EEELLDP_PMSK_MASK, pPortmask);
}

ret_t _rtl8367b_getAsicEeelldpFrameDataReg(rtk_uint32 regAddr, rtk_uint32 dataLength, rtk_int8 *readDataPtr)
{
    ret_t retVal;
    rtk_uint32 i;
    rtk_uint32 regData;
    rtk_uint16 *accessPtr;

    accessPtr = (rtk_uint16*)readDataPtr;

    for(i=0; i < dataLength / 2; i++)
    {
        retVal = rtl8367b_getAsicReg(regAddr - i, &regData);
        if(retVal != RT_ERR_OK)
            return retVal;
        
        *accessPtr = (((((rtk_int16)regData) & 0xFF00) >> 8) | ((((rtk_int16)regData) & 0x00FF) << 8));
		
        accessPtr++;
    }
    
    if(dataLength & 0x1)
    {
        retVal = rtl8367b_getAsicRegBits(regAddr - (dataLength / 2), 0xFF, &regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        *accessPtr = ((((rtk_int16)regData) & 0x00FF) << 8) ;
    }

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_getAsicEeelldpRxCapFrameLower
 * Description:
 *      Get octet 42~59 of received capability eeelldp frame of port n
 * Input:
 *      port 		- Physical port number (0~7)
 *      pFrames 	- frameL pointer
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicEeelldpRxFrameLower(rtk_uint32 port, rtk_int8 *pFrames)
{
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;
	
    return _rtl8367b_getAsicEeelldpFrameDataReg(RTL8367B_EEELLDP_RX_VALUE_PORT_REG(port), RTL8367B_EEELLDP_FRAMEL_LENGTH, pFrames);
}

