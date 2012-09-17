#ifndef _RTL8370_ASICDRV_FC_H_
#define _RTL8370_ASICDRV_FC_H_

#include "rtl8370_asicdrv.h"


extern ret_t rtl8370_setAsicShareMeter(uint32 index, uint32 rate ,uint32 ifg);
extern ret_t rtl8370_getAsicShareMeter(uint32 index, uint32 *rate ,uint32 *ifg);
extern ret_t rtl8370_setAsicShareMeterBucketSize(uint32 index, uint32 lbthreshold);
extern ret_t rtl8370_getAsicShareMeterBucketSize(uint32 index, uint32 *lbthreshold);
extern ret_t rtl8370_setAsicMeterState(uint32 index);
extern ret_t rtl8370_getAsicMeterState(uint32 index, uint32* state);

#endif /*_RTL8370_ASICDRV_FC_H_*/

