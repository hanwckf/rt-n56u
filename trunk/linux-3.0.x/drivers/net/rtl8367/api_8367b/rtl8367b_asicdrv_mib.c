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
 * Feature : MIB related functions
 *
 */
#include "rtl8367b_asicdrv_mib.h"
/* Function Name:
 *      rtl8367b_setAsicMIBsCounterReset
 * Description:
 *      Reset global/queue manage or per-port MIB counter
 * Input:
 *      greset 	- Global reset 
 *      qmreset - Queue maganement reset
 *      portmask 	- Port reset mask 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicMIBsCounterReset(rtk_uint32 greset, rtk_uint32 qmreset, rtk_uint32 portmask)
{
	ret_t retVal;
	rtk_uint32 regData;
	rtk_uint32 regBits;

	regBits = RTL8367B_GLOBAL_RESET_MASK | 
				RTL8367B_QM_RESET_MASK |
					RTL8367B_MIB_PORT07_MASK;
	regData = ((greset << RTL8367B_GLOBAL_RESET_OFFSET) & RTL8367B_GLOBAL_RESET_MASK) |
				((qmreset << RTL8367B_QM_RESET_OFFSET) & RTL8367B_QM_RESET_MASK) | 
				(((portmask & 0xFF) << RTL8367B_PORT0_RESET_OFFSET) & RTL8367B_MIB_PORT07_MASK) ;
				
	
	retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_MIB_CTRL0, regBits, (regData >> RTL8367B_PORT0_RESET_OFFSET));

	return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicMIBsCounter
 * Description:
 *      Get MIBs counter
 * Input:
 *      port 		- Physical port number (0~7)
 *      mibIdx 		- MIB counter index
 *      pCounter 	- MIB retrived counter
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_PORT_ID  		- Invalid port number
 *      RT_ERR_BUSYWAIT_TIMEOUT - MIB is busy at retrieving
 *      RT_ERR_STAT_CNTR_FAIL  	- MIB is resetting
 * Note:
 * 		Before MIBs counter retrieving, writting accessing address to ASIC at first and check the MIB 
 * 		control register status. If busy bit of MIB control is set, that means MIB counter have been 
 * 		waiting for preparing, then software must wait atfer this busy flag reset by ASIC. This driver
 * 		did not recycle reading user desired counter. Software must use driver again to get MIB counter 
 * 		if return value is not RT_ERR_OK.
 */
ret_t rtl8367b_getAsicMIBsCounter(rtk_uint32 port, RTL8367B_MIBCOUNTER mibIdx, rtk_uint64* pCounter)
{
	ret_t retVal;
	rtk_uint32 regAddr;
	rtk_uint32 regData;
	rtk_uint32 mibAddr;
	rtk_uint32 mibOff=0;

	/* address offset to MIBs counter */
	CONST rtk_uint16 mibLength[RTL8367B_MIBS_NUMBER]= {
        4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
        4,2,2,2,2,2,2,2,2,
        4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
        2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2};

	rtk_uint16 i;
	rtk_uint64 mibCounter;


	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

	if(mibIdx >= RTL8367B_MIBS_NUMBER)
		return RT_ERR_STAT_INVALID_CNTR;

	if(dot1dTpLearnedEntryDiscards == mibIdx)
	{
		mibAddr = RTL8367B_MIB_LEARNENTRYDISCARD_OFFSET;
	}
	else
	{
		i = 0;
		mibOff = RTL8367B_MIB_PORT_OFFSET * port;

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
	retVal = rtl8367b_setAsicReg(RTL8367B_REG_MIB_ADDRESS, (mibAddr >> 2));
	if(retVal != RT_ERR_OK)
		return retVal;



    /* polling busy flag */
    i = 100;
    while(i > 0)
    {
        /*read MIB control register*/
        retVal = rtl8367b_getAsicReg(RTL8367B_MIB_CTRL_REG,&regData);
    
        if((regData & RTL8367B_MIB_CTRL0_BUSY_FLAG_MASK) == 0)
        {
            break;
        }
    
        i--;
    }

	if(regData & RTL8367B_MIB_CTRL0_BUSY_FLAG_MASK)
		return RT_ERR_BUSYWAIT_TIMEOUT;

	if(regData & RTL8367B_RESET_FLAG_MASK)
		return RT_ERR_STAT_CNTR_FAIL;

	mibCounter = 0;
	i = mibLength[mibIdx];
	if(4 == i)
		regAddr = RTL8367B_MIB_COUNTER_BASE_REG + 3;
	else
		regAddr = RTL8367B_MIB_COUNTER_BASE_REG + ((mibOff + 1) % 4);
	
	while(i)
	{
		retVal = rtl8367b_getAsicReg(regAddr, &regData);
		if(retVal != RT_ERR_OK)
			return retVal;

		mibCounter = (mibCounter << 16) | (regData & 0xFFFF);

		regAddr --;
		i --;
		
	}
	
	*pCounter = mibCounter;	
	
	return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicMIBsLogCounter
 * Description:
 *      Get MIBs Loggin counter
 * Input:
 *      index 		- The index of 32 logging counter (0 ~ 31)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_ENTRY_INDEX      - Wrong index
 *      RT_ERR_BUSYWAIT_TIMEOUT - MIB is busy at retrieving
 *      RT_ERR_STAT_CNTR_FAIL  	- MIB is resetting
 * Note:
 * 		This API get 32 logging counter
 */
ret_t rtl8367b_getAsicMIBsLogCounter(rtk_uint32 index, rtk_uint32 *pCounter)
{
    ret_t retVal;
	rtk_uint32 regAddr;
	rtk_uint32 regData;
	rtk_uint32 mibAddr;
    rtk_uint16 i;
	rtk_uint64 mibCounter;

    if(index > RTL8367B_MIB_MAX_LOG_CNT_IDX)
        return RT_ERR_ENTRY_INDEX;

    mibAddr = RTL8367B_MIB_LOG_CNT_OFFSET + ((index / 2) * 4);

    retVal = rtl8367b_setAsicReg(RTL8367B_REG_MIB_ADDRESS, (mibAddr >> 2));
	if(retVal != RT_ERR_OK)
		return retVal;

	/*read MIB control register*/
	retVal = rtl8367b_getAsicReg(RTL8367B_MIB_CTRL_REG, &regData);
    if(retVal != RT_ERR_OK)
		return retVal;

	if(regData & RTL8367B_MIB_CTRL0_BUSY_FLAG_MASK)
		return RT_ERR_BUSYWAIT_TIMEOUT;

	if(regData & RTL8367B_RESET_FLAG_MASK)
		return RT_ERR_STAT_CNTR_FAIL;

    mibCounter = 0;
	if((index % 2) == 1)
		regAddr = RTL8367B_MIB_COUNTER_BASE_REG + 3;
	else
		regAddr = RTL8367B_MIB_COUNTER_BASE_REG + 1;
	
	for(i = 0; i <= 1; i++)
    {
        retVal = rtl8367b_getAsicReg(regAddr, &regData);

		if(retVal != RT_ERR_OK)
			return retVal;

		mibCounter = (mibCounter << 16) | (regData & 0xFFFF);

		regAddr --;
	}
	
	*pCounter = mibCounter;	
    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_getAsicMIBsControl
 * Description:
 *      Get MIB control register
 * Input:
 *      pMask 		- MIB control status mask bit[0]-busy bit[1]
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 * 		Software need to check this control register atfer doing port resetting or global resetting
 */
ret_t rtl8367b_getAsicMIBsControl(rtk_uint32* pMask)
{
	ret_t retVal;
	rtk_uint32 regData;

	retVal = rtl8367b_getAsicReg(RTL8367B_MIB_CTRL_REG, &regData);
	if(retVal != RT_ERR_OK)
		return retVal;

	*pMask = regData & (RTL8367B_MIB_CTRL0_BUSY_FLAG_MASK | RTL8367B_RESET_FLAG_MASK);
	
	return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicMIBsResetValue
 * Description:
 *      Reset all counter to 0 or 1
 * Input:
 *      value 			- Reset to value 0 or 1 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicMIBsResetValue(rtk_uint32 value)
{
	return rtl8367b_setAsicRegBit(RTL8367B_REG_MIB_CTRL0, RTL8367B_RESET_VALUE_OFFSET, value);
}
/* Function Name:
 *      rtl8367b_getAsicMIBsResetValue
 * Description:
 *      Reset all counter to 0 or 1
 * Input:
 *      value 			- Reset to value 0 or 1 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicMIBsResetValue(rtk_uint32* value)
{
	return rtl8367b_getAsicRegBit(RTL8367B_REG_MIB_CTRL0, RTL8367B_RESET_VALUE_OFFSET, value);
}

/* Function Name:
 *      rtl8367b_setAsicMIBsUsageMode
 * Description:
 *      MIB update mode
 * Input:
 *      mode 			- 1: latch all MIBs by timer 0:normal free run counting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicMIBsUsageMode(rtk_uint32 mode)
{
	return rtl8367b_setAsicRegBit(RTL8367B_REG_MIB_CTRL4, RTL8367B_MIB_USAGE_MODE_OFFSET, mode);
}
/* Function Name:
 *      rtl8367b_getAsicMIBsUsageMode
 * Description:
 *      MIB update mode
 * Input:
 *      pMode 			- 1: latch all MIBs by timer 0:normal free run counting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicMIBsUsageMode(rtk_uint32* pMode)
{
	return rtl8367b_getAsicRegBit(RTL8367B_REG_MIB_CTRL4, RTL8367B_MIB_USAGE_MODE_OFFSET, pMode);
}

/* Function Name:
 *      rtl8367b_setAsicMIBsTimer
 * Description:
 *      MIB latching timer 
 * Input:
 *      timer 			- latch timer, unit 1 second
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicMIBsTimer(rtk_uint32 timer)
{
	return rtl8367b_setAsicRegBits(RTL8367B_REG_MIB_CTRL4, RTL8367B_MIB_TIMER_MASK, timer);
}
/* Function Name:
 *      rtl8367b_getAsicMIBsTimer
 * Description:
 *      MIB latching timer 
 * Input:
 *      pTimer 			- latch timer, unit 1 second
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicMIBsTimer(rtk_uint32* pTimer)
{
	return rtl8367b_getAsicRegBits(RTL8367B_REG_MIB_CTRL4, RTL8367B_MIB_TIMER_MASK, pTimer);
}
/* Function Name:
 *      rtl8367b_setAsicMIBsLoggingMode
 * Description:
 *      MIB logging counter mode
 * Input:
 *      index 	- logging counter mode index (0~15)
 *      mode 	- 0:32-bits mode 1:64-bits mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE 	- input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicMIBsLoggingMode(rtk_uint32 index, rtk_uint32 mode)
{
	if(index > RTL8367B_MIB_MAX_LOG_MODE_IDX)
		return RT_ERR_OUT_OF_RANGE;

    return rtl8367b_setAsicRegBit(RTL8367B_REG_MIB_CTRL3, index,mode);        
}
/* Function Name:
 *      rtl8367b_getAsicMIBsLoggingMode
 * Description:
 *      MIB logging counter mode
 * Input:
 *      index 	- logging counter mode index (0~15)
 *      pMode 	- 0:32-bits mode 1:64-bits mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE 	- input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicMIBsLoggingMode(rtk_uint32 index, rtk_uint32* pMode)
{
	if(index > RTL8367B_MIB_MAX_LOG_MODE_IDX)
		return RT_ERR_OUT_OF_RANGE;

    return rtl8367b_getAsicRegBit(RTL8367B_REG_MIB_CTRL3, index,pMode);        
}

/* Function Name:
 *      rtl8367b_setAsicMIBsLoggingType
 * Description:
 *      MIB logging counter type
 * Input:
 *      index 	- logging counter mode index (0~15)
 *      type 	- 0:Packet count 1:Byte count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE 	- input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicMIBsLoggingType(rtk_uint32 index, rtk_uint32 type)
{
	if(index > RTL8367B_MIB_MAX_LOG_MODE_IDX)
		return RT_ERR_OUT_OF_RANGE;

    return rtl8367b_setAsicRegBit(RTL8367B_REG_MIB_CTRL5, index,type);        
}
/* Function Name:
 *      rtl8367b_getAsicMIBsLoggingType
 * Description:
 *      MIB logging counter type
 * Input:
 *      index 	- logging counter mode index (0~15)
 *      pType 	- 0:Packet count 1:Byte count
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE 	- input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicMIBsLoggingType(rtk_uint32 index, rtk_uint32* pType)
{
	if(index > RTL8367B_MIB_MAX_LOG_MODE_IDX)
		return RT_ERR_OUT_OF_RANGE;
	
    return rtl8367b_getAsicRegBit(RTL8367B_REG_MIB_CTRL5, index,pType);        
}

/* Function Name:
 *      rtl8367b_setAsicMIBsResetLoggingCounter
 * Description:
 *      MIB logging counter type
 * Input:
 *      index 	- logging counter index (0~31)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_OUT_OF_RANGE 	- input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicMIBsResetLoggingCounter(rtk_uint32 index)
{
    ret_t retVal;
	
	if(index > RTL8367B_MIB_MAX_LOG_CNT_IDX)
		return RT_ERR_OUT_OF_RANGE;

	if(index < 16)
    	retVal = rtl8367b_setAsicReg(RTL8367B_REG_MIB_CTRL1, 1<<index);  
	else
    	retVal = rtl8367b_setAsicReg(RTL8367B_REG_MIB_CTRL2, 1<<(index-16));  	

	return retVal;
}

