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
// RALINK CPU GPIO CONTROL
////////////////////////////////////////////////////////////////////////////////

int cpu_gpio_mode_set_bit(int bit, unsigned int value)
{
	return ralink_gpio_ioctl(IOCTL_GPIO_MODE_SET, bit, &value);
}

int cpu_gpio_mode_get_bit(int bit, unsigned int *p_value)
{
	return ralink_gpio_ioctl(IOCTL_GPIO_MODE_GET, bit, p_value);
}

int cpu_gpio_set_pin_direction(int pin, unsigned int use_output_direction)
{
	return ralink_gpio_ioctl(IOCTL_GPIO_DIR_OUT, pin, &use_output_direction);
}

int cpu_gpio_set_pin(int pin, unsigned int value)
{
	return ralink_gpio_ioctl(IOCTL_GPIO_WRITE, pin, &value);
}

int cpu_gpio_get_pin(int pin, unsigned int *p_value)
{
	return ralink_gpio_ioctl(IOCTL_GPIO_READ, pin, p_value);
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

////////////////////////////////////////////////////////////////////////////////
// GPIO PROCESS
////////////////////////////////////////////////////////////////////////////////

static void show_usage(char *cmd)
{
	printf("Usage: %s -d <pin> <in/out> - gpio pin set direction (in=0, out=1)\n", cmd);
	printf("       %s -r <pin> - gpio pin read value\n", cmd);
	printf("       %s -w <pin> <value> - gpio pin write value (0/1)\n\n", cmd);
	printf("       %s -m <bit> - gpio mode get bit\n", cmd);
	printf("       %s -s <bit> <value> - gpio mode set bit (0/1)\n", cmd);
}

int cpu_gpio_main(int argc, char **argv)
{
	int ret = 1, idx;
	unsigned int value = 0;

	if (argc < 2) {
		show_usage(argv[0]);
		return 1;
	}

	if (strlen(argv[1]) < 2 || argv[1][0] != '-') {
		show_usage(argv[0]);
		return 1;
	}

	switch (argv[1][1])
	{
	case 'd':
		if (argc == 4)
			ret = cpu_gpio_set_pin_direction(atoi(argv[2]), atoi(argv[3]));
		else
			show_usage(argv[0]);
		break;
	case 'r':
		if (argc == 3) {
			idx = atoi(argv[2]);
			ret = cpu_gpio_get_pin(idx, &value);
			if (ret == 0)
				printf("gpio pin %d = %d\n", idx, value);
		} else
			show_usage(argv[0]);
		break;
	case 'w':
		if (argc == 4)
			ret = cpu_gpio_set_pin(atoi(argv[2]), atoi(argv[3]));
		else
			show_usage(argv[0]);
		break;
	case 'm':
		if (argc == 3) {
			idx = atoi(argv[2]);
			ret = cpu_gpio_mode_get_bit(idx, &value);
			if (ret == 0)
				printf("gpio mode bit [%d] = %d\n", idx, value);
		} else
			show_usage(argv[0]);
		break;
	case 's':
		if (argc == 4)
			ret = cpu_gpio_mode_set_bit(atoi(argv[2]), atoi(argv[3]));
		else
			show_usage(argv[0]);
		break;
	default:
		show_usage(argv[0]);
	}

	return ret;
}
