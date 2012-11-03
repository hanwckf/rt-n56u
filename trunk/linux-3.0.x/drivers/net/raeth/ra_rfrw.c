#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ptrace.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/signal.h>
#include <linux/irq.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>

#include "ra2882ethreg.h"
#include "raether.h"
#include "ra_mac.h"

#define RF_CSR_CFG      0xb0180500
#define RF_CSR_KICK     (1<<17)
int rw_rf_reg(int write, int reg, int *data)
{
        unsigned long    rfcsr, i = 0;

        while (1) {
                rfcsr =  sysRegRead(RF_CSR_CFG);
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
                rfcsr =  sysRegRead(RF_CSR_CFG);
                if (! (rfcsr & (u32)RF_CSR_KICK) )
                        break;
                if (++i > 10000) {
                        printk("Warning: still busy\n");
                        return -1;
                }
        }

        rfcsr =  sysRegRead(RF_CSR_CFG);

        if (((rfcsr&0x1f00) >> 8) != (reg & 0x1f)) {
                printk("Error: rw register failed\n");
                return -1;
        }
        *data = (int)(rfcsr & 0xff);

        return 0;
}

