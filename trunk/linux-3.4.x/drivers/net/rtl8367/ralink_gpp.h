/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#ifndef __RALINK_GPP_H__
#define __RALINK_GPP_H__

void gpio_init(void);
int  gpio_set_pin_direction(u32 pin, u32 use_direction_output);
int  gpio_set_pin_value(u32 pin, u32 value);
int  gpio_get_pin_value(u32 pin, u32 *value);
int  gpio_set_mode_bit(u32 idx, u32 value);
void gpio_set_mode(u32 value);
void gpio_get_mode(u32 *value);
void gpio_set_mdio_unlocked(int enable);

void gpio_smi_init(u32 gpio_sda, u32 gpio_sck, u32 clk_delay_ns, u8 addr_slave);
int  gpio_smi_read(u32 addr, u32 *data);
int  gpio_smi_write(u32 addr, u32 data);

// for iNIC_mii.ko
extern int ralink_initGpioPin(u32 pin, u32 use_direction_output);
extern int ralink_gpio_write_bit(u32 pin, u32 value);

#endif


