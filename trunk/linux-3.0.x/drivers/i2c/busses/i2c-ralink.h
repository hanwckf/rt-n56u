#ifndef _I2C_RALINK_H_
#define _I2C_RALINK_H_

#define RALINK_I2C_STARTERR		0x10
#define RALINK_I2C_ACKERR		0x08
#define RALINK_I2C_DATARDY		0x04
#define RALINK_I2C_SDOEMPTY		0x02
#define RALINK_I2C_BUSY			0x01

#define RT2880_REG(x)			(*((volatile u32 *)(x)))

#define READ_BLOCK			16
#define RT2880_RSTCTRL_REG		(RALINK_SYSCTL_BASE+0x34)

#define RT2880_I2C_REG_BASE		(RALINK_I2C_BASE)
#define RT2880_I2C_CONFIG_REG		(RT2880_I2C_REG_BASE+0x00)
#define RT2880_I2C_CLKDIV_REG		(RT2880_I2C_REG_BASE+0x04)
#define RT2880_I2C_DEVADDR_REG		(RT2880_I2C_REG_BASE+0x08)
#define RT2880_I2C_ADDR_REG		(RT2880_I2C_REG_BASE+0x0C)
#define RT2880_I2C_DATAOUT_REG		(RT2880_I2C_REG_BASE+0x10)
#define RT2880_I2C_DATAIN_REG		(RT2880_I2C_REG_BASE+0x14)
#define RT2880_I2C_STATUS_REG		(RT2880_I2C_REG_BASE+0x18)
#define RT2880_I2C_STARTXFR_REG		(RT2880_I2C_REG_BASE+0x1C)
#define RT2880_I2C_BYTECNT_REG		(RT2880_I2C_REG_BASE+0x20)
#define RT2880_I2C_SM0_IS_AUTOMODE	(RT2880_I2C_REG_BASE+0x28)
#define RT2880_I2C_SM0CTL0		(RT2880_I2C_REG_BASE+0x40)

/* I2C_CFG register bit field */
#define I2C_CFG_DEVADLEN_8		(7<<5)	/* 8 bits */
#define I2C_CFG_DEVADLEN_7		(6<<2)	/* 7 bits */
#define I2C_CFG_ADDRDIS			(1<<1)	/* disable address transmission*/
#define I2C_CFG_DEVADDIS		(1<<0)	/* disable evice address transmission */
#define I2C_CFG_DEFAULT			(I2C_CFG_ADDRDIS | I2C_CFG_DEVADLEN_7 | I2C_CFG_DEVADLEN_8)

#define I2C_CTL0_ODRAIN			(1<<31)	/* the output is pulled hight by SIF master 0 */
#define I2C_CTL0_VSYNC_MODE		(1<<28)	/* allow triggered in VSYNC pulse */
#define I2C_CTL0_SM0_WAIT_LEVEL		(1<<6)	/* output H when SIF master 0 is in WAIT state */
#define I2C_CTL0_SM0_EN			(1<<1)	/* Enable SIF master 0 */

#define CLKDIV_VALUE			333

/* Instruction codes */
#define READ_CMD			0x01
#define WRITE_CMD			0x00

struct i2c_algo_ralink_data {
	u32 ioaddr;
	wait_queue_head_t waitq;
	spinlock_t lock;
	int id;
};

#endif /* _I2C_RALINK_H_ */
