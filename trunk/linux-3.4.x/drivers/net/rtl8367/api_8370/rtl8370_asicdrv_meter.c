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
 * $Date: 2010/12/02 04:34:26 $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367B
 * Feature : 
 *
 */

#include "rtl8370_asicdrv_meter.h"

/*
@func ret_t | rtl8370_setAsicShareMeter | Set meter configuration
@parm uint32 | index | Shared meter index (0-63) of 64 shared meter index
@parm uint32 | rate | 17-bits rate of share meter, unit is 8Kpbs
@parm uint32 | ifg | Rate's calculation including IFG 1:include 0:exclude 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter
@rvalue RT_ERR_RATE | Invalid rate
@rvalue RT_ERR_FILTER_METER_ID | Invalid meter
@comm
    The API can set shared meter rate and ifg include for each meter. Rate unit is 8Kbps.    
 */
ret_t rtl8370_setAsicShareMeter(uint32 index, uint32 rate ,uint32 ifg)
{
    ret_t retVal;

    if(index > RTL8370_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    if(rate > RTL8370_QOS_GRANULARTY_MAX)
        return RT_ERR_RATE;
    
    if(ifg > 1)
        return RT_ERR_INPUT;

    /*17-bits Rate*/
    retVal = rtl8370_setAsicReg(RTL8370_METER_RATE_REG(index), (rate & 0xFFFF));
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8370_setAsicReg(RTL8370_METER_RATE_REG(index) + 1, ((rate & 0x10000) >> 16));
    if(retVal != RT_ERR_OK)
        return retVal;

    /*IFG*/
    return rtl8370_setAsicRegBit(RTL8370_METER_IFG_CTRL_REG(index), RTL8370_METER_IFG_OFFSET(index), ifg);
}
/*
@func ret_t | rtl8370_getAsicShareMeter | Get meter configuration
@parm uint32 | index | Shared meter index (0-63) of 64 shared meter index
@parm uint32* | rate | 17-bits rate of share meter, unit is 8Kpbs
@parm uint32* | ifg | Rate's calculation including IFG 1:include 0:exclude 
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_FILTER_METER_ID | Invalid meter
@comm
    The API can get shared meter rate and ifg include for each meter. Rate unit is 8Kbps.    
 */
ret_t rtl8370_getAsicShareMeter(uint32 index, uint32 *rate ,uint32 *ifg)
{
    uint32 regData;
    uint32 regData2;
    ret_t retVal;

    if(index > RTL8370_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    /*17-bits Rate*/
    retVal = rtl8370_getAsicReg(RTL8370_METER_RATE_REG(index), &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8370_getAsicReg(RTL8370_METER_RATE_REG(index)+1, &regData2);
    if(retVal != RT_ERR_OK)
        return retVal;

    *rate = ((regData2 << 16) & 0x10000) | regData;
    /*IFG*/
    return rtl8370_getAsicRegBit(RTL8370_METER_IFG_CTRL_REG(index), RTL8370_METER_IFG_OFFSET(index), ifg);
}
/*
@func ret_t | rtl8370_setAsicShareMeterBucketSize | Set meter related leaky bucket threshold
@parm uint32 | index | Shared meter index (0-63) of 64 shared meter index
@parm uint32 | lbthreshold | Leaky bucket threshold of this meter
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_FILTER_METER_ID | Invalid meter
@comm
    The API can set shared meter leaky bucket threshold for each meter.    
 */
ret_t rtl8370_setAsicShareMeterBucketSize(uint32 index, uint32 lbthreshold)
{
    if(index > RTL8370_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    return rtl8370_setAsicReg(RTL8370_METER_BUCKET_SIZE_REG(index), lbthreshold);
}
/*
@func ret_t | rtl8370_getAsicShareMeterBucketSize | Get meter related leaky bucket threshold
@parm uint32 | index | Shared meter index (0-63) of 64 shared meter index
@parm uint32 | lbthreshold | Leaky bucket threshold of this meter
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_FILTER_METER_ID | Invalid meter
@comm
    The API can get shared meter leaky bucket threshold for each meter.     
 */
ret_t rtl8370_getAsicShareMeterBucketSize(uint32 index, uint32 *lbthreshold)
{
    if(index > RTL8370_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    return rtl8370_getAsicReg(RTL8370_METER_BUCKET_SIZE_REG(index), lbthreshold);
}
/*
@func ret_t | rtl8370_setAsicMeterState | Clear shared meter status.
@parm uint32 | index | Shared meter index (0-63) of 64 shared meter index
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_FILTER_METER_ID | Invalid meter
@rvalue RT_ERR_FAILED | Invalid parameter.
@comm
    If rate is over rate*8Kbps of a meter and exceed bit of meter is set, software can write 1 to clear related exceed bit.
 */
ret_t rtl8370_setAsicMeterState(uint32 index)
{
    if(index > RTL8370_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    return rtl8370_setAsicRegBit(RTL8370_METER_OVERRATE_INDICATOR_REG(index), RTL8370_METER_EXCEED_OFFSET(index), 1);
}


/*
@func ret_t | rtl8370_getAsicMeterState | Get shared meter status.
@parm uint32 | index | Shared meter index (0-63) of 64 shared meter index
@parm uint32* | state | 0: rate doesn't exceed     1: rate exceeds
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_FILTER_METER_ID | Invalid meter
@rvalue RT_ERR_FAILED | Invalid parameter.
@comm
    If rate is over rate*8Kbps of a meter, the state bit of this meter is set to 1.
    Clear by write.
 */
ret_t rtl8370_getAsicMeterState(uint32 index, uint32* state)
{
    if(index > RTL8370_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    return rtl8370_getAsicRegBit(RTL8370_METER_OVERRATE_INDICATOR_REG(index), RTL8370_METER_EXCEED_OFFSET(index), &(*state));
}

