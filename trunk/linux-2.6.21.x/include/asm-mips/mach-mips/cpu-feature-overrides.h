/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2003, 2004 Chris Dearman
 * Copyright (C) 2005 Ralf Baechle (ralf@linux-mips.org)
 */

#include <linux/config.h>

#ifndef __ASM_MACH_MIPS_CPU_FEATURE_OVERRIDES_H
#define __ASM_MACH_MIPS_CPU_FEATURE_OVERRIDES_H

/*
 * CPU feature overrides for MIPS boards
 */
#if defined (CONFIG_RALINK_RT2880) || \
    defined (CONFIG_RALINK_RT2883) || \
    defined (CONFIG_RALINK_RT3883) || \
    defined (CONFIG_RALINK_RT3352) || \
    defined (CONFIG_RALINK_RT3052) || \
    defined (CONFIG_RALINK_RT6855) || \
    defined (CONFIG_RALINK_RT5350)

/*
 * FOR Ralink only cpus
 */
#define cpu_has_tlb		1
#define cpu_has_4kex		1
#define cpu_has_3k_cache	0
#define cpu_has_4k_cache	1
#define cpu_has_tx39_cache	0
#define cpu_has_sb1_cache	0
#define cpu_has_fpu		0
#define cpu_has_32fpr		0
#define cpu_has_counter		1
#define cpu_has_watch		1
#define cpu_has_divec		1

#define cpu_has_prefetch	1
#define cpu_has_ejtag		1
#define cpu_has_llsc		1

#define cpu_has_mips16		1
#define cpu_has_mdmx		0
#define cpu_has_mips3d		0
#define cpu_has_smartmips	0

#define cpu_has_mips32r1	1
#define cpu_has_mips32r2	1
#define cpu_has_mips64r1	0
#define cpu_has_mips64r2	0

#define cpu_has_dsp		1
#define cpu_has_mipsmt		0

#define cpu_has_veic		1
#define cpu_has_vint		1

#define cpu_has_64bits		0
#define cpu_has_64bit_zero_reg	0
#define cpu_has_64bit_gp_regs	0
#define cpu_has_64bit_addresses	0

#ifdef CONFIG_RALINK_RT2880
#define cpu_dcache_line_size()	16
#define cpu_icache_line_size()	16
#else
#define cpu_dcache_line_size()	32
#define cpu_icache_line_size()	32
#endif

#else

/*
 * FOR all cpus
 */
#ifdef CONFIG_CPU_MIPS32
#define cpu_has_tlb		1
#define cpu_has_4kex		1
#define cpu_has_4k_cache	1
/* #define cpu_has_fpu		? */
/* #define cpu_has_32fpr	? */
#define cpu_has_counter		1
/* #define cpu_has_watch	? */
#define cpu_has_divec		1
#define cpu_has_vce		0
/* #define cpu_has_cache_cdex_p	? */
/* #define cpu_has_cache_cdex_s	? */
/* #define cpu_has_prefetch	? */
#define cpu_has_mcheck		1
/* #define cpu_has_ejtag	? */
#ifdef CONFIG_CPU_HAS_LLSC
#define cpu_has_llsc		1
#else
#define cpu_has_llsc		0
#endif
/* #define cpu_has_vtag_icache	? */
/* #define cpu_has_dc_aliases	? */
/* #define cpu_has_ic_fills_f_dc ? */
#define cpu_has_nofpuex		0
/* #define cpu_has_64bits	? */
/* #define cpu_has_64bit_zero_reg ? */
/* #define cpu_has_inclusive_pcaches ? */
#define cpu_icache_snoops_remote_store 1
#endif

#ifdef CONFIG_CPU_MIPS64
#define cpu_has_tlb		1
#define cpu_has_4kex		1
#define cpu_has_4k_cache	1
/* #define cpu_has_fpu		? */
/* #define cpu_has_32fpr	? */
#define cpu_has_counter		1
/* #define cpu_has_watch	? */
#define cpu_has_divec		1
#define cpu_has_vce		0
/* #define cpu_has_cache_cdex_p	? */
/* #define cpu_has_cache_cdex_s	? */
/* #define cpu_has_prefetch	? */
#define cpu_has_mcheck		1
/* #define cpu_has_ejtag	? */
#define cpu_has_llsc		1
/* #define cpu_has_vtag_icache	? */
/* #define cpu_has_dc_aliases	? */
/* #define cpu_has_ic_fills_f_dc ? */
#define cpu_has_nofpuex		0
/* #define cpu_has_64bits	? */
/* #define cpu_has_64bit_zero_reg ? */
/* #define cpu_has_inclusive_pcaches ? */
#define cpu_icache_snoops_remote_store 1
#endif

#endif /* __ASM_MACH_MIPS_CPU_FEATURE_OVERRIDES_H */
