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
 * $Date: 2010/12/02 04:34:22 $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367B
 * Feature : 
 *
 */
#include "rtl8370_asicdrv_mib.h"

/*
@func ret_t | rtl8370_setAsicMIBsCounterReset | Set MIBs global/queue manage reset or per-port reset.
@parm uint32 | greset | Global reset 
@parm uint32 | qmreset | Queue maganement reset
@parm uint32 | pmask | Port reset mask  
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
     ASIC will clear all MIBs counter by global resetting and clear counters associated with a particular port by mapped port resetting. 
*/
ret_t rtl8370_setAsicMIBsCounterReset(uint32 greset, uint32 qmreset, uint32 pmask)
{
    ret_t retVal;
    uint32 regData;
    uint32 regBits;

    regBits = RTL8370_GLOBAL_RESET_MASK | 
                RTL8370_QM_RESET_MASK |
                    RTL8370_MIB_PORT07_MASK;
    regData = ((greset << RTL8370_GLOBAL_RESET_OFFSET) & RTL8370_GLOBAL_RESET_MASK) |
                ((qmreset << RTL8370_QM_RESET_OFFSET) & RTL8370_QM_RESET_MASK) | 
                (((pmask & 0xFF) << RTL8370_PORT0_RESET_OFFSET) & RTL8370_MIB_PORT07_MASK) ;
                
    retVal = 0;
	
    if (regData != 0)
    {
	    retVal = rtl8370_setAsicRegBits(RTL8370_REG_MIB_CTRL0,regBits,(regData>>RTL8370_PORT0_RESET_OFFSET));
	    if (retVal !=  RT_ERR_OK) 
	        return retVal;
    }

    regData = (((pmask & 0xFF00)>>8) << RTL8370_PORT8_RESET_OFFSET) & RTL8370_MIB_PORT815_MASK;
    if (regData != 0)
    {
    	retVal = rtl8370_setAsicRegBits(RTL8370_REG_MIB_CTRL1, RTL8370_MIB_PORT815_MASK,(regData>>RTL8370_PORT8_RESET_OFFSET));
    }
    return retVal;
}


#ifdef EMBEDDED_SUPPORT

ret_t rtl8370_getAsicMIBsCounter(uint8 port,enum RTL8370_MIBCOUNTER mibIdx,uint32* counterH, uint32* counterL)
{
    ret_t retVal;
    uint32 regAddr;
    uint32 regData;
    uint16 mibAddr;
    uint32 mibOff = 0;

    /* address offset to MIBs counter */
    CONST_T uint8 mibLength[RTL8370_MIBS_NUMBER]= {
        4,2,2,2,2,2,2,2,2,
        4,2,2,2,2,2,2,2,2,2,2,
        4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2};

    uint16 i;
    uint32 mibCounter;

    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(mibIdx >= RTL8370_MIBS_NUMBER)
        return RT_ERR_STAT_INVALID_PORT_CNTR;

    if(Dot1dTpLearnEntryDiscard == mibIdx)
    {
        mibAddr = RTL8370_MIB_LEARNENTRYDISCARD_OFFSET;
    }
    else
    {
        i = 0;
        mibOff = RTL8370_MIB_PORT_OFFSET*(port);

        while(i < mibIdx)
        {
            mibOff += mibLength[i];
            i++;
        }        
        
        mibAddr = mibOff;
    }
    

    /*writing access counter address first*/
    /*This address is SRAM address, and SRAM address = MIB register address >> 2*/
    /*then ASIC will prepare 64bits counter wait for being retrived*/
    /*Write Mib related address to access control register*/
    retVal = rtl8370_setAsicReg(RTL8370_REG_MIB_ADDRESS,(mibAddr>>2));
    if(retVal !=  RT_ERR_OK)
        return retVal;

    /*read MIB control register*/
    retVal = rtl8370_getAsicReg(RTL8370_MIB_CTRL_REG,&regData);

    if(regData & RTL8370_BUSY_FLAG_MASK)
        return RT_ERR_BUSYWAIT_TIMEOUT;

    if(regData & RTL8370_RESET_FLAG_MASK)
        return RT_ERR_STAT_PORT_CNTR_FAIL;

	mibCounter = 0;
    i = mibLength[mibIdx];
    if(4 == i)
	{
        regAddr = RTL8370_MIB_COUNTER_BASE_REG + 3;
	}
    else
	{
        regAddr = RTL8370_MIB_COUNTER_BASE_REG + ((mibOff+1)%4);
    }

    while(i)
    {
        retVal = rtl8370_getAsicReg(regAddr,&regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        mibCounter = (mibCounter << 16) | (regData & 0xFFFF);

        regAddr --;
        i --;

        if(2 == i)
        {
            *counterH = mibCounter;
            mibCounter = 0;
        }

        if(0 == i)
        {
            *counterL = mibCounter;
        }
    }
    
    return RT_ERR_OK;
}

#else
/*
@func ret_t | rtl8370_getAsicMIBsCounter | Get MIBs counter.
@parm uint32 | port | Physical port number (0~5).
@num RTL8370_MIBCOUNTER | mibIdx | MIB counter index.
@parm uint64* | counter | MIB retrived counter.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_STAT_INVALID_PORT_CNTR | Invalid MIBs index.
@rvalue RT_ERR_BUSYWAIT_TIMEOUT | MIB is busy at retrieving
@rvalue RT_ERR_STAT_PORT_CNTR_FAIL | MIB is resetting.
@comm
     Before MIBs counter retrieving, writting accessing address to ASIC at first and check the MIB control register status. If busy bit of MIB control is set, that
     mean MIB counter have been waiting for preparing, then software must wait atfer this busy flag reset by ASIC. This driver did not recycle reading user desired
     counter. Software must use driver again to get MIB counter if return value is not RT_ERR_OK.

*/
ret_t rtl8370_getAsicMIBsCounter(uint32 port,enum RTL8370_MIBCOUNTER mibIdx,uint64* counter)
{
    ret_t retVal;
    uint32 regAddr;
    uint32 regData;
    uint32 mibAddr;
    uint32 mibOff = 0;

    /* address offset to MIBs counter */
    CONST_T uint16 mibLength[RTL8370_MIBS_NUMBER]= {
        4,2,2,2,2,2,2,2,2,
        4,2,2,2,2,2,2,2,2,2,2,
        4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2};

    uint16 i;
    uint64 mibCounter;


    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(mibIdx >= RTL8370_MIBS_NUMBER)
        return RT_ERR_STAT_INVALID_PORT_CNTR;

    if(Dot1dTpLearnEntryDiscard == mibIdx)
    {
        mibAddr = RTL8370_MIB_LEARNENTRYDISCARD_OFFSET;
    }
    else
    {
        i = 0;
        mibOff = RTL8370_MIB_PORT_OFFSET*(port);

        while(i < mibIdx)
        {
            mibOff += mibLength[i];
            i++;
        }        
        
        mibAddr = mibOff;
    }
    

    /*writing access counter address first*/
    /*This address is SRAM address, and SRAM address = MIB register address >> 2*/
    /*then ASIC will prepare 64bits counter wait for being retrived*/
    /*Write Mib related address to access control register*/
    retVal = rtl8370_setAsicReg(RTL8370_REG_MIB_ADDRESS,(mibAddr>>2));
    if(retVal !=  RT_ERR_OK)
        return retVal;

    /* polling busy flag */
    i = 100;
    while(i > 0)
    {
        /*read MIB control register*/
        retVal = rtl8370_getAsicReg(RTL8370_MIB_CTRL_REG,&regData);
    
        if((regData & RTL8370_BUSY_FLAG_MASK) == 0)
        {
            break;
        }
    
        i--;
    }

    if(regData & RTL8370_BUSY_FLAG_MASK)
        return RT_ERR_BUSYWAIT_TIMEOUT;

    if(regData & RTL8370_RESET_FLAG_MASK)
        return RT_ERR_STAT_PORT_CNTR_FAIL;

    mibCounter = 0;
    i = mibLength[mibIdx];
    if(4 == i)
        regAddr = RTL8370_MIB_COUNTER_BASE_REG + 3;
    else
        regAddr = RTL8370_MIB_COUNTER_BASE_REG + ((mibOff+1)%4);
    
    while(i)
    {
        retVal = rtl8370_getAsicReg(regAddr,&regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        mibCounter = (mibCounter << 16) | (regData & 0xFFFF);

        regAddr --;
        i --;
        
    }
    
    *counter = mibCounter;    
    
    return RT_ERR_OK;
}
#endif

/*
@func ret_t | rtl8370_getAsicMIBsControl | Get MIB control register.
@parm uint32* | mask | MIB control status mask bit[0]-busy bit[1].
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
     Software need to check this control register atfer doing port resetting or global resetting.
*/
ret_t rtl8370_getAsicMIBsControl(uint32* mask)
{
    ret_t retVal;
    uint32 regData;

    retVal = rtl8370_getAsicReg(RTL8370_MIB_CTRL_REG,&regData);
    if(retVal !=  RT_ERR_OK)
        return retVal;

    *mask = regData & (RTL8370_BUSY_FLAG_MASK | RTL8370_RESET_FLAG_MASK);
    
    return RT_ERR_OK;
}

