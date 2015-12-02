
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>

#include <asm/rt2880/rt_mmap.h>

#if defined (CONFIG_EEPROM_AT24) || defined (CONFIG_EEPROM_AT24_MODULE)
#include <linux/i2c/at24.h>

static struct at24_platform_data eeprom_data = {
	.byte_len	= 1024 / 8,
	.page_size	= 8,
};
#endif

static struct i2c_board_info ralink_i2c_info[] __initconst =  {
#if defined (CONFIG_EEPROM_AT24) || defined (CONFIG_EEPROM_AT24_MODULE)
	{
		I2C_BOARD_INFO("at24c01", 0x50),
		.platform_data = &eeprom_data,
	},
#endif
#if defined (CONFIG_RTC_DRV_PCF8563) || defined (CONFIG_RTC_DRV_PCF8563_MODULE)
	{
		I2C_BOARD_INFO("pcf8563", 0x51),
	},
#endif
};

static struct resource ralink_i2c_resources[] = {
	{
		.start	= RALINK_I2C_BASE,
		.end	= RALINK_I2C_BASE + 0xFF,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device ralink_i2c_device = {
	.name		= "Ralink-I2C",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(ralink_i2c_resources),
	.resource	= ralink_i2c_resources,
};

int __init ralink_i2c_register(void)
{
	platform_device_register(&ralink_i2c_device);
	i2c_register_board_info(0, ralink_i2c_info, ARRAY_SIZE(ralink_i2c_info));

	return 0;
}

arch_initcall(ralink_i2c_register);
