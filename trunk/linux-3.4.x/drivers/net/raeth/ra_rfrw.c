#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/sched.h>

#include "ra_eth_reg.h"

#define RF_CSR_CFG		0xb0180500
#define RF_CSR_KICK		(1<<17)

int rw_rf_reg(int write, int reg, int *data)
{
	u32 rfcsr, i = 0;

	while (1) {
		rfcsr = sysRegRead(RF_CSR_CFG);
		if (! (rfcsr & (u32)RF_CSR_KICK) )
			break;
		if (++i > 10000) {
			printk("Warning: Abort rw rf register: too busy\n");
			return -1;
		}
	}

	rfcsr = (u32)(RF_CSR_KICK | ((reg&0x3f) << 8) | (*data & 0xff));
	if (write)
		rfcsr |= 0x10000;

	sysRegRead(RF_CSR_CFG) = cpu_to_le32(rfcsr);

	i = 0;
	while (1) {
		rfcsr = sysRegRead(RF_CSR_CFG);
		if (! (rfcsr & (u32)RF_CSR_KICK) )
			break;
		if (++i > 10000) {
			printk("Warning: still busy\n");
			return -1;
		}
	}

	rfcsr = sysRegRead(RF_CSR_CFG);

	if (((rfcsr&0x1f00) >> 8) != (reg & 0x1f)) {
		printk("Error: rw register failed\n");
		return -1;
	}

	*data = (int)(rfcsr & 0xff);

	return 0;
}

EXPORT_SYMBOL(rw_rf_reg);
