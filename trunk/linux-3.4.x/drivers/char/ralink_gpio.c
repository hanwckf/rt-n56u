/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright, Ralink Technology, Inc.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 ***************************************************************************
 *
 */
#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <asm/uaccess.h>

#include <linux/ralink_gpio.h>
#include <asm/rt2880/surfboardint.h>

#define RALINK_LED_DEBUG 0

int ralink_gpio_major = 252;

#ifdef CONFIG_RALINK_GPIO_IRQ
static ralink_gpio_irq_info ralink_gpio_irq_data[RALINK_GPIO_NUMBER];
#endif

#ifdef CONFIG_RALINK_GPIO_LED
#define RALINK_GPIO_LED_FREQ (HZ/10)
static struct timer_list ralink_gpio_led_timer;
static ralink_gpio_led_info ralink_gpio_led_data[RALINK_GPIO_NUMBER];

struct ralink_gpio_led_status_t {
	int ticks;
	unsigned int ons;
	unsigned int offs;
	unsigned int resting;
	unsigned int times;
} ralink_gpio_led_stat[RALINK_GPIO_NUMBER];
#endif

/////////////////////////////////////////////////////////////////////////////////

#ifdef CONFIG_RALINK_GPIO_IRQ
static void
ralink_gpio_set_pin_irq_rise(u32 pin, u32 is_enabled)
{
	u32 tmp;

#if defined (RALINK_GPIO_HAS_2722)
	/* RT5350 */
	if (pin <= 21) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIORENA));
		if (is_enabled)
			tmp |=  (1u << pin);
		else
			tmp &= ~(1u << pin);
		*(volatile u32 *)(RALINK_REG_PIORENA) = cpu_to_le32(tmp);
	} else if (pin <= 27) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO2722RENA));
		if (is_enabled)
			tmp |=  (1u << (pin-22));
		else
			tmp &= ~(1u << (pin-22));
		*(volatile u32 *)(RALINK_REG_PIO2722RENA) = cpu_to_le32(tmp);
	}
#elif defined (RALINK_GPIO_HAS_3232)
	/* MT7628 */
	if (pin <= 31) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIORENA));
		if (is_enabled)
			tmp |=  (1u << pin);
		else
			tmp &= ~(1u << pin);
		*(volatile u32 *)(RALINK_REG_PIORENA) = cpu_to_le32(tmp);
	}
#elif defined (RALINK_GPIO_HAS_9532)
	/* MT7621 */
	if (pin <= 31) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIORENA));
		if (is_enabled)
			tmp |=  (1u << pin);
		else
			tmp &= ~(1u << pin);
		*(volatile u32 *)(RALINK_REG_PIORENA) = cpu_to_le32(tmp);
	} else if (pin <= 63) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO6332RENA));
		if (is_enabled)
			tmp |=  (1u << (pin-32));
		else
			tmp &= ~(1u << (pin-32));
		*(volatile u32 *)(RALINK_REG_PIO6332RENA) = cpu_to_le32(tmp);
	} else if (pin <= 95) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO9564RENA));
		if (is_enabled)
			tmp |=  (1u << (pin-64));
		else
			tmp &= ~(1u << (pin-64));
		*(volatile u32 *)(RALINK_REG_PIO9564RENA) = cpu_to_le32(tmp);
	}
#else
	if (pin <= 23) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIORENA));
		if (is_enabled)
			tmp |=  (1u << pin);
		else
			tmp &= ~(1u << pin);
		*(volatile u32 *)(RALINK_REG_PIORENA) = cpu_to_le32(tmp);
	} else if (pin <= 39) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924RENA));
		if (is_enabled)
			tmp |=  (1u << (pin-24));
		else
			tmp &= ~(1u << (pin-24));
		*(volatile u32 *)(RALINK_REG_PIO3924RENA) = cpu_to_le32(tmp);
	}
#if defined (RALINK_GPIO_HAS_4524)
	/* RT3352 */
	else if (pin <= 45) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO4540RENA));
		if (is_enabled)
			tmp |=  (1u << (pin-40));
		else
			tmp &= ~(1u << (pin-40));
		*(volatile u32 *)(RALINK_REG_PIO4540RENA) = cpu_to_le32(tmp);
	}
#elif defined (RALINK_GPIO_HAS_5124)
	/* RT3052 */
	else if (pin <= 51) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140RENA));
		if (is_enabled)
			tmp |=  (1u << (pin-40));
		else
			tmp &= ~(1u << (pin-40));
		*(volatile u32 *)(RALINK_REG_PIO5140RENA) = cpu_to_le32(tmp);
	}
#elif defined (RALINK_GPIO_HAS_7224) || defined (RALINK_GPIO_HAS_9524)
	/* MT7620, RT3883 */
	else if (pin <= 71) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140RENA));
		if (is_enabled)
			tmp |=  (1u << (pin-40));
		else
			tmp &= ~(1u << (pin-40));
		*(volatile u32 *)(RALINK_REG_PIO7140RENA) = cpu_to_le32(tmp);
	}
#if defined (RALINK_GPIO_HAS_7224)
	/* MT7620 */
	else if (pin <= 72) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO72RENA));
		if (is_enabled)
			tmp |=  (1u << (pin-72));
		else
			tmp &= ~(1u << (pin-72));
		*(volatile u32 *)(RALINK_REG_PIO72RENA) = cpu_to_le32(tmp);
#else
	/* RT3883 */
	else if (pin <= 95) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO9572RENA));
		if (is_enabled)
			tmp |=  (1u << (pin-72));
		else
			tmp &= ~(1u << (pin-72));
		*(volatile u32 *)(RALINK_REG_PIO9572RENA) = cpu_to_le32(tmp);
#endif
	}
#endif
#endif
}

static void
ralink_gpio_set_pin_irq_fall(u32 pin, u32 is_enabled)
{
	u32 tmp;

#if defined (RALINK_GPIO_HAS_2722)
	/* RT5350 */
	if (pin <= 21) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIOFENA));
		if (is_enabled)
			tmp |=  (1u << pin);
		else
			tmp &= ~(1u << pin);
		*(volatile u32 *)(RALINK_REG_PIOFENA) = cpu_to_le32(tmp);
	} else if (pin <= 27) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO2722FENA));
		if (is_enabled)
			tmp |=  (1u << (pin-22));
		else
			tmp &= ~(1u << (pin-22));
		*(volatile u32 *)(RALINK_REG_PIO2722FENA) = cpu_to_le32(tmp);
	}
#elif defined (RALINK_GPIO_HAS_3232)
	/* MT7628 */
	if (pin <= 31) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIOFENA));
		if (is_enabled)
			tmp |=  (1u << pin);
		else
			tmp &= ~(1u << pin);
		*(volatile u32 *)(RALINK_REG_PIOFENA) = cpu_to_le32(tmp);
	}
#elif defined (RALINK_GPIO_HAS_9532)
	/* MT7621 */
	if (pin <= 31) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIOFENA));
		if (is_enabled)
			tmp |=  (1u << pin);
		else
			tmp &= ~(1u << pin);
		*(volatile u32 *)(RALINK_REG_PIOFENA) = cpu_to_le32(tmp);
	} else if (pin <= 63) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO6332FENA));
		if (is_enabled)
			tmp |=  (1u << (pin-32));
		else
			tmp &= ~(1u << (pin-32));
		*(volatile u32 *)(RALINK_REG_PIO6332FENA) = cpu_to_le32(tmp);
	} else if (pin <= 95) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO9564FENA));
		if (is_enabled)
			tmp |=  (1u << (pin-64));
		else
			tmp &= ~(1u << (pin-64));
		*(volatile u32 *)(RALINK_REG_PIO9564FENA) = cpu_to_le32(tmp);
	}
#else
	if (pin <= 23) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIOFENA));
		if (is_enabled)
			tmp |=  (1u << pin);
		else
			tmp &= ~(1u << pin);
		*(volatile u32 *)(RALINK_REG_PIOFENA) = cpu_to_le32(tmp);
	} else if (pin <= 39) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924FENA));
		if (is_enabled)
			tmp |=  (1u << (pin-24));
		else
			tmp &= ~(1u << (pin-24));
		*(volatile u32 *)(RALINK_REG_PIO3924FENA) = cpu_to_le32(tmp);
	}
#if defined (RALINK_GPIO_HAS_4524)
	/* RT3352 */
	else if (pin <= 45) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO4540FENA));
		if (is_enabled)
			tmp |=  (1u << (pin-40));
		else
			tmp &= ~(1u << (pin-40));
		*(volatile u32 *)(RALINK_REG_PIO4540FENA) = cpu_to_le32(tmp);
	}
#elif defined (RALINK_GPIO_HAS_5124)
	/* RT3052 */
	else if (pin <= 51) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140FENA));
		if (is_enabled)
			tmp |=  (1u << (pin-40));
		else
			tmp &= ~(1u << (pin-40));
		*(volatile u32 *)(RALINK_REG_PIO5140FENA) = cpu_to_le32(tmp);
	}
#elif defined (RALINK_GPIO_HAS_7224) || defined (RALINK_GPIO_HAS_9524)
	/* MT7620, RT3883 */
	else if (pin <= 71) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140FENA));
		if (is_enabled)
			tmp |=  (1u << (pin-40));
		else
			tmp &= ~(1u << (pin-40));
		*(volatile u32 *)(RALINK_REG_PIO7140FENA) = cpu_to_le32(tmp);
	}
#if defined (RALINK_GPIO_HAS_7224)
	/* MT7620 */
	else if (pin <= 72) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO72FENA));
		if (is_enabled)
			tmp |=  (1u << (pin-72));
		else
			tmp &= ~(1u << (pin-72));
		*(volatile u32 *)(RALINK_REG_PIO72FENA) = cpu_to_le32(tmp);
#else
	/* RT3883 */
	else if (pin <= 95) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO9572FENA));
		if (is_enabled)
			tmp |=  (1u << (pin-72));
		else
			tmp &= ~(1u << (pin-72));
		*(volatile u32 *)(RALINK_REG_PIO9572FENA) = cpu_to_le32(tmp);
#endif
	}
#endif
#endif
}
#endif

/////////////////////////////////////////////////////////////////////////////////

void ralink_gpio_set_pin_direction(u32 pin, u32 is_output)
{
	u32 tmp;

#if defined (RALINK_GPIO_HAS_2722)
	/* RT5350 */
	if (pin <= 21) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODIR));
		if (is_output)
			tmp |=  (1u << pin);
		else
			tmp &= ~(1u << pin);
		*(volatile u32 *)(RALINK_REG_PIODIR) = cpu_to_le32(tmp);
	} else if (pin <= 27) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO2722DIR));
		if (is_output)
			tmp |=  (1u << (pin-22));
		else
			tmp &= ~(1u << (pin-22));
		*(volatile u32 *)(RALINK_REG_PIO2722DIR) = cpu_to_le32(tmp);
	}
#elif defined (RALINK_GPIO_HAS_3232)
	/* MT7628 */
	if (pin <= 31) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODIR));
		if (is_output)
			tmp |=  (1u << pin);
		else
			tmp &= ~(1u << pin);
		*(volatile u32 *)(RALINK_REG_PIODIR) = cpu_to_le32(tmp);
	}
#elif defined (RALINK_GPIO_HAS_9532)
	/* MT7621 */
	if (pin <= 31) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODIR));
		if (is_output)
			tmp |=  (1u << pin);
		else
			tmp &= ~(1u << pin);
		*(volatile u32 *)(RALINK_REG_PIODIR) = cpu_to_le32(tmp);
	} else if (pin <= 63) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO6332DIR));
		if (is_output)
			tmp |=  (1u << (pin-32));
		else
			tmp &= ~(1u << (pin-32));
		*(volatile u32 *)(RALINK_REG_PIO6332DIR) = cpu_to_le32(tmp);
	} else if (pin <= 95) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO9564DIR));
		if (is_output)
			tmp |=  (1u << (pin-64));
		else
			tmp &= ~(1u << (pin-64));
		*(volatile u32 *)(RALINK_REG_PIO9564DIR) = cpu_to_le32(tmp);
	}
#else
	if (pin <= 23) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODIR));
		if (is_output)
			tmp |=  (1u << pin);
		else
			tmp &= ~(1u << pin);
		*(volatile u32 *)(RALINK_REG_PIODIR) = cpu_to_le32(tmp);
	} else if (pin <= 39) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DIR));
		if (is_output)
			tmp |=  (1u << (pin-24));
		else
			tmp &= ~(1u << (pin-24));
		*(volatile u32 *)(RALINK_REG_PIO3924DIR) = cpu_to_le32(tmp);
	}
#if defined (RALINK_GPIO_HAS_4524)
	/* RT3352 */
	else if (pin <= 45) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO4540DIR));
		if (is_output)
			tmp |=  (1u << (pin-40));
		else
			tmp &= ~(1u << (pin-40));
		*(volatile u32 *)(RALINK_REG_PIO4540DIR) = cpu_to_le32(tmp);
	}
#elif defined (RALINK_GPIO_HAS_5124)
	/* RT3052 */
	else if (pin <= 51) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DIR));
		if (is_output)
			tmp |=  (1u << (pin-40));
		else
			tmp &= ~(1u << (pin-40));
		*(volatile u32 *)(RALINK_REG_PIO5140DIR) = cpu_to_le32(tmp);
	}
#elif defined (RALINK_GPIO_HAS_7224) || defined (RALINK_GPIO_HAS_9524)
	/* MT7620, RT3883 */
	else if (pin <= 71) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140DIR));
		if (is_output)
			tmp |=  (1u << (pin-40));
		else
			tmp &= ~(1u << (pin-40));
		*(volatile u32 *)(RALINK_REG_PIO7140DIR) = cpu_to_le32(tmp);
	}
#if defined (RALINK_GPIO_HAS_7224)
	/* MT7620 */
	else if (pin <= 72) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO72DIR));
		if (is_output)
			tmp |=  (1u << (pin-72));
		else
			tmp &= ~(1u << (pin-72));
		*(volatile u32 *)(RALINK_REG_PIO72DIR) = cpu_to_le32(tmp);
#else
	/* RT3883 */
	else if (pin <= 95) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO9572DIR));
		if (is_output)
			tmp |=  (1u << (pin-72));
		else
			tmp &= ~(1u << (pin-72));
		*(volatile u32 *)(RALINK_REG_PIO9572DIR) = cpu_to_le32(tmp);
#endif
	}
#endif
#endif
}
EXPORT_SYMBOL(ralink_gpio_set_pin_direction);

/////////////////////////////////////////////////////////////////////////////////

void ralink_gpio_set_pin_value(u32 pin, u32 value)
{
	u32 tmp;

#if defined (RALINK_GPIO_HAS_2722)
	/* RT5350 */
	if (pin <= 21) {
		tmp = (1u << pin);
		if (value)
			*(volatile u32 *)(RALINK_REG_PIOSET) = cpu_to_le32(tmp);
		else
			*(volatile u32 *)(RALINK_REG_PIORESET) = cpu_to_le32(tmp);
	} else if (pin <= 27) {
		tmp = (1u << (pin-22));
		if (value)
			*(volatile u32 *)(RALINK_REG_PIO2722SET) = cpu_to_le32(tmp);
		else
			*(volatile u32 *)(RALINK_REG_PIO2722RESET) = cpu_to_le32(tmp);
	}
#elif defined (RALINK_GPIO_HAS_3232)
	/* MT7628 */
	if (pin <= 31) {
		tmp = (1u << pin);
		if (value)
			*(volatile u32 *)(RALINK_REG_PIOSET) = cpu_to_le32(tmp);
		else
			*(volatile u32 *)(RALINK_REG_PIORESET) = cpu_to_le32(tmp);
	}
#elif defined (RALINK_GPIO_HAS_9532)
	/* MT7621 */
	if (pin <= 31) {
		tmp = (1u << pin);
		if (value)
			*(volatile u32 *)(RALINK_REG_PIOSET) = cpu_to_le32(tmp);
		else
			*(volatile u32 *)(RALINK_REG_PIORESET) = cpu_to_le32(tmp);
	} else if (pin <= 63) {
		tmp = (1u << (pin-32));
		if (value)
			*(volatile u32 *)(RALINK_REG_PIO6332SET) = cpu_to_le32(tmp);
		else
			*(volatile u32 *)(RALINK_REG_PIO6332RESET) = cpu_to_le32(tmp);
	} else if (pin <= 95) {
		tmp = (1u << (pin-64));
		if (value)
			*(volatile u32 *)(RALINK_REG_PIO9564SET) = cpu_to_le32(tmp);
		else
			*(volatile u32 *)(RALINK_REG_PIO9564RESET) = cpu_to_le32(tmp);
	}
#else
	if (pin <= 23) {
		tmp = (1u << pin);
		if (value)
			*(volatile u32 *)(RALINK_REG_PIOSET) = cpu_to_le32(tmp);
		else
			*(volatile u32 *)(RALINK_REG_PIORESET) = cpu_to_le32(tmp);
	} else if (pin <= 39) {
		tmp = (1u << (pin-24));
		if (value)
			*(volatile u32 *)(RALINK_REG_PIO3924SET) = cpu_to_le32(tmp);
		else
			*(volatile u32 *)(RALINK_REG_PIO3924RESET) = cpu_to_le32(tmp);
	}
#if defined (RALINK_GPIO_HAS_4524)
	/* RT3352 */
	else if (pin <= 45) {
		tmp = (1u << (pin-40));
		if (value)
			*(volatile u32 *)(RALINK_REG_PIO4540SET) = cpu_to_le32(tmp);
		else
			*(volatile u32 *)(RALINK_REG_PIO4540RESET) = cpu_to_le32(tmp);
	}
#elif defined (RALINK_GPIO_HAS_5124)
	/* RT3052 */
	else if (pin <= 51) {
		tmp = (1u << (pin-40));
		if (value)
			*(volatile u32 *)(RALINK_REG_PIO5140SET) = cpu_to_le32(tmp);
		else
			*(volatile u32 *)(RALINK_REG_PIO5140RESET) = cpu_to_le32(tmp);
	}
#elif defined (RALINK_GPIO_HAS_7224) || defined (RALINK_GPIO_HAS_9524)
	/* MT7620, RT3883 */
	else if (pin <= 71) {
		tmp = (1u << (pin-40));
		if (value)
			*(volatile u32 *)(RALINK_REG_PIO7140SET) = cpu_to_le32(tmp);
		else
			*(volatile u32 *)(RALINK_REG_PIO7140RESET) = cpu_to_le32(tmp);
	}
#if defined (RALINK_GPIO_HAS_7224)
	/* MT7620 */
	else if (pin <= 72) {
		tmp = (1u << (pin-72));
		if (value)
			*(volatile u32 *)(RALINK_REG_PIO72SET) = cpu_to_le32(tmp);
		else
			*(volatile u32 *)(RALINK_REG_PIO72RESET) = cpu_to_le32(tmp);
#else
	/* RT3883 */
	else if (pin <= 95) {
		tmp = (1u << (pin-72));
		if (value)
			*(volatile u32 *)(RALINK_REG_PIO9572SET) = cpu_to_le32(tmp);
		else
			*(volatile u32 *)(RALINK_REG_PIO9572RESET) = cpu_to_le32(tmp);
#endif
	}
#endif
#endif
}
EXPORT_SYMBOL(ralink_gpio_set_pin_value);

/////////////////////////////////////////////////////////////////////////////////

u32 ralink_gpio_get_pin_value(u32 pin)
{
	u32 tmp = 0;

#if defined (RALINK_GPIO_HAS_2722)
	/* RT5350 */
	if (pin <= 21) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));
		tmp = (tmp >> pin) & 1u;
	} else if (pin <= 27) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO2722DATA));
		tmp = (tmp >> (pin-22)) & 1u;
	}
#elif defined (RALINK_GPIO_HAS_3232)
	/* MT7628 */
	if (pin <= 31) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));
		tmp = (tmp >> pin) & 1u;
	}
#elif defined (RALINK_GPIO_HAS_9532)
	/* MT7621 */
	if (pin <= 31) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));
		tmp = (tmp >> pin) & 1u;
	} else if (pin <= 63) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO6332DATA));
		tmp = (tmp >> (pin-32)) & 1u;
	} else if (pin <= 95) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO9564DATA));
		tmp = (tmp >> (pin-64)) & 1u;
	}
#else
	if (pin <= 23) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));
		tmp = (tmp >> pin) & 1u;
	} else if (pin <= 39) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DATA));
		tmp = (tmp >> (pin-24)) & 1u;
	}
#if defined (RALINK_GPIO_HAS_4524)
	/* RT3352 */
	else if (pin <= 45) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO4540DATA));
		tmp = (tmp >> (pin-40)) & 1u;
	}
#elif defined (RALINK_GPIO_HAS_5124)
	/* RT3052 */
	else if (pin <= 51) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DATA));
		tmp = (tmp >> (pin-40)) & 1u;
	}
#elif defined (RALINK_GPIO_HAS_7224) || defined (RALINK_GPIO_HAS_9524)
	/* MT7620, RT3883 */
	else if (pin <= 71) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140DATA));
		tmp = (tmp >> (pin-40)) & 1u;
	}
#if defined (RALINK_GPIO_HAS_7224)
	/* MT7620 */
	else if (pin <= 72) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO72DATA));
		tmp = (tmp >> (pin-72)) & 1u;
#else
	/* RT3883 */
	else if (pin <= 95) {
		tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO9572DATA));
		tmp = (tmp >> (pin-72)) & 1u;
#endif
	}
#endif
#endif

	return tmp;
}
EXPORT_SYMBOL(ralink_gpio_get_pin_value);

/////////////////////////////////////////////////////////////////////////////////

void ralink_gpio_mode_set_bit(u32 idx, u32 value)
{
	u32 tmp;

	if (idx > 31) idx = 31;

	tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_GPIOMODE));
	if (value)
		tmp |=  (1u << idx);
	else
		tmp &= ~(1u << idx);
	*(volatile u32 *)(RALINK_REG_GPIOMODE) = cpu_to_le32(tmp);
}
EXPORT_SYMBOL(ralink_gpio_mode_set_bit);

/////////////////////////////////////////////////////////////////////////////////

u32 ralink_gpio_mode_get_bit(u32 idx)
{
	u32 tmp = 0;

	if (idx > 31) idx = 31;

	tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_GPIOMODE));
	tmp = (tmp >> idx) & 1u;

	return tmp;
}
EXPORT_SYMBOL(ralink_gpio_mode_get_bit);

/////////////////////////////////////////////////////////////////////////////////

void ralink_gpio_mode_set(u32 value)
{
	*(volatile u32 *)(RALINK_REG_GPIOMODE) = cpu_to_le32(value);
}
EXPORT_SYMBOL(ralink_gpio_mode_set);

/////////////////////////////////////////////////////////////////////////////////

u32 ralink_gpio_mode_get(void)
{
	return le32_to_cpu(*(volatile u32 *)(RALINK_REG_GPIOMODE));
}
EXPORT_SYMBOL(ralink_gpio_mode_get);

/////////////////////////////////////////////////////////////////////////////////

#ifdef CONFIG_RALINK_GPIO_LED
int ralink_gpio_led_blink(int led_gpio)
{
	if (led_gpio < 0 || led_gpio >= RALINK_GPIO_NUMBER)
		return -1;

	if (ralink_gpio_led_data[led_gpio].gpio >= 0 &&
	   !ralink_gpio_led_stat[led_gpio].times)
		return 0;

	// clear previous led status
	memset(&ralink_gpio_led_stat[led_gpio], 0, sizeof(struct ralink_gpio_led_status_t));

	ralink_gpio_led_data[led_gpio].on = 1;
	ralink_gpio_led_data[led_gpio].off = 1;
	ralink_gpio_led_data[led_gpio].blinks = 1;
	ralink_gpio_led_data[led_gpio].rests = 1;
	ralink_gpio_led_data[led_gpio].times = 1;
	ralink_gpio_led_data[led_gpio].gpio = led_gpio;

	return 0;

}
EXPORT_SYMBOL(ralink_gpio_led_blink);

static int
ralink_gpio_led_set(ralink_gpio_led_info *led)
{
	if (0 <= led->gpio && led->gpio < RALINK_GPIO_NUMBER) {
		if (led->on > RALINK_GPIO_LED_INFINITY)
			led->on = RALINK_GPIO_LED_INFINITY;
		if (led->off > RALINK_GPIO_LED_INFINITY)
			led->off = RALINK_GPIO_LED_INFINITY;
		if (led->blinks > RALINK_GPIO_LED_INFINITY)
			led->blinks = RALINK_GPIO_LED_INFINITY;
		if (led->rests > RALINK_GPIO_LED_INFINITY)
			led->rests = RALINK_GPIO_LED_INFINITY;
		if (led->times > RALINK_GPIO_LED_INFINITY)
			led->times = RALINK_GPIO_LED_INFINITY;
		if (led->on == 0 && led->off == 0 && led->blinks == 0 &&
				led->rests == 0) {
			ralink_gpio_led_data[led->gpio].gpio = -1; //stop it
			return 0;
		}
		
		// set gpio direction to 'out'
		ralink_gpio_set_pin_direction(led->gpio, RALINK_GPIO_DIR_OUT);
		
		// clear previous led status
		memset(&ralink_gpio_led_stat[led->gpio], 0, sizeof(struct ralink_gpio_led_status_t));
		
		// register led data
		ralink_gpio_led_data[led->gpio].on = (led->on == 0)? 1 : led->on;
		ralink_gpio_led_data[led->gpio].off = (led->off == 0)? 1 : led->off;
		ralink_gpio_led_data[led->gpio].blinks = (led->blinks == 0)? 1 : led->blinks;
		ralink_gpio_led_data[led->gpio].rests = (led->rests == 0)? 1 : led->rests;
		ralink_gpio_led_data[led->gpio].times = (led->times == 0)? 1 : led->times;
		ralink_gpio_led_data[led->gpio].gpio = led->gpio;
		
#if RALINK_LED_DEBUG
		printk("led=%d, on=%d, off=%d, blinks,=%d, reset=%d, time=%d\n",
				ralink_gpio_led_data[led->gpio].gpio,
				ralink_gpio_led_data[led->gpio].on,
				ralink_gpio_led_data[led->gpio].off,
				ralink_gpio_led_data[led->gpio].blinks,
				ralink_gpio_led_data[led->gpio].rests,
				ralink_gpio_led_data[led->gpio].times);
#endif
	}
	else {
		printk(KERN_ERR NAME ": gpio(%d) out of range\n", led->gpio);
		return -1;
	}
	return 0;
}

#if RALINK_GPIO_LED_LOW_ACT
#define __LED_ON(gpio,set,clr)  clr |= RALINK_GPIO(gpio);
#define __LED_OFF(gpio,set,clr) set |= RALINK_GPIO(gpio);
#else
#define __LED_ON(gpio,set,clr)  set |= RALINK_GPIO(gpio);
#define __LED_OFF(gpio,set,clr) clr |= RALINK_GPIO(gpio);
#endif

static void
control_gpio_led(int i, int offset, u32 *led_set, u32 *led_clr)
{
	int gpio_bit = i - offset;
	unsigned int x;

	if (gpio_bit < 0 || gpio_bit > 31)
		return;

	 // always on
	if (ralink_gpio_led_data[i].on == RALINK_GPIO_LED_INFINITY ||
	    ralink_gpio_led_data[i].off == 0) {
		__LED_ON(gpio_bit, (*led_set), (*led_clr));
		return;
	}

	 // always off
	if (ralink_gpio_led_data[i].off == RALINK_GPIO_LED_INFINITY ||
	    ralink_gpio_led_data[i].rests == RALINK_GPIO_LED_INFINITY ||
	    ralink_gpio_led_data[i].on == 0 ||
	    ralink_gpio_led_data[i].blinks == 0 ||
	    ralink_gpio_led_data[i].times == 0) {
		__LED_OFF(gpio_bit, (*led_set), (*led_clr));
		return;
	}

	// led turn on or off

	// always blinking
	if (ralink_gpio_led_data[i].blinks == RALINK_GPIO_LED_INFINITY ||
	    ralink_gpio_led_data[i].rests == 0) {
		x = ralink_gpio_led_stat[i].ticks % (ralink_gpio_led_data[i].on + ralink_gpio_led_data[i].off);
	}
	else {
		unsigned int a, b, c, d, o, t;
		a = ralink_gpio_led_data[i].blinks / 2;
		b = ralink_gpio_led_data[i].rests / 2;
		c = ralink_gpio_led_data[i].blinks % 2;
		d = ralink_gpio_led_data[i].rests % 2;
		o = ralink_gpio_led_data[i].on + ralink_gpio_led_data[i].off;
		//t = blinking ticks
		t = a * o + ralink_gpio_led_data[i].on * c;
		//x = ticks % (blinking ticks + resting ticks)
		x = ralink_gpio_led_stat[i].ticks % (t + b * o + ralink_gpio_led_data[i].on * d);
		//starts from 0 at resting cycles
		if (x >= t)
			x -= t;
		x %= o;
	}

	if (x < ralink_gpio_led_data[i].on) {
		__LED_ON(gpio_bit, (*led_set), (*led_clr));
		if (ralink_gpio_led_stat[i].ticks && x == 0)
			ralink_gpio_led_stat[i].offs++;
#if RALINK_LED_DEBUG
		printk("t%d gpio%d on,", ralink_gpio_led_stat[i].ticks, i);
#endif
	}
	else {
		__LED_OFF(gpio_bit, (*led_set), (*led_clr));
		if (x == ralink_gpio_led_data[i].on)
			ralink_gpio_led_stat[i].ons++;
#if RALINK_LED_DEBUG
		printk("t%d gpio%d off,", ralink_gpio_led_stat[i].ticks, i);
#endif
	}

	ralink_gpio_led_stat[i].ticks++;

	// blinking or resting

	// always blinking
	if (ralink_gpio_led_data[i].blinks == RALINK_GPIO_LED_INFINITY ||
	    ralink_gpio_led_data[i].rests == 0) {
		return;
	}
	else {
		x = ralink_gpio_led_stat[i].ons + ralink_gpio_led_stat[i].offs;
		if (!ralink_gpio_led_stat[i].resting) {
			if (x == ralink_gpio_led_data[i].blinks) {
				ralink_gpio_led_stat[i].resting = 1;
				ralink_gpio_led_stat[i].ons = 0;
				ralink_gpio_led_stat[i].offs = 0;
				ralink_gpio_led_stat[i].times++;
			}
		}
		else {
			if (x == ralink_gpio_led_data[i].rests) {
				ralink_gpio_led_stat[i].resting = 0;
				ralink_gpio_led_stat[i].ons = 0;
				ralink_gpio_led_stat[i].offs = 0;
			}
		}
	}

	if (ralink_gpio_led_stat[i].resting) {
		__LED_OFF(gpio_bit, (*led_set), (*led_clr));
#if RALINK_LED_DEBUG
		printk("resting,");
	} else {
		printk("blinking,");
#endif
	}

	// number of times
	if (ralink_gpio_led_data[i].times != RALINK_GPIO_LED_INFINITY) {
		if (ralink_gpio_led_stat[i].times == ralink_gpio_led_data[i].times) {
			__LED_OFF(gpio_bit, (*led_set), (*led_clr));
			ralink_gpio_led_data[i].gpio = -1; //stop
		}
#if RALINK_LED_DEBUG
		printk("T%d\n", ralink_gpio_led_stat[i].times);
	} else {
		printk("T@\n");
#endif
	}
}

static void
ralink_gpio_led_do_timer(unsigned long unused)
{
	int i;

	u32 ra_gpio_led_set = 0;
	u32 ra_gpio_led_clr = 0;
#if defined (RALINK_GPIO_HAS_2722)
	u32 ra_gpio2722_led_set = 0;
	u32 ra_gpio2722_led_clr = 0;
#elif defined (RALINK_GPIO_HAS_4524)
	u32 ra_gpio3924_led_set = 0;
	u32 ra_gpio3924_led_clr = 0;
	u32 ra_gpio4540_led_set = 0;
	u32 ra_gpio4540_led_clr = 0;
#elif defined (RALINK_GPIO_HAS_5124)
	u32 ra_gpio3924_led_set = 0;
	u32 ra_gpio3924_led_clr = 0;
	u32 ra_gpio5140_led_set = 0;
	u32 ra_gpio5140_led_clr = 0;
#elif defined (RALINK_GPIO_HAS_9524) || defined (RALINK_GPIO_HAS_7224)
	u32 ra_gpio3924_led_set = 0;
	u32 ra_gpio3924_led_clr = 0;
	u32 ra_gpio7140_led_set = 0;
	u32 ra_gpio7140_led_clr = 0;
#if defined (RALINK_GPIO_HAS_7224)
	u32 ra_gpio72_led_set = 0;
	u32 ra_gpio72_led_clr = 0;
#else
	u32 ra_gpio9572_led_set = 0;
	u32 ra_gpio9572_led_clr = 0;
#endif
#elif defined (RALINK_GPIO_HAS_9532)
	u32 ra_gpio6332_led_set = 0;
	u32 ra_gpio6332_led_clr = 0;
	u32 ra_gpio9564_led_set = 0;
	u32 ra_gpio9564_led_clr = 0;
#endif

#if defined (RALINK_GPIO_HAS_2722)
	for (i = 0; i < 22; i++) {
		if (ralink_gpio_led_data[i].gpio < 0)
			continue;
		control_gpio_led(i, 0, &ra_gpio_led_set, &ra_gpio_led_clr);
	}
	for (i = 22; i < RALINK_GPIO_NUMBER; i++) {
		if (ralink_gpio_led_data[i].gpio < 0)
			continue;
		control_gpio_led(i, 22, &ra_gpio2722_led_set, &ra_gpio2722_led_clr);
	}
#elif defined (RALINK_GPIO_HAS_3232)
	for (i = 0; i < 32; i++) {
		if (ralink_gpio_led_data[i].gpio < 0)
			continue;
		control_gpio_led(i, 0, &ra_gpio_led_set, &ra_gpio_led_clr);
	}
#elif defined (RALINK_GPIO_HAS_9532)
	for (i = 0; i < 32; i++) {
		if (ralink_gpio_led_data[i].gpio < 0)
			continue;
		control_gpio_led(i, 0, &ra_gpio_led_set, &ra_gpio_led_clr);
	}
	for (i = 32; i < 64; i++) {
		if (ralink_gpio_led_data[i].gpio < 0)
			continue;
		control_gpio_led(i, 32, &ra_gpio6332_led_set, &ra_gpio6332_led_clr);
	}
	for (i = 64; i < RALINK_GPIO_NUMBER; i++) {
		if (ralink_gpio_led_data[i].gpio < 0)
			continue;
		control_gpio_led(i, 64, &ra_gpio9564_led_set, &ra_gpio9564_led_clr);
	}
#else
	for (i = 0; i < 24; i++) {
		if (ralink_gpio_led_data[i].gpio < 0)
			continue;
		control_gpio_led(i, 0, &ra_gpio_led_set, &ra_gpio_led_clr);
	}
#if defined (RALINK_GPIO_HAS_4524)
	for (i = 24; i < 40; i++) {
		if (ralink_gpio_led_data[i].gpio < 0)
			continue;
		control_gpio_led(i, 24, &ra_gpio3924_led_set, &ra_gpio3924_led_clr);
	}
	for (i = 40; i < RALINK_GPIO_NUMBER; i++) {
		if (ralink_gpio_led_data[i].gpio < 0)
			continue;
		control_gpio_led(i, 40, &ra_gpio4540_led_set, &ra_gpio4540_led_clr);
	}
#elif defined (RALINK_GPIO_HAS_5124)
	for (i = 24; i < 40; i++) {
		if (ralink_gpio_led_data[i].gpio < 0)
			continue;
		control_gpio_led(i, 24, &ra_gpio3924_led_set, &ra_gpio3924_led_clr);
	}
	for (i = 40; i < RALINK_GPIO_NUMBER; i++) {
		if (ralink_gpio_led_data[i].gpio < 0)
			continue;
		control_gpio_led(i, 40, &ra_gpio5140_led_set, &ra_gpio5140_led_clr);
	}
#elif defined (RALINK_GPIO_HAS_9524) || defined (RALINK_GPIO_HAS_7224)
	for (i = 24; i < 40; i++) {
		if (ralink_gpio_led_data[i].gpio < 0)
			continue;
		control_gpio_led(i, 24, &ra_gpio3924_led_set, &ra_gpio3924_led_clr);
	}
	for (i = 40; i < 72; i++) {
		if (ralink_gpio_led_data[i].gpio < 0)
			continue;
		control_gpio_led(i, 40, &ra_gpio7140_led_set, &ra_gpio7140_led_clr);
	}
	for (i = 72; i < RALINK_GPIO_NUMBER; i++) {
		if (ralink_gpio_led_data[i].gpio < 0)
			continue;
#if defined (RALINK_GPIO_HAS_7224)
		control_gpio_led(i, 72, &ra_gpio72_led_set, &ra_gpio72_led_clr);
#else
		control_gpio_led(i, 72, &ra_gpio9572_led_set, &ra_gpio9572_led_clr);
#endif
	}
#endif
#endif

	if (ra_gpio_led_clr)
		*(volatile u32 *)(RALINK_REG_PIORESET) = ra_gpio_led_clr;
	if (ra_gpio_led_set)
		*(volatile u32 *)(RALINK_REG_PIOSET) = ra_gpio_led_set;
#if defined (RALINK_GPIO_HAS_2722)
	if (ra_gpio2722_led_clr)
		*(volatile u32 *)(RALINK_REG_PIO2722RESET) = ra_gpio2722_led_clr;
	if (ra_gpio2722_led_set)
		*(volatile u32 *)(RALINK_REG_PIO2722SET) = ra_gpio2722_led_set;
#elif defined (RALINK_GPIO_HAS_4524)
	if (ra_gpio3924_led_clr)
		*(volatile u32 *)(RALINK_REG_PIO3924RESET) = ra_gpio3924_led_clr;
	if (ra_gpio3924_led_set)
		*(volatile u32 *)(RALINK_REG_PIO3924SET) = ra_gpio3924_led_set;
	if (ra_gpio4540_led_clr)
		*(volatile u32 *)(RALINK_REG_PIO4540RESET) = ra_gpio4540_led_clr;
	if (ra_gpio4540_led_set)
		*(volatile u32 *)(RALINK_REG_PIO4540SET) = ra_gpio4540_led_set;
#elif defined (RALINK_GPIO_HAS_5124)
	if (ra_gpio3924_led_clr)
		*(volatile u32 *)(RALINK_REG_PIO3924RESET) = ra_gpio3924_led_clr;
	if (ra_gpio3924_led_set)
		*(volatile u32 *)(RALINK_REG_PIO3924SET) = ra_gpio3924_led_set;
	if (ra_gpio5140_led_clr)
		*(volatile u32 *)(RALINK_REG_PIO5140RESET) = ra_gpio5140_led_clr;
	if (ra_gpio5140_led_set)
		*(volatile u32 *)(RALINK_REG_PIO5140SET) = ra_gpio5140_led_set;
#elif defined (RALINK_GPIO_HAS_9524) || defined (RALINK_GPIO_HAS_7224)
	if (ra_gpio3924_led_clr)
		*(volatile u32 *)(RALINK_REG_PIO3924RESET) = ra_gpio3924_led_clr;
	if (ra_gpio3924_led_set)
		*(volatile u32 *)(RALINK_REG_PIO3924SET) = ra_gpio3924_led_set;
	if (ra_gpio7140_led_clr)
		*(volatile u32 *)(RALINK_REG_PIO7140RESET) = ra_gpio7140_led_clr;
	if (ra_gpio7140_led_set)
		*(volatile u32 *)(RALINK_REG_PIO7140SET) = ra_gpio7140_led_set;
#if defined (RALINK_GPIO_HAS_7224)
	if (ra_gpio72_led_clr)
		*(volatile u32 *)(RALINK_REG_PIO72RESET) = ra_gpio72_led_clr;
	if (ra_gpio72_led_set)
		*(volatile u32 *)(RALINK_REG_PIO72SET) = ra_gpio72_led_set;
#else
	if (ra_gpio9572_led_clr)
		*(volatile u32 *)(RALINK_REG_PIO9572RESET) = ra_gpio9572_led_clr;
	if (ra_gpio9572_led_set)
		*(volatile u32 *)(RALINK_REG_PIO9572SET) = ra_gpio9572_led_set;
#endif
#elif defined (RALINK_GPIO_HAS_9532)
	if (ra_gpio6332_led_clr)
		*(volatile u32 *)(RALINK_REG_PIO6332RESET) = ra_gpio6332_led_clr;
	if (ra_gpio6332_led_set)
		*(volatile u32 *)(RALINK_REG_PIO6332SET) = ra_gpio6332_led_set;
	if (ra_gpio9564_led_clr)
		*(volatile u32 *)(RALINK_REG_PIO9564RESET) = ra_gpio9564_led_clr;
	if (ra_gpio9564_led_set)
		*(volatile u32 *)(RALINK_REG_PIO9564SET) = ra_gpio9564_led_set;
#endif

	mod_timer(&ralink_gpio_led_timer, jiffies + RALINK_GPIO_LED_FREQ);
}

void ralink_gpio_led_add_timer(void)
{
	int i;

	for (i = 0; i < RALINK_GPIO_NUMBER; i++)
		ralink_gpio_led_data[i].gpio = -1; //-1 means unused

	init_timer(&ralink_gpio_led_timer);
	ralink_gpio_led_timer.function = ralink_gpio_led_do_timer;
	ralink_gpio_led_timer.expires = jiffies + RALINK_GPIO_LED_FREQ;
	add_timer(&ralink_gpio_led_timer);
}

void ralink_gpio_led_del_timer(void)
{
	del_timer_sync(&ralink_gpio_led_timer);
}
#endif

#ifdef CONFIG_RALINK_GPIO_IRQ
static int
ralink_gpio_int_set(ralink_gpio_irq_info *info)
{
	if (info->gpio < 0 || info->gpio >= RALINK_GPIO_NUMBER) {
		printk(KERN_ERR NAME ": irq pin number (%d) out of range\n", info->gpio);
		return -1;
	}

	ralink_gpio_irq_data[info->gpio].rise = info->rise;
	ralink_gpio_irq_data[info->gpio].fall = info->fall;
	ralink_gpio_irq_data[info->gpio].pid  = info->pid;

	ralink_gpio_set_pin_irq_rise(info->gpio, info->rise | info->fall);
	ralink_gpio_set_pin_irq_fall(info->gpio, info->rise | info->fall);

	return 0;
}

static void
ralink_gpio_irq_clear(void)
{
#if defined (RALINK_GPIO_HAS_2722)
	*(volatile u32 *)(RALINK_REG_PIOINT)     = cpu_to_le32(0x003FFFFF);
	*(volatile u32 *)(RALINK_REG_PIO2722INT) = cpu_to_le32(0x0000003F);
#elif defined (RALINK_GPIO_HAS_3232)
	*(volatile u32 *)(RALINK_REG_PIOINT)     = cpu_to_le32(0xFFFFFFFF);
#elif defined (RALINK_GPIO_HAS_9532)
	*(volatile u32 *)(RALINK_REG_PIOINT)     = cpu_to_le32(0xFFFFFFFF);
	*(volatile u32 *)(RALINK_REG_PIO6332INT) = cpu_to_le32(0xFFFFFFFF);
	*(volatile u32 *)(RALINK_REG_PIO9564INT) = cpu_to_le32(0xFFFFFFFF);
#else
	*(volatile u32 *)(RALINK_REG_PIOINT)     = cpu_to_le32(0x00FFFFFF);
#if defined (RALINK_GPIO_HAS_4524)
	*(volatile u32 *)(RALINK_REG_PIO3924INT) = cpu_to_le32(0x0000FFFF);
	*(volatile u32 *)(RALINK_REG_PIO4540INT) = cpu_to_le32(0x0000003F);
#elif defined (RALINK_GPIO_HAS_5124)
	*(volatile u32 *)(RALINK_REG_PIO3924INT) = cpu_to_le32(0x0000FFFF);
	*(volatile u32 *)(RALINK_REG_PIO5140INT) = cpu_to_le32(0x00000FFF);
#elif defined (RALINK_GPIO_HAS_9524) || defined (RALINK_GPIO_HAS_7224)
	*(volatile u32 *)(RALINK_REG_PIO3924INT) = cpu_to_le32(0x0000FFFF);
	*(volatile u32 *)(RALINK_REG_PIO7140INT) = cpu_to_le32(0xFFFFFFFF);
#if defined (RALINK_GPIO_HAS_7224)
	*(volatile u32 *)(RALINK_REG_PIO72INT)   = cpu_to_le32(0x00000001);
#else
	*(volatile u32 *)(RALINK_REG_PIO9572INT) = cpu_to_le32(0x00FFFFFF);
#endif
#endif
#endif
}

static void
ralink_gpio_int_enabled(u32 is_enabled)
{
	if (is_enabled) {
		ralink_gpio_irq_clear();
		*(volatile u32 *)(RALINK_INTENA) = cpu_to_le32(RALINK_INTCTL_PIO);
	} else {
		*(volatile u32 *)(RALINK_INTDIS) = cpu_to_le32(RALINK_INTCTL_PIO);
	}
}

/*
 * send a signal(SIGUSR2) to the registered user process whenever
 * any gpio interrupt comes (called by interrupt handler)
 */
static int
ralink_gpio_notify_user(u32 irq_gpio, u32 rise_edge)
{
	struct task_struct *p = NULL;

	if (irq_gpio < 0 || irq_gpio >= RALINK_GPIO_NUMBER)
		return 0;

	// don't send any signal if pid is 0 or 1
	if (ralink_gpio_irq_data[irq_gpio].pid < 2)
		return 0;

	if (!(ralink_gpio_irq_data[irq_gpio].rise && rise_edge) &&
	    !(ralink_gpio_irq_data[irq_gpio].fall && !rise_edge))
		return 0;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	p = find_task_by_vpid(ralink_gpio_irq_data[irq_gpio].pid);
#else
	p = find_task_by_pid(ralink_gpio_irq_data[irq_gpio].pid);
#endif
	if (!p)
		return 0;

	send_sig(SIGUSR2, p, 0);

	return 1;
}

irqreturn_t ralink_gpio_irq_handler(int irq, void *dev_id)
{
	u32 i, rise_edge;

	u32 ralink_gpio_intp     = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIOINT));
	u32 ralink_gpio_edge     = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIOEDGE));
#if defined (RALINK_GPIO_HAS_2722)
	u32 ralink_gpio2722_intp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO2722INT));
	u32 ralink_gpio2722_edge = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO2722EDGE));
#elif defined (RALINK_GPIO_HAS_4524)
	u32 ralink_gpio3924_intp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924INT));
	u32 ralink_gpio3924_edge = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924EDGE));
	u32 ralink_gpio4540_intp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO4540INT));
	u32 ralink_gpio4540_edge = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO4540EDGE));
#elif defined (RALINK_GPIO_HAS_5124)
	u32 ralink_gpio3924_intp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924INT));
	u32 ralink_gpio3924_edge = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924EDGE));
	u32 ralink_gpio5140_intp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140INT));
	u32 ralink_gpio5140_edge = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140EDGE));
#elif defined (RALINK_GPIO_HAS_9524) || defined (RALINK_GPIO_HAS_7224)
	u32 ralink_gpio3924_intp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924INT));
	u32 ralink_gpio3924_edge = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924EDGE));
	u32 ralink_gpio7140_intp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140INT));
	u32 ralink_gpio7140_edge = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO7140EDGE));
#if defined (RALINK_GPIO_HAS_7224)
	u32 ralink_gpio72_intp   = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO72INT));
	u32 ralink_gpio72_edge   = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO72EDGE));
#else
	u32 ralink_gpio9572_intp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO9572INT));
	u32 ralink_gpio9572_edge = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO9572EDGE));
#endif
#elif defined (RALINK_GPIO_HAS_9532)
	u32 ralink_gpio6332_intp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO6332INT));
	u32 ralink_gpio6332_edge = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO6332EDGE));
	u32 ralink_gpio9564_intp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO9564INT));
	u32 ralink_gpio9564_edge = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO9564EDGE));
#endif

	ralink_gpio_irq_clear();

#if defined (RALINK_GPIO_HAS_2722)
	for (i = 0; i < 22; i++) {
		if ( !(ralink_gpio_intp & RALINK_GPIO(i)) )
			continue;
		rise_edge = (ralink_gpio_edge & RALINK_GPIO(i)) ? 1 : 0;
		if (ralink_gpio_notify_user(i, rise_edge))
			break;
	}
	for (i = 22; i < 28; i++) {
		if ( !(ralink_gpio2722_intp & RALINK_GPIO((i-22))) )
			continue;
		rise_edge = (ralink_gpio2722_edge & RALINK_GPIO((i-22))) ? 1 : 0;
		if (ralink_gpio_notify_user(i, rise_edge))
			break;
	}
#elif defined (RALINK_GPIO_HAS_3232)
	for (i = 0; i < 32; i++) {
		if ( !(ralink_gpio_intp & RALINK_GPIO(i)) )
			continue;
		rise_edge = (ralink_gpio_edge & RALINK_GPIO(i)) ? 1 : 0;
		if (ralink_gpio_notify_user(i, rise_edge))
			break;
	}
#elif defined (RALINK_GPIO_HAS_9532)
	for (i = 0; i < 32; i++) {
		if ( !(ralink_gpio_intp & RALINK_GPIO(i)) )
			continue;
		rise_edge = (ralink_gpio_edge & RALINK_GPIO(i)) ? 1 : 0;
		if (ralink_gpio_notify_user(i, rise_edge))
			break;
	}
	for (i = 32; i < 64; i++) {
		if ( !(ralink_gpio6332_intp & RALINK_GPIO((i-32))) )
			continue;
		rise_edge = (ralink_gpio6332_edge & RALINK_GPIO((i-32))) ? 1 : 0;
		if (ralink_gpio_notify_user(i, rise_edge))
			break;
	}
	for (i = 64; i < RALINK_GPIO_NUMBER; i++) {
		if ( !(ralink_gpio9564_intp & RALINK_GPIO((i-64))) )
			continue;
		rise_edge = (ralink_gpio9564_edge & RALINK_GPIO((i-64))) ? 1 : 0;
		if (ralink_gpio_notify_user(i, rise_edge))
			break;
	}
#else
	for (i = 0; i < 24; i++) {
		if ( !(ralink_gpio_intp & RALINK_GPIO(i)) )
			continue;
		rise_edge = (ralink_gpio_edge & RALINK_GPIO(i)) ? 1 : 0;
		if (ralink_gpio_notify_user(i, rise_edge))
			break;
	}
#if defined (RALINK_GPIO_HAS_4524)
	for (i = 24; i < 40; i++) {
		if ( !(ralink_gpio3924_intp & RALINK_GPIO((i-24))) )
			continue;
		rise_edge = (ralink_gpio3924_edge & RALINK_GPIO((i-24))) ? 1 : 0;
		if (ralink_gpio_notify_user(i, rise_edge))
			break;
	}
	for (i = 40; i < RALINK_GPIO_NUMBER; i++) {
		if ( !(ralink_gpio4540_intp & RALINK_GPIO((i-40))) )
			continue;
		rise_edge = (ralink_gpio4540_edge & RALINK_GPIO((i-40))) ? 1 : 0;
		if (ralink_gpio_notify_user(i, rise_edge))
			break;
	}
#elif defined (RALINK_GPIO_HAS_5124)
	for (i = 24; i < 40; i++) {
		if ( !(ralink_gpio3924_intp & RALINK_GPIO((i-24))) )
			continue;
		rise_edge = (ralink_gpio3924_edge & RALINK_GPIO((i-24))) ? 1 : 0;
		if (ralink_gpio_notify_user(i, rise_edge))
			break;
	}
	for (i = 40; i < RALINK_GPIO_NUMBER; i++) {
		if ( !(ralink_gpio5140_intp & RALINK_GPIO((i-40))) )
			continue;
		rise_edge = (ralink_gpio5140_edge & RALINK_GPIO((i-40))) ? 1 : 0;
		if (ralink_gpio_notify_user(i, rise_edge))
			break;
	}
#elif defined (RALINK_GPIO_HAS_9524) || defined (RALINK_GPIO_HAS_7224)
	for (i = 24; i < 40; i++) {
		if ( !(ralink_gpio3924_intp & RALINK_GPIO((i-24))) )
			continue;
		rise_edge = (ralink_gpio3924_edge & RALINK_GPIO((i-24))) ? 1 : 0;
		if (ralink_gpio_notify_user(i, rise_edge))
			break;
	}
	for (i = 40; i < 72; i++) {
		if ( !(ralink_gpio7140_intp & RALINK_GPIO((i-40))) )
			continue;
		rise_edge = (ralink_gpio7140_edge & RALINK_GPIO((i-40))) ? 1 : 0;
		if (ralink_gpio_notify_user(i, rise_edge))
			break;
	}
#if defined (RALINK_GPIO_HAS_7224)
	for (i = 72; i < RALINK_GPIO_NUMBER; i++) {
		if ( !(ralink_gpio72_intp & RALINK_GPIO((i-72))) )
			continue;
		rise_edge = (ralink_gpio72_edge & RALINK_GPIO((i-72))) ? 1 : 0;
		if (ralink_gpio_notify_user(i, rise_edge))
			break;
	}
#else
	for (i = 72; i < RALINK_GPIO_NUMBER; i++) {
		if ( !(ralink_gpio9572_intp & RALINK_GPIO((i-72))) )
			continue;
		rise_edge = (ralink_gpio9572_edge & RALINK_GPIO((i-72))) ? 1 : 0;
		if (ralink_gpio_notify_user(i, rise_edge))
			break;
	}
#endif
#endif
#endif
	return IRQ_HANDLED;
}

static void
ralink_gpio_init_irq(void)
{
	int err;
	unsigned int i;

	ralink_gpio_irq_clear();

	memset(ralink_gpio_irq_data, 0, sizeof(ralink_gpio_irq_data));

	for (i = 0; i < RALINK_GPIO_NUMBER; i++)
		ralink_gpio_irq_data[i].gpio = i;

	err = request_irq(SURFBOARDINT_GPIO, ralink_gpio_irq_handler, IRQF_DISABLED, "ralink_gpio", NULL);
}

static void
ralink_gpio_uninit_irq(void)
{
	// disable gpio interrupt
	ralink_gpio_int_enabled(0);

	free_irq(SURFBOARDINT_GPIO, NULL);
}
#endif

long ralink_gpio_ioctl(struct file *file, unsigned int req, unsigned long arg)
{
	unsigned int uint_value;
	unsigned int uint_param = (req >> IOCTL_GPIO_CMD_LENGTH_BITS);
#ifdef CONFIG_RALINK_GPIO_IRQ
	ralink_gpio_irq_info irq_info;
#endif
#ifdef CONFIG_RALINK_GPIO_LED
	ralink_gpio_led_info led_info;
#endif

	req &= ((1u << IOCTL_GPIO_CMD_LENGTH_BITS)-1);

	switch(req)
	{
	case IOCTL_GPIO_DIR_OUT:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ralink_gpio_set_pin_direction(uint_param, uint_value);
		break;
	case IOCTL_GPIO_READ:
		uint_value = ralink_gpio_get_pin_value(uint_param);
		put_user(uint_value, (int __user *)arg);
		break;
	case IOCTL_GPIO_WRITE:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ralink_gpio_set_pin_value(uint_param, uint_value);
		break;
#ifdef CONFIG_RALINK_GPIO_IRQ
	case IOCTL_GPIO_IRQ_SET:
		copy_from_user(&irq_info, (ralink_gpio_irq_info __user *)arg, sizeof(irq_info));
		ralink_gpio_int_set(&irq_info);
		break;
	case IOCTL_GPIO_IRQ_INT_ENABLED:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ralink_gpio_int_enabled(uint_value);
		break;
#endif
#ifdef CONFIG_RALINK_GPIO_LED
	case IOCTL_GPIO_LED_SET:
		copy_from_user(&led_info, (ralink_gpio_led_info __user *)arg, sizeof(led_info));
		ralink_gpio_led_set(&led_info);
		break;
	case IOCTL_GPIO_LED_TIMER_ENABLED:
		copy_from_user(&uint_value, (int __user *)arg, sizeof(int));
		ralink_gpio_led_del_timer();
		if (uint_value)
			ralink_gpio_led_add_timer();
		break;
#endif
	default:
		return -ENOIOCTLCMD;
	}
	return 0;
}

static int
ralink_gpio_open(struct inode *inode, struct file *file)
{
	try_module_get(THIS_MODULE);
	return 0;
}

static int
ralink_gpio_release(struct inode *inode, struct file *file)
{
	module_put(THIS_MODULE);
	return 0;
}

static const struct
file_operations ralink_gpio_fops =
{
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= ralink_gpio_ioctl,
	.open		= ralink_gpio_open,
	.release	= ralink_gpio_release,
};

int __init ralink_gpio_init(void)
{
	u32 gpiomode;
	int r = register_chrdev(ralink_gpio_major, RALINK_GPIO_DEVNAME, &ralink_gpio_fops);
	if (r < 0) {
		printk(KERN_ERR NAME ": unable to register character device\n");
		return r;
	}

	if (ralink_gpio_major == 0) {
		ralink_gpio_major = r;
	}

	//config these pins to gpio mode
	gpiomode = le32_to_cpu(*(volatile u32 *)(RALINK_REG_GPIOMODE));
	gpiomode &= ~(RALINK_GPIOMODE_DFT);		// clear bit[2:4] UARTF_SHARE_MODE
#if defined (CONFIG_RALINK_MT7620) || defined (CONFIG_RALINK_MT7628)
	gpiomode &= ~(RALINK_GPIOMODE_WLED);		// clear bit[13] WLAN_LED
#endif
#if defined (CONFIG_RALINK_MT7620)
	gpiomode &= ~(RALINK_GPIOMODE_SPI_REFCLK);	// clear bit[12] SPI_REFCLK0_MODE
#endif
	gpiomode |= RALINK_GPIOMODE_DFT;
#ifdef CONFIG_RALINK_GPIOMODE_I2C
#ifdef CONFIG_RALINK_I2C
 #error "Please disable Ralink I2C (RALINK_I2C) to support GPIO mode."
#else
	gpiomode |= RALINK_GPIOMODE_I2C;
#endif
#endif
#ifdef CONFIG_RALINK_GPIOMODE_SPI
#ifdef CONFIG_RALINK_SPI
 #error "Please disable Ralink SPI (RALINK_SPI) to support GPIO mode."
#else
	gpiomode |= RALINK_GPIOMODE_SPI;
#endif
#elif defined(CONFIG_RALINK_GPIOMODE_SPI_REFCLK) && defined(RALINK_GPIOMODE_SPI_REFCLK)
	gpiomode |= RALINK_GPIOMODE_SPI_REFCLK;
#endif
#if defined(CONFIG_RALINK_GPIOMODE_UARTF) && defined(RALINK_GPIOMODE_UARTF)
	gpiomode |= RALINK_GPIOMODE_UARTF;
#endif
#if defined(CONFIG_RALINK_GPIOMODE_JTAG) && defined(RALINK_GPIOMODE_JTAG)
	gpiomode |= RALINK_GPIOMODE_JTAG;
#endif
#if defined(CONFIG_RALINK_GPIOMODE_EPHY) && defined(RALINK_GPIOMODE_EPHY)
	gpiomode |= RALINK_GPIOMODE_EPHY;
#endif
#if defined(CONFIG_RALINK_GPIOMODE_MDIO) && defined(RALINK_GPIOMODE_MDIO)
	gpiomode |= RALINK_GPIOMODE_MDIO;
#endif
#if defined(CONFIG_RALINK_GPIOMODE_PCI) && defined(RALINK_GPIOMODE_PCI)
	gpiomode |= RALINK_GPIOMODE_PCI;
#endif
#if defined(CONFIG_RALINK_GPIOMODE_LNA_A) && defined(RALINK_GPIOMODE_LNA_A)
	gpiomode |= RALINK_GPIOMODE_LNA_A;
#endif
#if defined(CONFIG_RALINK_GPIOMODE_LNA_G) && defined(RALINK_GPIOMODE_LNA_G)
	gpiomode |= RALINK_GPIOMODE_LNA_G;
#endif
#if defined(CONFIG_RALINK_GPIOMODE_PA_G) && defined(RALINK_GPIOMODE_PA_G)
	gpiomode |= RALINK_GPIOMODE_PA_G;
#endif
	*(volatile u32 *)(RALINK_REG_GPIOMODE) = cpu_to_le32(gpiomode);

#ifdef CONFIG_RALINK_GPIO_IRQ
	ralink_gpio_init_irq();
#endif

#ifdef CONFIG_RALINK_GPIO_LED
	init_timer(&ralink_gpio_led_timer);
#endif

	printk("Ralink GPIO driver initialized. Number of GPIO: %d, GPIO mode: %08X\n", RALINK_GPIO_NUMBER, gpiomode);
	return 0;
}

void __exit ralink_gpio_exit(void)
{
	unregister_chrdev(ralink_gpio_major, RALINK_GPIO_DEVNAME);

#ifdef CONFIG_RALINK_GPIO_LED
	ralink_gpio_led_del_timer();
#endif

#ifdef CONFIG_RALINK_GPIO_IRQ
	ralink_gpio_uninit_irq();
#endif
}

module_init(ralink_gpio_init);
module_exit(ralink_gpio_exit);

MODULE_DESCRIPTION("Ralink SoC GPIO Driver");
MODULE_AUTHOR("Winfred Lu <winfred_lu@ralinktech.com.tw>");
MODULE_LICENSE("GPL");

