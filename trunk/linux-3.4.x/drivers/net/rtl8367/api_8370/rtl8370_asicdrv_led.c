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
 * $Date: 2010/12/02 04:34:24 $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367B
 * Feature : 
 *
 */
#include "rtl8370_asicdrv_led.h"

/*
@func ret_t | rtl8370_setAsicLedOperationMode | Set LED operation mode
@parm uint32 | mode | LED mode. 1:scan mode 1, 2:parallel mode, 3:mdx mode (serial mode) 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can turn on/off led serial mode and set signal to active high/low.
 */
ret_t rtl8370_setAsicLedOperationMode(uint32 mode)
{
    ret_t retVal;
    
    /* Invalid input parameter */
    if( mode >= LEDOP_MAX)
        return RT_ERR_INPUT; 

    if( mode == LEDOP_SCAN0)
        return RT_ERR_INPUT;   

    switch(mode)
    {
        case LEDOP_SCAN0:
            break;
        case LEDOP_SCAN1:
            if((retVal = rtl8370_setAsicReg(RTL8370_REG_LED_SYS_CONFIG,0x1471))!=  RT_ERR_OK)
		    return retVal;
            if((retVal = rtl8370_setAsicReg(RTL8370_REG_SCAN1_LED_IO_EN,0xFFBF))!=  RT_ERR_OK)
		        return retVal;       
            break;
        case LEDOP_PARALLEL:
            if((retVal = rtl8370_setAsicReg(RTL8370_REG_LED_SYS_CONFIG,0x1472))!=  RT_ERR_OK)
		        return retVal;
            break;
        case LEDOP_SERIAL:
            if((retVal = rtl8370_setAsicReg(RTL8370_REG_LED_SYS_CONFIG,0x14F7))!=  RT_ERR_OK)
		        return retVal;        
            break;
        default:
            break;
    }
    
    return RT_ERR_OK;
}


/*
@func ret_t | rtl8370_getAsicLedOperationMode | Get LED serial mode setup
@parm uint32*| mode | LED mode. 1:scan mode 1, 2:parallel mode, 3:mdx mode (serial mode)  
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can get LED serial mode setup and get signal active high/low.
 */
ret_t rtl8370_getAsicLedOperationMode(uint32 *mode)
{
    ret_t retVal;
    uint32 regData;
    
    if((retVal = rtl8370_getAsicReg(RTL8370_REG_LED_SYS_CONFIG,&regData))!=  RT_ERR_OK)
		return retVal;

    if (regData==0x1471)
        *mode = LEDOP_SCAN1;
    else if (regData==0x1472)
        *mode = LEDOP_PARALLEL;
    else if (regData==0x14F7)
        *mode = LEDOP_SERIAL;
    else
        *mode = LEDOP_SCAN0;
    
    return RT_ERR_OK;        
}





/*
@func ret_t | rtl8370_setAsicLedIndicateInfoConfig | Set Leds indicated information mode
@parm uint32 | ledno | LED group number. There are 1 to 1 led mapping to each port in each led group. 
@parm enum RTL8370_LEDCONF | config | Support 16 types configuration.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can set LED indicated information configuration for each LED group with 1 to 1 led mapping to each port.
    Definition        LED Statuses            Description
    0000        LED_Off                LED pin Tri-State.
    0001        Dup/Col                Collision, Full duplex Indicator. Blinking every 43ms when collision happens. Low for full duplex, and high for half duplex mode.
    0010        Link/Act               Link, Activity Indicator. Low for link established. Link/Act Blinks every 43ms when the corresponding port is transmitting or receiving.
    0011        Spd1000                1000Mb/s Speed Indicator. Low for 1000Mb/s.
    0100        Spd100                 100Mb/s Speed Indicator. Low for 100Mb/s.
    0101        Spd10                  10Mb/s Speed Indicator. Low for 10Mb/s.
    0110        Spd1000/Act            1000Mb/s Speed/Activity Indicator. Low for 1000Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
    0111        Spd100/Act             100Mb/s Speed/Activity Indicator. Low for 100Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
    1000        Spd10/Act              10Mb/s Speed/Activity Indicator. Low for 10Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
    1001        Spd100 (10)/Act        10/100Mb/s Speed/Activity Indicator. Low for 10/100Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
    1010        Fiber                  Fiber link Indicator. Low for Fiber.
    1011        Fault                  Auto-negotiation     Fault Indicator. Low for Fault.
    1100        Link/Rx                Link, Activity Indicator. Low for link established. Link/Rx Blinks every 43ms when the corresponding port is transmitting.
    1101        Link/Tx                Link, Activity Indicator. Low for link established. Link/Tx Blinks every 43ms when the corresponding port is receiving.
    1110        Master                 Link on Master Indicator. Low for link Master established.
    1111        Act                    Activity Indicator. Low for link established. 
 */
ret_t rtl8370_setAsicLedIndicateInfoConfig(uint32 ledno, enum RTL8370_LEDCONF config)
{
    ret_t retVal;
    uint32 regData;
    CONST_T uint16 bits[RTL8370_LEDGROUPMAX+1]= { RTL8370_LED0_CFG_MASK, RTL8370_LED1_CFG_MASK, RTL8370_LED2_CFG_MASK};
    CONST_T uint16 offsets[RTL8370_LEDGROUPMAX+1]= { RTL8370_LED0_CFG_OFFSET, RTL8370_LED1_CFG_OFFSET, RTL8370_LED2_CFG_OFFSET};

    if(ledno > RTL8370_LEDGROUPMAX)
        return RT_ERR_INPUT;

    if(config > LEDCONF_ACT)    
        return RT_ERR_INPUT;

    retVal = rtl8370_getAsicReg(RTL8370_REG_LED_CONFIGURATION,&regData);  
	if( retVal !=  RT_ERR_OK)
		return retVal;

    regData = regData & (~RTL8370_LED_CONFIG_SEL_MASK);
    regData = regData & (~bits[ledno]);
    regData = regData | ((config & RTL8370_LED0_CFG_MASK)<<offsets[ledno]);

    retVal = rtl8370_setAsicReg(RTL8370_REG_LED_CONFIGURATION,regData);     

    return retVal;
}

/*
@func ret_t | rtl8370_getAsicLedIndicateInfoConfig | Get Leds indicated information mode
@parm uint32 | ledno | LED group number. There are 1 to 1 led mapping to each port in each led group. 
@parm enum RTL8370_LEDCONF* | config | Support 16 types configuration.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get LED indicated information configuration for each LED group.
 */
ret_t rtl8370_getAsicLedIndicateInfoConfig(uint32 ledno, enum RTL8370_LEDCONF* config)
{
    ret_t retVal;
    uint32 regData;
    CONST_T uint16 bits[RTL8370_LEDGROUPMAX+1]= { RTL8370_LED0_CFG_MASK, RTL8370_LED1_CFG_MASK, RTL8370_LED2_CFG_MASK};
    CONST_T uint16 offsets[RTL8370_LEDGROUPMAX+1]= { RTL8370_LED0_CFG_OFFSET, RTL8370_LED1_CFG_OFFSET, RTL8370_LED2_CFG_OFFSET};

    if(ledno >RTL8370_LEDGROUPMAX)
        return RT_ERR_INPUT;

    /* Get register value */
    retVal = rtl8370_getAsicReg(RTL8370_REG_LED_CONFIGURATION,&regData); 

    if (regData&RTL8370_LED_CONFIG_SEL_MASK)
        return RT_ERR_FAILED;

    *(uint32*)config = (regData&bits[ledno])>>offsets[ledno];

    return retVal;
}

/*
@func ret_t | rtl8370_setAsicLedGroupMode | Turn on/off Led of dedicated port
@parm uint32 | mode | LED mode, 0b00:mode 0, 0b01:mode 1, 0b10:mode 2, 0b11:mode 3.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can turn on/off leds of dedicated port while indicated information configuration of LED group is set to force mode.
	    set  led0    led1   led2
mode 0	00   0010  0011  0100
mode 1	01   0110  0111  1000
mode 2	10   0001  0110  1001
mode 3	11   1000  0110  0111

    
 */
ret_t rtl8370_setAsicLedGroupMode(uint32 mode)
{
    ret_t retVal;
    uint32 regData;
	
    /* Invalid input parameter */
    if(mode >= LEDFORCEMODE_MAX)
        return RT_ERR_INPUT; 
	
    regData = (mode<<RTL8370_DATA_LED_OFFSET)|(1<<RTL8370_LED_CONFIG_SEL_OFFSET);

    retVal = rtl8370_setAsicReg(RTL8370_REG_LED_CONFIGURATION, regData);    

    return retVal;
}

/*
@func ret_t | rtl8370_getAsicForceLed | Turn on/off Led of dedicated port
@parm uint32* | mode | LED mode, 0b00:normal mode, 0b01:force blink, 0b10:force off, 0b11:force on.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can turn on/off leds of dedicated port while indicated information configuration of LED group is set to force mode.
 */
ret_t rtl8370_getAsicLedGroupMode(uint32* mode)
{
    ret_t retVal;
	uint32 regData;
 
    retVal = rtl8370_getAsicReg(RTL8370_REG_LED_CONFIGURATION,&regData); 	
    if (retVal !=  RT_ERR_OK) 
        return retVal;

	if ((regData&RTL8370_LED_CONFIG_SEL_MASK)!=RTL8370_LED_CONFIG_SEL_MASK)
		return RT_ERR_FAILED;

    *mode = (regData&RTL8370_DATA_LED_MASK)>>RTL8370_DATA_LED_OFFSET;

    return retVal;    
}




/*
@func ret_t | rtl8370_setAsicForceLeds | Turn on/off Led of dedicated port
@parm uint32 | port | The port number. 
@parm uint32 | group | LED group number. 
@parm uint32 | mode | LED mode, 0b00:normal mode, 0b01:force blink, 0b10:force off, 0b11:force on.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can turn on/off leds of dedicated port while indicated information configuration of LED group is set to force mode.
 */
ret_t rtl8370_setAsicForceLed(uint32 port, uint32 group, uint32 mode)
{
    ret_t retVal;
    uint16 regAddr;
    CONST_T uint16 bits[8]= {0x0003,0x000C,0x0030,0x00C0,0x0300,0x0C00,0x3000,0xC000};
   
    /* Invalid input parameter */
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID; 

    if(group > RTL8370_LEDGROUPMAX)
        return RT_ERR_INPUT; 

    if(mode >= LEDFORCEMODE_MAX)
        return RT_ERR_INPUT; 
	
    /* Set Related Registers */
    regAddr = RTL8370_LED_FORCE_MODE_BASE + (group<<1);

    retVal = rtl8370_setAsicRegBits(regAddr,bits[port&0x7],mode); 	  

    return retVal ;        
}

/*
@func ret_t | rtl8370_getAsicForceLed | Turn on/off Led of dedicated port
@parm uint32 | port | The port number. 
@parm uint32 | group | LED group number. 
@parm uint32* | mode | LED mode, 0b00:normal mode, 0b01:force blink, 0b10:force off, 0b11:force on.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can turn on/off leds of dedicated port while indicated information configuration of LED group is set to force mode.
 */
ret_t rtl8370_getAsicForceLed(uint32 port, uint32 group, uint32* mode)
{
    uint16 regAddr;
    CONST_T uint16 bits[8]= {0x0003,0x000C,0x0030,0x00C0,0x0300,0x0C00,0x3000,0xC000};
    
    /* Invalid input parameter */
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID; 

    if(group > RTL8370_LEDGROUPMAX)
        return RT_ERR_INPUT; 	

    /* Get Related Registers */
    regAddr = RTL8370_LED_FORCE_MODE_BASE + (group<<1);

    return rtl8370_getAsicRegBits(regAddr,bits[port&0x7],mode);
}

/*
@func ret_t | rtl8370_setAsicForceGroupLed | Turn on/off Led of all ports
@parm uint32 | groupmask | LED group mask. 
@parm uint32 | mode | LED mode, 0b00:normal mode, 0b01:force blink, 0b10:force off, 0b11:force on.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can turn on/off leds of dedicated port while indicated information configuration of LED group is set to force mode.
 */
ret_t rtl8370_setAsicForceGroupLed(uint32 groupmask, uint32 mode)
{
    ret_t retVal;
	uint32 i,bitmask;
    CONST_T uint16 bits[3]= {0x0004,0x0010,0x0040};
   
    /* Invalid input parameter */
    if(groupmask > RTL8370_LEDGROUPMASK)
        return RT_ERR_INPUT; 

    if(mode >= LEDFORCEMODE_MAX)
        return RT_ERR_INPUT;

    bitmask = 0;
	for(i=0;i<=RTL8370_LEDGROUPMAX;i++)
	{
	    if(groupmask&(1<<i))
	    {
            bitmask = bitmask | bits[i];
	    }
	}
		
    bitmask = bitmask | mode;
   
    retVal = rtl8370_setAsicReg(RTL8370_LED_FORCE_CTRL, bitmask);  
	if( retVal !=  RT_ERR_OK)
    return retVal;

    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_getAsicForceGroupLed | Turn on/off Led of all ports
@parm uint32 | groupmask | LED group mask. 
@parm uint32* | mode | LED mode, 0b00:normal mode, 0b01:force blink, 0b10:force off, 0b11:force on.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can turn on/off leds of dedicated port while indicated information configuration of LED group is set to force mode.
 */
ret_t rtl8370_getAsicForceGroupLed(uint32* groupmask, uint32* mode)
{
    ret_t retVal;
	uint32 i,regData;
	CONST_T uint16 bits[3]= {0x0004,0x0010,0x0040};
	
    /* Get Related Registers */
    retVal = rtl8370_getAsicReg(RTL8370_LED_FORCE_CTRL, &regData);
	if( retVal !=  RT_ERR_OK)
		return retVal;

    *groupmask = 0;
    for(i=0;i<=RTL8370_LEDGROUPMAX;i++)
    {
        if((regData&bits[i])==bits[i])
        {
            *groupmask = *groupmask | (1<<i);
        }
    }	
   
    *mode = regData&RTL8370_FORCE_MODE_MASK;
   
    return RT_ERR_OK;        
}


/*
@func ret_t | rtl8370_setAsicLedBlinkRate | Set led blinking rate ate mode 0 to mode 3
@parm enum RTL8370_LEDBLINKRATE | blinkRate | Support 6 blink rates.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can set LED blink rate at 43ms, 84ms, 120ms, 170ms, 340ms and 670ms.
 */
ret_t rtl8370_setAsicLedBlinkRate(enum RTL8370_LEDBLINKRATE blinkRate)
{
    if(blinkRate >=LEDBLINKRATE_MAX)
        return RT_ERR_INPUT;

    return rtl8370_setAsicRegBits(RTL8370_REG_LED_MODE, RTL8370_SEL_LEDRATE_MASK,blinkRate);
}

/*
@func ret_t | rtl8370_getAsicLedBlinkRate | Get led blinking rate ate mode 0 to mode 3
@parm enum RTL8370_LEDBLINKRATE* | blinkRate | Support 6 blink rates.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can set LED blink rate at 43ms, 84ms, 120ms, 170ms, 340ms and 670ms.
 */
ret_t rtl8370_getAsicLedBlinkRate(enum RTL8370_LEDBLINKRATE* blinkRate)
{
    return rtl8370_getAsicRegBits(RTL8370_REG_LED_MODE, RTL8370_SEL_LEDRATE_MASK,(uint32*)blinkRate);
}


/*
@func ret_t | rtl8370_setAsicLedForceBlinkRate | Set led blinking rate for force mode led.
@parm enum RTL8370_LEDBLINKRATE | blinkRate | Support 4 blink rates.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can set LED blink rate for force mode LED at 512ms, 1024ms, 2048ms, or identical to normal mode LED.
 */
ret_t rtl8370_setAsicLedForceBlinkRate(enum RTL8370_LEDFORCERATE blinkRate)
{
    if(blinkRate >=LEDFORCERATE_MAX)
        return RT_ERR_INPUT;

    return rtl8370_setAsicRegBits(RTL8370_REG_LED_MODE, RTL8370_FORCE_RATE_MASK,blinkRate);
}

/*
@func ret_t | rtl8370_getAsicLedForceBlinkRate | Get led blinking rate ate mode 0 to mode 3
@parm enum RTL8370_LEDBLINKRATE* | blinkRate | Support 4 blink rates.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get LED blink rate for force mode LED at 512ms, 1024ms, 2048ms, or identical to normal mode LED.
 */
ret_t rtl8370_getAsicLedForceBlinkRate(enum RTL8370_LEDFORCERATE* blinkRate)
{
    return rtl8370_getAsicRegBits(RTL8370_REG_LED_MODE, RTL8370_FORCE_RATE_MASK,(uint32*)blinkRate);
}


/*
@func ret_t | rtl8370_setAsicLedSerialModeConfig | Set LED serial mode
@parm uint32 | active | Active High or Low. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can turn on/off led serial mode and set signal to active high/low.
 */
ret_t rtl8370_setAsicLedSerialModeConfig(uint32 active, uint32 serimode)
{
    ret_t retVal;
    
    /* Invalid input parameter */
    if( active >= LEDSERACT_MAX)
        return RT_ERR_INPUT;
    if( serimode >= LEDSER_MAX)
        return RT_ERR_INPUT;

    /* Set Active High or Low */
    if((retVal = rtl8370_setAsicRegBit(RTL8370_REG_LED_SYS_CONFIG,RTL8370_SERI_LED_ACT_LOW_OFFSET,active)) !=  RT_ERR_OK)
        return retVal;

    /*set to 8G mode (not 16G mode)*/
    if((retVal = rtl8370_setAsicRegBit(RTL8370_REG_LED_MODE,RTL8370_RTCT_TEST_TIME_OFFSET, serimode))!=  RT_ERR_OK)
        return retVal;

    
    return RT_ERR_OK;   
}


/*
@func ret_t | rtl8370_getAsicLedSerialModeConfig | Get LED serial mode setup
@parm uint32*| active | Active High or Low. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can get LED serial mode setup and get signal active high/low.
 */
ret_t rtl8370_getAsicLedSerialModeConfig(uint32 *active, uint32 *serimode)
{
    ret_t retVal;
    
    if((retVal = rtl8370_getAsicRegBit(RTL8370_REG_LED_SYS_CONFIG,RTL8370_SERI_LED_ACT_LOW_OFFSET,active))!=  RT_ERR_OK)
        return retVal;

    /*get to 8G mode (not 16G mode)*/
    if((retVal = rtl8370_getAsicRegBit(RTL8370_REG_LED_MODE,RTL8370_RTCT_TEST_TIME_OFFSET, serimode))!=  RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;        
}

/*
@func ret_t | rtl8370_setAsicLedSystemEnable | Turn on/off Led of all system ports
@parm uint32 | enable | LED system configuration. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can turn on/off leds of dedicated port while indicated information configuration of LED group is set to force mode.
 */
ret_t rtl8370_setAsicLedSystemEnable(uint32 enable)
{
    ret_t retVal;

    if (enable > 1)
        return RT_ERR_FAILED;

    if (enable == 1)
    {
        if ((retVal = rtl8370_setAsicRegBit(RTL8370_REG_LED_SYS_CONFIG, RTL8370_LED_IO_DISABLE_OFFSET, 0))!=RT_ERR_OK)
            return retVal; 
    }
    else
    {
        if ((retVal = rtl8370_setAsicRegBit(RTL8370_REG_LED_SYS_CONFIG, RTL8370_LED_IO_DISABLE_OFFSET, 1))!=RT_ERR_OK)
            return retVal; 
    
    }
    
    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_getAsicLedSystemEnable | Turn on/off Led of all system ports
@parm uint32 | *enable | LED system configuration. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can turn on/off leds of dedicated port while indicated information configuration of LED group is set to force mode.
 */
ret_t rtl8370_getAsicLedSystemEnable(uint32 *enable)
{
    ret_t retVal;
    uint32 regData;
    
	if ((retVal = rtl8370_getAsicRegBit(RTL8370_REG_LED_SYS_CONFIG, RTL8370_LED_IO_DISABLE_OFFSET, &regData))!=RT_ERR_OK)
        return retVal;

    if (regData == 1)
    {
        *enable = 0;
    }
    else
    {
        *enable = 1;
    }
        
    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_setAsicLedGroupEnable | Turn on/off Led of all system ports
@parm uint32 | group | LED group id.
@parm uint32 | portmask | LED port mask. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can turn on/off leds of dedicated port while indicated information configuration of LED group is set to force mode.
 */
ret_t rtl8370_setAsicLedGroupEnable(uint32 group, uint32 portmask)
{
    ret_t retVal;
    uint32 regAddr;
    uint32 regDataMask;

    if ( group > RTL8370_LEDGROUPMAX )
        return RT_ERR_INPUT;

    if ( portmask > 0xFF )
        return RT_ERR_INPUT;

    regAddr = RTL8370_REG_PARA_LED_IO_EN1 + group/2;

    regDataMask = 0xFF << ((group%2)*8);

    if ((retVal = rtl8370_setAsicRegBits(regAddr, regDataMask, portmask))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_getAsicLedGroupEnable | Turn on/off Led of all system ports
@parm uint32 | group | LED group id.
@parm uint32 | *portmask | LED port mask.  
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can turn on/off leds of dedicated port while indicated information configuration of LED group is set to force mode.
 */
ret_t rtl8370_getAsicLedGroupEnable(uint32 group, uint32 *portmask)
{
    ret_t retVal;
    uint32 regAddr;
    uint32 regDataMask;
    
    if ( group > RTL8370_LEDGROUPMAX )
        return RT_ERR_INPUT;

    regAddr = RTL8370_REG_PARA_LED_IO_EN1 + group/2;

    regDataMask = 0xFF << ((group%2)*8);

    if ((retVal = rtl8370_getAsicRegBits(regAddr, regDataMask, portmask))!=RT_ERR_OK)
        return retVal;
        
    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_setAsicLedSerialEnable | Turn on/off Led serial mode
@parm uint32 | enable | LED serial configuration. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can turn on/off led of serial mode.
 */
ret_t rtl8370_setAsicLedSerialEnable(uint32 enable)
{
    ret_t retVal;

    if (enable > 1)
        return RT_ERR_FAILED;

    if (enable == 1)
    {
        /*Enable serial CLK mode*/
        if((retVal = rtl8370_setAsicRegBit(RTL8370_REG_SCAN0_LED_IO_EN,RTL8370_LED_SERI_CLK_EN_OFFSET, 1))!=  RT_ERR_OK)
		    return retVal;
        /*Enable serial DATA mode*/
        if((retVal = rtl8370_setAsicRegBit(RTL8370_REG_SCAN0_LED_IO_EN,RTL8370_LED_SERI_DATA_EN_OFFSET, 1))!=  RT_ERR_OK)
		    return retVal;
        
    }
    else
    {
        /*Disable serial CLK mode*/
        if((retVal = rtl8370_setAsicRegBit(RTL8370_REG_SCAN0_LED_IO_EN,RTL8370_LED_SERI_CLK_EN_OFFSET, 0))!=  RT_ERR_OK)
		    return retVal;
        /*Disable serial DATA mode*/
        if((retVal = rtl8370_setAsicRegBit(RTL8370_REG_SCAN0_LED_IO_EN,RTL8370_LED_SERI_DATA_EN_OFFSET, 0))!=  RT_ERR_OK)
		    return retVal;
    
    }
    
    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_getAsicLedSerialEnable | Turn on/off Led serial mode
@parm uint32 | *enable | LED serial configuration. 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_INPUT | Invalid input value.
@comm
    The API can turn on/off led of serial mode.
 */
ret_t rtl8370_getAsicLedSerialEnable(uint32 *enable)
{
    ret_t retVal;
    uint32 regData;
    
	if ((retVal = rtl8370_getAsicReg(RTL8370_REG_SCAN0_LED_IO_EN, &regData))!=RT_ERR_OK)
        return retVal;

    if ((regData&RTL8370_LED_SERI_CLK_EN_MASK) && (regData&RTL8370_LED_SERI_DATA_EN_MASK))
    {
        *enable = 1;
    }
    else
    {
        *enable = 0;
    }
        
    return RT_ERR_OK;
}


