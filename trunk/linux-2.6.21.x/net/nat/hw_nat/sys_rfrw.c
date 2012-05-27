/*
    Module Name:
    sys_rfrw.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Steven Liu  2011-05-02      Initial version
*/

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/types.h>
#include "sys_rfrw.h"
#include "util.h"

int rw_rf_reg(int write, int reg, int *data)
{
	unsigned long rfcsr, i = 0;

	while (1) {
		rfcsr = RegRead(RF_CSR_CFG);
		if (!(rfcsr & (u32) RF_CSR_KICK))
			break;
		if (++i > 10000) {
			printk("Warning: Abort rw rf register: too busy\n");
			return -1;
		}
	}

	rfcsr = (u32) (RF_CSR_KICK | ((reg & 0x3f) << 8) | (*data & 0xff));
	if (write)
		rfcsr |= 0x10000;

	RegRead(RF_CSR_CFG) = cpu_to_le32(rfcsr);

	i = 0;
	while (1) {
		rfcsr = RegRead(RF_CSR_CFG);
		if (!(rfcsr & (u32) RF_CSR_KICK))
			break;
		if (++i > 10000) {
			printk("Warning: still busy\n");
			return -1;
		}
	}

	rfcsr = RegRead(RF_CSR_CFG);

	if (((rfcsr & 0x1f00) >> 8) != (reg & 0x1f)) {
		printk("Error: rw register failed\n");
		return -1;
	}
	*data = (int)(rfcsr & 0xff);

	return 0;
}
