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

#include "rtl8370_asicdrv_oam.h"

/*
@func ret_t | rtl8370_setAsicOamParser | Set OAM parser state
@parm uint32 | port | Physical port number (0~7).
@parm uint32 | parser | Per-Port OAM parser state
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can set per-port OAM parser state. Using OAM parser and multiplex configuration to implement 
    OAM loopback testing.
 */
 ret_t rtl8370_setAsicOamParser(uint32 port, uint32 parser)
{
    uint32 regBits;
    uint32 regAddr;

    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    if(parser > OAM_PARFWDCPU)
        return RT_ERR_NOT_ALLOWED;

    regAddr = RTL8370_OAM_PARSER_REG(port);
    regBits = RTL8370_OAM_PARSER_MASK(port);

    return rtl8370_setAsicRegBits(regAddr, regBits, parser);
}

/*
@func ret_t | rtl8370_getAsicOamParser | Set OAM parser state
@parm uint32 | port | Physical port number (0~7).
@parm uint32* | parser | Per-Port OAM parser state
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can get per-port OAM parser state. Using OAM parser and multiplex configuration to implement 
    OAM loopback testing.
 */
ret_t rtl8370_getAsicOamParser(uint32 port, uint32* parser)
{
    uint32 regAddr;
    uint32 regBits;

    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    regAddr = RTL8370_OAM_PARSER_REG(port);
    regBits = RTL8370_OAM_PARSER_MASK(port);

    return rtl8370_getAsicRegBits(regAddr,regBits, &(*parser));
}

/*
@func ret_t | rtl8370_setAsicOamMultiplexer | Set OAM multiplexer state
@parm uint32 | port | Physical port number (0~7).
@parm uint32 | multiplexer | Per-Port OAM multiplexer state
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can set per-port OAM multiplexer state. Using OAM parser and multiplex configuration to implement 
    OAM loopback testing.
 */
ret_t rtl8370_setAsicOamMultiplexer(uint32 port, uint32 multiplexer)
{
    uint32 regBits;
    uint32 regAddr;

    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    if(multiplexer > OAM_MULCPU)
        return RT_ERR_NOT_ALLOWED;

    regAddr = RTL8370_OAM_MULTIPLEXER_REG(port);
    regBits = RTL8370_OAM_MULTIPLEXER_MASK(port);

    return rtl8370_setAsicRegBits(regAddr, regBits, multiplexer);
}

/*
@func ret_t | rtl8370_getAsicOamMultiplexer | Get OAM multiplexer state
@parm uint32 | port | Physical port number (0~7).
@parm uint32* | multiplexer | Per-Port OAM multiplexer state
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can get per-port OAM multiplexer state. Using OAM parser and multiplex configuration to implement 
    OAM loopback testing.
 */
ret_t rtl8370_getAsicOamMultiplexer(uint32 port, uint32* multiplexer)
{
    uint32 regAddr;
    uint32 regBits;

    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    regAddr = RTL8370_OAM_MULTIPLEXER_REG(port);
    regBits = RTL8370_OAM_MULTIPLEXER_MASK(port);

    return rtl8370_getAsicRegBits(regAddr,regBits, &(*multiplexer));
}
/*
@func ret_t | rtl8370_setAsicOamCpuPri | Set trap priority for OAM packet
@parm uint32 | priority | Mirrored priority (0~7). 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_QOS_INT_PRIORITY | Invalid priority. 
@comm
    The API can set trapped priority for OAM packet
 */

ret_t rtl8370_setAsicOamCpuPri(uint32 priority)
{
    if(priority > RTL8370_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    return rtl8370_setAsicRegBits(RTL8370_REG_QOS_TRAP_PRIORITY0, RTL8370_OAM_PRIOIRTY_MASK, priority);
}
/*
@func ret_t | rtl8370_getAsicOamCpuPri | Get trap priority for OAM packet
@parm uint32* | priority | Mirrored priority (0~7). 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get trapped priority for OAM packet
 */
ret_t rtl8370_getAsicOamCpuPri(uint32 *priority)
{
    return rtl8370_getAsicRegBits(RTL8370_REG_QOS_TRAP_PRIORITY0, RTL8370_OAM_PRIOIRTY_MASK, priority);
}

/*
@func ret_t | rtl8370_setAsicOamEnable | Set OAM function
@parm uint32 | enabled | OAM function usage 1:enable, 0:disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can set OAM function enabled or not
 */
ret_t rtl8370_setAsicOamEnable(uint32 enabled)
{
    return rtl8370_setAsicRegBit(RTL8370_REG_OAM_CTRL, RTL8370_OAM_CTRL_OFFSET, enabled);
}
/*
@func ret_t | rtl8370_getAsicOamEnable | Get OAM function
@parm uint32* | enabled | OAM function usage 1:enable, 0:disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get OAM function enabled or not
 */
ret_t rtl8370_getAsicOamEnable(uint32 *enabled)
{
    return rtl8370_getAsicRegBit(RTL8370_REG_OAM_CTRL, RTL8370_OAM_CTRL_OFFSET, enabled);
}


