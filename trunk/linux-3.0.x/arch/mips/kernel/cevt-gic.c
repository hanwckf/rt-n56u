/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2013  Imagination Technologies Ltd.
 */
#include <linux/clockchips.h>
#include <linux/interrupt.h>
#include <linux/percpu.h>
#include <linux/smp.h>
#include <linux/irq.h>

#include <asm/gic.h>

static DEFINE_PER_CPU(struct clock_event_device, gic_clockevent_device);
static int gic_timer_irq_installed;

static int gic_next_event(unsigned long delta, struct clock_event_device *evt)
{
	u64 cnt;
	int res;

	cnt = gic_read_count();
	cnt += (u64)delta;
	gic_write_cpu_compare(cnt, cpumask_first(evt->cpumask));
	res = ((int)(gic_read_count() - cnt) >= 0) ? -ETIME : 0;

	return res;
}

static void gic_set_clock_mode(enum clock_event_mode mode,
				struct clock_event_device *evt)
{
	/* Nothing to do ...  */
}

static irqreturn_t gic_compare_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *cd;
	int cpu = smp_processor_id();

	gic_write_compare(gic_read_compare());
	cd = &per_cpu(gic_clockevent_device, cpu);
	cd->event_handler(cd);

	return IRQ_HANDLED;
}

struct irqaction gic_compare_irqaction = {
	.handler = gic_compare_interrupt,
	.flags = IRQF_PERCPU | IRQF_TIMER,
	.name = "timer",
};

static void gic_event_handler(struct clock_event_device *dev)
{
}

int __cpuinit gic_clockevent_init(void)
{
	unsigned int cpu = smp_processor_id();
	struct clock_event_device *cd;
	unsigned int irq;

	if (!cpu_has_counter || !gic_frequency)
		return -ENXIO;

	irq = MIPS_GIC_IRQ_BASE + MIPS_GIC_LOCAL_INT_COMPARE;

	cd = &per_cpu(gic_clockevent_device, cpu);

	cd->name		= "MIPS GIC";
	cd->features		= CLOCK_EVT_FEAT_ONESHOT;

	cd->rating		= 350;
	cd->irq			= irq;
	cd->cpumask		= cpumask_of(cpu);
	cd->set_next_event	= gic_next_event;
	cd->set_mode		= gic_set_clock_mode;
	cd->event_handler	= gic_event_handler;

	clockevents_config_and_register(cd, gic_frequency, 0x300, 0x7fffffff);

	if (gic_timer_irq_installed)
		return 0;

	gic_timer_irq_installed = 1;

	setup_irq(irq, &gic_compare_irqaction);
	irq_set_handler(irq, handle_percpu_irq);

	return 0;
}
