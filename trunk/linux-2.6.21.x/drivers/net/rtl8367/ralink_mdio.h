#ifndef __RALINK_MDIO_H__
#define __RALINK_MDIO_H__

void mdio_init(u32 phy_id);
int  smi_read(u32 addr, u32 *data);
int  smi_write(u32 addr, u32 data);

#endif


