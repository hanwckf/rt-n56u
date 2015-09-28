#ifndef __RALINK_SMI_H__
#define __RALINK_SMI_H__

void smi_init(void);
int  smi_read(u32 addr, u32 *data);
int  smi_write(u32 addr, u32 data);

#endif


