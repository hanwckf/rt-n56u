#ifndef _RTL8367B_ASICDRV_EEE_H_
#define _RTL8367B_ASICDRV_EEE_H_

#include "rtl8367b_asicdrv.h"

extern ret_t rtl8367b_setAsicEee100M(rtk_uint32 port, rtk_uint32 enable);
extern ret_t rtl8367b_getAsicEee100M(rtk_uint32 port, rtk_uint32 *enable);
extern ret_t rtl8367b_setAsicEeeGiga(rtk_uint32 port, rtk_uint32 enable);
extern ret_t rtl8367b_getAsicEeeGiga(rtk_uint32 port, rtk_uint32 *enable);


#endif /*_RTL8367B_ASICDRV_EEE_H_*/
