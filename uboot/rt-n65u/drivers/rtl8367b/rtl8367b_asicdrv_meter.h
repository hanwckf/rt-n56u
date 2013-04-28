#ifndef _RTL8367B_ASICDRV_METER_H_
#define _RTL8367B_ASICDRV_METER_H_

#include "rtl8367b_asicdrv.h"


extern ret_t rtl8367b_setAsicShareMeter(rtk_uint32 index, rtk_uint32 rate, rtk_uint32 ifg);
extern ret_t rtl8367b_getAsicShareMeter(rtk_uint32 index, rtk_uint32 *pRate, rtk_uint32 *pIfg);
extern ret_t rtl8367b_setAsicShareMeterBucketSize(rtk_uint32 index, rtk_uint32 lbThreshold);
extern ret_t rtl8367b_getAsicShareMeterBucketSize(rtk_uint32 index, rtk_uint32 *pLbThreshold);
extern ret_t rtl8367b_setAsicMeterExceedStatus(rtk_uint32 index);
extern ret_t rtl8367b_getAsicMeterExceedStatus(rtk_uint32 index, rtk_uint32* pStatus);

#endif /*_RTL8367B_ASICDRV_FC_H_*/

