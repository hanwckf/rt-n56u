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
 * $Date: 2010/12/02 04:34:29 $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367B
 * Feature : 
 *
 */
#include "rtl8370_asicdrv_specialCongest.h"

/*
@func ret_t | rtl8370_setAsicSpecialCongestModeConfig | Set ASIC special congest mode configuration. 
@parm uint32 | port | port number (0~15).
@uint32 | sustain | sustain timer (0-15).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_OUT_OF_RANGE | Invalid timer.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can set ASIC special congest mode per port. It is a timer that if the source port was blocked by 
    pause frame and get into congestion state, the port will return normal state after sustain time.
*/

ret_t rtl8370_setAsicSpecialCongestModeConfig(uint32 port, uint32 sustain)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;
    if(sustain > RTL8370_SPECIALCONGEST_SUSTAIN_TIMERMAX)
        return RT_ERR_OUT_OF_RANGE;

    return rtl8370_setAsicRegBits(RTL8370_PORT_MISC_CFG_REG(port), RTL8370_SPECIALCONGEST_SUSTAIN_TIMER_MASK, sustain);
}

/*
@func ret_t | rtl8370_getAsicSpecialCongestModeConfig | Get ASIC special congest mode configuration. 
@parm uint32 | port | port number (0~8).
@uint32* | sustain | sustain timer (0-15).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can get ASIC special congest mode setup per port.
*/
ret_t rtl8370_getAsicSpecialCongestModeConfig(uint32 port, uint32 *sustain)
{
    if(port >= RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBits(RTL8370_PORT_MISC_CFG_REG(port), RTL8370_SPECIALCONGEST_SUSTAIN_TIMER_MASK, sustain); 
}

/*
@func ret_t | rtl8370_getAsicSpecialCongestModeTimer | Get ASIC special congest mode timer. 
@parm uint32 | port | port number (0~15).
@uint32* | timer | time (0-15).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid port number.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can get ASIC special congest mode time from congestion state to normal state now per port.
*/
ret_t rtl8370_getAsicSpecialCongestModeTimer(uint32 port, uint32* timer)
{
    if(port > RTL8370_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8370_getAsicRegBits(RTL8370_PORT_SPECIAL_CONGEST_MODE_TIMER_REG(port), RTL8370_PORT_SPECIAL_CONGEST_MODE_TIMER_MASK, timer);
}
