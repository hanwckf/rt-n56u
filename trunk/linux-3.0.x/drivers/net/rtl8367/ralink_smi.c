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

#include <ralink/ralink_gpio.h>

#if defined(CONFIG_RTL8367_API_8370)
#include "api_8370/rtk_error.h"
#else
#include "api_8367b/rtk_error.h"
#endif

#include "ralink_smi.h"

#define SMI_ACK_RETRY_COUNT	5
#if defined (CONFIG_RALINK_MT7621)
#define SMI_CLK_DELAY_NS	250
#else
#define SMI_CLK_DELAY_NS	1500
#endif

static u32 g_gpio_sda = 1;
static u32 g_gpio_sck = 2;
static const u8  g_addr_slave = 0xB8;
static const u32 g_clk_delay_ns = SMI_CLK_DELAY_NS;

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

void smi_init(u32 gpio_sda, u32 gpio_sck)
{
	g_gpio_sda = gpio_sda;
	g_gpio_sck = gpio_sck;

	/* set GPIO pins to input mode */
	ralink_gpio_set_pin_direction(g_gpio_sda, 0);
	ralink_gpio_set_pin_direction(g_gpio_sck, 0);
}

int smi_read(u32 addr, u32 *data)
{
	u32 rd_lo = 0;
	u32 rd_hi = 0;
	int ret = RT_ERR_FAILED;

	ralink_gpio_lock();

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
	ralink_gpio_unlock();

	return ret;
}

int smi_write(u32 addr, u32 data)
{
	int ret = RT_ERR_FAILED;

	ralink_gpio_lock();

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
	ralink_gpio_unlock();

	return ret;
}

