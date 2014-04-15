/******************************************************************************
* Filename : gpio.c
* This part is used to control LED and detect button-press
* 
******************************************************************************/

#include <common.h>
#include <rt_mmap.h>
#include <ralink_gpio.h>

#define LED_POWER	0
#define BTN_RESET	13
#define BTN_WPS		26


int ralink_gpio_ioctl2(unsigned int req, int idx, unsigned long arg)
{
	unsigned long tmp;
	int *value;

	req &= RALINK_GPIO_DATA_MASK;

	switch(req) {
	case RALINK_GPIO_READ_BIT:
		if (idx <= 23)
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));
		else if (idx <= 39)
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DATA));
		else
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DATA));

		if (idx <= 23)
			tmp = (tmp >> idx) & 1L;
		else if (idx <= 39)
			tmp = (tmp >> (idx-24)) & 1L;
		else
			tmp = (tmp >> (idx-40)) & 1L;
		value = (int *)arg;
		*value = tmp;
		return tmp;

		break;
	case RALINK_GPIO_WRITE_BIT:
		if (idx <= 23) {
			tmp =le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODATA));
			if (arg & 1L)
				tmp |= (1L << idx);
			else
				tmp &= ~(1L << idx);
			*(volatile u32 *)(RALINK_REG_PIODATA)= cpu_to_le32(tmp);
		}
		else if (idx <= 39) {
			tmp =le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DATA));
			if (arg & 1L)
				tmp |= (1L << (idx-24));
			else
				tmp &= ~(1L << (idx-24));
			*(volatile u32 *)(RALINK_REG_PIO3924DATA)= cpu_to_le32(tmp);
		}
		else {
			tmp =le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DATA));
			if (arg & 1L)
				tmp |= (1L << (idx-40));
			else
				tmp &= ~(1L << (idx-40));
			*(volatile u32 *)(RALINK_REG_PIO5140DATA)= cpu_to_le32(tmp);
		}

		break;
	default:
		return -1;
	}
	return 0;
}

int
ralink_gpio_write_bit2(int idx, int value)
{
	unsigned int req;
	value &= 1;
       
	if (0L <= idx && idx < RALINK_GPIO_NUMBER)
		req = RALINK_GPIO_WRITE_BIT;
	else
		return -1;

	return ralink_gpio_ioctl2(req, idx, value);
}

int
ralink_initGpioPin2(unsigned int idx, int dir)
{
	unsigned long tmp;

	if (idx < 0 || RALINK_GPIO_NUMBER <= idx)
		return -1;

	if (dir == GPIO_DIR_OUT)
	{
		if (idx <= 23) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODIR));
			tmp |= (1L << idx);
		}
		else if (idx <= 39) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DIR));
			tmp |= (1L << (idx-24));
		}
		else {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DIR));
			tmp |= (1L << (idx-40));
		}
	}
	else if (dir == GPIO_DIR_IN)
	{		
		if (idx <= 23) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIODIR));
			tmp &= ~(1L << idx);
		}
		else if (idx <= 39) {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO3924DIR));
			tmp &= ~(1L << (idx-24));
		}
		else {
			tmp = le32_to_cpu(*(volatile u32 *)(RALINK_REG_PIO5140DIR));
			tmp &= ~(1L << (idx-40));
		}
	}
	else
		return -1;

	if (idx <= 23) {
		*(volatile u32 *)(RALINK_REG_PIODIR) = cpu_to_le32(tmp);
	}
	else if (idx <= 39) {
		*(volatile u32 *)(RALINK_REG_PIO3924DIR) = cpu_to_le32(tmp);
	}
	else {
		*(volatile u32 *)(RALINK_REG_PIO5140DIR) = cpu_to_le32(tmp);
	}

	return 0;
}

int
ralink_gpio_read_bit2(int idx)
{
	unsigned int req;
	int value = 0;

	if (0L <= idx && idx < RALINK_GPIO_NUMBER)
		req = RALINK_GPIO_READ_BIT;
	else
		return -1;

	if (ralink_gpio_ioctl2(req, idx, (unsigned long)&value) < 0)
		return -1;

	return value;
}

unsigned long DETECT(void)
{
	int key = 0;

	if (!ralink_gpio_read_bit2(BTN_RESET)) {
		key = 1;
		printf("BTN_RESET pressed\n");
	}

	return key;
}

unsigned long DETECT_WPS(void)
{
	int key = 0;

	if (!ralink_gpio_read_bit2(BTN_WPS)) {
		key = 1;
		printf("BTN_WPS pressed\n");
	}

	return key;
}

void LEDON(void)
{
	ralink_gpio_write_bit2(LED_POWER, 0);
}

void LEDOFF(void)
{
	ralink_gpio_write_bit2(LED_POWER, 1);
}

void asus_gpio_init(void)
{
	unsigned long gpiomode;

	gpiomode = le32_to_cpu(*(volatile u32 *)(RALINK_REG_GPIOMODE));
	gpiomode |= RALINK_GPIOMODE_I2C;
	gpiomode |= RALINK_GPIOMODE_UARTF;
	gpiomode |= RALINK_GPIOMODE_JTAG;
	*(volatile u32 *)(RALINK_REG_GPIOMODE) = cpu_to_le32(gpiomode);

	ralink_initGpioPin2(LED_POWER, GPIO_DIR_OUT);
	ralink_initGpioPin2(BTN_RESET, GPIO_DIR_IN);
	ralink_initGpioPin2(BTN_WPS, GPIO_DIR_IN);
}

