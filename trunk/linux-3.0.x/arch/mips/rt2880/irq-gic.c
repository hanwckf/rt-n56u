/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     Interrupt routines for Ralink RT2880 solution
 *
 *  Copyright 2007 Ralink Inc. (bruce_chang@ralinktech.com.tw)
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 **************************************************************************
 * May 2007 Bruce Chang
 * Initial Release
 *
 * May 2009 Bruce Chang
 * support RT3883 PCIe
 *
 **************************************************************************
 */

#include <linux/init.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>
#include <linux/hardirq.h>
#include <linux/preempt.h>

#include <asm/irq_cpu.h>
#include <asm/mipsregs.h>
#include <asm/gic.h>
#include <asm/gcmpregs.h>

#include <asm/irq.h>
#include <asm/rt2880/surfboard.h>
#include <asm/rt2880/surfboardint.h>
#include <asm/rt2880/rt_mmap.h>
#include <asm/rt2880/eureka_ep430.h>

int gic_present;
int gcmp_present = -1;
static unsigned long _gcmp_base;
static unsigned int ipi_map[NR_CPUS];

/*
 * This GIC specific tabular array defines the association between External
 * Interrupts and CPUs/Core Interrupts. The nature of the External
 * Interrupts is also defined here - polarity/trigger.
 */
#define X GIC_UNUSED
static struct gic_intr_map gic_intr_map[GIC_NUM_INTRS] = {
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT }, //0
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT },
#if 0
	{ 0, GIC_CPU_INT4, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT }, //PCIE0
	{ 0, GIC_CPU_INT3, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT }, //FE
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT }, //USB3
#else
	{ X, X,            X,           X,              0                    },
	{ 0, GIC_CPU_INT3, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT }, //FE
	{ 0, GIC_CPU_INT4, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT }, //PCIE0
#endif

#if defined (CONFIG_RALINK_SYSTICK_COUNTER)
	{ 0, GIC_CPU_INT5, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT }, //5, aux timer(system tick)
#else
	{ X, X,            X,           X,              0                    }, //5
#endif
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT },
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT },
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT },
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT },

	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT }, //10
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT },
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT },
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT },
	{ X, X,            X,           X,              0                    }, //14: NFI

	{ X, X,            X,           X,              0                    }, //15
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT },
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT },
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT },
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT },

	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT }, //20
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT },
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT },
	{ X, X,            X,           X,              0                    }, //23 : FIXME
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT },

	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT }, //25
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT },
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT },
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT },
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT },

	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT }, //30
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT },

	/* The remainder of this table is initialised by fill_ipi_map */
};
#undef X

/*
 * GCMP needs to be detected before any SMP initialisation
 */
int __init gcmp_probe(unsigned long addr, unsigned long size)
{
	if (gcmp_present >= 0)
		return gcmp_present;

	_gcmp_base = (unsigned long) ioremap_nocache(GCMP_BASE_ADDR, GCMP_ADDRSPACE_SZ);
	gcmp_present = (GCMPGCB(GCMPB) & GCMP_GCB_GCMPB_GCMPBASE_MSK) == GCMP_BASE_ADDR;

	if (gcmp_present)
		printk("GCMP present\n");

	return gcmp_present;
}


#if defined (CONFIG_MIPS_MT_SMP)

static int gic_call_int_base;
static int gic_resched_int_base;

#define GIC_CALL_INT(cpu)	(gic_call_int_base+(cpu))
#define GIC_RESCHED_INT(cpu)	(gic_resched_int_base+(cpu))

static irqreturn_t ipi_resched_interrupt(int irq, void *dev_id)
{
	scheduler_ipi();

	return IRQ_HANDLED;
}

#if defined (CONFIG_RALINK_SYSTICK_COUNTER)
extern spinlock_t ra_teststat_lock;
extern void ra_percpu_event_handler(void);

static irqreturn_t ipi_call_interrupt(int irq, void *dev_id)
{
	unsigned int cpu = smp_processor_id();
	unsigned int cd_event = 0;
	unsigned long flags;

	spin_lock_irqsave(&ra_teststat_lock, flags);

	cd_event = (*( (volatile u32 *)(RALINK_TESTSTAT))) & ((0x1UL) << cpu);
	if (cd_event)
		(*((volatile u32 *)(RALINK_TESTSTAT))) &= ~cd_event;

	spin_unlock_irqrestore(&ra_teststat_lock, flags);

	// FIXME!!!
	if (cd_event)
		ra_percpu_event_handler();

	smp_call_function_interrupt();

	return IRQ_HANDLED;
}
#else
static irqreturn_t ipi_call_interrupt(int irq, void *dev_id)
{
	smp_call_function_interrupt();

	return IRQ_HANDLED;
}
#endif

static struct irqaction irq_resched = {
	.handler	= ipi_resched_interrupt,
	.flags		= IRQF_PERCPU,
	.name		= "ipi_resched"
};

static struct irqaction irq_call = {
	.handler	= ipi_call_interrupt,
	.flags		= IRQF_PERCPU,
	.name		= "ipi_call"
};

static void __init fill_ipi_map1(int baseintr, int cpu, int cpupin)
{
	int intr = baseintr + cpu;
	gic_intr_map[intr].cpunum = cpu;
	gic_intr_map[intr].pin = cpupin;
	gic_intr_map[intr].polarity = GIC_POL_POS;
	gic_intr_map[intr].trigtype = GIC_TRIG_EDGE;
	gic_intr_map[intr].flags = GIC_FLAG_IPI;
	ipi_map[cpu] |= (1 << (cpupin + 2));
}

static void __init fill_ipi_map(void)
{
	int cpu;

	for (cpu = 0; cpu < NR_CPUS; cpu++) {
		fill_ipi_map1(gic_resched_int_base, cpu, GIC_CPU_INT1);
		fill_ipi_map1(gic_call_int_base, cpu, GIC_CPU_INT2);
	}
}

void __init arch_init_ipiirq(int irq, struct irqaction *action)
{
	setup_irq(irq, action);
	irq_set_handler(irq, handle_percpu_irq);
}

unsigned int plat_ipi_call_int_xlate(unsigned int cpu)
{
	return GIC_CALL_INT(cpu);
}

unsigned int plat_ipi_resched_int_xlate(unsigned int cpu)
{
	return GIC_RESCHED_INT(cpu);
}
#endif /* CONFIG_MIPS_MT_SMP */

unsigned int __cpuinit get_c0_compare_int(void)
{
	return SURFBOARDINT_MIPS_TIMER;
}

void gic_irq_ack(struct irq_data *d)
{
	int irq = (d->irq - gic_irq_base);

	GIC_CLR_INTR_MASK(irq);

	if (gic_irq_flags[irq] & GIC_TRIG_EDGE)
		GICWRITE(GIC_REG(SHARED, GIC_SH_WEDGE), irq);
}

void gic_finish_irq(struct irq_data *d)
{
	/* Enable interrupts. */
	GIC_SET_INTR_MASK(d->irq - gic_irq_base);
}

void __init gic_platform_init(int irqs, struct irq_chip *irq_controller)
{
	int i;

	for (i = gic_irq_base; i < (gic_irq_base + irqs); i++)
		irq_set_chip(i, irq_controller);
}

void __init prom_init_irq(void)
{
	/* Early detection of CMP support */
	if (gcmp_probe(GCMP_BASE_ADDR, GCMP_ADDRSPACE_SZ))
		if (!register_cmp_smp_ops())
			return;

	if (!register_vsmp_smp_ops())
		return;
}

void __init arch_init_irq(void)
{
#if defined (CONFIG_MIPS_MT_SMP)
	int i;
#endif

	mips_cpu_irq_init();

	irq_set_handler(SURFBOARDINT_PCIE0, handle_percpu_irq);
	irq_set_handler(SURFBOARDINT_PCIE1, handle_percpu_irq);
	irq_set_handler(SURFBOARDINT_PCIE2, handle_percpu_irq);
	irq_set_handler(SURFBOARDINT_FE, handle_percpu_irq);
	irq_set_handler(SURFBOARDINT_USB, handle_percpu_irq);
	irq_set_handler(SURFBOARDINT_SYSCTL, handle_percpu_irq);
	irq_set_handler(SURFBOARDINT_DRAMC, handle_percpu_irq);
	irq_set_handler(SURFBOARDINT_PCM, handle_percpu_irq);
	irq_set_handler(SURFBOARDINT_HSGDMA, handle_percpu_irq);
	irq_set_handler(SURFBOARDINT_GPIO, handle_percpu_irq);
	irq_set_handler(SURFBOARDINT_DMA, handle_percpu_irq);
	irq_set_handler(SURFBOARDINT_NAND, handle_percpu_irq);
	irq_set_handler(SURFBOARDINT_I2S, handle_percpu_irq);
	irq_set_handler(SURFBOARDINT_SPI, handle_percpu_irq);
	irq_set_handler(SURFBOARDINT_SPDIF, handle_percpu_irq);
	irq_set_handler(SURFBOARDINT_CRYPTO, handle_percpu_irq);
	irq_set_handler(SURFBOARDINT_SDXC, handle_percpu_irq);
	irq_set_handler(SURFBOARDINT_PCTRL, handle_percpu_irq);
	irq_set_handler(SURFBOARDINT_ESW, handle_percpu_irq);
	irq_set_handler(SURFBOARDINT_UART_LITE1, handle_percpu_irq);
	irq_set_handler(SURFBOARDINT_UART_LITE2, handle_percpu_irq);
	irq_set_handler(SURFBOARDINT_UART_LITE3, handle_percpu_irq);
	irq_set_handler(SURFBOARDINT_NAND_ECC, handle_percpu_irq);
	irq_set_handler(SURFBOARDINT_I2C, handle_percpu_irq);
	irq_set_handler(SURFBOARDINT_WDG, handle_percpu_irq);
	irq_set_handler(SURFBOARDINT_TIMER0, handle_percpu_irq);
	irq_set_handler(SURFBOARDINT_TIMER1, handle_percpu_irq);

	if (gcmp_present) {
		GCMPGCB(GICBA) = GIC_BASE_ADDR | GCMP_GCB_GICBA_EN_MSK;
		gic_present = 1;
	}

	if (gic_present) {
#if defined (CONFIG_MIPS_MT_SMP)
		gic_call_int_base = GIC_NUM_INTRS - NR_CPUS;
		gic_resched_int_base = gic_call_int_base - NR_CPUS;
		fill_ipi_map();
#endif
		gic_init(GIC_BASE_ADDR, GIC_ADDRSPACE_SZ, gic_intr_map,
				ARRAY_SIZE(gic_intr_map), MIPS_GIC_IRQ_BASE);
#if defined (CONFIG_MIPS_MT_SMP)
		/* Argh.. this really needs sorting out.. */
		printk("CPU%d: status register was %08x\n", smp_processor_id(), read_c0_status());
		write_c0_status(read_c0_status() | STATUSF_IP3 | STATUSF_IP4);
		printk("CPU%d: status register now %08x\n", smp_processor_id(), read_c0_status());
		write_c0_status(0x1100dc00);
		printk("CPU%d: status register frc %08x\n", smp_processor_id(), read_c0_status());
		/* set up ipi interrupts */
		for (i = 0; i < NR_CPUS; i++) {
			arch_init_ipiirq(MIPS_GIC_IRQ_BASE + GIC_RESCHED_INT(i), &irq_resched);
			arch_init_ipiirq(MIPS_GIC_IRQ_BASE + GIC_CALL_INT(i), &irq_call);
		}
#endif
	}

	change_c0_status(ST0_IM, STATUSF_IP7 | STATUSF_IP4 | STATUSF_IP3 | STATUSF_IP2);
}

static inline void gic_irqdispatch(void)
{
	int irq = gic_get_int();

	if (irq < 0)
		return;  /* interrupt has already been cleared */

	do_IRQ(MIPS_GIC_IRQ_BASE + irq);
}

asmlinkage void plat_irq_dispatch(void)
{
	unsigned int pending;

	pending = read_c0_status() & read_c0_cause() & ST0_IM;
	if (!pending) {
		spurious_interrupt();
		return;
	}

	if (pending & CAUSEF_IP7) {
		do_IRQ(cp0_compare_irq);	// CPU Timer
		return;
	}

	if (pending & (CAUSEF_IP4 | CAUSEF_IP3 | CAUSEF_IP2)) {
		gic_irqdispatch();
	}
}
