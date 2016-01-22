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

#ifndef _GPIO_PINS_H_
#define _GPIO_PINS_H_

////////////////////////////////////////////////////////////////////////////////
// Ralink CPU GPIO CONTROL
////////////////////////////////////////////////////////////////////////////////

int cpu_gpio_mode_set_bit(int bit, unsigned int value);
int cpu_gpio_mode_get_bit(int bit, unsigned int *p_value);
int cpu_gpio_set_pin_direction(int pin, unsigned int use_output_direction);
int cpu_gpio_set_pin(int pin, unsigned int value);
int cpu_gpio_get_pin(int pin, unsigned int *p_value);

////////////////////////////////////////////////////////////////////////////////

int cpu_gpio_led_set(unsigned int led_pin, int blink_inverted);
int cpu_gpio_led_enabled(unsigned int led_pin, int enabled);
int cpu_gpio_irq_set(unsigned int irq_pin, int rising_edge, int falling_edge, pid_t pid);

////////////////////////////////////////////////////////////////////////////////

int cpu_gpio_main(int argc, char **argv);

#endif
