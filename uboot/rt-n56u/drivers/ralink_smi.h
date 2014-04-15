
#ifndef __RALINK_SMI_H__
#define __RALINK_SMI_H__

#include "rtl8370/rtk_types.h"
#include "rtl8370/rtk_error.h"

int32 smi_read(uint32 mAddrs, uint32 *rData);
int32 smi_write(uint32 mAddrs, uint32 rData);

#endif /* __SMI_H__ */


