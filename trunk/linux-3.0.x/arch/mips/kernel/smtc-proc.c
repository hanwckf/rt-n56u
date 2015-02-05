/*
 * /proc hooks for SMTC kernel
 * Copyright (C) 2005 Mips Technologies, Inc
 */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/cpumask.h>
#include <linux/interrupt.h>

#include <asm/cpu.h>
#include <asm/processor.h>
#include <asm/atomic.h>
#include <asm/system.h>
#include <asm/hardirq.h>
#include <asm/mmu_context.h>
#include <asm/mipsregs.h>
#include <asm/cacheflush.h>
#include <linux/proc_fs.h>

#include <asm/smtc_proc.h>

/*
 * /proc diagnostic and statistics hooks
 */

/*
 * Statistics gathered
 */
unsigned long selfipis[NR_CPUS];

struct smtc_cpu_proc smtc_cpu_stats[NR_CPUS];

static struct proc_dir_entry *smtc_stats;

atomic_t smtc_fpu_recoveries;

static int proc_read_smtc(char *page, char **start, off_t off,
                          int count, int *eof, void *data)
{
	int totalen = 0;
	int len;
	int i;
	extern unsigned long ebase;

	len = sprintf(page, "SMTC Status Word: 0x%08x\n", smtc_status);
	totalen += len;
	page += len;
	len = sprintf(page, "Config7: 0x%08x\n", read_c0_config7());
	totalen += len;
	page += len;
	len = sprintf(page, "EBASE: 0x%08lx\n", ebase);
	totalen += len;
	page += len;
	len = sprintf(page, "Counter Interrupts taken per CPU (TC)\n");
	totalen += len;
	page += len;
	for (i=0; i < NR_CPUS; i++) {
		len = sprintf(page, "%d: %ld\n", i, smtc_cpu_stats[i].timerints);
		totalen += len;
		page += len;
	}
	len = sprintf(page, "Self-IPIs by CPU:\n");
	totalen += len;
	page += len;
	for(i = 0; i < NR_CPUS; i++) {
		len = sprintf(page, "%d: %ld\n", i, smtc_cpu_stats[i].selfipis);
		totalen += len;
		page += len;
	}
	len = sprintf(page, "%d Recoveries of \"stolen\" FPU\n",
	              atomic_read(&smtc_fpu_recoveries));
	totalen += len;
	page += len;

	return totalen;
}

void init_smtc_stats(void)
{
	int i;

	for (i=0; i<NR_CPUS; i++) {
		smtc_cpu_stats[i].timerints = 0;
		smtc_cpu_stats[i].selfipis = 0;
	}

	atomic_set(&smtc_fpu_recoveries, 0);

	smtc_stats = create_proc_read_entry("smtc", 0444, NULL,
	                                    proc_read_smtc, NULL);
}

static int proc_cpuinfo_chain_call(struct notifier_block *nfb,
	unsigned long action_unused, void *data)
{
	struct proc_cpuinfo_notifier_args *pcn = data;
	struct seq_file *m = pcn->m;
	unsigned long n = pcn->n;

	if (!cpu_has_mipsmt)
		return NOTIFY_OK;

	seq_printf(m, "VPE\t\t\t: %d\n", cpu_data[n].vpe_id);
	seq_printf(m, "TC\t\t\t: %d\n", cpu_data[n].tc_id);

	return NOTIFY_OK;
}

static int __init proc_cpuinfo_notifier_init(void)
{
	return proc_cpuinfo_notifier(proc_cpuinfo_chain_call, 0);
}

subsys_initcall(proc_cpuinfo_notifier_init);
