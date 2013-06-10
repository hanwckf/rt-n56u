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
 * $Revision: 17550 $
 * $Date: 2011-05-03 19:20:51 +0800 (星期二, 03 五月 2011) $
 *
 * Purpose : RTL8367B switch high-level API for RTL8367B
 * Feature : RLDP related functions
 *
 */

#include "rtl8367b_asicdrv_rldp.h"
/* Function Name:
 *      rtl8367b_setAsicRldp
 * Description:
 *      Set RLDP function enable/disable
 * Input:
 *      enabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRldp(rtk_uint32 enabled)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_RLDP_CTRL0, RTL8367B_RLDP_ENABLE_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicRldp
 * Description:
 *      Get RLDP function enable/disable
 * Input:
 *      pEnabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRldp(rtk_uint32 *pEnabled)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_RLDP_CTRL0, RTL8367B_RLDP_ENABLE_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicRldpEnable8051
 * Description:
 *      Set RLDP function handled by ASIC or 8051
 * Input:
 *      enabled 	- 1: enabled 8051, 0: disabled 8051 (RLDP is handled by ASIC)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRldpEnable8051(rtk_uint32 enabled)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_RLDP_CTRL0, RTL8367B_RLDP_8051_ENABLE_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_setAsicRldrtl8367b_getAsicRldpEnable8051pEnable8051
 * Description:
 *      Get RLDP function handled by ASIC or 8051
 * Input:
 *      pEnabled 	- 1: enabled 8051, 0: disabled 8051 (RLDP is handled by ASIC)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRldpEnable8051(rtk_uint32 *pEnabled)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_RLDP_CTRL0, RTL8367B_RLDP_8051_ENABLE_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicRldpCompareRandomNumber
 * Description:
 *      Set enable compare the random number field and seed field of RLDP frame
 * Input:
 *      enabled 	- 1: enabled comparing random number, 0: disabled comparing random number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRldpCompareRandomNumber(rtk_uint32 enabled)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_RLDP_CTRL0, RTL8367B_RLDP_COMP_ID_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicRldpCompareRandomNumber
 * Description:
 *      Get enable compare the random number field and seed field of RLDP frame
 * Input:
 *      pEnabled 	- 1: enabled comparing random number, 0: disabled comparing random number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRldpCompareRandomNumber(rtk_uint32 *pEnabled)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_RLDP_CTRL0, RTL8367B_RLDP_COMP_ID_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicRldpIndicatorSource
 * Description:
 *      Set buzzer and LED source when detecting a loop
 * Input:
 *      src 	- 0: ASIC, 1: 8051
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRldpIndicatorSource(rtk_uint32 src)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_RLDP_CTRL0, RTL8367B_RLDP_INDICATOR_SOURCE_OFFSET, src);
}
/* Function Name:
 *      rtl8367b_getAsicRldpIndicatorSource
 * Description:
 *      Get buzzer and LED source when detecting a loop
 * Input:
 *      pSrc 	- 0: ASIC, 1: 8051
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRldpIndicatorSource(rtk_uint32 *pSrc)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_RLDP_CTRL0, RTL8367B_RLDP_INDICATOR_SOURCE_OFFSET, pSrc);
}
/* Function Name:
 *      rtl8367b_setAsicRldpCheckingStatePara
 * Description:
 *      Set retry count and retry period of checking state
 * Input:
 *      retryCount 	- 0~0xFF (times)
 *      retryPeriod - 0~0xFFFF (ms)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRldpCheckingStatePara(rtk_uint32 retryCount, rtk_uint32 retryPeriod)
{
    ret_t retVal;

    if(retryCount > 0xFF)
        return RT_ERR_OUT_OF_RANGE;
    if(retryPeriod > RTL8367B_REGDATAMAX)
        return RT_ERR_OUT_OF_RANGE;

    retVal = rtl8367b_setAsicRegBits(RTL8367B_RLDP_RETRY_COUNT_REG, RTL8367B_RLDP_RETRY_COUNT_CHKSTATE_MASK, retryCount);
    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8367b_setAsicReg(RTL8367B_RLDP_RETRY_PERIOD_CHKSTATE_REG, retryPeriod);
}
/* Function Name:
 *      rtl8367b_getAsicRldpCheckingStatePara
 * Description:
 *      Get retry count and retry period of checking state
 * Input:
 *      pRetryCount 	- 0~0xFF (times)
 *      pRetryPeriod 	- 0~0xFFFF (ms)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRldpCheckingStatePara(rtk_uint32 *pRetryCount, rtk_uint32 *pRetryPeriod)
{
    ret_t retVal;

    retVal = rtl8367b_getAsicRegBits(RTL8367B_RLDP_RETRY_COUNT_REG, RTL8367B_RLDP_RETRY_COUNT_CHKSTATE_MASK, pRetryCount);
    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8367b_getAsicReg(RTL8367B_RLDP_RETRY_PERIOD_CHKSTATE_REG, pRetryPeriod);
}
/* Function Name:
 *      rtl8367b_setAsicRldpLoopStatePara
 * Description:
 *      Set retry count and retry period of loop state
 * Input:
 *      retryCount 	- 0~0xFF (times)
 *      retryPeriod - 0~0xFFFF (ms)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRldpLoopStatePara(rtk_uint32 retryCount, rtk_uint32 retryPeriod)
{
    ret_t retVal;

    if(retryCount > 0xFF)
        return RT_ERR_OUT_OF_RANGE;

	if(retryPeriod > RTL8367B_REGDATAMAX)
        return RT_ERR_OUT_OF_RANGE;

    retVal = rtl8367b_setAsicRegBits(RTL8367B_RLDP_RETRY_COUNT_REG, RTL8367B_RLDP_RETRY_COUNT_LOOPSTATE_MASK, retryCount);
    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8367b_setAsicReg(RTL8367B_RLDP_RETRY_PERIOD_LOOPSTATE_REG, retryPeriod);
}
/* Function Name:
 *      rtl8367b_getAsicRldpLoopStatePara
 * Description:
 *      Get retry count and retry period of loop state
 * Input:
 *      pRetryCount 	- 0~0xFF (times)
 *      pRetryPeriod 	- 0~0xFFFF (ms)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRldpLoopStatePara(rtk_uint32 *pRetryCount, rtk_uint32 *pRetryPeriod)
{
    ret_t retVal;

    retVal = rtl8367b_getAsicRegBits(RTL8367B_RLDP_RETRY_COUNT_REG, RTL8367B_RLDP_RETRY_COUNT_LOOPSTATE_MASK, pRetryCount);
    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8367b_getAsicReg(RTL8367B_RLDP_RETRY_PERIOD_LOOPSTATE_REG, pRetryPeriod);
}
/* Function Name:
 *      rtl8367b_setAsicRldpTxPortmask
 * Description:
 *      Set portmask that send/forward RLDP frame
 * Input:
 *      portmask 	- 0~0xFF
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_MASK  	- Invalid portmask
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRldpTxPortmask(rtk_uint32 portmask)
{
    if(portmask > RTL8367B_PORTMASK)
        return RT_ERR_PORT_MASK;

    return rtl8367b_setAsicReg(RTL8367B_RLDP_TX_PMSK_REG, portmask);
}
/* Function Name:
 *      rtl8367b_getAsicRldpTxPortmask
 * Description:
 *      Get portmask that send/forward RLDP frame
 * Input:
 *      pPortmask 	- 0~0xFF
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRldpTxPortmask(rtk_uint32 *pPortmask)
{
    return rtl8367b_getAsicReg(RTL8367B_RLDP_TX_PMSK_REG, pPortmask);
}
/* Function Name:
 *      rtl8367b_setAsicRldpMagicNum
 * Description:
 *      Set Random seed of RLDP
 * Input:
 *      seed 	- MAC
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRldpMagicNum(ether_addr_t seed)
{
    ret_t retVal;
    rtk_uint32 regData;
    rtk_uint16 *accessPtr;
    rtk_uint32 i;

    accessPtr = (rtk_uint16*)&seed;

    for (i = 0; i < 3; i++)
    {
        regData = *accessPtr;
        retVal = rtl8367b_setAsicReg(RTL8367B_RLDP_MAGIC_NUM_REG_BASE + i, regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        accessPtr++;
    }

    return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicRldpMagicNum
 * Description:
 *      Get Random seed of RLDP
 * Input:
 *      pSeed 	- MAC
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRldpMagicNum(ether_addr_t *pSeed)
{
    ret_t retVal;
    rtk_uint32 regData;
    rtk_int16 *accessPtr;
    rtk_uint32 i;

    accessPtr = (rtk_uint16*)pSeed;

    for(i = 0; i < 3; i++)
    {
        retVal = rtl8367b_getAsicReg(RTL8367B_RLDP_MAGIC_NUM_REG_BASE + i, &regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        *accessPtr = regData;
        accessPtr++;
    }

    return retVal;
}
/* Function Name:
 *      rtl8367b_setAsicRldpLoopedPortmask
 * Description:
 *      Clear looped portmask
 * Input:
 *      portmask 	- 0~0xFF
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRldpLoopedPortmask(rtk_uint32 portmask)
{
    return rtl8367b_setAsicReg(RTL8367B_RLDP_LOOP_PMSK_REG, portmask);
}
/* Function Name:
 *      rtl8367b_getAsicRldpLoopedPortmask
 * Description:
 *      Get looped portmask
 * Input:
 *      pPortmask 	- 0~0xFF
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRldpLoopedPortmask(rtk_uint32 *pPortmask)
{
    return rtl8367b_getAsicReg(RTL8367B_RLDP_LOOP_PMSK_REG, pPortmask);
}
/* Function Name:
 *      rtl8367b_getAsicRldpRandomNumber
 * Description:
 *      Get Random number of RLDP
 * Input:
 *      pRandNumber 	- MAC
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRldpRandomNumber(ether_addr_t *pRandNumber)
{
    ret_t retVal;
    rtk_uint32 regData;
    rtk_int16 accessPtr[3];
    rtk_uint32 i;

    for(i = 0; i < 3; i++)
    {
        retVal = rtl8367b_getAsicReg(RTL8367B_RLDP_RAND_NUM_REG_BASE+ i, &regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        accessPtr[i] = regData;
    }

    memcpy(pRandNumber, accessPtr, 6);
    return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicRldpLoopedPortmask
 * Description:
 *      Get port number of looped pair
 * Input:
 *      port 		- Physical port number (0~7)
 *      pLoopedPair 	- port (0~7)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRldpLoopedPortPair(rtk_uint32 port, rtk_uint32 *pLoopedPair)
{
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBits(RTL8367B_RLDP_LOOP_PORT_REG(port), RTL8367B_RLDP_LOOP_PORT_MASK(port), pLoopedPair);
}
/* Function Name:
 *      rtl8367b_setAsicRlppTrap8051
 * Description:
 *      Set trap RLPP packet to 8051
 * Input:
 *      enabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRlppTrap8051(rtk_uint32 enabled)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_RLDP_CTRL0, RTL8367B_RLPP_8051_TRAP_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicRlppTrap8051
 * Description:
 *      Get trap RLPP packet to 8051
 * Input:
 *      pEnabled 	- 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRlppTrap8051(rtk_uint32 *pEnabled)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_RLDP_CTRL0, RTL8367B_RLPP_8051_TRAP_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367b_setAsicRldpLeaveLoopedPortmask
 * Description:
 *      Clear leaved looped portmask
 * Input:
 *      portmask 	- 0~0xFF
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRldpLeaveLoopedPortmask(rtk_uint32 portmask)
{
    return rtl8367b_setAsicReg(RTL8367B_REG_RLDP_RELEASED_INDICATOR, portmask);
}
/* Function Name:
 *      rtl8367b_getAsicRldpLeaveLoopedPortmask
 * Description:
 *      Get leaved looped portmask
 * Input:
 *      pPortmask 	- 0~0xFF
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRldpLeaveLoopedPortmask(rtk_uint32 *pPortmask)
{
    return rtl8367b_getAsicReg(RTL8367B_REG_RLDP_RELEASED_INDICATOR, pPortmask);
}
/* Function Name:
 *      rtl8367b_setAsicRldpConfiguredLoopedPortmask
 * Description:
 *      Set 8051/CPU configured looped portmask
 * Input:
 *      portmask 	- 0~0xFF
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRldpConfiguredLoopedPortmask(rtk_uint32 portmask)
{
    return rtl8367b_setAsicRegBits(RTL8367B_REG_RLDP_CTRL0, RTL8367B_RLDP_8051_LOOP_PORTMSK_MASK, portmask);
}
/* Function Name:
 *      rtl8367b_getAsicRldpConfiguredLoopedPortmask
 * Description:
 *      Get 8051/CPU configured looped portmask
 * Input:
 *      pPortmask 	- 0~0xFF
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRldpConfiguredLoopedPortmask(rtk_uint32 *pPortmask)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_RLDP_CTRL0, RTL8367B_RLDP_8051_LOOP_PORTMSK_MASK, pPortmask);
}
/* Function Name:
 *      rtl8367b_setAsicRldpTriggerMode
 * Description:
 *      Set trigger RLDP mode
 * Input:
 *      mode 	- 1: Periodically, 0: SA moving
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRldpTriggerMode(rtk_uint32 enabled)
{
    return rtl8367b_setAsicRegBit(RTL8367B_REG_RLDP_CTRL0, RTL8367B_RLDP_TRIGGER_MODE_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367b_getAsicRldpTriggerMode
 * Description:
 *      Get trigger RLDP mode
 * Input:
 *      pMode 	- - 1: Periodically, 0: SA moving
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRldpTriggerMode(rtk_uint32 *pEnabled)
{
    return rtl8367b_getAsicRegBit(RTL8367B_REG_RLDP_CTRL0, RTL8367B_RLDP_TRIGGER_MODE_OFFSET, pEnabled);
}

/* Function Name:
 *      rtl8367b_setAsicRldp8051Portmask
 * Description:
 *      Set portmask that send/forward RLDP frame
 * Input:
 *      portmask 	- 0~0xFF
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_MASK  	- Invalid portmask
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicRldp8051Portmask(rtk_uint32 portmask)
{
    if(portmask > RTL8367B_PORTMASK)
        return RT_ERR_PORT_MASK;

    return rtl8367b_setAsicRegBits(RTL8367B_RLDP_CTRL0_REG,RTL8367B_RLDP_8051_LOOP_PORTMSK_MASK,portmask);
}
/* Function Name:
 *      rtl8367b_getAsicRldp8051Portmask
 * Description:
 *      Get portmask that send/forward RLDP frame
 * Input:
 *      pPortmask 	- 0~0xFF
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicRldp8051Portmask(rtk_uint32 *pPortmask)
{
    return rtl8367b_getAsicRegBits(RTL8367B_RLDP_CTRL0_REG,RTL8367B_RLDP_8051_LOOP_PORTMSK_MASK,pPortmask);
}

/*
@func ret_t | rtl8367b_setAsicRldp_mode | Set RLDP transmitting mode.
@parm uint32 | mode | 1: Periodically send RLDP packet, 0: Send RLDP packet when SA moving is detected.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API Set RLDP transmitting mode. When set to RLDP_TRANSMIT_MODE_0, Asic will send RLDP packet
    when SA moving is detected. When set to RLDP_TRANSMIT_MODE_1, Asic will send RLDP packet periodically.
*/
ret_t rtl8367b_setAsicRldp_mode(rtk_uint32 mode)
{
    if(mode > 2)
        return RT_ERR_INPUT;

    return rtl8367b_setAsicRegBit(RTL8367B_RLDP_CTRL0_REG, RTL8367B_RLDP_MODE_OFFSET, mode);
}



