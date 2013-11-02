#ifndef I2C_RALINK_H
#define I2C_RALINK_H 1

#define RALINK_I2C_STARTERR	0x10
#define RALINK_I2C_ACKERR	0x08
#define RALINK_I2C_DATARDY	0x04
#define RALINK_I2C_SDOEMPTY	0x02
#define RALINK_I2C_BUSY		0x01

#define RT2880_REG(x)                   (*((volatile u32 *)(x)))

#define READ_BLOCK              64
#define RT2880_RSTCTRL_REG              (RALINK_SYSCTL_BASE+0x34)

#define RT2880_I2C_REG_BASE             (RALINK_I2C_BASE)
#define RT2880_I2C_CONFIG_REG           (RT2880_I2C_REG_BASE+0x00)
#define RT2880_I2C_CLKDIV_REG           (RT2880_I2C_REG_BASE+0x04)
#define RT2880_I2C_DEVADDR_REG          (RT2880_I2C_REG_BASE+0x08)
#define RT2880_I2C_ADDR_REG             (RT2880_I2C_REG_BASE+0x0C)
#define RT2880_I2C_DATAOUT_REG          (RT2880_I2C_REG_BASE+0x10)
#define RT2880_I2C_DATAIN_REG           (RT2880_I2C_REG_BASE+0x14)
#define RT2880_I2C_STATUS_REG           (RT2880_I2C_REG_BASE+0x18)
#define RT2880_I2C_STARTXFR_REG         (RT2880_I2C_REG_BASE+0x1C)
#define RT2880_I2C_BYTECNT_REG          (RT2880_I2C_REG_BASE+0x20)

/* I2C_CFG register bit field */
#define I2C_CFG_DEVADLEN_7              (6<<2)  /* 7 bits */
#define I2C_CFG_ADDRDIS                 (1<<1)  /* disable address transmission*/
#define I2C_CFG_DEVADDIS                (1<<0)  /* disable evice address transmission */

#define I2C_CFG_DEFAULT			(I2C_CFG_ADDRDIS | I2C_CFG_DEVADLEN_7)
#define CLKDIV_VALUE			333

/* Instruction codes */
#define READ_CMD        0x01
#define WRITE_CMD       0x00

struct i2c_algo_ralink_data {
	u32 ioaddr;
	wait_queue_head_t waitq;
	spinlock_t lock;
	int id;
};

#endif /* I2C_RALINK_H */
