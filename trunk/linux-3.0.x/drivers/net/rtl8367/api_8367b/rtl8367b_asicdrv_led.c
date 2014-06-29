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
 * Feature : LED related functions
 *
 */
#include "rtl8367b_asicdrv_led.h"
/* Function Name:
 *      rtl8367b_setAsicLedIndicateInfoConfig
 * Description:
 *      Set Leds indicated information mode
 * Input:
 *      ledno 	- LED group number. There are 1 to 1 led mapping to each port in each led group
 *      config 	- Support 16 types configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *		The API can set LED indicated information configuration for each LED group with 1 to 1 led mapping to each port.
 *		Definition        LED Statuses            Description
 *		0000        LED_Off                LED pin Tri-State.
 *		0001        Dup/Col                Collision, Full duplex Indicator. Blinking every 43ms when collision happens. Low for full duplex, and high for half duplex mode.
 *		0010        Link/Act               Link, Activity Indicator. Low for link established. Link/Act Blinks every 43ms when the corresponding port is transmitting or receiving.
 *		0011        Spd1000                1000Mb/s Speed Indicator. Low for 1000Mb/s.
 *		0100        Spd100                 100Mb/s Speed Indicator. Low for 100Mb/s.
 *		0101        Spd10                  10Mb/s Speed Indicator. Low for 10Mb/s.
 *		0110        Spd1000/Act            1000Mb/s Speed/Activity Indicator. Low for 1000Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
 *		0111        Spd100/Act             100Mb/s Speed/Activity Indicator. Low for 100Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
 *		1000        Spd10/Act              10Mb/s Speed/Activity Indicator. Low for 10Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
 *		1001        Spd100 (10)/Act        10/100Mb/s Speed/Activity Indicator. Low for 10/100Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
 *		1010        Fiber                  Fiber link Indicator. Low for Fiber.
 *		1011        Fault                  Auto-negotiation     Fault Indicator. Low for Fault.
 *		1100        Link/Rx                Link, Activity Indicator. Low for link established. Link/Rx Blinks every 43ms when the corresponding port is transmitting.
 *		1101        Link/Tx                Link, Activity Indicator. Low for link established. Link/Tx Blinks every 43ms when the corresponding port is receiving.
 *		1110        Master                 Link on Master Indicator. Low for link Master established.
 *		1111        LED_Force              Force LED output, LED output value reference
 */
ret_t rtl8367b_setAsicLedIndicateInfoConfig(rtk_uint32 ledno, rtk_uint32 config)
{
    ret_t   retVal;
    CONST rtk_uint16 bits[RTL8367B_LEDGROUPNO] = {RTL8367B_LED0_CFG_MASK, RTL8367B_LED1_CFG_MASK, RTL8367B_LED2_CFG_MASK};

    if(ledno >= RTL8367B_LEDGROUPNO)
        return RT_ERR_OUT_OF_RANGE;

    if(config >= LEDCONF_END)
        return RT_ERR_OUT_OF_RANGE;

    retVal = rtl8367b_setAsicRegBit(RTL8367B_REG_LED_CONFIGURATION, RTL8367B_LED_CONFIG_SEL_OFFSET, 0);
    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_LED_CONFIGURATION, bits[ledno], config);
}
/* Function Name:
 *      rtl8367b_getAsicLedIndicateInfoConfig
 * Description:
 *      Get Leds indicated information mode
 * Input:
 *      ledno 	- LED group number. There are 1 to 1 led mapping to each port in each led group
 *      pConfig 	- Support 16 types configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *		None
 */
ret_t rtl8367b_getAsicLedIndicateInfoConfig(rtk_uint32 ledno, rtk_uint32* pConfig)
{
    CONST rtk_uint16 bits[RTL8367B_LEDGROUPNO]= {RTL8367B_LED0_CFG_MASK, RTL8367B_LED1_CFG_MASK, RTL8367B_LED2_CFG_MASK};

    if(ledno >= RTL8367B_LEDGROUPNO)
        return RT_ERR_OUT_OF_RANGE;

    /* Get register value */
    return rtl8367b_getAsicRegBits(RTL8367B_REG_LED_CONFIGURATION, bits[ledno], pConfig);
}
/* Function Name:
 *      rtl8367b_setAsicLedGroupMode
 * Description:
 *      Set Led Group mode
 * Input:
 *      mode 	- LED mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *		None
 */
ret_t rtl8367b_setAsicLedGroupMode(rtk_uint32 mode)
{
    ret_t retVal;

    /* Invalid input parameter */
    if(mode >= RTL8367B_LED_MODE_END)
        return RT_ERR_OUT_OF_RANGE;

    retVal = rtl8367b_setAsicRegBit(RTL8367B_REG_LED_CONFIGURATION, RTL8367B_LED_CONFIG_SEL_OFFSET, 1);
    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_LED_CONFIGURATION, RTL8367B_DATA_LED_MASK, mode);
}
/* Function Name:
 *      rtl8367b_getAsicLedGroupMode
 * Description:
 *      Get Led Group mode
 * Input:
 *      pMode 	- LED mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *		None
 */
ret_t rtl8367b_getAsicLedGroupMode(rtk_uint32* pMode)
{
    ret_t retVal;
	rtk_uint32 regData;

    retVal = rtl8367b_getAsicRegBit(RTL8367B_REG_LED_CONFIGURATION, RTL8367B_LED_CONFIG_SEL_OFFSET, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

	if(regData!=1)
		return RT_ERR_FAILED;

    return rtl8367b_getAsicRegBits(RTL8367B_REG_LED_CONFIGURATION, RTL8367B_DATA_LED_MASK, pMode);
}
/* Function Name:
 *      rtl8367b_setAsicForceLeds
 * Description:
 *      Set group LED mode
 * Input:
 *      port 	- Physical port number (0~7)
 *      group 	- LED group number
 *      mode 	- LED mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID  	- Invalid port number
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *		None
 */
ret_t rtl8367b_setAsicForceLed(rtk_uint32 port, rtk_uint32 group, rtk_uint32 mode)
{
    rtk_uint16 regAddr;

    /* Invalid input parameter */
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(group >= RTL8367B_LEDGROUPNO)
        return RT_ERR_OUT_OF_RANGE;

    if(mode >= LEDFORCEMODE_END)
        return RT_ERR_OUT_OF_RANGE;
    /* Set Related Registers */
    regAddr = RTL8367B_LED_FORCE_MODE_BASE + (group << 1);
    return rtl8367b_setAsicRegBits(regAddr, 0x3 << (port * 2), mode);
}
/* Function Name:
 *      rtl8367b_getAsicForceLed
 * Description:
 *      Get group LED mode
 * Input:
 *      port 	- Physical port number (0~7)
 *      group 	- LED group number
 *      pMode 	- LED mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID  	- Invalid port number
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *		None
 */
ret_t rtl8367b_getAsicForceLed(rtk_uint32 port, rtk_uint32 group, rtk_uint32* pMode)
{
    rtk_uint16 regAddr;

    /* Invalid input parameter */
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(group >= RTL8367B_LEDGROUPNO)
        return RT_ERR_INPUT;

    /* Get Related Registers */
    regAddr = RTL8367B_LED_FORCE_MODE_BASE + (group << 1);

    return rtl8367b_getAsicRegBits(regAddr,0x3 << (port * 2), pMode);
}
/* Function Name:
 *      rtl8367b_setAsicForceGroupLed
 * Description:
 *      Turn on/off Led of all ports
 * Input:
 *      group 	- LED group number
 *      mode 	- 0b00:normal mode, 0b01:force blink, 0b10:force off, 0b11:force on
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *		None
 */
ret_t rtl8367b_setAsicForceGroupLed(rtk_uint32 groupmask, rtk_uint32 mode)
{
    ret_t retVal;
	rtk_uint32 i,bitmask;
    CONST rtk_uint16 bits[3]= {0x0004,0x0010,0x0040};

    /* Invalid input parameter */
    if(groupmask > RTL8367B_LEDGROUPMASK)
        return RT_ERR_OUT_OF_RANGE;

    if(mode >= LEDFORCEMODE_END)
        return RT_ERR_OUT_OF_RANGE;

    bitmask = 0;
	for(i = 0; i <  RTL8367B_LEDGROUPNO; i++)
	{
	    if(groupmask & (1 << i))
	    {
            bitmask = bitmask | bits[i];
	    }

	}

    retVal = rtl8367b_setAsicRegBits(RTL8367B_LED_FORCE_CTRL, RTL8367B_LED_FORCE_MODE_MASK, bitmask);

    retVal = rtl8367b_setAsicRegBits(RTL8367B_LED_FORCE_CTRL, RTL8367B_FORCE_MODE_MASK, mode);

    if(LEDFORCEMODE_NORMAL == mode)
        retVal = rtl8367b_setAsicRegBits(RTL8367B_LED_FORCE_CTRL, RTL8367B_LED_FORCE_MODE_MASK, 0);

    return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicForceGroupLed
 * Description:
 *      Turn on/off Led of all ports
 * Input:
 *      group 	- LED group number
 *      pMode 	- 0b00:normal mode, 0b01:force blink, 0b10:force off, 0b11:force on
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 * Note:
 *		None
 */
ret_t rtl8367b_getAsicForceGroupLed(rtk_uint32* groupmask, rtk_uint32* pMode)
{
    ret_t retVal;
	rtk_uint32 i,regData;
	CONST rtk_uint16 bits[3] = {0x0004,0x0010,0x0040};

    /* Get Related Registers */
    retVal = rtl8367b_getAsicRegBits(RTL8367B_LED_FORCE_CTRL, RTL8367B_LED_FORCE_MODE_MASK, &regData);

	for(i = 0; i< RTL8367B_LEDGROUPNO; i++)
	{
	    if((regData & bits[i]) == bits[i])
	    {
            *groupmask = *groupmask | (1 << i);
	    }
 	}

    return rtl8367b_getAsicRegBits(RTL8367B_LED_FORCE_CTRL, RTL8367B_FORCE_MODE_MASK, pMode);
}
/* Function Name:
 *      rtl8367b_setAsicLedBlinkRate
 * Description:
 *      Set led blinking rate at mode 0 to mode 3
 * Input:
 *      blinkRate 	- Support 6 blink rates
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *		LED blink rate can be at 43ms, 84ms, 120ms, 170ms, 340ms and 670ms
 */
ret_t rtl8367b_setAsicLedBlinkRate(rtk_uint32 blinkRate)
{
    if(blinkRate >= LEDBLINKRATE_END)
        return RT_ERR_OUT_OF_RANGE;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_LED_MODE, RTL8367B_SEL_LEDRATE_MASK, blinkRate);
}
/* Function Name:
 *      rtl8367b_getAsicLedBlinkRate
 * Description:
 *      Get led blinking rate at mode 0 to mode 3
 * Input:
 *      pBlinkRate 	- Support 6 blink rates
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 * Note:
 *		None
 */
ret_t rtl8367b_getAsicLedBlinkRate(rtk_uint32* pBlinkRate)
{
    return rtl8367b_getAsicRegBits(RTL8367B_REG_LED_MODE, RTL8367B_SEL_LEDRATE_MASK, pBlinkRate);
}
/* Function Name:
 *      rtl8367b_setAsicLedForceBlinkRate
 * Description:
 *      Set LEd blinking rate for force mode led
 * Input:
 *      blinkRate 	- Support 6 blink rates
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *		None
 */
ret_t rtl8367b_setAsicLedForceBlinkRate(rtk_uint32 blinkRate)
{
    if(blinkRate >= LEDFORCERATE_END)
        return RT_ERR_OUT_OF_RANGE;

    return rtl8367b_setAsicRegBits(RTL8367B_REG_LED_MODE, RTL8367B_FORCE_RATE_MASK, blinkRate);
}
/* Function Name:
 *      rtl8367b_getAsicLedForceBlinkRate
 * Description:
 *      Get LED blinking rate for force mode led
 * Input:
 *      pBlinkRate 	- Support 6 blink rates
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 * Note:
 *		None
 */
ret_t rtl8367b_getAsicLedForceBlinkRate(rtk_uint32* pBlinkRate)
{
     return rtl8367b_getAsicRegBits(RTL8367B_REG_LED_MODE, RTL8367B_FORCE_RATE_MASK, pBlinkRate);
}

/*
@func ret_t | rtl8367b_setAsicLedGroupEnable | Turn on/off Led of all system ports
@parm rtk_uint32 | group | LED group id.
@parm rtk_uint32 | portmask | LED port mask.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can turn on/off leds of dedicated port while indicated information configuration of LED group is set to force mode.
 */
ret_t rtl8367b_setAsicLedGroupEnable(rtk_uint32 group, rtk_uint32 portmask)
{
    ret_t retVal;
    rtk_uint32 regAddr;
    rtk_uint32 regDataMask;

    if ( group >= RTL8367B_LEDGROUPNO )
        return RT_ERR_INPUT;

    if ( portmask > 0x1F )
        return RT_ERR_INPUT;

    regAddr = RTL8367B_REG_PARA_LED_IO_EN1 + group/2;

    regDataMask = 0xFF << ((group%2)*8);

    if ((retVal = rtl8367b_setAsicRegBits(regAddr, regDataMask, portmask))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/*
@func ret_t | rtl8367b_getAsicLedGroupEnable | Get on/off status of Led of all system ports
@parm rtk_uint32 | group | LED group id.
@parm rtk_uint32 | *portmask | LED port mask.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can turn on/off leds of dedicated port while indicated information configuration of LED group is set to force mode.
 */
ret_t rtl8367b_getAsicLedGroupEnable(rtk_uint32 group, rtk_uint32 *portmask)
{
    ret_t retVal;
    rtk_uint32 regAddr;
    rtk_uint32 regDataMask;

    if ( group >= RTL8367B_LEDGROUPNO )
        return RT_ERR_INPUT;

    regAddr = RTL8367B_REG_PARA_LED_IO_EN1 + group/2;

    regDataMask = 0xFF << ((group%2)*8);

    if ((retVal = rtl8367b_getAsicRegBits(regAddr, regDataMask, portmask))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/*
@func ret_t | rtl8367b_setAsicLedOperationMode | Set LED operation mode
@parm rtk_uint32 | mode | LED mode. 1:scan mode 1, 2:parallel mode, 3:mdx mode (serial mode)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can turn on/off led serial mode and set signal to active high/low.
 */
ret_t rtl8367b_setAsicLedOperationMode(rtk_uint32 mode)
{
    ret_t retVal;

    /* Invalid input parameter */
    if( mode >= LEDOP_END)
        return RT_ERR_INPUT;

    if( (mode == LEDOP_SCAN0) || (mode == LEDOP_SCAN1) )
        return RT_ERR_INPUT;

    switch(mode)
    {
        case LEDOP_PARALLEL:
            if((retVal = rtl8367b_setAsicRegBit(RTL8367B_REG_LED_SYS_CONFIG, RTL8367B_LED_SELECT_OFFSET, 0))!=  RT_ERR_OK)
		        return retVal;
            /*Disable serial CLK mode*/
            if((retVal = rtl8367b_setAsicRegBit(RTL8367B_REG_SCAN0_LED_IO_EN,RTL8367B_LED_SERI_CLK_EN_OFFSET, 0))!=  RT_ERR_OK)
                return retVal;
            /*Disable serial DATA mode*/
            if((retVal = rtl8367b_setAsicRegBit(RTL8367B_REG_SCAN0_LED_IO_EN,RTL8367B_LED_SERI_DATA_EN_OFFSET, 0))!=  RT_ERR_OK)
                return retVal;
            break;
        case LEDOP_SERIAL:
            if((retVal = rtl8367b_setAsicRegBit(RTL8367B_REG_LED_SYS_CONFIG, RTL8367B_LED_SELECT_OFFSET, 1))!=  RT_ERR_OK)
		        return retVal;
            /*Enable serial CLK mode*/
            if((retVal = rtl8367b_setAsicRegBit(RTL8367B_REG_SCAN0_LED_IO_EN,RTL8367B_LED_SERI_CLK_EN_OFFSET, 1))!=  RT_ERR_OK)
                return retVal;
            /*Enable serial DATA mode*/
            if((retVal = rtl8367b_setAsicRegBit(RTL8367B_REG_SCAN0_LED_IO_EN,RTL8367B_LED_SERI_DATA_EN_OFFSET, 1))!=  RT_ERR_OK)
                return retVal;
            break;
        default:
            break;
    };

    return RT_ERR_OK;
}


/*
@func ret_t | rtl8367b_getAsicLedOperationMode | Get LED OP mode setup
@parm rtk_uint32*| mode | LED mode. 1:scan mode 1, 2:parallel mode, 3:mdx mode (serial mode)
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can get LED serial mode setup and get signal active high/low.
 */
ret_t rtl8367b_getAsicLedOperationMode(rtk_uint32 *mode)
{
    ret_t retVal;
    rtk_uint32 regData;

    if((retVal = rtl8367b_getAsicRegBit(RTL8367B_REG_LED_SYS_CONFIG, RTL8367B_LED_SELECT_OFFSET, &regData))!=  RT_ERR_OK)
		return retVal;

    if (regData == 1)
        *mode = LEDOP_SERIAL;
    else if (regData == 0)
        *mode = LEDOP_PARALLEL;
    else
        return RT_ERR_FAILED;

    return RT_ERR_OK;
}

/*
@func ret_t | rtl8367b_setAsicLedSerialModeConfig | Set LED serial mode
@parm rtk_uint32 | active | Active High or Low.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can turn on/off led serial mode and set signal to active high/low.
 */
ret_t rtl8367b_setAsicLedSerialModeConfig(rtk_uint32 active, rtk_uint32 serimode)
{
    ret_t retVal;

    /* Invalid input parameter */
    if( active >= LEDSERACT_MAX)
        return RT_ERR_INPUT;
    if( serimode >= LEDSER_MAX)
        return RT_ERR_INPUT;

    /* Set Active High or Low */
    if((retVal = rtl8367b_setAsicRegBit(RTL8367B_REG_LED_SYS_CONFIG, RTL8367B_SERI_LED_ACT_LOW_OFFSET, active)) !=  RT_ERR_OK)
        return retVal;

    /*set to 8G mode (not 16G mode)*/
    if((retVal = rtl8367b_setAsicRegBit(RTL8367B_REG_LED_MODE, RTL8367B_DLINK_TIME_OFFSET, serimode))!=  RT_ERR_OK)
        return retVal;


    return RT_ERR_OK;
}


/*
@func ret_t | rtl8367b_getAsicLedSerialModeConfig | Get LED serial mode setup
@parm rtk_uint32*| active | Active High or Low.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can get LED serial mode setup and get signal active high/low.
 */
ret_t rtl8367b_getAsicLedSerialModeConfig(rtk_uint32 *active, rtk_uint32 *serimode)
{
    ret_t retVal;

    if((retVal = rtl8367b_getAsicRegBit(RTL8367B_REG_LED_SYS_CONFIG, RTL8367B_SERI_LED_ACT_LOW_OFFSET, active))!=  RT_ERR_OK)
        return retVal;

    /*get to 8G mode (not 16G mode)*/
    if((retVal = rtl8367b_getAsicRegBit(RTL8367B_REG_LED_MODE, RTL8367B_DLINK_TIME_OFFSET, serimode))!=  RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}
