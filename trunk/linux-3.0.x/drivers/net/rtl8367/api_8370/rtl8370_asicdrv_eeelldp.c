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
#include "rtl8370_asicdrv_eeelldp.h"

ret_t _rtl8370_setAsicEeelldpFrameDataReg(uint32 regAddr, uint32 dataLength, int8 *writeDataPtr);
ret_t _rtl8370_getAsicEeelldpFrameDataReg(uint32 regAddr, uint32 dataLength, int8 *writeDataPtr);

/*
@func ret_t | rtl8370_setAsicEeelldp | Set eeelldp function enable/disable.
@parm uint32 | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set the EEELLDP enable function.
    If EEELLDP function is enabled, upon receiving an eeelldp frame,
    ASIC will signal 8051 (interrupt or trap) to handled the received
    EEELLDP frame
*/
ret_t rtl8370_setAsicEeelldp(uint32 enable)
{
    if (enable > 1)
        return RT_ERR_INPUT;
    
    return rtl8370_setAsicRegBit(RTL8370_EEELLDP_CTRL0_REG, RTL8370_EEELLDP_ENABLE_OFFSET, enable);
}

/*
@func ret_t | rtl8370_getAsicEeelldp | Get eeelldp function enable/disable.
@parm uint32* | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get the EEELLDP enable function.
    If EEELLDP function is enabled, upon receiving an eeelldp frame,
    ASIC will signal 8051 (interrupt or trap) to handled the received
    EEELLDP frame
*/
ret_t rtl8370_getAsicEeelldp(uint32 *enable)
{
    return rtl8370_getAsicRegBit(RTL8370_EEELLDP_CTRL0_REG, RTL8370_EEELLDP_ENABLE_OFFSET, enable);
}

/*
@func ret_t | rtl8370_setAsicEeelldpTrapCpu | Set trap eeelldp to CPU/not trap to CPU.
@parm uint32 | trap | 1: trap, 0: do not trap.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set the enabling of trap EEELLDP packet to CPU function.
    If EEELLDP function is enabled, upon receiving an eeelldp frame,
    ASIC will trap the eeelldp frame to CPU
*/
ret_t rtl8370_setAsicEeelldpTrapCpu(uint32 trap)
{
    if (trap > 1)
        return RT_ERR_INPUT;
    
    return rtl8370_setAsicRegBit(RTL8370_EEELLDP_CTRL0_REG, RTL8370_EEELLDP_TRAP_CPU_OFFSET, trap);
}

/*
@func ret_t | rtl8370_getAsicEeelldpTrap8051 | Get trap eeelldp to CPU/not trap to CPU.
@parm uint32* | trap | 1: trap, 0: do not trap.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get the enabling of trap EEELLDP packet to CPU function.
    If EEELLDP function is enabled, upon receiving an eeelldp frame,
    ASIC will trap the eeelldp frame to CPU
*/
ret_t rtl8370_getAsicEeelldpTrapCpu(uint32 *trap)
{    
    return rtl8370_getAsicRegBit(RTL8370_EEELLDP_CTRL0_REG, RTL8370_EEELLDP_TRAP_CPU_OFFSET, trap);
}

/*
@func ret_t | rtl8370_getAsicEeelldpTrap8051 | Set trap eeelldp to 8051/not trap to 8051.
@parm uint32 | trap | 1: trap, 0: do not trap.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set the enabling of trap EEELLDP packet to 8051 function.
    If EEELLDP function is enabled, upon receiving an eeelldp frame,
    ASIC will trap the eeelldp frame to 8051
*/
ret_t rtl8370_setAsicEeelldpTrap8051(uint32 trap)
{
    if (trap > 1)
        return RT_ERR_INPUT;
    
    return rtl8370_setAsicRegBit(RTL8370_EEELLDP_CTRL0_REG, RTL8370_EEELLDP_TRAP_DW8051_OFFSET, trap);
}

/*
@func ret_t | rtl8370_getAsicEeelldpTrap8051 | Get trap eeelldp to 8051/not trap to 8051.
@parm uint32 | trap | 1: trap, 0: do not trap.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get the enabling of trap EEELLDP packet to 8051 function.
    If EEELLDP function is enabled, upon receiving an eeelldp frame,
    ASIC will trap the eeelldp frame to CPU
*/
ret_t rtl8370_getAsicEeelldpTrap8051(uint32 *trap)
{    
    return rtl8370_getAsicRegBit(RTL8370_EEELLDP_CTRL0_REG, RTL8370_EEELLDP_TRAP_DW8051_OFFSET, trap);
}

/*
@func ret_t | rtl8370_setAsicEeelldpTrapCpuPri | Set trap eeelldp trap to CPU priority.
@parm uint32 | priority | trap to CPU priority (0~7).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set the priority of EEELLDP pakcet trapping to CPU.
    If EEELLDP function is enabled, upon receiving an eeelldp frame,
    ASIC will trap the eeelldp frame to 8051
*/
ret_t rtl8370_setAsicEeelldpTrapCpuPri(uint32 priority)
{
    if (priority > RTL8370_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;
    
    return rtl8370_setAsicRegBits(RTL8370SG_QOS_TRAP_PRIORITY_CTRL1_REG, RTL8370_EEELLDP_TRAP_PRI_MASK, priority);
}

/*
@func ret_t | rtl8370_getAsicEeelldpTrapCpuPri | Get trap eeelldp trap to CPU priority.
@parm uint32* | priority | trap to CPU priority (0~7).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set the priority of EEELLDP pakcet trapping to CPU.
    If EEELLDP function is enabled, upon receiving an eeelldp frame,
    ASIC will trap the eeelldp frame to 8051
*/
ret_t rtl8370_getAsicEeelldpTrapCpuPri(uint32 *priority)
{
    return rtl8370_getAsicRegBits(RTL8370SG_QOS_TRAP_PRIORITY_CTRL1_REG, RTL8370_EEELLDP_TRAP_PRI_MASK, priority);
}

/*
@func ret_t | rtl8370_setAsicEeelldpInterrupt8051 | Set interrupt to 8051/not interrupt to 8051 while receiving a eeelldp.
@parm uint32 | interrupt | 1: interrupt, 0: do not interrupt.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set the enabling of interrupting to 8051 when switch receives a EEELLDP packet.
    If EEELLDP function is enabled, upon receiving an eeelldp frame,
    ASIC will interrupt to 8051
*/
ret_t rtl8370_setAsicEeelldpInterrupt8051(uint32 interrupt_en)
{
    if (interrupt_en > 1)
        return RT_ERR_INPUT;
    
    return rtl8370_setAsicRegBit(RTL8370_EEELLDP_CTRL0_REG, RTL8370_EEELLDP_INT_8051_OFFSET, interrupt_en);
}

/*
@func ret_t | rtl8370_getAsicEeelldpInterrupt8051 | Get interrupt to 8051/not interrupt to 8051 while receiving a eeelldp.
@parm uint32* | interrupt | 1: interrupt, 0: do not interrupt.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set the enabling of interrupting to 8051 when switch receives a EEELLDP packet.
    If EEELLDP function is enabled, upon receiving an eeelldp frame,
    ASIC will interrupt to 8051
*/
ret_t rtl8370_getAsicEeelldpInterrupt8051(uint32 *interrupt_en)
{
    return rtl8370_getAsicRegBit(RTL8370_EEELLDP_CTRL0_REG, RTL8370_EEELLDP_INT_8051_OFFSET, interrupt_en);
}

/*
@func ret_t | rtl8370_setAsicEeelldpSubtype | Set eeelldp frame subtype of ASIC.
@parm uint32 | subtype | (0~0xFF).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set subtype of EEELLDP.
    The subtype is used to identify a eeelldp frame
*/
ret_t rtl8370_setAsicEeelldpSubtype(uint32 subtype)
{
    if (subtype > 0xFF)
        return RT_ERR_INPUT;
    
    return rtl8370_setAsicRegBits(RTL8370_EEELLDP_CTRL0_REG, RTL8370_EEELLDP_SUBTYPE_MASK, subtype);
}

/*
@func ret_t | rtl8370_getAsicEeelldpSubtype | Get eeelldp frame subtype of ASIC.
@parm uint32 | subtype | (0~0xFF).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get subtype of EEELLDP.
    The subtype is used to identify a eeelldp frame
*/
ret_t rtl8370_getAsicEeelldpSubtype(uint32 *subtype)
{
    return rtl8370_getAsicRegBits(RTL8370_EEELLDP_CTRL0_REG, RTL8370_EEELLDP_SUBTYPE_MASK, subtype);
}

ret_t _rtl8370_setAsicEeelldpFrameDataReg(uint32 regAddr, uint32 dataLength, int8 *writeDataPtr)
{
    ret_t ret;
    uint32 i;
    uint32 regData;
    uint16 *accessPtr;

    accessPtr = (uint16*)writeDataPtr;

    for(i=0; i < dataLength / 2; i++)
    {
        regData = *accessPtr;
        ret = rtl8370_setAsicReg(regAddr + i, regData);
        if(RT_ERR_OK != ret)
            return ret;

        accessPtr++;
    }
    
    if (dataLength & 0x1)
    {
        regData = *accessPtr;
        ret = rtl8370_setAsicRegBits(regAddr + dataLength / 2, 0xFF, regData);
        if(RT_ERR_OK != ret)
            return ret;
    }

    return RT_ERR_OK;
}

ret_t _rtl8370_getAsicEeelldpFrameDataReg(uint32 regAddr, uint32 dataLength, int8 *readDataPtr)
{
    ret_t ret;
    uint32 i;
    uint32 regData;
    uint16 *accessPtr;

    accessPtr = (uint16*)readDataPtr;

    for(i=0; i < dataLength / 2; i++)
    {
        ret = rtl8370_getAsicReg(regAddr + i, &regData);
        if(RT_ERR_OK != ret)
            return ret;
        
        *accessPtr = (int16)regData;
        accessPtr++;
    }
    
    if (dataLength & 0x1)
    {
        ret = rtl8370_getAsicRegBits(regAddr + dataLength / 2, 0xFF, &regData);
        if (RT_ERR_OK != ret)
            return ret;

        *accessPtr = (int16)regData;
    }

    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_setAsicEeelldpTxFrameUpper | Set octet 14~40 of eeelldp frame template
@parm int8* | frameUPtr | .
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set the template of octect 14~40 in EEELLDP frame that will be transmitted
    frameUPtr points to the starting address.
*/
ret_t rtl8370_setAsicEeelldpTxFrameUpper(int8 *frameUPtr)
{
    return _rtl8370_setAsicEeelldpFrameDataReg(RTL8370_EEELLDP_TX_FRAMEU_REG_BASE, RTL8370_EEELLDP_FRAMEU_LENGTH, frameUPtr);
}


/*
@func ret_t | rtl8370_getAsicEeelldpTxFrameUpper | Get octet 14~40 of eeelldp frame template
@parm int8* | frameUPtr | .
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get the template of octect 14~40 in EEELLDP frame that will be transmitted
    frameUPtr points to the starting address.
*/
ret_t rtl8370_getAsicEeelldpTxFrameUpper(int8 *frameUPtr)
{
    return _rtl8370_getAsicEeelldpFrameDataReg(RTL8370_EEELLDP_TX_FRAMEU_REG_BASE, RTL8370_EEELLDP_FRAMEU_LENGTH, frameUPtr);
}

/*
@func ret_t | rtl8370_setAsicEeelldpTxCapFrameLower | Set octet 42~59 of capability eeelldp frame template
@parm int8* | frameLPtr | .
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set the template of octect 42~59 in Capability EEELLDP frame that will be transmitted
    frameLPtr points to the starting address.
*/
ret_t rtl8370_setAsicEeelldpTxCapFrameLower(int8 *frameLPtr)
{
    return _rtl8370_setAsicEeelldpFrameDataReg(RTL8370_EEELLDP_TX_CAP_FRAMEL_REG_BASE, RTL8370_EEELLDP_FRAMEL_LENGTH, frameLPtr);
}

/*
@func ret_t | rtl8370_getAsicEeelldpTxCapFrameLower | Get octet 42~59 of capability eeelldp frame template
@parm int8* | dataPtr | .
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get the template of octect 42~59 in Capability EEELLDP frame that will be transmitted
    frameLPtr points the starting address.
*/
ret_t rtl8370_getAsicEeelldpTxCapFrameLower(int8 *frameLPtr)
{
    return _rtl8370_getAsicEeelldpFrameDataReg(RTL8370_EEELLDP_TX_CAP_FRAMEL_REG_BASE, RTL8370_EEELLDP_FRAMEL_LENGTH, frameLPtr);
}

/*
@func ret_t | rtl8370_setAsicEeelldpTxAckFrameLower | Set octet 42~59 of capability eeelldp frame template
@parm int8* | frameLPtr | .
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set the template of octect 42~59 in Acknowledge EEELLDP frame that will be transmitted
    frameLPtr points the starting address.
*/
ret_t rtl8370_setAsicEeelldpTxAckFrameLower(int8 *frameLPtr)
{
    return _rtl8370_setAsicEeelldpFrameDataReg(RTL8370_EEELLDP_TX_ACK_FRAMEL_REG_BASE, RTL8370_EEELLDP_FRAMEL_LENGTH, frameLPtr);
}

/*
@func ret_t | rtl8370_getAsicEeelldpTxAckFrameLower | Get octet 42~59 of capability eeelldp frame template
@parm int8* | frameLPtr | .
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get the template of octect 42~59 in Acknowledge EEELLDP frame that will be transmitted
    frameLPtr points the starting address.
*/
ret_t rtl8370_getAsicEeelldpTxAckFrameLower(int8 *frameLPtr)
{
    return _rtl8370_getAsicEeelldpFrameDataReg(RTL8370_EEELLDP_TX_ACK_FRAMEL_REG_BASE, RTL8370_EEELLDP_FRAMEL_LENGTH, frameLPtr);
}

/*
@func ret_t | rtl8370_setAsicEeelldpRxPortmask | Get portmask of ports that receive EEELLDP frame.
@parm uint32 | pmsk | (0~0xFFFF).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get ports that just received EEELLDP packet. 
    The portmask is used to record which port has received a eeelldp frame
    Software should remember to clear the corresponding bit after reading
    the rx value
*/
ret_t rtl8370_setAsicEeelldpRxPortmask(uint32 pmsk)
{
    return rtl8370_setAsicReg(RTL8370_EEELLDP_PMSK_REG, pmsk);
}

/*
@func ret_t | rtl8370_getAsicEeelldpRxPortmask | Get portmask of ports that receive EEELLDP frame.
@parm uint32* | pmsk | (0~0xFFFF).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set ports that just received EEELLDP packet. 
    The portmask is used to record which port has received a eeelldp frame
    Software should remember to clear the corresponding bit after reading
    the rx value
*/
ret_t rtl8370_getAsicEeelldpRxPortmask(uint32 *pmsk)
{
    return rtl8370_getAsicReg(RTL8370_EEELLDP_PMSK_REG, pmsk);
}

/*
@func ret_t | rtl8370_getAsicEeelldpRxCapFrameLower | Get octet 42~59 of received capability eeelldp frame of port n
@parm enum | port | port number (0~15).
@parm int8* | frameLPtr | .
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error. 
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    This API get octet 42~59 of received EEELLDP packet.
    frameLPtr points to the starting address.
*/
ret_t rtl8370_getAsicEeelldpRxFrameLower(uint32 port, int8 *frameLPtr)
{
    if (port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;
	
    return _rtl8370_getAsicEeelldpFrameDataReg(RTL8370_EEELLDP_RX_VALUE_PORT_REG(port), RTL8370_EEELLDP_FRAMEL_LENGTH, frameLPtr);
}

