#ifndef __RALINK_SMI_H__
#define __RALINK_SMI_H__

#include "rtl8367b/rtk_types.h"
#include "rtl8367b/rtk_error.h"

void smi_init(rtk_uint32 gpio_clock, rtk_uint32 gpio_data);
int  smi_read(rtk_uint32 addr, rtk_uint32 *data);
int  smi_write(rtk_uint32 addr, rtk_uint32 data);

#endif


