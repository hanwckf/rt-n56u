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
 * $Revision: 15536 $
 * $Date: 2011-01-21 17:31:17 +0800 (星期五, 21 一月 2011) $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367B
 * Feature : 
 *
 */

#include "rtl8370_asicdrv_rldp.h"

/*
@func ret_t | rtl8370_setAsicRldp | Set RLDP function enable/disable.
@parm uint32 | enable | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API enable / disable RLDP function
    If RLDP is disabled, switch will not enter checking state when
    switch detects the event of SA moving. The Loop Detection Frame
    will be treated as normal broadcasting frame
*/
ret_t rtl8370_setAsicRldp(uint32 enable)
{
    if(enable > 1)
        return RT_ERR_INPUT;

    return rtl8370_setAsicRegBit(RTL8370_RLDP_CTRL0_REG, RTL8370_RLDP_ENABLE_OFFSET, enable);
}

/*
@func ret_t | rtl8370_getAsicRldp | Get RLDP function enable/disable.
@parm uint32* | enable | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API enable / disable RLDP function
    If RLDP is disabled, switch will not enter checking state when
    switch detects the event of SA moving. The Loop Detection Frame
    will be treated as normal broadcasting frame
*/
ret_t rtl8370_getAsicRldp(uint32 *enable)
{
    return rtl8370_getAsicRegBit(RTL8370_RLDP_CTRL0_REG, RTL8370_RLDP_ENABLE_OFFSET, enable);
}

/*
@func ret_t | rtl8370_setAsicRldpEnable | Set RLDP function handled by ASIC or 8051.
@parm uint32 | enable | 1: enabled 8051, 0: disabled 8051 (RLDP is handled by ASIC).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set  RLDP_8051_ENABLE
    If RLDP_8051_ENABLE is set, RLDP is handled by 8051. Otherwise, 
    ASIC will handled RLDP. 
*/
ret_t rtl8370_setAsicRldpEnable8051(uint32 enable)
{
    if(enable > 1)
        return RT_ERR_INPUT;

    return rtl8370_setAsicRegBit(RTL8370_RLDP_CTRL0_REG, RTL8370_RLDP_8051_ENABLE_OFFSET, enable);
}

/*
@func ret_t | rtl8370_getAsicRldpEnable | Get RLDP function handled by ASIC or 8051.
@parm uint32* | enable | 1: enabled 8051, 0: disabled 8051 (RLDP is handled by ASIC).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get  RLDP_8051_ENABLE
    If RLDP_8051_ENABLE is set, RLDP is handled by 8051. Otherwise, 
    ASIC will handled RLDP. 
*/
ret_t rtl8370_getAsicRldpEnable8051(uint32 *enable)
{
    return rtl8370_getAsicRegBit(RTL8370_RLDP_CTRL0_REG, RTL8370_RLDP_8051_ENABLE_OFFSET, enable);
}

/*
@func ret_t | rtl8370_setAsicRldpCompareRandomNumber | Set RLDP_COMP_ID.
@parm uint32 | enable | 1: enabled comparing random number, 0: disabled comparing random number.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set RLDP_COMP_ID
    If RLDP_COMP_ID is set, ASIC will compare the random number field and
    seed field of RLDP frame with those of ASIC to recognize whether a RLDP
    frame is sent by itself. If RLDP_COMP_ID is 0, ASIC only compare the seed
    field
*/
ret_t rtl8370_setAsicRldpCompareRandomNumber(uint32 enable)
{
    if(enable > 1)
        return RT_ERR_INPUT;

    return rtl8370_setAsicRegBit(RTL8370_RLDP_CTRL0_REG, RTL8370_RLDP_COMP_ID_OFFSET, enable);
}

/*
@func ret_t | rtl8370_getAsicRldpCompareRandomNumber | Get RLDP_COMP_ID.
@parm uint32* | enable | 1: enabled comparing random number, 0: disabled comparing random number.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get RLDP_COMP_ID
    If RLDP_COMP_ID is set, ASIC will compare the random number field and
    seed field of RLDP frame with those of ASIC to recognize whether a RLDP
    frame is sent by itself. If RLDP_COMP_ID is 0, ASIC only compare the seed
    field
*/
ret_t rtl8370_getAsicRldpCompareRandomNumber(uint32 *enable)
{
    return rtl8370_getAsicRegBit(RTL8370_RLDP_CTRL0_REG, RTL8370_RLDP_COMP_ID_OFFSET, enable);
}

/*
@func ret_t | rtl8370_setAsicRldpIndicatorSource | Set buzzer and LED source when detecting a loop.
@parm uint32 | enable | 0: ASIC, 1: 8051
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set the buzzer and LED source.
    Buzzer and LED source can be from ASIC or 8051. RLDP_INDICATOR_SOURCE
    decides the source
*/
ret_t rtl8370_setAsicRldpIndicatorSource(uint32 src)
{
    if(src > 1)
        return RT_ERR_INPUT;

    return rtl8370_setAsicRegBit(RTL8370_RLDP_CTRL0_REG, RTL8370_RLDP_INDICATOR_SOURCE_OFFSET, src);
}

/*
@func ret_t | rtl8370_getAsicRldpIndicatorSource | Get buzzer and LED source when detecting a loop.
@parm uint32* | enable | 0: ASIC, 1: 8051
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get the buzzer and LED source.
    Buzzer and LED source can be from ASIC or 8051. RLDP_INDICATOR_SOURCE
    decides the source
*/
ret_t rtl8370_getAsicRldpIndicatorSource(uint32 *src)
{
    return rtl8370_getAsicRegBit(RTL8370_RLDP_CTRL0_REG, RTL8370_RLDP_INDICATOR_SOURCE_OFFSET, src);
}

/*
@func ret_t | rtl8370_setAsicRldpCheckingStatePara | Set retry count and retry period of checking state.
@parm uint32 | retryCount | 0~0xFF (times)
@parm uint32 | retryPeriod | 0~0xFFFF (ms)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set the retry count and retry period of checking state
    Retry count and retry period decide the times Switch sends RLDP frame and 
    the interval between two transmission respecitively in checking state.
*/
ret_t rtl8370_setAsicRldpCheckingStatePara(uint32 retryCount, uint32 retryPeriod)
{
    ret_t ret;
    uint32 regData;
    
    if(retryCount > 0xFF)
        return RT_ERR_INPUT;
    if(retryPeriod > RTL8370_REGDATAMAX)
        return RT_ERR_INPUT;

    ret = rtl8370_getAsicReg(RTL8370_RLDP_RETRY_COUNT_REG, &regData);
    if(RT_ERR_OK != ret)
        return ret;

    regData = (regData & 0xFF) << 8;
    regData |= retryCount;

    ret = rtl8370_setAsicReg(RTL8370_RLDP_RETRY_COUNT_REG, regData);
    if(RT_ERR_OK != ret)
        return ret;

    return rtl8370_setAsicReg(RTL8370_RLDP_RETRY_PERIOD_CHKSTATE_REG, retryPeriod);
}

/*
@func ret_t | rtl8370_getAsicRldpCheckingStatePara | Get retry count and retry period of checking state.
@parm uint32* | retryCount | 0~0xFF (times)
@parm uint32 | retryPeriod | 0~0xFFFF (ms)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get the retry count and retry period of checking state
    Retry count and retry period decide the times Switch sends RLDP frame and 
    the interval between two transmission respecitively in checking state.
*/
ret_t rtl8370_getAsicRldpCheckingStatePara(uint32 *retryCount, uint32 *retryPeriod)
{
    ret_t ret;

    ret = rtl8370_getAsicRegBits(RTL8370_RLDP_RETRY_COUNT_REG, RTL8370_RLDP_RETRY_COUNT_CHKSTATE_READ_MASK, retryCount);
    if(RT_ERR_OK != ret)
        return ret;

    return rtl8370_getAsicReg(RTL8370_RLDP_RETRY_PERIOD_CHKSTATE_REG, retryPeriod);
}

/*
@func ret_t | rtl8370_setAsicRldpLoopStatePara | Set retry count and retry period of loop state.
@parm uint32 | retryCount | 0~0xFF (times)
@parm uint32 | retryPeriod | 0~0xFFFF (ms)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set the retry count and retry period of loop state
    Retry count and retry period decide the times Switch sends RLDP frame and 
    the interval between two transmission respecitively in loop state.
*/
ret_t rtl8370_setAsicRldpLoopStatePara(uint32 retryCount, uint32 retryPeriod)
{
    ret_t ret;
    uint32 regData;
    
    if(retryCount > 0xFF)
        return RT_ERR_INPUT;
    if(retryPeriod > RTL8370_REGDATAMAX)
        return RT_ERR_INPUT;

    ret = rtl8370_getAsicReg(RTL8370_RLDP_RETRY_COUNT_REG, &regData);
    if(RT_ERR_OK != ret)
        return ret;

    /* shift retry count of loop state right 8 bits */
    regData = (regData & 0xFF00) >> 8;
    regData |= (retryCount << 8);

    ret = rtl8370_setAsicReg(RTL8370_RLDP_RETRY_COUNT_REG, regData);
    if(RT_ERR_OK != ret)
        return ret;

    return rtl8370_setAsicReg(RTL8370_RLDP_RETRY_PERIOD_LOOPSTATE_REG, retryPeriod);
}

/*
@func ret_t | rtl8370_getAsicRldpLoopStatePara | Get retry count and retry period of loop state.
@parm uint32* | retryCount | 0~0xFF (times)
@parm uint32 | retryPeriod | 0~0xFFFF (ms)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API et the retry count and retry period of loop state
    Retry count and retry period decide the times Switch sends RLDP frame and 
    the interval between two transmission respecitively in loop state.
*/
ret_t rtl8370_getAsicRldpLoopStatePara(uint32 *retryCount, uint32 *retryPeriod)
{
    ret_t ret;

    ret = rtl8370_getAsicRegBits(RTL8370_RLDP_RETRY_COUNT_REG, RTL8370_RLDP_RETRY_COUNT_LOOPSTATE_READ_MASK, retryCount);
    if(RT_ERR_OK != ret)
        return ret;

    return rtl8370_getAsicReg(RTL8370_RLDP_RETRY_PERIOD_LOOPSTATE_REG, retryPeriod);
}

/*
@func ret_t | rtl8370_setAsicRldpTxPortmask | Set portmask that send/forward RLDP frame.
@parm uint32 | pmsk | 0~0xFFFF
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set the portmask of sending broadcast packet and flooding port mask of RLDP packet
    RLDP frame is a kind ofbroadcast frame, but the destination ports will
    follow RLDP_TX_PMSK.
*/
ret_t rtl8370_setAsicRldpTxPortmask(uint32 pmsk)
{
    if(pmsk > RTL8370_PORTMASK)
        return RT_ERR_PORT_MASK;

    return rtl8370_setAsicReg(RTL8370_RLDP_TX_PMSK_REG, pmsk);
}

/*
@func ret_t | rtl8370_getAsicRldpTxPortmask | Get portmask that send/forward RLDP frame.
@parm uint32* | pmsk | 0~0xFFFF
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set the portmask of sending broadcast packet and flooding port mask of RLDP packet
    RLDP frame is a kind ofbroadcast frame, but the destination ports will
    follow RLDP_TX_PMSK.
*/
ret_t rtl8370_getAsicRldpTxPortmask(uint32 *pmsk)
{
    return rtl8370_getAsicReg(RTL8370_RLDP_TX_PMSK_REG, pmsk);
}

/*
@func ret_t | rtl8370_setAsicRldpRandomSeed | Set Random seed of RLDP.
@parm ether_addr_t | seed | MAC
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set the random seed
    Random seed is used to help to generate distinct random number.
    Random seed is also used as a key.
*/
ret_t rtl8370_setAsicRldpRandomSeed(ether_addr_t seed)
{
    ret_t ret;
    uint32 regData;
    uint16 *accessPtr;
    uint32 i;

    accessPtr = (uint16*)&seed;
    
    for (i = 0; i < 3; i++)
    {
        regData = *accessPtr;
        ret = rtl8370_setAsicReg(RTL8370_RLDP_SEED_NUM_REG_BASE - i, regData);
        if(RT_ERR_OK != ret)
            return ret;

        accessPtr++;
    }

    return ret;
}

/*
@func ret_t | rtl8370_getAsicRldpRandomSeed | Get Random seed of RLDP.
@parm ether_addr_t | seed | MAC
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get the random seed
    Random seed is used to help to generate distinct random number.
    Random seed is also used as a key.
*/
ret_t rtl8370_getAsicRldpRandomSeed(ether_addr_t *seed)
{
    ret_t ret;
    uint32 regData;
    uint16 *accessPtr;
    uint32 i;

    accessPtr = (uint16*)seed;

    for(i = 0; i < 3; i++)
    {
        ret = rtl8370_getAsicReg(RTL8370_RLDP_SEED_NUM_REG_BASE - i, &regData);
        if(RT_ERR_OK != ret)
            return ret;
        
        *accessPtr = regData;
        accessPtr++;
    }

    return ret;
}

/*
@func ret_t | rtl8370_setAsicRldpLoopedPortmask | Set looped portmask.
@parm uint32 | pmsk | 0~0xFFFF
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set the looped portmask. The API mainly used to clear
    When Switch detects looped ports, the corresponding bit in looped 
    portmask of looped ports will be set
*/
ret_t rtl8370_setAsicRldpLoopedPortmask(uint32 pmsk)
{
    return rtl8370_setAsicReg(RTL8370_RLDP_LOOP_PMSK_REG, pmsk);
}

/*
@func ret_t | rtl8370_getAsicRldpLoopedPortmask | Get looped portmask.
@parm uint32* | pmsk | 0~0xFFFF
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get the looped portmask.
    When Switch detects looped ports, the corresponding bit in looped 
    portmask of looped ports will be set
*/
ret_t rtl8370_getAsicRldpLoopedPortmask(uint32 *pmsk)
{
    return rtl8370_getAsicReg(RTL8370_RLDP_LOOP_PMSK_REG, pmsk);
}

/*
@func ret_t | rtl8370_getAsicRldpRandomNumber | Get Random number of RLDP.
@parm ether_addr_t | seed | MAC
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get the current random number.
    Every time when Switch enters checking state, switch will generate a new
    random number. The random number may be used as a key.
*/
ret_t rtl8370_getAsicRldpRandomNumber(ether_addr_t *randNumber)
{
    ret_t ret;
    uint32 regData;
    uint16 *accessPtr;
    uint32 i;

    accessPtr = (uint16*)randNumber;

    for(i = 0; i < 3; i++)
    {
        ret = rtl8370_getAsicReg(RTL8370_RLDP_RAND_NUM_REG_BASE + i, &regData);
        if(RT_ERR_OK != ret)
            return ret;
        
        *accessPtr = regData;
        accessPtr++;
    }

    return ret;
}

/*
@func ret_t | rtl8370_getAsicRldpLoopedPortmask | Get port number of looped pair.
@parm uint32 | port | 0~15
@parm uint32* | port | 0~15
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    This API get the looped pair of each port.
    If port n is a looped port, port that looped with port n in switch 
    will be record. 
*/
ret_t rtl8370_getAsicRldpLoopedPortPair(uint32 port, uint32 *loopedPair)
{
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBits(RTL8370_RLDP_LOOP_PORT_REG(port), RTL8370_RLDP_LOOP_PORT_MASK(port), loopedPair);
}

/*
@func ret_t | rtl8370_setAsicRldp_mode | Set RLDP transmitting mode.
@parm uint32 | mode | 1: Periodically send RLDP packet, 0: Send RLDP packet when SA moving is detected.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API Set RLDP transmitting mode. When set to RLDP_TRANSMIT_MODE_0, Asic will send RLDP packet 
    when SA moving is detected. When set to RLDP_TRANSMIT_MODE_1, Asic will send RLDP packet periodically.
*/
ret_t rtl8370_setAsicRldp_mode(uint32 mode)
{
    if(mode > 2)
        return RT_ERR_INPUT;

    return rtl8370_setAsicRegBit(RTL8370_RRCP_CTRL0_REG, RTL8370_RRCP_RLDP_MODE_OFFSET, mode);
}

