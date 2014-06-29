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
 * Feature : PHY related functions
 *
 */
#include "rtl8367b_asicdrv_phy.h"


#if defined(MDC_MDIO_OPERATION)
/* Function Name:
 *      rtl8367b_setAsicPHYReg
 * Description:
 *      Set PHY registers
 * Input:
 *      phyNo 	- Physical port number (0~4)
 *      phyAddr - PHY address (0~31)
 *      phyData - Writing data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_PHY_REG_ID  		- invalid PHY address
 *      RT_ERR_PHY_ID  			- invalid PHY no
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPHYReg( rtk_uint32 phyNo, rtk_uint32 phyAddr, rtk_uint32 value)
{
	rtk_uint32 regAddr;

    if(phyNo > RTL8367B_PHY_INTERNALNOMAX)
        return RT_ERR_PORT_ID;

    if(phyAddr > RTL8367B_PHY_REGNOMAX)
        return RT_ERR_PHY_REG_ID;

    regAddr = 0x2000 + (phyNo << 5) + phyAddr;

    return rtl8367b_setAsicReg(regAddr, value);
}

/* Function Name:
 *      rtl8367b_getAsicPHYReg
 * Description:
 *      Get PHY registers
 * Input:
 *      phyNo 	- Physical port number (0~4)
 *      phyAddr - PHY address (0~31)
 *      pRegData - Writing data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_PHY_REG_ID  		- invalid PHY address
 *      RT_ERR_PHY_ID  			- invalid PHY no
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPHYReg( rtk_uint32 phyNo, rtk_uint32 phyAddr, rtk_uint32 *value)
{
	rtk_uint32 regAddr;

    if(phyNo > RTL8367B_PHY_INTERNALNOMAX)
        return RT_ERR_PORT_ID;

    if(phyAddr > RTL8367B_PHY_REGNOMAX)
        return RT_ERR_PHY_REG_ID;

    regAddr = 0x2000 + (phyNo << 5) + phyAddr;

    return rtl8367b_getAsicReg(regAddr, value);
}

#else

/* Function Name:
 *      rtl8367b_setAsicPHYReg
 * Description:
 *      Set PHY registers
 * Input:
 *      phyNo 	- Physical port number (0~7)
 *      phyAddr - PHY address (0~31)
 *      phyData - Writing data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_PHY_REG_ID  		- invalid PHY address
 *      RT_ERR_PHY_ID  			- invalid PHY no
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPHYReg(rtk_uint32 phyNo, rtk_uint32 phyAddr, rtk_uint32 phyData )
{
	ret_t retVal;
	rtk_uint32 regData;
    rtk_uint32 busyFlag, checkCounter;

#ifdef RTK_X86_CLE

#else
	if(phyNo > RTL8367B_PHY_INTERNALNOMAX)
		return RT_ERR_PHY_ID;
#endif

	if(phyAddr > RTL8367B_PHY_REGNOMAX)
		return RT_ERR_PHY_REG_ID;

    /*Check internal phy access busy or not*/
    /*retVal = rtl8367b_getAsicRegBit(RTL8367B_REG_INDRECT_ACCESS_STATUS, RTL8367B_INDRECT_ACCESS_STATUS_OFFSET,&busyFlag);*/
    retVal = rtl8367b_getAsicReg(RTL8367B_REG_INDRECT_ACCESS_STATUS,&busyFlag);
	if(retVal != RT_ERR_OK)
		return retVal;

    if(busyFlag)
        return RT_ERR_BUSYWAIT_TIMEOUT;

    /*prepare access data*/
    retVal = rtl8367b_setAsicReg(RTL8367B_REG_INDRECT_ACCESS_WRITE_DATA, phyData);
	if(retVal != RT_ERR_OK)
		return retVal;

    /*prepare access address*/
    regData = RTL8367B_PHY_BASE | (phyNo << RTL8367B_PHY_OFFSET) | phyAddr;

    retVal = rtl8367b_setAsicReg(RTL8367B_REG_INDRECT_ACCESS_ADDRESS, regData);
	if(retVal != RT_ERR_OK)
		return retVal;

    /*Set WRITE Command*/
    retVal = rtl8367b_setAsicReg(RTL8367B_REG_INDRECT_ACCESS_CTRL, RTL8367B_CMD_MASK | RTL8367B_RW_MASK);

    checkCounter = 5;
	while(checkCounter)
	{
    	retVal = rtl8367b_getAsicReg(RTL8367B_REG_INDRECT_ACCESS_STATUS,&busyFlag);
		if((retVal != RT_ERR_OK) || busyFlag)
		{
			checkCounter --;
			if(0 == checkCounter)
                return RT_ERR_BUSYWAIT_TIMEOUT;
		}
		else
		{
			checkCounter = 0;
		}
	}

    return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicPHYReg
 * Description:
 *      Get PHY registers
 * Input:
 *      phyNo 	- Physical port number (0~7)
 *      phyAddr - PHY address (0~31)
 *      pRegData - Writing data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_PHY_REG_ID  		- invalid PHY address
 *      RT_ERR_PHY_ID  			- invalid PHY no
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPHYReg(rtk_uint32 phyNo, rtk_uint32 phyAddr, rtk_uint32 *pRegData )
{
	ret_t retVal;
	rtk_uint32 regData;
    rtk_uint32 busyFlag,checkCounter;

#ifdef RTK_X86_CLE

#else
	if(phyNo > RTL8367B_PHY_INTERNALNOMAX)
		return RT_ERR_PHY_ID;
#endif
	if(phyAddr > RTL8367B_PHY_REGNOMAX)
		return RT_ERR_PHY_REG_ID;

    /*Check internal phy access busy or not*/
    /*retVal = rtl8367b_getAsicRegBit(RTL8367B_REG_INDRECT_ACCESS_STATUS, RTL8367B_INDRECT_ACCESS_STATUS_OFFSET,&busyFlag);*/
    retVal = rtl8367b_getAsicReg(RTL8367B_REG_INDRECT_ACCESS_STATUS,&busyFlag);
	if(retVal != RT_ERR_OK)
		return retVal;

    if(busyFlag)
        return RT_ERR_BUSYWAIT_TIMEOUT;

    /*prepare access address*/
    regData = RTL8367B_PHY_BASE | (phyNo << RTL8367B_PHY_OFFSET) | phyAddr;

    retVal = rtl8367b_setAsicReg(RTL8367B_REG_INDRECT_ACCESS_ADDRESS, regData);
	if(retVal != RT_ERR_OK)
		return retVal;

    /*Set READ Command*/
    retVal = rtl8367b_setAsicReg(RTL8367B_REG_INDRECT_ACCESS_CTRL, RTL8367B_CMD_MASK );
	if(retVal != RT_ERR_OK)
		return retVal;

	checkCounter = 5;
	while(checkCounter)
	{
    	retVal = rtl8367b_getAsicReg(RTL8367B_REG_INDRECT_ACCESS_STATUS,&busyFlag);
		if((retVal != RT_ERR_OK) || busyFlag)
		{
			checkCounter --;
			if(0 == checkCounter)
                return RT_ERR_FAILED;
		}
		else
		{
			checkCounter = 0;
		}
	}

    /*get PHY register*/
    retVal = rtl8367b_getAsicReg(RTL8367B_REG_INDRECT_ACCESS_READ_DATA, &regData);
	if(retVal != RT_ERR_OK)
		return retVal;

    *pRegData = regData;

    return RT_ERR_OK;
}

#endif
