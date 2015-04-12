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
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <ralink_gpio.h>

////////////////////////////////////////////////////////////////////////////////
// IOCTL
////////////////////////////////////////////////////////////////////////////////

static int
ralink_gpio_ioctl(unsigned int cmd, unsigned int par, void *value)
{
	int fd, retVal = 0;

	fd = open(RALINK_GPIO_DEVPATH, O_RDONLY);
	if (fd < 0) {
		perror(RALINK_GPIO_DEVPATH);
		return errno;
	}

	cmd &= ((1u << IOCTL_GPIO_CMD_LENGTH_BITS) - 1);
	cmd |= (par << IOCTL_GPIO_CMD_LENGTH_BITS);

	if (ioctl(fd, cmd, value) < 0) {
		perror("ioctl");
		retVal = errno;
	}

	close(fd);

	return retVal;
}

////////////////////////////////////////////////////////////////////////////////
// GPIO LED
////////////////////////////////////////////////////////////////////////////////

int cpu_gpio_led_set(unsigned int led_pin, int blink_inverted)
{
	ralink_gpio_led_info gli;

	gli.invert = (blink_inverted) ? 1 : 0;
	gli.on = 1;
	gli.off = 1;
	gli.blinks = 1;
	gli.rests = 1;
	gli.times = 1;

	return ralink_gpio_ioctl(IOCTL_GPIO_LED_SET, led_pin, &gli);
}

int cpu_gpio_led_enabled(unsigned int led_pin, int enabled)
{
	return ralink_gpio_ioctl(IOCTL_GPIO_LED_ENABLED, led_pin, &enabled);
}

////////////////////////////////////////////////////////////////////////////////
// GPIO IRQ
////////////////////////////////////////////////////////////////////////////////

int cpu_gpio_irq_set(unsigned int irq_pin, int rising_edge, int falling_edge, pid_t pid)
{
	ralink_gpio_irq_info gii;

	gii.pid = pid;
	gii.rise = (rising_edge) ? 1 : 0;
	gii.fall = (falling_edge) ? 1 : 0;

	return ralink_gpio_ioctl(IOCTL_GPIO_IRQ_SET, irq_pin, &gii);
}

