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
 * Feature : Packet Scheduling related functions
 *
 */

#include "rtl8367b_asicdrv_scheduling.h"
/* Function Name:
 *      rtl8367b_setAsicLeakyBucketParameter
 * Description:
 *      Set Leaky Bucket Paramters
 * Input:
 *      tick 	- Tick is used for time slot size unit
 *      token 	- Token is used for adding budget in each time slot
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_TICK  	- Invalid TICK
 *      RT_ERR_TOKEN  	- Invalid TOKEN
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicLeakyBucketParameter(rtk_uint32 tick, rtk_uint32 token)
{
    ret_t retVal;

    if(tick > 0xFF)
        return RT_ERR_TICK;

    if(token > 0xFF)
        return RT_ERR_TOKEN;
        
    retVal = rtl8367b_setAsicRegBits(RTL8367B_LEAKY_BUCKET_TICK_REG, RTL8367B_LEAKY_BUCKET_TICK_MASK, tick);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_setAsicRegBits(RTL8367B_LEAKY_BUCKET_TOKEN_REG, RTL8367B_LEAKY_BUCKET_TOKEN_MASK, token);

    return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicLeakyBucketParameter
 * Description:
 *      Get Leaky Bucket Paramters
 * Input:
 *      tick 	- Tick is used for time slot size unit
 *      token 	- Token is used for adding budget in each time slot
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicLeakyBucketParameter(rtk_uint32 *tick, rtk_uint32 *token)
{
    ret_t retVal;

    retVal = rtl8367b_getAsicRegBits(RTL8367B_LEAKY_BUCKET_TICK_REG, RTL8367B_LEAKY_BUCKET_TICK_MASK, tick);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367b_getAsicRegBits(RTL8367B_LEAKY_BUCKET_TOKEN_REG, RTL8367B_LEAKY_BUCKET_TOKEN_MASK, token);

    return retVal;
}
/* Function Name:
 *      rtl8367b_setAsicAprMeter
 * Description:
 *      Set per-port per-queue APR shared meter index
 * Input:
 *      port 	- Physical port number (0~7)
 *      qid 	- Queue id
 *      apridx 	- dedicated shared meter index for APR (0~7)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_PORT_ID  		- Invalid port number
 *      RT_ERR_QUEUE_ID  		- Invalid queue id
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicAprMeter(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 apridx)
{
    ret_t retVal;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;
    
    if(qid > RTL8367B_QIDMAX)
        return RT_ERR_QUEUE_ID;
    
    if(apridx > RTL8367B_PORT_QUEUE_METER_INDEX_MAX)
        return RT_ERR_FILTER_METER_ID;
    
    retVal = rtl8367b_setAsicRegBits(RTL8367B_SCHEDULE_PORT_APR_METER_REG(port, qid), RTL8367B_SCHEDULE_PORT_APR_METER_MASK(qid), apridx);

    return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicAprMeter
 * Description:
 *      Get per-port per-queue APR shared meter index
 * Input:
 *      port 	- Physical port number (0~7)
 *      qid 	- Queue id
 *      apridx 	- dedicated shared meter index for APR (0~7)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 *      RT_ERR_QUEUE_ID - Invalid queue id
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicAprMeter(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 *apridx)
{
    ret_t retVal;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;
    
    if(qid > RTL8367B_QIDMAX)
        return RT_ERR_QUEUE_ID;

    retVal = rtl8367b_getAsicRegBits(RTL8367B_SCHEDULE_PORT_APR_METER_REG(port, qid), RTL8367B_SCHEDULE_PORT_APR_METER_MASK(qid), apridx);

    return retVal;
}
/* Function Name:
 *      rtl8367b_setAsicAprEnable
 * Description:
 *      Set per-port APR enable
 * Input:
 *      port 		- Physical port number (0~7)
 *      aprEnable 	- APR enable seting 1:enable 0:disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicAprEnable(rtk_uint32 port, rtk_uint32 aprEnable)
{
    ret_t retVal;
    
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    retVal = rtl8367b_setAsicRegBit(RTL8367B_SCHEDULE_APR_CTRL_REG, RTL8367B_SCHEDULE_APR_CTRL_OFFSET(port), aprEnable);

    return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicAprEnable
 * Description:
 *      Get per-port APR enable
 * Input:
 *      port 		- Physical port number (0~7)
 *      aprEnable 	- APR enable seting 1:enable 0:disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicAprEnable(rtk_uint32 port, rtk_uint32 *aprEnable)
{
    ret_t retVal;
    
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    retVal = rtl8367b_getAsicRegBit(RTL8367B_SCHEDULE_APR_CTRL_REG, RTL8367B_SCHEDULE_APR_CTRL_OFFSET(port), aprEnable);

    return retVal;
}
/* Function Name:
 *      rtl8367b_setAsicWFQWeight
 * Description:
 *      Set weight  of a queue
 * Input:
 *      port 	- Physical port number (0~7)
 *      qid 	- The queue ID wanted to set
 *      qWeight - The weight value wanted to set (valid:0~127)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_PORT_ID  		- Invalid port number
 *      RT_ERR_QUEUE_ID  		- Invalid queue id
 *      RT_ERR_QOS_QUEUE_WEIGHT - Invalid queue weight
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicWFQWeight(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 qWeight)
{
	ret_t retVal;

	/* Invalid input parameter */
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    if(qid > RTL8367B_QIDMAX)
        return RT_ERR_QUEUE_ID;

	if(qWeight > RTL8367B_QWEIGHTMAX && qid > 0)
		return RT_ERR_QOS_QUEUE_WEIGHT;

	retVal = rtl8367b_setAsicReg(RTL8367B_SCHEDULE_PORT_QUEUE_WFQ_WEIGHT_REG(port, qid), qWeight);
    
	return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicWFQWeight
 * Description:
 *      Get weight  of a queue
 * Input:
 *      port 	- Physical port number (0~7)
 *      qid 	- The queue ID wanted to set
 *      qWeight - The weight value wanted to set (valid:0~127)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_PORT_ID  		- Invalid port number
 *      RT_ERR_QUEUE_ID  		- Invalid queue id
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicWFQWeight(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 *qWeight)
{
	ret_t retVal;


	/* Invalid input parameter */
	if(port  > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    if(qid > RTL8367B_QIDMAX)
        return RT_ERR_QUEUE_ID;


	retVal = rtl8367b_getAsicReg(RTL8367B_SCHEDULE_PORT_QUEUE_WFQ_WEIGHT_REG(port, qid), qWeight);	

	return retVal;
}
/* Function Name:
 *      rtl8367b_setAsicWFQBurstSize
 * Description:
 *      Set WFQ leaky bucket burst size
 * Input:
 *      burstsize 	- Leaky bucket burst size, unit byte
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicWFQBurstSize(rtk_uint32 burstsize)
{
	ret_t retVal;

	retVal = rtl8367b_setAsicReg(RTL8367B_SCHEDULE_WFQ_BURST_SIZE_REG, burstsize);
    
	return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicWFQBurstSize
 * Description:
 *      Get WFQ leaky bucket burst size
 * Input:
 *      burstsize 	- Leaky bucket burst size, unit byte
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicWFQBurstSize(rtk_uint32 *burstsize)
{
	ret_t retVal;

	retVal = rtl8367b_getAsicReg(RTL8367B_SCHEDULE_WFQ_BURST_SIZE_REG, burstsize);
    
	return retVal;
}
/* Function Name:
 *      rtl8367b_setAsicQueueType
 * Description:
 *      Set type of a queue
 * Input:
 *      port 		- Physical port number (0~7)
 *      qid 		- The queue ID wanted to set
 *      queueType 	- The specified queue type. 0b0: Strict priority, 0b1: WFQ
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 *      RT_ERR_QUEUE_ID - Invalid queue id
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicQueueType(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 queueType)
{
	ret_t retVal;

	/* Invalid input parameter */
	if(port > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    if(qid > RTL8367B_QIDMAX)
        return RT_ERR_QUEUE_ID;

	/* Set Related Registers */
	retVal = rtl8367b_setAsicRegBit(RTL8367B_SCHEDULE_QUEUE_TYPE_REG(port), RTL8367B_SCHEDULE_QUEUE_TYPE_OFFSET(port, qid),queueType);	
  
	return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicQueueType
 * Description:
 *      Get type of a queue
 * Input:
 *      port 		- Physical port number (0~7)
 *      qid 		- The queue ID wanted to set
 *      queueType 	- The specified queue type. 0b0: Strict priority, 0b1: WFQ
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 *      RT_ERR_QUEUE_ID - Invalid queue id
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicQueueType(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 *queueType)
{
	ret_t retVal;

	/* Invalid input parameter */
	if(port  > RTL8367B_PORTIDMAX)
		return RT_ERR_PORT_ID;

    if(qid > RTL8367B_QIDMAX)
        return RT_ERR_QUEUE_ID;

	retVal = rtl8367b_getAsicRegBit(RTL8367B_SCHEDULE_QUEUE_TYPE_REG(port), RTL8367B_SCHEDULE_QUEUE_TYPE_OFFSET(port, qid),queueType);	

	return retVal;
}
/* Function Name:
 *      rtl8367b_setAsicPortEgressRate
 * Description:
 *      Set per-port egress rate
 * Input:
 *      port 		- Physical port number (0~7)
 *      rate 		- Egress rate
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID  	- Invalid port number
 *      RT_ERR_QOS_EBW_RATE - Invalid bandwidth/rate
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPortEgressRate(rtk_uint32 port, rtk_uint32 rate)
{
    ret_t retVal;
    rtk_uint32 regAddr, regData;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;
    
    if(rate > RTL8367B_QOS_GRANULARTY_MAX)
        return RT_ERR_QOS_EBW_RATE;

    regAddr = RTL8367B_PORT_EGRESSBW_LSB_REG(port);
    regData = RTL8367B_QOS_GRANULARTY_LSB_MASK & rate;
    
    retVal = rtl8367b_setAsicReg(regAddr, regData);

    if(retVal != RT_ERR_OK)
        return retVal;

    regAddr = RTL8367B_PORT_EGRESSBW_MSB_REG(port);
    regData = (RTL8367B_QOS_GRANULARTY_MSB_MASK & rate) >> RTL8367B_QOS_GRANULARTY_MSB_OFFSET;

    retVal = rtl8367b_setAsicRegBit(regAddr, RTL8367B_PORT0_EGRESSBW_CTRL1_OFFSET, regData);

	return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicPortEgressRate
 * Description:
 *      Get per-port egress rate
 * Input:
 *      port 		- Physical port number (0~7)
 *      rate 		- Egress rate
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 			- Success
 *      RT_ERR_SMI  		- SMI access error
 *      RT_ERR_PORT_ID  	- Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortEgressRate(rtk_uint32 port, rtk_uint32 *rate)
{
    ret_t retVal;
    rtk_uint32 regAddr, regData,regData2;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    regAddr = RTL8367B_PORT_EGRESSBW_LSB_REG(port);
    
    retVal = rtl8367b_getAsicReg(regAddr, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    regAddr = RTL8367B_PORT_EGRESSBW_MSB_REG(port);
    retVal = rtl8367b_getAsicRegBit(regAddr, RTL8367B_PORT0_EGRESSBW_CTRL1_OFFSET, &regData2);
    if(retVal != RT_ERR_OK)
        return retVal;

    *rate = regData | (regData2 << RTL8367B_QOS_GRANULARTY_MSB_OFFSET);

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicPortEgressRateIfg
 * Description:
 *      Set per-port egress rate calculate include/exclude IFG
 * Input:
 *      ifg 	- 1:include IFG 0:exclude IFG
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicPortEgressRateIfg(rtk_uint32 ifg)
{
    ret_t retVal;

    retVal = rtl8367b_setAsicRegBit(RTL8367B_REG_SCHEDULE_WFQ_CTRL, RTL8367B_SCHEDULE_WFQ_CTRL_OFFSET, ifg);

	return retVal;
}
/* Function Name:
 *      rtl8367b_getAsicPortEgressRateIfg
 * Description:
 *      Get per-port egress rate calculate include/exclude IFG
 * Input:
 *      ifg 	- 1:include IFG 0:exclude IFG
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 	- Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicPortEgressRateIfg(rtk_uint32 *ifg)
{
    ret_t retVal;

    retVal = rtl8367b_getAsicRegBit(RTL8367B_REG_SCHEDULE_WFQ_CTRL, RTL8367B_SCHEDULE_WFQ_CTRL_OFFSET, ifg);

	return retVal;
}
