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
 * Feature : Shared meter related functions
 *
 */
#include "rtl8367b_asicdrv_meter.h"
/* Function Name:
 *      rtl8367b_setAsicShareMeter
 * Description:
 *      Set meter configuration
 * Input:
 *      index 	- hared meter index (0-31)
 *      rate 	- 17-bits rate of share meter, unit is 8Kpbs
 *      ifg 	- Including IFG in rate calculation, 1:include 0:exclude 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicShareMeter(rtk_uint32 index, rtk_uint32 rate, rtk_uint32 ifg)
{
	ret_t retVal;

	if(index > RTL8367B_METERMAX)
		return RT_ERR_FILTER_METER_ID;

	/*17-bits Rate*/
	retVal = rtl8367b_setAsicReg(RTL8367B_METER_RATE_REG(index), rate&0xFFFF);
    if(retVal != RT_ERR_OK)
        return retVal;

	retVal = rtl8367b_setAsicReg(RTL8367B_METER_RATE_REG(index) + 1, (rate &0x10000) >> 16);
    if(retVal != RT_ERR_OK)
        return retVal;

	/*IFG*/
	retVal = rtl8367b_setAsicRegBit(RTL8367B_METER_IFG_CTRL_REG(index), RTL8367B_METER_IFG_OFFSET(index), ifg);
	
	return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicShareMeter
 * Description:
 *      Get meter configuration
 * Input:
 *      index 	- hared meter index (0-31)
 *      pRate 	- 17-bits rate of share meter, unit is 8Kpbs
 *      pIfg 	- Including IFG in rate calculation, 1:include 0:exclude 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicShareMeter(rtk_uint32 index, rtk_uint32 *pRate, rtk_uint32 *pIfg)
{
	rtk_uint32 regData;
	rtk_uint32 regData2;
	ret_t retVal;

	if(index > RTL8367B_METERMAX)
		return RT_ERR_FILTER_METER_ID;

	/*17-bits Rate*/
	retVal = rtl8367b_getAsicReg(RTL8367B_METER_RATE_REG(index), &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

	retVal = rtl8367b_getAsicReg(RTL8367B_METER_RATE_REG(index) + 1, &regData2);
    if(retVal != RT_ERR_OK)
        return retVal;

	*pRate = ((regData2 << 16) & 0x10000) | regData;
	/*IFG*/
	retVal = rtl8367b_getAsicRegBit(RTL8367B_METER_IFG_CTRL_REG(index), RTL8367B_METER_IFG_OFFSET(index), pIfg);

	return retVal;
}
/* Function Name:
 *      rtl8367b_setAsicShareMeterBucketSize
 * Description:
 *      Set meter related leaky bucket threshold
 * Input:
 *      index 		- hared meter index (0-31)
 *      lbthreshold - Leaky bucket threshold of meter
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicShareMeterBucketSize(rtk_uint32 index, rtk_uint32 lbthreshold)
{
	if(index > RTL8367B_METERMAX)
		return RT_ERR_FILTER_METER_ID;

	return rtl8367b_setAsicReg(RTL8367B_METER_BUCKET_SIZE_REG(index), lbthreshold);
}
/* Function Name:
 *      rtl8367b_getAsicShareMeterBucketSize
 * Description:
 *      Get meter related leaky bucket threshold
 * Input:
 *      index 		- hared meter index (0-31)
 *      pLbthreshold - Leaky bucket threshold of meter
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicShareMeterBucketSize(rtk_uint32 index, rtk_uint32 *pLbthreshold)
{
	if(index > RTL8367B_METERMAX)
		return RT_ERR_FILTER_METER_ID;

	return rtl8367b_getAsicReg(RTL8367B_METER_BUCKET_SIZE_REG(index), pLbthreshold);
}
/* Function Name:
 *      rtl8367b_setAsicMeterExceedStatus
 * Description:
 *      Clear shared meter status
 * Input:
 *      index 		- hared meter index (0-31)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicMeterExceedStatus(rtk_uint32 index)
{
	if(index > RTL8367B_METERMAX)
		return RT_ERR_FILTER_METER_ID;

	return rtl8367b_setAsicRegBit(RTL8367B_METER_OVERRATE_INDICATOR_REG(index), RTL8367B_METER_EXCEED_OFFSET(index), 1);
}
/* Function Name:
 *      rtl8367b_getAsicMeterExceedStatus
 * Description:
 *      Get shared meter status
 * Input:
 *      index 	- hared meter index (0-31)
 *      pStatus 	- 0: rate doesn't exceed 	1: rate exceeds
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      If rate is over rate*8Kbps of a meter, the state bit of this meter is set to 1.
 */
ret_t rtl8367b_getAsicMeterExceedStatus(rtk_uint32 index, rtk_uint32* pStatus)
{
	if(index > RTL8367B_METERMAX)
		return RT_ERR_FILTER_METER_ID;

	return rtl8367b_getAsicRegBit(RTL8367B_METER_OVERRATE_INDICATOR_REG(index), RTL8367B_METER_EXCEED_OFFSET(index), pStatus);
}

