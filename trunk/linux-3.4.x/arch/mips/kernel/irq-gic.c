/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2008 Ralf Baechle (ralf@linux-mips.org)
 * Copyright (C) 2012 MIPS Technologies, Inc.  All rights reserved.
 */
#include <linux/bitmap.h>
#include <linux/init.h>
#include <linux/smp.h>
#include <linux/irq.h>
#include <linux/clocksource.h>

#include <asm/io.h>
#include <asm/gic.h>
#include <asm/gcmpregs.h>
#include <linux/hardirq.h>
#include <asm-generic/bitops/find.h>

unsigned int gic_frequency;
unsigned int gic_present;
unsigned long _gic_base;
unsigned int gic_irq_base;
unsigned int gic_irq_flags[GIC_NUM_INTRS];

struct gic_pcpu_mask {
	DECLARE_BITMAP(pcpu_mask, GIC_NUM_INTRS);
};

static struct gic_pcpu_mask pcpu_masks[NR_CPUS];

#if defined(CONFIG_CSRC_GIC) || defined(CONFIG_CEVT_GIC)
cycle_t gic_read_count(void)
{
	unsigned int hi, hi2, lo;

	do {
		GICREAD(GIC_REG(SHARED, GIC_SH_COUNTER_63_32), hi);
		GICREAD(GIC_REG(SHARED, GIC_SH_COUNTER_31_00), lo);
		GICREAD(GIC_REG(SHARED, GIC_SH_COUNTER_63_32), hi2);
	} while (hi2 != hi);

	return (((cycle_t) hi) << 32) + lo;
}

unsigned int gic_get_count_width(void)
{
	unsigned int bits, config;

	GICREAD(GIC_REG(SHARED, GIC_SH_CONFIG), config);
	bits = 32 + 4 * ((config & GIC_SH_CONFIG_COUNTBITS_MSK) >>
			 GIC_SH_CONFIG_COUNTBITS_SHF);

	return bits;
}

void gic_write_compare(cycle_t cnt)
{
	GICWRITE(GIC_REG(VPE_LOCAL, GIC_VPE_COMPARE_HI),
				(int)(cnt >> 32));
	GICWRITE(GIC_REG(VPE_LOCAL, GIC_VPE_COMPARE_LO),
				(int)(cnt & 0xffffffff));
}

void gic_write_cpu_compare(cycle_t cnt, int cpu)
{
	unsigned long flags;

	local_irq_save(flags);

	GICWRITE(GIC_REG(VPE_LOCAL, GIC_VPE_OTHER_ADDR), cpu);
	GICWRITE(GIC_REG(VPE_OTHER, GIC_VPE_COMPARE_HI),
				(int)(cnt >> 32));
	GICWRITE(GIC_REG(VPE_OTHER, GIC_VPE_COMPARE_LO),
				(int)(cnt & 0xffffffff));

	local_irq_restore(flags);
}

cycle_t gic_read_compare(void)
{
	unsigned int hi, lo;

	GICREAD(GIC_REG(VPE_LOCAL, GIC_VPE_COMPARE_HI), hi);
	GICREAD(GIC_REG(VPE_LOCAL, GIC_VPE_COMPARE_LO), lo);

	return (((cycle_t) hi) << 32) + lo;
}

void gic_start_count(void)
{
	unsigned int gicconfig;

	/* Start the counter */
	GICREAD(GIC_REG(SHARED, GIC_SH_CONFIG), gicconfig);
	gicconfig &= ~(1 << GIC_SH_CONFIG_COUNTSTOP_SHF);
	GICWRITE(GIC_REG(SHARED, GIC_SH_CONFIG), gicconfig);
}

void gic_stop_count(void)
{
	unsigned int gicconfig;

	/* Stop the counter */
	GICREAD(GIC_REG(SHARED, GIC_SH_CONFIG), gicconfig);
	gicconfig |= 1 << GIC_SH_CONFIG_COUNTSTOP_SHF;
	GICWRITE(GIC_REG(SHARED, GIC_SH_CONFIG), gicconfig);
}
#endif

void gic_send_ipi(unsigned int intr)
{
	GICWRITE(GIC_REG(SHARED, GIC_SH_WEDGE), GIC_SH_WEDGE_SET(intr));
}

static void __init vpe_local_setup(unsigned int numvpes)
{
	unsigned long timer_interrupt = GIC_INT_TMR;
	unsigned long perf_interrupt = GIC_INT_PERFCTR;
	unsigned int vpe_ctl;
	int i;

#if defined (CONFIG_CEVT_GIC)
	GICWRITE(GIC_REG(VPE_LOCAL, GIC_VPE_COMPARE_MAP),
		GIC_MAP_TO_PIN_MSK | GIC_CPU_INT0);
#else
	/* Program MIPS GIC to turn off(mask) local compare interrupt. */
	GICWRITE(GIC_REG(VPE_LOCAL, GIC_VPE_RMASK), GIC_VPE_RMASK_CMP_MSK);
#endif

	/*
	 * Setup the default performance counter timer interrupts
	 * for all VPEs
	 */
	for (i = 0; i < numvpes; i++) {
		GICWRITE(GIC_REG(VPE_LOCAL, GIC_VPE_OTHER_ADDR), i);

		/* Are Interrupts locally routable? */
		GICREAD(GIC_REG(VPE_OTHER, GIC_VPE_CTL), vpe_ctl);
		if (vpe_ctl & GIC_VPE_CTL_TIMER_RTBL_MSK)
			GICWRITE(GIC_REG(VPE_OTHER, GIC_VPE_TIMER_MAP),
				 GIC_MAP_TO_PIN_MSK | timer_interrupt);

		if (vpe_ctl & GIC_VPE_CTL_PERFCNT_RTBL_MSK)
			GICWRITE(GIC_REG(VPE_OTHER, GIC_VPE_PERFCTR_MAP),
				 GIC_MAP_TO_PIN_MSK | perf_interrupt);

#if defined (CONFIG_CEVT_GIC) || defined (CONFIG_RALINK_SYSTICK_COUNTER)
		/* Program MIPS GIC to turn off(mask) each VPE's local timer interrupt. */
		GICWRITE(GIC_REG(VPE_OTHER, GIC_VPE_RMASK), GIC_VPE_RMASK_TIMER_MSK);
#else
		/* Program MIPS GIC to turn on(unmask) each VPE's local timer interrupt. */
		GICWRITE(GIC_REG(VPE_OTHER, GIC_VPE_SMASK), GIC_VPE_SMASK_TIMER_MSK);
#endif
	}
}

#if defined (CONFIG_CEVT_GIC)
static inline unsigned int gic_compare_int(void)
{
	unsigned int pending_local = 0;

	GICREAD(GIC_REG(VPE_LOCAL, GIC_VPE_PEND), pending_local);
	if (pending_local & GIC_VPE_PEND_CMP_MSK)
		return 1;
	else
		return 0;
}
#endif

void gic_irq_dispatch(void)
{
	unsigned int i, intr;
	unsigned long *pcpu_mask;
	unsigned long *pending_abs, *intrmask_abs;
	DECLARE_BITMAP(pending, GIC_NUM_INTRS);
	DECLARE_BITMAP(intrmask, GIC_NUM_INTRS);

#if defined (CONFIG_CEVT_GIC)
	if (gic_compare_int())
		do_IRQ(gic_irq_base + MIPS_GIC_LOCAL_INT_COMPARE);
#endif

	/* Get per-cpu bitmaps */
	pcpu_mask = pcpu_masks[smp_processor_id()].pcpu_mask;

	pending_abs = (unsigned long *) GIC_REG_ABS_ADDR(SHARED,
							 GIC_SH_PEND_31_0_OFS);
	intrmask_abs = (unsigned long *) GIC_REG_ABS_ADDR(SHARED,
							  GIC_SH_MASK_31_0_OFS);

	for (i = 0; i < BITS_TO_LONGS(GIC_NUM_INTRS); i++) {
		GICREAD(*pending_abs, pending[i]);
		GICREAD(*intrmask_abs, intrmask[i]);
		pending_abs++;
		intrmask_abs++;
	}

	bitmap_and(pending, pending, intrmask, GIC_NUM_INTRS);
	bitmap_and(pending, pending, pcpu_mask, GIC_NUM_INTRS);

	intr = find_first_bit(pending, GIC_NUM_INTRS);
	while (intr < GIC_NUM_INTRS) {
		do_IRQ(gic_irq_base + intr);
		
		/* go to next pending bit */
		bitmap_clear(pending, intr, 1);
		intr = find_first_bit(pending, GIC_NUM_INTRS);
	}
}

static void gic_mask_irq(struct irq_data *d)
{
#if defined (CONFIG_CEVT_GIC)
	/* handle GIC local compare */
	if (d->irq == (gic_irq_base + MIPS_GIC_LOCAL_INT_COMPARE)) {
		GICWRITE(GIC_REG(VPE_LOCAL, GIC_VPE_RMASK), GIC_VPE_RMASK_CMP_MSK);
		return;
	}
#endif

	GIC_CLR_INTR_MASK(d->irq - gic_irq_base);
}

static void gic_unmask_irq(struct irq_data *d)
{
#if defined (CONFIG_CEVT_GIC)
	/* handle GIC local compare */
	if (d->irq == (gic_irq_base + MIPS_GIC_LOCAL_INT_COMPARE)) {
		GICWRITE(GIC_REG(VPE_LOCAL, GIC_VPE_SMASK), GIC_VPE_SMASK_CMP_MSK);
		return;
	}
#endif

	GIC_SET_INTR_MASK(d->irq - gic_irq_base);
}

static void gic_ack_irq(struct irq_data *d)
{
	int irq = (d->irq - gic_irq_base);

	/* Clear edge detector */
	if (gic_irq_flags[irq] & GIC_TRIG_EDGE)
		GICWRITE(GIC_REG(SHARED, GIC_SH_WEDGE), GIC_SH_WEDGE_CLR(irq));
}

#ifdef CONFIG_SMP
static DEFINE_SPINLOCK(gic_lock);

static int gic_set_affinity(struct irq_data *d, const struct cpumask *cpumask,
			    bool force)
{
	unsigned int irq = (d->irq - gic_irq_base);
	cpumask_t	tmp = CPU_MASK_NONE;
	unsigned long	flags;
	int		i;

	cpumask_and(&tmp, cpumask, cpu_online_mask);
	if (cpus_empty(tmp))
		return -EINVAL;

	/* Assumption : cpumask refers to a single CPU */
	spin_lock_irqsave(&gic_lock, flags);

	/* Re-route this IRQ */
	GIC_SH_MAP_TO_VPE_SMASK(irq, first_cpu(tmp));

	/* Update the pcpu_masks */
	for (i = 0; i < NR_CPUS; i++)
		clear_bit(irq, pcpu_masks[i].pcpu_mask);
	set_bit(irq, pcpu_masks[first_cpu(tmp)].pcpu_mask);

	cpumask_copy(d->affinity, cpumask);
	spin_unlock_irqrestore(&gic_lock, flags);

	return IRQ_SET_MASK_OK_NOCOPY;
}
#endif

static struct irq_chip gic_irq_controller = {
	.name			=	"MIPS GIC",
	.irq_ack		=	gic_ack_irq,
	.irq_mask		=	gic_mask_irq,
	.irq_unmask		=	gic_unmask_irq,
	.irq_disable		=	gic_mask_irq,
	.irq_enable		=	gic_unmask_irq,
#ifdef CONFIG_SMP
	.irq_set_affinity	=	gic_set_affinity,
#endif
};

static void __init gic_setup_intr(unsigned int intr, unsigned int cpu,
	unsigned int pin, unsigned int polarity, unsigned int trigtype,
	unsigned int flags)
{
	/* Setup Intr to Pin mapping */
	if (pin & GIC_MAP_TO_NMI_MSK) {
		int i;

		GICWRITE(GIC_REG_ADDR(SHARED, GIC_SH_MAP_TO_PIN(intr)), pin);
		/* FIXME: hack to route NMI to all cpu's */
		for (i = 0; i < NR_CPUS; i += 32) {
			GICWRITE(GIC_REG_ADDR(SHARED,
					  GIC_SH_MAP_TO_VPE_REG_OFF(intr, i)),
				 0xffffffff);
		}
	} else {
		GICWRITE(GIC_REG_ADDR(SHARED, GIC_SH_MAP_TO_PIN(intr)),
			 GIC_MAP_TO_PIN_MSK | pin);
		/* Setup Intr to CPU mapping */
		GIC_SH_MAP_TO_VPE_SMASK(intr, cpu);
	}

	/* Setup Intr Polarity */
	GIC_SET_POLARITY(intr, polarity);

	/* Setup Intr Trigger Type */
	GIC_SET_TRIGGER(intr, trigtype);

	/* Init Intr Masks */
	GIC_CLR_INTR_MASK(intr);

	/* Initialise per-cpu Interrupt software masks */
	if (flags & GIC_FLAG_IPI)
		set_bit(intr, pcpu_masks[cpu].pcpu_mask);
	if (flags & GIC_FLAG_TRANSPARENT)
		GIC_SET_INTR_MASK(intr);
	if (trigtype == GIC_TRIG_EDGE)
		gic_irq_flags[intr] |= GIC_TRIG_EDGE;
}

static void __init gic_basic_init(int numintrs, int numvpes,
			struct gic_intr_map *intrmap, int mapsize)
{
	unsigned int i, cpu;

	/* Setup defaults */
	for (i = 0; i < numintrs; i++) {
		GIC_SET_POLARITY(i, GIC_POL_POS);
		GIC_SET_TRIGGER(i, GIC_TRIG_LEVEL);
		GIC_CLR_INTR_MASK(i);
		if (i < GIC_NUM_INTRS)
			gic_irq_flags[i] = 0;
	}

	/* Setup specifics */
	for (i = 0; i < mapsize; i++) {
		cpu = intrmap[i].cpunum;
		if (cpu == GIC_UNUSED)
			continue;
		if (cpu == 0 && i != 0 && intrmap[i].flags == 0)
			continue;
		gic_setup_intr(i,
			intrmap[i].cpunum,
			intrmap[i].pin,
			intrmap[i].polarity,
			intrmap[i].trigtype,
			intrmap[i].flags);
	}

	vpe_local_setup(numvpes);
}

void __init gic_init(unsigned long gic_base_addr,
		     unsigned long gic_addrspace_size,
		     struct gic_intr_map *intr_map, unsigned int intr_map_size,
		     unsigned int irqbase)
{
	unsigned int gicconfig;
	int numvpes, numintrs;

	_gic_base = (unsigned long) ioremap_nocache(gic_base_addr,
						    gic_addrspace_size);
	gic_irq_base = irqbase;

	GICREAD(GIC_REG(SHARED, GIC_SH_CONFIG), gicconfig);
	numintrs = (gicconfig & GIC_SH_CONFIG_NUMINTRS_MSK) >>
		   GIC_SH_CONFIG_NUMINTRS_SHF;
	numintrs = ((numintrs + 1) * 8);

	numvpes = (gicconfig & GIC_SH_CONFIG_NUMVPES_MSK) >>
		  GIC_SH_CONFIG_NUMVPES_SHF;
	numvpes = numvpes + 1;

	gic_basic_init(numintrs, numvpes, intr_map, intr_map_size);

	gic_platform_init(numintrs, &gic_irq_controller);
}
