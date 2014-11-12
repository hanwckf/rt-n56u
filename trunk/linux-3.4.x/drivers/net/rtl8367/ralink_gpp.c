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

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/spinlock.h>

#include <linux/ralink_gpio.h>

#include "ralink_gpp.h"

#if defined(CONFIG_RTL8367_API_8370)
#include "api_8370/rtk_error.h"
#else
#include "api_8367b/rtk_error.h"
#endif

#define SMI_ACK_RETRY_COUNT 5

static spinlock_t g_smi_lock;
static u32 g_gpio_sda = 1;
static u32 g_gpio_sck = 2;
static u32 g_clk_delay_ns = 1700;
static u8  g_addr_slave = 0xB8;

/////////////////////////////////////////////////////////////////////////////////

static void _smi_start(void)
{
	/*
	 * Set GPIO pins to output mode, with initial state:
	 * SCK = 0, SDA = 1
	 */
	ralink_gpio_set_pin_direction(g_gpio_sck, 1);
	ralink_gpio_set_pin_direction(g_gpio_sda, 1);

	ralink_gpio_set_pin_value(g_gpio_sck, 0);
	ralink_gpio_set_pin_value(g_gpio_sda, 1);
	ndelay(g_clk_delay_ns);

	/* CLK 1: 0 -> 1, 1 -> 0 */
	ralink_gpio_set_pin_value(g_gpio_sck, 1);
	ndelay(g_clk_delay_ns);
	ralink_gpio_set_pin_value(g_gpio_sck, 0);
	ndelay(g_clk_delay_ns);

	/* CLK 2: */
	ralink_gpio_set_pin_value(g_gpio_sck, 1);
	ndelay(g_clk_delay_ns);
	ralink_gpio_set_pin_value(g_gpio_sda, 0);
	ndelay(g_clk_delay_ns);
	ralink_gpio_set_pin_value(g_gpio_sck, 0);
	ndelay(g_clk_delay_ns);
	ralink_gpio_set_pin_value(g_gpio_sda, 1);
}

static void _smi_stop(void)
{
	ndelay(g_clk_delay_ns);
	ralink_gpio_set_pin_value(g_gpio_sda, 0);
	ralink_gpio_set_pin_value(g_gpio_sck, 1);
	ndelay(g_clk_delay_ns);
	ralink_gpio_set_pin_value(g_gpio_sda, 1);
	ndelay(g_clk_delay_ns);
	ralink_gpio_set_pin_value(g_gpio_sck, 1);
	ndelay(g_clk_delay_ns);
	ralink_gpio_set_pin_value(g_gpio_sck, 0);
	ndelay(g_clk_delay_ns);
	ralink_gpio_set_pin_value(g_gpio_sck, 1);

	/* add a click */
	ndelay(g_clk_delay_ns);
	ralink_gpio_set_pin_value(g_gpio_sck, 0);
	ndelay(g_clk_delay_ns);
	ralink_gpio_set_pin_value(g_gpio_sck, 1);

	/* set GPIO pins to input mode */
	ralink_gpio_set_pin_direction(g_gpio_sda, 0);
	ralink_gpio_set_pin_direction(g_gpio_sck, 0);
}

static void _smi_write_bits(u32 data, u32 len)
{
	for (; len > 0; len--) {
		ndelay(g_clk_delay_ns);
		
		/* prepare data */
		ralink_gpio_set_pin_value(g_gpio_sda, !!(data & ( 1 << (len - 1))));
		ndelay(g_clk_delay_ns);
		
		/* clocking */
		ralink_gpio_set_pin_value(g_gpio_sck, 1);
		ndelay(g_clk_delay_ns);
		ralink_gpio_set_pin_value(g_gpio_sck, 0);
	}
}

static void _smi_read_bits(u32 len, u32 *data)
{
	u32 u;
	ralink_gpio_set_pin_direction(g_gpio_sda, 0);

	for (*data = 0; len > 0; len--) {
		ndelay(g_clk_delay_ns);
		
		/* clocking */
		ralink_gpio_set_pin_value(g_gpio_sck, 1);
		ndelay(g_clk_delay_ns);
		u = !!ralink_gpio_get_pin_value(g_gpio_sda);
		ralink_gpio_set_pin_value(g_gpio_sck, 0);
		
		*data |= (u << (len - 1));
	}

	ralink_gpio_set_pin_direction(g_gpio_sda, 1);
}

static int _smi_wait_for_ack(void)
{
	int retry_cnt;

	retry_cnt = 0;
	do {
		u32 ack = 0;
		
		_smi_read_bits(1, &ack);
		if (ack == 0)
			break;
		
		if (++retry_cnt > SMI_ACK_RETRY_COUNT) {
			return -1;
		}
	} while (1);

	return 0;
}

static int _smi_write_byte(u32 data)
{
	_smi_write_bits((data & 0xFF), 8);
	return _smi_wait_for_ack();
}

static u32 _smi_read_byte_x(u32 byte_index)
{
	u32 rd = 0;

	/* read data */
	_smi_read_bits(8, &rd);

	/* send an ACK */
	_smi_write_bits(byte_index, 1);
	return (rd & 0xFF);
}

/////////////////////////////////////////////////////////////////////////////////

void gpio_smi_init(u32 gpio_sda, u32 gpio_sck, u32 clk_delay_ns, u8 addr_slave)
{
	g_gpio_sda = gpio_sda;
	g_gpio_sck = gpio_sck;
	g_clk_delay_ns = clk_delay_ns;
	g_addr_slave = addr_slave;

	/* set GPIO pins to input mode */
	ralink_gpio_set_pin_direction(g_gpio_sda, 0);
	ralink_gpio_set_pin_direction(g_gpio_sck, 0);
}

int gpio_smi_read(u32 addr, u32 *data)
{
	u32 rd_lo = 0;
	u32 rd_hi = 0;
	int ret = RT_ERR_FAILED;

	spin_lock(&g_smi_lock);

	_smi_start();

	/* send READ command */
	ret = _smi_write_byte(g_addr_slave | 0x01);
	if (ret)
		goto out;

	/* set ADDR[7:0] */
	ret = _smi_write_byte(addr & 0xff);
	if (ret)
		goto out;

	/* set ADDR[15:8] */
	ret = _smi_write_byte(addr >> 8);
	if (ret)
		goto out;

	/* read DATA[7:0] */
	rd_lo = _smi_read_byte_x(0);

	/* read DATA[15:8] */
	rd_hi = _smi_read_byte_x(1);

	*data = (rd_hi << 8) | rd_lo;

	ret = RT_ERR_OK;

 out:
	_smi_stop();
	spin_unlock(&g_smi_lock);

	return ret;
}

int gpio_smi_write(u32 addr, u32 data)
{
	int ret = RT_ERR_FAILED;

	spin_lock(&g_smi_lock);

	_smi_start();

	/* send WRITE command */
	ret = _smi_write_byte(g_addr_slave);
	if (ret)
		goto out;

	/* set ADDR[7:0] */
	ret = _smi_write_byte(addr & 0xff);
	if (ret)
		goto out;

	/* set ADDR[15:8] */
	ret = _smi_write_byte(addr >> 8);
	if (ret)
		goto out;

	/* write DATA[7:0] */
	ret = _smi_write_byte(data & 0xff);
	if (ret)
		goto out;

	/* write DATA[15:8] */
	ret = _smi_write_byte(data >> 8);
	if (ret)
		goto out;

	ret = RT_ERR_OK;

 out:
	_smi_stop();
	spin_unlock(&g_smi_lock);

	return ret;
}

/////////////////////////////////////////////////////////////////////////////////

void gpio_init(void)
{
	spin_lock_init(&g_smi_lock);
}

int gpio_set_pin_direction(u32 pin, u32 use_direction_output)
{
	if (pin > RALINK_GPIO_NUMBER)
		return -EINVAL;

	spin_lock(&g_smi_lock);
	ralink_gpio_set_pin_direction(pin, use_direction_output);
	spin_unlock(&g_smi_lock);

	return 0;
}

int gpio_set_pin_value(u32 pin, u32 value)
{
	if (pin > RALINK_GPIO_NUMBER)
		return -EINVAL;

	spin_lock(&g_smi_lock);
	ralink_gpio_set_pin_value(pin, value);
	spin_unlock(&g_smi_lock);

	return 0;
}

int gpio_get_pin_value(u32 pin, u32 *value)
{
	if (pin > RALINK_GPIO_NUMBER)
		return -EINVAL;

	*value = ralink_gpio_get_pin_value(pin);

	return 0;
}

int gpio_set_mode_bit(u32 idx, u32 value)
{
	if (idx > 31)
		return -EINVAL;

	spin_lock(&g_smi_lock);
	ralink_gpio_mode_set_bit(idx, value);
	spin_unlock(&g_smi_lock);

	return 0;
}

void gpio_set_mode(u32 value)
{
	spin_lock(&g_smi_lock);
	ralink_gpio_mode_set(value);
	spin_unlock(&g_smi_lock);
}

void gpio_get_mode(u32 *value)
{
	*value = ralink_gpio_mode_get();
}

int ralink_initGpioPin(u32 pin, u32 use_direction_output)
{
	gpio_set_pin_direction(pin, use_direction_output);
	return 0;
}

int ralink_gpio_write_bit(u32 pin, u32 value)
{
	gpio_set_pin_value(pin, value);
	return 0;
}

