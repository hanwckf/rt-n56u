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

////////////////////////////////////////////////////////////////////////////////
// IOCTL
////////////////////////////////////////////////////////////////////////////////

#define RALINK_GPIO_DEVPATH		"/dev/gpio"

#define IOCTL_GPIO_CMD_LENGTH_BITS	(8)

#define IOCTL_GPIO_DIR_OUT		0x01
#define IOCTL_GPIO_READ			0x02
#define IOCTL_GPIO_WRITE		0x03

#define IOCTL_GPIO_IRQ_SET		0x10
#define IOCTL_GPIO_IRQ_INT_ENABLED	0x11

#define IOCTL_GPIO_LED_SET		0x20
#define IOCTL_GPIO_LED_TIMER_ENABLED	0x21

typedef struct {
	unsigned int gpio: 30;		//request irq pin number
	unsigned int rise: 1;		//rising edge
	unsigned int fall: 1;		//falling edge
	pid_t pid;			//process id to notify
} ralink_gpio_reg_info;

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

int cpu_gpio_led_timer(int timer_on)
{
	return ralink_gpio_ioctl(IOCTL_GPIO_LED_TIMER_ENABLED, 0, &timer_on);
}

////////////////////////////////////////////////////////////////////////////////
// GPIO IRQ
////////////////////////////////////////////////////////////////////////////////

int cpu_gpio_irq_enable(int irq_on)
{
	return ralink_gpio_ioctl(IOCTL_GPIO_IRQ_INT_ENABLED, 0, &irq_on);
}

int cpu_gpio_irq_set(unsigned int irq_pin, unsigned int rising_edge, unsigned int falling_edge, pid_t pid)
{
	ralink_gpio_reg_info reg;

	reg.gpio = irq_pin;
	reg.rise = rising_edge;
	reg.fall = falling_edge;
	reg.pid = pid;

	return ralink_gpio_ioctl(IOCTL_GPIO_IRQ_SET, 0, &reg);
}

