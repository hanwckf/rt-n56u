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
 * $Date: 2010/12/02 04:34:25 $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367B
 * Feature : 
 *
 */
#include "rtl8370_asicdrv_phy.h"

#if defined (EMBEDDED_SUPPORT) || defined(MDC_MDIO_OPERATION)
/*
@func int32 | rtl8370_setAsicPHYReg | Set PHY registers .
@parm uint32 | phyNo | PHY number (0~7).
@parm uint32 | phyAddr | PHY address (0~31).
@parm uint32 | data | Writing data.
@rvalue RT_ERR_OK | 
@rvalue RT_ERR_FAILED | invalid parameter
@rvalue RT_ERR_PHY_REG_ID | invalid PHY address
@rvalue RT_ERR_PORT_ID | invalid port id.
@rvalue RT_ERR_BUSYWAIT_TIMEOUT | PHY access busy
@comm
    The API can set internal PHY register 0~31. There are 8 internal PHYs in switch and each PHY can be
    accessed by software.
 */
ret_t rtl8370_setAsicPHYReg( uint32 phyNo, uint32 phyAddr, uint32 value)
{
	uint32 regAddr;

    if(phyNo > RTL8370_PHY_INTERNALNOMAX)
        return RT_ERR_PORT_ID;

    if(phyAddr > RTL8370_PHY_REGNOMAX)
        return RT_ERR_PHY_REG_ID;

    regAddr = 0x2000 + (phyNo << 5) + phyAddr;

    return rtl8370_setAsicReg(regAddr, value);
}

/*
@func int32 | rtl8370_getAsicPHYReg | Set PHY registers .
@parm uint32 | phyNo | PHY number (0~7).
@parm uint32 | phyAddr | PHY address (0~31).
@parm uint32* | data | Read data.
@rvalue RT_ERR_OK | 
@rvalue RT_ERR_FAILED | invalid parameter
@rvalue RT_ERR_PHY_REG_ID | invalid PHY address
@rvalue RT_ERR_PORT_ID | iinvalid port id
@rvalue RT_ERR_BUSYWAIT_TIMEOUT | PHY access busy
@comm
     The API can get internal PHY register 0~31. There are 8 internal PHYs in switch and each PHY can be
    accessed by software.
 */
ret_t rtl8370_getAsicPHYReg( uint32 phyNo, uint32 phyAddr, uint32 *value)
{
	uint32 regAddr;

    if(phyNo > RTL8370_PHY_INTERNALNOMAX)
        return RT_ERR_PORT_ID;

    if(phyAddr > RTL8370_PHY_REGNOMAX)
        return RT_ERR_PHY_REG_ID;

    regAddr = 0x2000 + (phyNo << 5) + phyAddr;

    return rtl8370_getAsicReg(regAddr, value);
}

#else

/*
@func ret_t | rtl8370_setAsicPHYReg | Set PHY registers .
@parm uint32 | phyNo | PHY number (0~7).
@parm uint32 | phyAddr | PHY address (0~31).
@parm uint32 | data | Writing data.
@rvalue RT_ERR_OK | 
@rvalue RT_ERR_FAILED | invalid parameter
@rvalue RT_ERR_PHY_REG_ID | invalid PHY address
@rvalue RT_ERR_PORT_ID | invalid port id.
@rvalue RT_ERR_BUSYWAIT_TIMEOUT | PHY access busy
@comm
    The API can set internal PHY register 0~31. There are 8 internal PHYs in switch and each PHY can be
    accessed by software.
 */
ret_t rtl8370_setAsicPHYReg( uint32 phyNo, uint32 phyAddr, uint32 data )
{
    ret_t retVal;
    uint32 regData;
    uint32 busyFlag;

    if(phyNo > RTL8370_PHY_INTERNALNOMAX)
        return RT_ERR_PORT_ID;


    if(phyAddr > RTL8370_PHY_REGNOMAX)
        return RT_ERR_PHY_REG_ID;

/*
word address    a[15]    a[14]    a[13]    a[12]    a[11]    a[10]    a[9]    a[8]    a[7]    a[6]    a[5]    a[4]    a[3]    a[2]    a[1]    a[0]
phy0 ~ phy7     [        3'd1         ]    [ 0        0        0        0      0 ]    [      PHY No.     ]    [        reg adr[4:0]              ]
*/

    /*Check internal phy access busy or not*/
    retVal = rtl8370_getAsicRegBit(RTL8370_REG_INDRECT_ACCESS_STATUS, RTL8370_PHY_BUSY_OFFSET,&busyFlag);
    if(retVal !=  RT_ERR_OK)
        return retVal;

    if(busyFlag)
        return RT_ERR_BUSYWAIT_TIMEOUT;

    /*prepare access data*/
    retVal = rtl8370_setAsicReg(RTL8370_REG_INDRECT_ACCESS_WRITE_DATA, data);
    if(retVal !=  RT_ERR_OK)
        return retVal;
    
    /*prepare access address*/
    regData = RTL8370_PHY_BASE | (phyNo<<RTL8370_PHY_OFFSET) | phyAddr; 
	
    retVal = rtl8370_setAsicReg(RTL8370_REG_INDRECT_ACCESS_ADDRESS, regData);
    if(retVal !=  RT_ERR_OK)
        return retVal;

    /*Set WRITE Command*/
    return rtl8370_setAsicReg(RTL8370_REG_INDRECT_ACCESS_CRTL, RTL8370_CMD_MASK | RTL8370_RW_MASK);
}

/*
@func ret_t | rtl8370_getAsicPHYReg | Set PHY registers .
@parm uint32 | phyNo | PHY number (0~7).
@parm uint32 | phyAddr | PHY address (0~31).
@parm uint32* | data | Read data.
@rvalue RT_ERR_OK | 
@rvalue RT_ERR_FAILED | invalid parameter
@rvalue RT_ERR_PHY_REG_ID | invalid PHY address
@rvalue RT_ERR_PORT_ID | iinvalid port id
@rvalue RT_ERR_BUSYWAIT_TIMEOUT | PHY access busy
@comm
     The API can get internal PHY register 0~31. There are 8 internal PHYs in switch and each PHY can be
    accessed by software.
 */
ret_t rtl8370_getAsicPHYReg( uint32 phyNo, uint32 phyAddr, uint32 *data )
{
    ret_t retVal;
    uint32 regData;
    uint32 busyFlag,checkCounter;

    if(phyNo > RTL8370_PHY_INTERNALNOMAX)
        return RT_ERR_PORT_ID;

    if(phyAddr > RTL8370_PHY_REGNOMAX)
        return RT_ERR_PHY_REG_ID;

    /*Check internal phy access busy or not*/
    retVal = rtl8370_getAsicRegBit(RTL8370_REG_INDRECT_ACCESS_STATUS, RTL8370_PHY_BUSY_OFFSET,&busyFlag);
    if(retVal !=  RT_ERR_OK)
        return retVal;

    if(busyFlag)
        return RT_ERR_BUSYWAIT_TIMEOUT;

    /*prepare access address*/
    regData = RTL8370_PHY_BASE | (phyNo<<RTL8370_PHY_OFFSET) | phyAddr; 
    
    retVal = rtl8370_setAsicReg(RTL8370_REG_INDRECT_ACCESS_ADDRESS, regData);
    if(retVal !=  RT_ERR_OK)
        return retVal;

    /*Set READ Command*/
    retVal = rtl8370_setAsicReg(RTL8370_REG_INDRECT_ACCESS_CRTL, RTL8370_CMD_MASK );
    if(retVal !=  RT_ERR_OK)
        return retVal;

    checkCounter = 5;
    while(checkCounter)
    {
        retVal = rtl8370_getAsicRegBit(RTL8370_REG_INDRECT_ACCESS_STATUS, RTL8370_PHY_BUSY_OFFSET,&busyFlag);
        if(retVal !=  RT_ERR_OK)
        {
            checkCounter --;
            if(0 == checkCounter)
                return retVal;
        }
        else
        {
            checkCounter = 0;
        }        
    }

    /*get PHY register*/
    retVal = rtl8370_getAsicReg(RTL8370_REG_INDRECT_ACCESS_READ_DATA, &regData);
    if(retVal !=  RT_ERR_OK)
        return retVal;

    *data = regData;

    return RT_ERR_OK;
}


#endif
