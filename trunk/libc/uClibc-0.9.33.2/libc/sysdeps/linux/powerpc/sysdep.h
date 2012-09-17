/* Copyright (C) 1999, 2001, 2002, 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <common/sysdep.h>

/* 
 * Powerpc Feature masks for the Aux Vector Hardware Capabilities (AT_HWCAP). 
 * This entry is copied to _dl_hwcap or rtld_global._dl_hwcap during startup.
 * The following must match the kernels linux/asm/cputable.h.  
 */
#define PPC_FEATURE_32			0x80000000 /* 32-bit mode. */
#define PPC_FEATURE_64			0x40000000 /* 64-bit mode. */
#define PPC_FEATURE_601_INSTR		0x20000000 /* 601 chip, Old POWER ISA.  */
#define PPC_FEATURE_HAS_ALTIVEC		0x10000000 /* SIMD/Vector Unit.  */
#define PPC_FEATURE_HAS_FPU		0x08000000 /* Floating Point Unit.  */
#define PPC_FEATURE_HAS_MMU		0x04000000 /* Memory Management Unit.  */
#define PPC_FEATURE_HAS_4xxMAC		0x02000000 /* 4xx Multiply Accumulator.  */
#define PPC_FEATURE_UNIFIED_CACHE	0x01000000 /* Unified I/D cache.  */
#define PPC_FEATURE_HAS_SPE		0x00800000 /* Signal Processing ext.  */
#define PPC_FEATURE_HAS_EFP_SINGLE	0x00400000 /* SPE Float.  */
#define PPC_FEATURE_HAS_EFP_DOUBLE	0x00200000 /* SPE Double.  */
#define PPC_FEATURE_NO_TB		0x00100000 /* 601/403gx have no timebase */
#define PPC_FEATURE_POWER4		0x00080000 /* POWER4 ISA 2.00 */
#define PPC_FEATURE_POWER5		0x00040000 /* POWER5 ISA 2.02 */
#define PPC_FEATURE_POWER5_PLUS		0x00020000 /* POWER5+ ISA 2.03 */
#define PPC_FEATURE_CELL_BE		0x00010000 /* CELL Broadband Engine */
#define PPC_FEATURE_BOOKE		0x00008000
#define PPC_FEATURE_SMT			0x00004000 /* Simultaneous Multi-Threading */
#define PPC_FEATURE_ICACHE_SNOOP	0x00002000
#define PPC_FEATURE_ARCH_2_05		0x00001000 /* ISA 2.05 */
#define PPC_FEATURE_PA6T		0x00000800 /* PA Semi 6T Core */
#define PPC_FEATURE_HAS_DFP		0x00000400 /* Decimal FP Unit */
#define PPC_FEATURE_POWER6_EXT		0x00000200 /* P6 + mffgpr/mftgpr */
#define PPC_FEATURE_ARCH_2_06	        0x00000100 /* ISA 2.06 */
#define PPC_FEATURE_HAS_VSX		0x00000080 /* P7 Vector Extension.  */
#define PPC_FEATURE_970 (PPC_FEATURE_POWER4 + PPC_FEATURE_HAS_ALTIVEC)

#ifdef __ASSEMBLER__

/* Symbolic names for the registers.  The only portable way to write asm
   code is to use number but this produces really unreadable code.
   Therefore these symbolic names.  */

/* Integer registers.  */
#define r0	0
#define r1	1
#define r2	2
#define r3	3
#define r4	4
#define r5	5
#define r6	6
#define r7	7
#define r8	8
#define r9	9
#define r10	10
#define r11	11
#define r12	12
#define r13	13
#define r14	14
#define r15	15
#define r16	16
#define r17	17
#define r18	18
#define r19	19
#define r20	20
#define r21	21
#define r22	22
#define r23	23
#define r24	24
#define r25	25
#define r26	26
#define r27	27
#define r28	28
#define r29	29
#define r30	30
#define r31	31

/* Floating-point registers.  */
#define fp0	0
#define fp1	1
#define fp2	2
#define fp3	3
#define fp4	4
#define fp5	5
#define fp6	6
#define fp7	7
#define fp8	8
#define fp9	9
#define fp10	10
#define fp11	11
#define fp12	12
#define fp13	13
#define fp14	14
#define fp15	15
#define fp16	16
#define fp17	17
#define fp18	18
#define fp19	19
#define fp20	20
#define fp21	21
#define fp22	22
#define fp23	23
#define fp24	24
#define fp25	25
#define fp26	26
#define fp27	27
#define fp28	28
#define fp29	29
#define fp30	30
#define fp31	31

/* Condition code registers.  */
#define cr0	0
#define cr1	1
#define cr2	2
#define cr3	3
#define cr4	4
#define cr5	5
#define cr6	6
#define cr7	7

/* Vector registers. */
#define v0	0
#define v1	1
#define v2	2
#define v3	3
#define v4	4
#define v5	5
#define v6	6
#define v7	7
#define v8	8
#define v9	9
#define v10	10
#define v11	11
#define v12	12
#define v13	13
#define v14	14
#define v15	15
#define v16	16
#define v17	17
#define v18	18
#define v19	19
#define v20	20
#define v21	21
#define v22	22
#define v23	23
#define v24	24
#define v25	25
#define v26	26
#define v27	27
#define v28	28
#define v29	29
#define v30	30
#define v31	31

#define VRSAVE	256


#ifdef __ELF__

/* This seems to always be the case on PPC.  */
#define ALIGNARG(log2) log2
/* For ELF we need the `.type' directive to make shared libs work right.  */
#define ASM_TYPE_DIRECTIVE(name,typearg) .type name,typearg;
#define ASM_SIZE_DIRECTIVE(name) .size name,.-name

/* In ELF C symbols are asm symbols.  */
#undef	NO_UNDERSCORES
#define NO_UNDERSCORES

#endif /* __ELF__ */

# include <sys/syscall.h>
# if defined(__powerpc64__)
#  include "powerpc64/sysdep.h"
# else
#  include "powerpc32/sysdep.h"
# endif

#endif	/* __ASSEMBLER__ */

