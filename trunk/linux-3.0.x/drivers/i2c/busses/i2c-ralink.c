#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/io.h>

#include <ralink/ralink_gpio.h>
#include "i2c-ralink.h"

#define I2C_MAX_BUSY_US		200000
#define I2C_MAX_DONE_US		500000

static int i2c_id;

static void
ralink_i2c_reset(void)
{
	u32 val;

	/* reset i2c block */
	val = RT2880_REG(RT2880_RSTCTRL_REG);
	val |= RALINK_I2C_RST;
	RT2880_REG(RT2880_RSTCTRL_REG) = val;

	udelay(1);

	val &= ~RALINK_I2C_RST;
	RT2880_REG(RT2880_RSTCTRL_REG) = val;

	udelay(500);
}

static void
ralink_i2c_master_init(struct i2c_msg *msg)
{
	u32 reg;

	RT2880_REG(RT2880_I2C_CONFIG_REG) = I2C_CFG_DEFAULT;

#if defined (CONFIG_RALINK_MT7621) || defined (CONFIG_RALINK_MT7628)
	reg  = (CLKDIV_VALUE << 16);		// clk div
	reg |= I2C_CTL0_ODRAIN;			// the output is pulled hight by SIF master 0
	reg |= I2C_CTL0_VSYNC_MODE;		// allow triggered in VSYNC pulse
	reg |= I2C_CTL0_SM0_WAIT_LEVEL;		// output H when SIF master 0 is in WAIT state
	reg |= I2C_CTL0_SM0_EN;			// enable SIF master 0
	RT2880_REG(RT2880_I2C_SM0CTL0) = reg;
	RT2880_REG(RT2880_I2C_SM0_IS_AUTOMODE) = 1; // auto mode
#else
	reg = CLKDIV_VALUE;
	RT2880_REG(RT2880_I2C_CLKDIV_REG) = reg;
#endif

	/*
	 * Device Address :
	 *
	 * ATMEL 24C152 serial EEPROM
	 *       1|0|1|0|0|A1|A2|R/W
	 *      MSB              LSB
	 *
	 * ATMEL 24C01A/02/04/08A/16A
	 *      MSB               LSB
	 * 1K/2K 1|0|1|0|A2|A1|A0|R/W
	 * 4K            A2 A1 P0
	 * 8K            A2 P1 P0
	 * 16K           P2 P1 P0
	 *
	 * so device address needs 7 bits
	 * if device address is 0,
	 * write 0xA0 >> 1 into DEVADDR(max 7-bits) REG
	 */
	RT2880_REG(RT2880_I2C_DEVADDR_REG) = msg->addr;

	/*
	 * Use Address Disabled Transfer Options
	 * because it just support 8-bits, 
	 * ATMEL eeprom needs two 8-bits address
	 */
	RT2880_REG(RT2880_I2C_ADDR_REG) = 0;
}

static int
ralink_i2c_wait_tx_done(void)
{
	int timeout = I2C_MAX_DONE_US;

	while(timeout > 0) {
		if (RT2880_REG(RT2880_I2C_STATUS_REG) & RALINK_I2C_SDOEMPTY)
			return 0;
		timeout--;
		udelay(1);
	}

	return -1;
}

static int
ralink_i2c_wait_rx_done(void)
{
	int timeout = I2C_MAX_DONE_US;

	while(timeout > 0) {
		if (RT2880_REG(RT2880_I2C_STATUS_REG) & RALINK_I2C_DATARDY)
			return 0;
		timeout--;
		udelay(1);
	}

	return -1;
}

static int
ralink_i2c_wait_idle(void)
{
	int timeout = I2C_MAX_BUSY_US;

	while(timeout > 0) {
		if (!(RT2880_REG(RT2880_I2C_STATUS_REG) & RALINK_I2C_BUSY))
			return 0;
		timeout--;
		udelay(1);
	}

	return -1;
}

static int
ralink_i2c_handle_msg(struct i2c_adapter *i2c_adap, struct i2c_msg* msg)
{
	int i=0, j=0, pos=0;
	int nblock = msg->len / READ_BLOCK;
	int rem = msg->len % READ_BLOCK;

	if (msg->flags & I2C_M_TEN) {
		printk("%s: 10 bits addr not supported\n", "Ralink-I2C");
		return -EINVAL;
	}

	if ((msg->flags&I2C_M_RD)) {
		for(i=0; i< nblock; i++) {
			ralink_i2c_wait_idle();
			RT2880_REG(RT2880_I2C_BYTECNT_REG) = READ_BLOCK-1;
			RT2880_REG(RT2880_I2C_STARTXFR_REG) = READ_CMD;
			for(j=0; j< READ_BLOCK; j++) {
				ralink_i2c_wait_rx_done();
				msg->buf[pos++] = RT2880_REG(RT2880_I2C_DATAIN_REG);
			}
		}

		ralink_i2c_wait_idle();
		RT2880_REG(RT2880_I2C_BYTECNT_REG) = rem-1;
		RT2880_REG(RT2880_I2C_STARTXFR_REG) = READ_CMD;
		for(i=0; i< rem; i++) {
			ralink_i2c_wait_rx_done();
			msg->buf[pos++] = RT2880_REG(RT2880_I2C_DATAIN_REG);
		}
	} else {
		ralink_i2c_wait_idle();
		RT2880_REG(RT2880_I2C_BYTECNT_REG) = msg->len-1;

		for(i=0; i< msg->len; i++) {
			RT2880_REG(RT2880_I2C_DATAOUT_REG) = msg->buf[i];

			if (i==0)
				RT2880_REG(RT2880_I2C_STARTXFR_REG) = WRITE_CMD;

			ralink_i2c_wait_tx_done();
		}
	}

	return 0;
}

/*
 * master_xfer() - main read/write entry
 */
static int
ralink_i2c_master_xfer(struct i2c_adapter *i2c_adap, struct i2c_msg *msgs, 
				int num)
{
	int im = 0;
	int ret = 0;

	ralink_i2c_wait_idle();
	ralink_i2c_reset();
	ralink_i2c_master_init(msgs);

	for (im = 0; ret == 0 && im != num; im++) {
		ret = ralink_i2c_handle_msg(i2c_adap, &msgs[im]);
	}

	if (ret)
		return ret;

	return im;
}

static u32
ralink_i2c_func(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

static const struct i2c_algorithm ralink_i2c_algo = {
	.master_xfer	= ralink_i2c_master_xfer,
	.smbus_xfer	= NULL,
	.functionality	= ralink_i2c_func,
};

static int
ralink_i2c_remove(struct platform_device *pdev)
{
	struct i2c_adapter *padapter = platform_get_drvdata(pdev);
	struct i2c_algo_ralink_data *adapter_data = 
		(struct i2c_algo_ralink_data *)padapter->algo_data;

	release_mem_region(RALINK_I2C_BASE, 256);
	kfree(adapter_data);
	kfree(padapter);

	platform_set_drvdata(pdev, NULL);

	return 0;
}

static int
ralink_i2c_probe(struct platform_device *pdev)
{
	struct resource *res;
	int ret;
	struct i2c_adapter *new_adapter;
	struct i2c_algo_ralink_data *adapter_data;

	new_adapter = kzalloc(sizeof(struct i2c_adapter), GFP_KERNEL);
	if (!new_adapter) {
		ret = -ENOMEM;
		goto out;
	}

	adapter_data = kzalloc(sizeof(struct i2c_algo_ralink_data), GFP_KERNEL);
	if (!adapter_data) {
		ret = -ENOMEM;
		goto free_adapter;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		ret = -ENODEV;
		goto free_both;
	}

	if (!request_mem_region(RALINK_I2C_BASE, resource_size(res), pdev->name)) {
		ret = -EBUSY;
		goto free_both;
	}

	/* configure i2c to normal mode */
	RT2880_REG(RALINK_SYSCTL_BASE + 0x60) &= ~RALINK_GPIOMODE_I2C;

	adapter_data->id = i2c_id++;

	memcpy(new_adapter->name, pdev->name, strlen(pdev->name));
	new_adapter->owner = THIS_MODULE;
	new_adapter->class = I2C_CLASS_HWMON | I2C_CLASS_SPD;
	new_adapter->dev.parent = &pdev->dev;
	new_adapter->nr = pdev->id;

	new_adapter->timeout = HZ;
	new_adapter->algo = &ralink_i2c_algo;

	init_waitqueue_head(&adapter_data->waitq);
	spin_lock_init(&adapter_data->lock);

	platform_set_drvdata(pdev, new_adapter);
	new_adapter->algo_data = adapter_data;

	i2c_add_numbered_adapter(new_adapter);

	return 0;

free_both:
	kfree(adapter_data);

free_adapter:
	kfree(new_adapter);

out:
	return ret;
}

static struct platform_driver ralink_i2c_driver = {
	.probe		= ralink_i2c_probe,
	.remove		= ralink_i2c_remove,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "Ralink-I2C",
	},
};

static int __init i2c_ralink_init(void)
{
	return platform_driver_register(&ralink_i2c_driver);
}

static void __exit i2c_ralink_exit(void)
{
	platform_driver_unregister(&ralink_i2c_driver);
}

module_init(i2c_ralink_init);
module_exit(i2c_ralink_exit);

MODULE_AUTHOR("Steven Liu <steven_liu@mediatek.com>");
MODULE_DESCRIPTION("Ralink/MTK I2C host driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:Ralink-I2C");
