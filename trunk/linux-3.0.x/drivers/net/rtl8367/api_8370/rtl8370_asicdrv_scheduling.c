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
 * $Date: 2010/12/02 04:34:20 $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367B
 * Feature : 
 *
 */

#include "rtl8370_asicdrv_scheduling.h"

/*
@func ret_t | rtl8370_setAsicLeakyBucketParameter | Set Leaky Bucket Paramters.
@parm uint32 | tick | Tick is used for time slot size unit.
@parm uint32 | token | Token is used for adding budget in each time slot.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_TICK | Invalid TICK.
@rvalue RT_ERR_TOKEN | Invalid TOKEN.
@comm
    The API can set leaky bucket parameters as token and tick.
    The default suggesting values of (tick, token) are (0x77, 0x62).
 */
ret_t rtl8370_setAsicLeakyBucketParameter(uint32 tick, uint32 token)
{
    ret_t retVal;

    if(tick > 0xFF)
        return RT_ERR_TICK;

    if(token > 0xFF)
        return RT_ERR_TOKEN;
        
    retVal = rtl8370_setAsicRegBits(RTL8370_LEAKY_BUCKET_TICK_REG, RTL8370_LEAKY_BUCKET_TICK_MASK, tick);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8370_setAsicRegBits(RTL8370_LEAKY_BUCKET_TOKEN_REG, RTL8370_LEAKY_BUCKET_TOKEN_MASK, token);

    return retVal;
}

/*
@func ret_t | rtl8370_getAsicLeakyBucketParameter | Set Leaky Bucket Paramters.
@parm uint32* | tick | Tick is used for time slot size unit.
@parm uint32* | token | Token is used for adding budget in each time slot.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_TICK | Invalid TICK.
@rvalue RT_ERR_TOKEN | Invalid TOKEN.
@comm
    The API can set leaky bucket parameters as token and tick.
    The default suggesting values of (tick, token) are (0x77, 0x62).
 */
ret_t rtl8370_getAsicLeakyBucketParameter(uint32 *tick, uint32 *token)
{
    ret_t retVal;

    retVal = rtl8370_getAsicRegBits(RTL8370_LEAKY_BUCKET_TICK_REG, RTL8370_LEAKY_BUCKET_TICK_MASK, tick);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8370_getAsicRegBits(RTL8370_LEAKY_BUCKET_TOKEN_REG, RTL8370_LEAKY_BUCKET_TOKEN_MASK, token);

    return retVal;
}

/*
@func ret_t | rtl8370_setAsicAprMeter | Set per-port per-queue APR shared meter index
@parm uint32 | port | The port number
@parm uint32 | qid | Queue id.
@parm uint32 | apridx | dedicated shared meter index for APR (0~7).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_QUEUE_ID | Invalid queue id. 
@rvalue RT_ERR_FILTER_METER_ID | Invalid meter
@comm
    The API can set APR shared meter index. Each port has dedicated shared meter usage range.
    Port 0 & 8   ARP 7~0
    Port 1 & 9   ARP 15~8
    Port 2 & 10  ARP 23~16
    :
    Port 7 & 15  ARP 63~56
*/
ret_t rtl8370_setAsicAprMeter(uint32 port, uint32 qid, uint32 apridx)
{
    ret_t retVal;

    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;
    
    if(qid > RTL8370_QIDMAX)
        return RT_ERR_QUEUE_ID;
    
    if(apridx > RTL8370_PORT_QUEUE_METER_INDEX_MAX)
        return RT_ERR_FILTER_METER_ID;
    
    retVal = rtl8370_setAsicRegBits(RTL8370_SCHEDULE_PORT_APR_METER_REG(port, qid), RTL8370_SCHEDULE_PORT_APR_METER_MASK(qid), apridx);

    return retVal;
}
/*
@func ret_t | rtl8370_getAsicAprMeter | Get per-port per-queue APR shared meter index
@parm uint32 | port | The port number
@parm uint32 | qid | Queue id.
@parm uint32* | apridx | shared meter index for APR.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_QUEUE_ID | Invalid queue id. 
@rvalue RT_ERR_FILTER_METER_ID | Invalid meter
@comm
    The API can get APR shared meter index. Each port has dedicated shared meter usage range.
*/
ret_t rtl8370_getAsicAprMeter(uint32 port, uint32 qid, uint32 *apridx)
{
    ret_t retVal;

    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;
    
    if(qid > RTL8370_QIDMAX)
        return RT_ERR_QUEUE_ID;

    retVal = rtl8370_getAsicRegBits(RTL8370_SCHEDULE_PORT_APR_METER_REG(port, qid), RTL8370_SCHEDULE_PORT_APR_METER_MASK(qid), apridx);

    return retVal;
}

/*
@func ret_t | rtl8370_setAsicAprEnable | Set per-port APR enable.
@parm uint32 | port | The port number
@parm uint32 | aprEnable | APR enable seting 1:enable 0:disable.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_ENABLE | Invalid enable parameter.
@comm
    The API can set per-port APR enable setting
*/
ret_t rtl8370_setAsicAprEnable(uint32 port, uint32 aprEnable)
{
    ret_t retVal;
    
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if((aprEnable != 0) && (aprEnable != 1))
        return RT_ERR_ENABLE; 


    retVal = rtl8370_setAsicRegBit(RTL8370_SCHEDULE_APR_CRTL_REG, RTL8370_SCHEDULE_APR_CRTL_OFFSET(port), aprEnable);

    return retVal;
}
/*
@func ret_t | rtl8370_getAsicAprEnable | Set per-port APR enable.
@parm uint32 | port | The port number
@parm uint32* | aprEnable | APR enable seting 1:enable 0:disable.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can get per-port APR enable setting
*/
ret_t rtl8370_getAsicAprEnable(uint32 port, uint32 *aprEnable)
{
    ret_t retVal;
    
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    retVal = rtl8370_getAsicRegBit(RTL8370_SCHEDULE_APR_CRTL_REG, RTL8370_SCHEDULE_APR_CRTL_OFFSET(port), aprEnable);

    return retVal;
}

/*
@func ret_t | rtl8370_setAsicWFQWeight | Set weight  of a queue.
@parm uint32 | port | The port number.
@parm uint32 | qid | The queue ID wanted to set.
@parm uint32 | qWeight | The weight value wanted to set (valid:0~127).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_QUEUE_ID | Invalid queue id.
@rvalue RT_ERR_QOS_QUEUE_WEIGHT | Invalid queue weight.
@comm
    The API can set weight of the specified queue.  Parameter 'qweight' is only used in queueType = strict priority.  
    If queue type is strict priority, the parameter 'qweight' can be ignored.  
 */
ret_t rtl8370_setAsicWFQWeight(uint32 port, uint32 qid, uint32 qWeight)
{
    ret_t retVal;

    /* Invalid input parameter */
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(qid > RTL8370_QIDMAX)
        return RT_ERR_QUEUE_ID;

    if(qWeight > RTL8370_QWEIGHTMAX)
        return RT_ERR_QOS_QUEUE_WEIGHT;

    retVal = rtl8370_setAsicReg(RTL8370_SCHEDULE_PORT_QUEUE_WFQ_WEIGHT_REG(port, qid), qWeight);
    
    return retVal;
}

/*
@func ret_t | rtl8370_getAsicWFQWeight | Get weight of WFQ.
@parm uint32 | port | The port number.
@parm uint32 | qid | The queue ID wanted to set.
@parm uint32* | qWeight | Pointer to the returned weight value.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_QUEUE_ID | Invalid queue id.
@comm
    The API can get weight of the specified queue.  Parameter 'qweight' is only used in queueType = strict priority.  
    If queue type is strict priority, the parameter 'qweight' can be ignored.  
 */
ret_t rtl8370_getAsicWFQWeight(uint32 port, uint32 qid, uint32 *qWeight)
{
    ret_t retVal;


    /* Invalid input parameter */
    if(port  > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(qid > RTL8370_QIDMAX)
        return RT_ERR_QUEUE_ID;


    retVal = rtl8370_getAsicReg(RTL8370_SCHEDULE_PORT_QUEUE_WFQ_WEIGHT_REG(port, qid), qWeight);    

    return retVal;
}
/*
@func ret_t | rtl8370_setAsicWFQBurstSize | Set WFQ leaky bucket burst size.
@parm uint32 | burstsize | Leaky bucket burst size, unit byte
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can set WFQ leaky bucket burst size(aka high threshold) 
 */
ret_t rtl8370_setAsicWFQBurstSize(uint32 burstsize)
{
    ret_t retVal;

    retVal = rtl8370_setAsicReg(RTL8370_SCHEDULE_WFQ_BURST_SIZE_REG, burstsize);
    
    return retVal;
}
/*
@func ret_t | rtl8370_getAsicWFQBurstSize | Set WFQ leaky bucket burst size.
@parm uint32* | burstsize | Leaky bucket burst size, unit byte
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can set WFQ leaky bucket burst size(aka high threshold) 
 */
ret_t rtl8370_getAsicWFQBurstSize(uint32 *burstsize)
{
    ret_t retVal;

    retVal = rtl8370_getAsicReg(RTL8370_SCHEDULE_WFQ_BURST_SIZE_REG, burstsize);
    
    return retVal;
}

/*
@func ret_t | rtl8370_setAsicQueueType | Set type of a queue.
@parm uint32 | port | The port number.
@parm uint32 | qid | The queue ID wanted to set.
@parm uint32 | queueType | The specified queue type. 0b0: Strict priority, 0b1: WFQ.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_QUEUE_ID | Invalid queue id.
@rvalue RT_ERR_QOS_SCHE_TYPE | Invalid queue type.
@comm
    The API can set type, strict priority or weight fair queue (WFQ), of the specified queue.  
 */
ret_t rtl8370_setAsicQueueType(uint32 port, uint32 qid, uint32 queueType)
{
    ret_t retVal;

    /* Invalid input parameter */
    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(qid > RTL8370_QIDMAX)
        return RT_ERR_QUEUE_ID;

    if((queueType != QTYPE_WFQ) && (queueType != QTYPE_STRICT))
        return RT_ERR_QOS_SCHE_TYPE;


    /* Set Related Registers */
    retVal = rtl8370_setAsicRegBit(RTL8370_SCHEDULE_QUEUE_TYPE_REG(port), RTL8370_SCHEDULE_QUEUE_TYPE_OFFSET(port, qid),queueType);    
  
    return retVal;
}

/*
@func ret_t | rtl8370_getAsicQueueType | Get type of WFQ.
@parm uint32 | port | The port number.
@parm uint32 | qid | The queue ID wanted to set.
@parm uint32* | queueType | Pointer to the returned queue type.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_QUEUE_ID | Invalid queue id.
@comm
    The API can get type, strict priority or weight fair queue (WFQ), of the specified queue.  
 */
ret_t rtl8370_getAsicQueueType(uint32 port, uint32 qid, uint32 *queueType)
{
    ret_t retVal;

    /* Invalid input parameter */
    if(port  > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(qid > RTL8370_QIDMAX)
        return RT_ERR_QUEUE_ID;

    retVal = rtl8370_getAsicRegBit(RTL8370_SCHEDULE_QUEUE_TYPE_REG(port), RTL8370_SCHEDULE_QUEUE_TYPE_OFFSET(port, qid),queueType);    

    return retVal;
}

/*
@func ret_t | rtl8370_setAsicPortEgressRate | Set per-port egress rate
@parm uint32 | port | The port number.
@parm uint32 | rate | Egress rate
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@rvalue RT_ERR_QOS_EBW_RATE | Invalid egress bandwidth/rate
@comm
    The API can set per-port egress rate
 */
ret_t rtl8370_setAsicPortEgressRate(uint32 port, uint32 rate)
{
    ret_t retVal;
    uint32 regAddr, regData;

    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;
    
    if(rate > RTL8370_QOS_GRANULARTY_MAX)
        return RT_ERR_QOS_EBW_RATE;

    regAddr = RTL8370_PORT_EGRESSBW_LSB_REG(port);
    regData = RTL8370_QOS_GRANULARTY_LSB_MASK & rate;
    
    retVal = rtl8370_setAsicReg(regAddr, regData);

    if(retVal != RT_ERR_OK)
        return retVal;

    regAddr = RTL8370_PORT_EGRESSBW_MSB_REG(port);
    regData = (RTL8370_QOS_GRANULARTY_MSB_MASK & rate) >> RTL8370_QOS_GRANULARTY_MSB_OFFSET;

    return rtl8370_setAsicRegBit(regAddr, RTL8370_PORT_EGRESSBW_MSB_OFFSET, regData);
}
/*
@func ret_t | rtl8370_getAsicPortEgressRate | Set per-port egress rate
@parm uint32 | port | The port number.
@parm uint32* | rate | Egress rate
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can set per-port egress rate.
 */
ret_t rtl8370_getAsicPortEgressRate(uint32 port, uint32 *rate)
{
    ret_t retVal;
    uint32 regAddr, regData,regData2;

    if(port > RTL8370_PORTIDMAX)
        return RT_ERR_PORT_ID;

    regAddr = RTL8370_PORT_EGRESSBW_LSB_REG(port);
    
    retVal = rtl8370_getAsicReg(regAddr, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    regAddr = RTL8370_PORT_EGRESSBW_MSB_REG(port);
    retVal = rtl8370_getAsicRegBit(regAddr, RTL8370_PORT_EGRESSBW_MSB_OFFSET, &regData2);
    if(retVal != RT_ERR_OK)
        return retVal;

    *rate = regData | (regData2 << RTL8370_QOS_GRANULARTY_MSB_OFFSET);

    return RT_ERR_OK;
}

/*
@func ret_t | rtl8370_setAsicPortEgressRateIfg | Set per-port egress rate calculate include/exclude IFG
@parm uint32 | ifg | 1:include IFG 0:exclude IFG
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    The API can set egress rate with IFG or not.
 */
ret_t rtl8370_setAsicPortEgressRateIfg(uint32 ifg)
{
    if(ifg > 1)
        return RT_ERR_INPUT;

    return rtl8370_setAsicRegBit(RTL8370_REG_SCHEDULE_WFQ_CTRL, RTL8370_WFQ_IFG_OFFSET, ifg);
}
/*
@func ret_t | rtl8370_getAsicPortEgressRateIfg | Get per-port egress rate calculate include/exclude IFG
@parm uint32* | ifg | 1:include IFG 0:exclude IFG
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
    The API can get egress rate with IFG or not.
 */
ret_t rtl8370_getAsicPortEgressRateIfg(uint32 *ifg)
{
    return rtl8370_getAsicRegBit(RTL8370_REG_SCHEDULE_WFQ_CTRL, RTL8370_WFQ_IFG_OFFSET, ifg);
}
