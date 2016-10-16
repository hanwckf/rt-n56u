/*
 * Ralink specific CPU feature overrides
 *
 * This file was derived from: include/asm-mips/cpu-features.h
 *	Copyright (C) 2003, 2004 Ralf Baechle
 *	Copyright (C) 2004 Maciej W. Rozycki
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 */
#ifndef __ASM_MACH_MIPS_RT2880_CPU_FEATURE_OVERRIDES_H
#define __ASM_MACH_MIPS_RT2880_CPU_FEATURE_OVERRIDES_H

/* CPU options */
#define cpu_has_tlb			1
#define cpu_has_4kex			1
#define cpu_has_3k_cache		0
#define cpu_has_4k_cache		1
#define cpu_has_tx39_cache		0
#define cpu_has_fpu			0
#define cpu_has_32fpr			0
#define cpu_has_counter			1

#define cpu_has_watch			1
#define cpu_has_divec			1
#define cpu_has_vce			0
#define cpu_has_cache_cdex_p		0
#define cpu_has_cache_cdex_s		0
#define cpu_has_mcheck			1
#define cpu_has_ejtag			1
#define cpu_has_nofpuex			0

#define cpu_has_llsc			1
#define cpu_has_inclusive_pcaches	0
#define cpu_has_prefetch		1
//#define cpu_has_vint			1 // do not override, depend from CONFIG_CPU_MIPSR2_IRQ_VI
#define cpu_has_veic			0
#define cpu_has_userlocal		1

/* CPU ases */
#define cpu_has_mips16			1
#define cpu_has_mdmx			0
#define cpu_has_mips3d			0
#define cpu_has_smartmips		0
#define cpu_has_rixi			0
#define cpu_has_dsp			1
#if defined (CONFIG_RALINK_MT7621)
#define cpu_has_mipsmt			1
#else
#define cpu_has_mipsmt			0
#endif

/* CPU ISA level */
#define cpu_has_mips32r1		0
#define cpu_has_mips32r2		1
#define cpu_has_mips64r1		0
#define cpu_has_mips64r2		0

#define cpu_has_64bits			0
#define cpu_has_64bit_zero_reg		0
#define cpu_has_64bit_gp_regs		0
#define cpu_has_64bit_addresses		0

/* CPU cache info */
#define cpu_has_vtag_icache		0
#define cpu_has_ic_fills_f_dc		0
#define cpu_has_dc_aliases		0
#define cpu_dcache_line_size()		32
#define cpu_icache_line_size()		32

#endif
