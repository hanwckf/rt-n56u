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

#include <asm/setup.h>
#include <asm/irq_cpu.h>
#include <asm/mipsregs.h>
#include <asm/gcmpregs.h>
#include <asm/gic.h>

#include <asm/rt2880/rt_mmap.h>

static int gcmp_present;
unsigned long _gcmp_base;

/*
 * This GIC specific tabular array defines the association between External
 * Interrupts and CPUs/Core Interrupts. The nature of the External
 * Interrupts is also defined here - polarity/trigger.
 */
#define X GIC_UNUSED
static struct gic_intr_map gic_intr_map[GIC_NUM_INTRS] = {
	{ X, X,            X,           X,              0            },		// 00: N/A
	{ X, X,            X,           X,              0            },		// 01: N/A
	{ X, X,            X,           X,              0            },		// 02: N/A
	{ 0, GIC_CPU_INT3, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },		// 03: FE
	{ 0, GIC_CPU_INT4, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },		// 04: PCIe0
#if defined (CONFIG_RALINK_SYSTICK_COUNTER)
	{ 0, GIC_CPU_INT5, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_TRANSPARENT },	// 05: AUX timer (system tick)
#else
	{ X, X,            X,           X,              0            },		// 05: N/A
#endif
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },		// 06: SYSCTL
	{ X, X,            X,           X,              0            },		// 07: MIPS timer
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },		// 08: I2C
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },		// 09: DRAMC
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },		// 10: PCM
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },		// 11: HS GDMA
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },		// 12: GPIO
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },		// 13: GDMA
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_EDGE,  GIC_FLAG_IPI },		// 14: NFI NAND (use edge interrupt)
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_EDGE,  GIC_FLAG_IPI },		// 15: NFI ECC (use edge interrupt)
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },		// 16: I2S
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },		// 17: SPI
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },		// 18: SPDIF
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },		// 19: CryptoEngine
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },		// 20: SDXC
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },		// 21: Rbus to Pbus
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },		// 22: USB XHCI
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_EDGE,  GIC_FLAG_IPI },		// 23: ESW MT7530 (use edge interrupt)
	{ 0, GIC_CPU_INT4, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },		// 24: PCIe1
	{ 0, GIC_CPU_INT4, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },		// 25: PCIe2
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },		// 26: UART 1
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },		// 27: UART 2
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },		// 28: UART 3
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },		// 29: Timer WDG
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },		// 30: Timer0
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_LEVEL, GIC_FLAG_IPI },		// 31: Timer1

	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_EDGE,  GIC_FLAG_IPI },		// 32: PCIE_P0_LINT_DOWN_RST
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_EDGE,  GIC_FLAG_IPI },		// 33: PCIE_P1_LINT_DOWN_RST
	{ 0, GIC_CPU_INT0, GIC_POL_POS, GIC_TRIG_EDGE,  GIC_FLAG_IPI },		// 34: PCIE_P2_LINT_DOWN_RST
	/* The remainder of this table is initialised by fill_ipi_map */
};
#undef X

/*
 * GCMP needs to be detected before any SMP initialisation
 */
int __init gcmp_probe(unsigned long addr, unsigned long size)
{
	_gcmp_base = (unsigned long) ioremap_nocache(GCMP_BASE_ADDR, GCMP_ADDRSPACE_SZ);
	gcmp_present = (GCMPGCB(GCMPB) & GCMP_GCB_GCMPB_GCMPBASE_MSK) == GCMP_BASE_ADDR;

	return gcmp_present;
}

/* Return the number of IOCU's present */
int __init gcmp_niocu(void)
{
	return (gcmp_present) ?
		(GCMPGCB(GC) & GCMP_GCB_GC_NUMIOCU_MSK) >> GCMP_GCB_GC_NUMIOCU_SHF : 0;
}

#if defined (CONFIG_MIPS_GIC_IPI)

static int gic_call_int_base;
static int gic_resched_int_base;
static unsigned int ipi_map[NR_CPUS];

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

	for (cpu = 0; cpu < nr_cpu_ids; cpu++) {
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
#endif /* CONFIG_MIPS_GIC_IPI */

static int mips_cpu_timer_irq = CP0_LEGACY_COMPARE_IRQ;

static void mips_timer_dispatch(void)
{
	do_IRQ(mips_cpu_timer_irq);
}

unsigned int __cpuinit get_c0_compare_int(void)
{
	mips_cpu_timer_irq = MIPS_CPU_IRQ_BASE + cp0_compare_irq;

	return mips_cpu_timer_irq;
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
	if (gcmp_probe(GCMP_BASE_ADDR, GCMP_ADDRSPACE_SZ)) {
#if defined (CONFIG_MIPS_CMP)
		if (!register_cmp_smp_ops())
			return;
#endif
	}

#if defined (CONFIG_MIPS_MT_SMP)
	if (!register_vsmp_smp_ops())
		return;
#endif
}

void __init arch_init_irq(void)
{
	int i;
	unsigned int gic_rev;

	mips_cpu_irq_init();

	if (cpu_has_vint)
		set_vi_handler(cp0_compare_irq, mips_timer_dispatch);

	if (gcmp_present) {
		GCMPGCB(GICBA) = GIC_BASE_ADDR | GCMP_GCB_GICBA_EN_MSK;
		gic_present = 1;
	}

	if (gic_present) {
#if defined (CONFIG_MIPS_GIC_IPI)
		gic_call_int_base = GIC_IPI_CALL_VPE0;
		gic_resched_int_base = GIC_IPI_RESCHED_VPE0;
		fill_ipi_map();
#endif
		gic_init(GIC_BASE_ADDR, GIC_ADDRSPACE_SZ, gic_intr_map,
				ARRAY_SIZE(gic_intr_map), MIPS_GIC_IRQ_BASE);

		GICREAD(GIC_REG(SHARED, GIC_SH_REVISIONID), gic_rev);
		printk("MIPS GIC RevID: %d.%d\n", (gic_rev >> 8) & 0xff, gic_rev & 0xff);

		if (cpu_has_vint) {
			pr_info("Setting up vectored interrupts\n");
			set_vi_handler(2 + GIC_CPU_INT0, gic_irq_dispatch);	// CPU
#if defined (CONFIG_MIPS_GIC_IPI)
			set_vi_handler(2 + GIC_CPU_INT1, gic_irq_dispatch);	// IPI resched
			set_vi_handler(2 + GIC_CPU_INT2, gic_irq_dispatch);	// IPI call
#endif
			set_vi_handler(2 + GIC_CPU_INT3, gic_irq_dispatch);	// FE
			set_vi_handler(2 + GIC_CPU_INT4, gic_irq_dispatch);	// PCIe
		}

#if defined (CONFIG_MIPS_GIC_IPI)
		set_c0_status(STATUSF_IP7 | STATUSF_IP6 | STATUSF_IP5 | STATUSF_IP2 |
			      STATUSF_IP4 | STATUSF_IP3);
		
		/* setup ipi interrupts */
		for (i = 0; i < nr_cpu_ids; i++) {
			arch_init_ipiirq(MIPS_GIC_IRQ_BASE + GIC_RESCHED_INT(i), &irq_resched);
			arch_init_ipiirq(MIPS_GIC_IRQ_BASE + GIC_CALL_INT(i), &irq_call);
		}
#else
		set_c0_status(STATUSF_IP7 | STATUSF_IP6 | STATUSF_IP5 | STATUSF_IP2);
#endif
		/* set hardware irq, mapped to GIC shared (skip 0, 1, 2, 5, 7) */
		for (i = 3; i <= 31; i++) {
			if (i != 5 && i != 7)
				irq_set_handler(MIPS_GIC_IRQ_BASE + i, handle_level_irq);
		}
		
	} else {
		/* Hardware without GCMP/GIC, not applicable for MT7621 */
		BUG();
	}
}

asmlinkage void plat_irq_dispatch(void)
{
	unsigned int pending;

	pending = read_c0_status() & read_c0_cause() & ST0_IM;
	if (unlikely(!pending)) {
		spurious_interrupt();
		return;
	}

	if (pending & CAUSEF_IP7)
		mips_timer_dispatch();

	if (pending & (CAUSEF_IP6 | CAUSEF_IP5 | CAUSEF_IP4 | CAUSEF_IP3 | CAUSEF_IP2))
		gic_irq_dispatch();
}
