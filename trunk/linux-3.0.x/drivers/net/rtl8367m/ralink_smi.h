#ifndef __RALINK_SMI_H__
#define __RALINK_SMI_H__

#include "rtk_types.h"
#include "rtk_error.h"

void smi_init(u32 gpio_sda, u32 gpio_sck, u32 clk_delay_ns, u8 addr_slave);
int  smi_read(uint32 addr, uint32 *data);
int  smi_write(uint32 addr, uint32 data);

int  gpio_set_pin_direction(u32 pin, u32 use_direction_output);
int  gpio_set_pin_value(u32 pin, u32 value);
int  gpio_get_pin_value(u32 pin, u32 *value);
int  gpio_set_mode_bit(u32 idx, u32 value);
void gpio_set_mode(u32 value);
void gpio_get_mode(u32 *value);


#endif


