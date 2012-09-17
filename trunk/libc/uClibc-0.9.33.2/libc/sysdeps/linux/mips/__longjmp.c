/* Copyright (C) 1992, 1995, 1997, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Brendan Kehoe (brendan@zen.org).

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

#include <features.h>
#include <setjmp.h>
#include <stdlib.h>
#include <sgidefs.h>

#ifndef	__GNUC__
  #error This file uses GNU C extensions; you must compile with GCC.
#endif

extern void __longjmp (__jmp_buf __env, int __val) attribute_noreturn;
libc_hidden_proto(__longjmp)

void __longjmp (__jmp_buf env, int val_arg)
{
    /* gcc 1.39.19 miscompiled the longjmp routine (as it did setjmp before
       the hack around it); force it to use $a1 for the longjmp value.
       Without this it saves $a1 in a register which gets clobbered
       along the way.  */
    register int val __asm__ ("a1");

    /* Pull back the floating point callee-saved registers.  */
#if defined __UCLIBC_HAS_FLOATS__ && ! defined __UCLIBC_HAS_SOFT_FLOAT__
#if _MIPS_SIM == _MIPS_SIM_ABI64
    __asm__ __volatile__ ("l.d $f24, %0" : : "m" (env[0].__fpregs[0]));
    __asm__ __volatile__ ("l.d $f25, %0" : : "m" (env[0].__fpregs[1]));
    __asm__ __volatile__ ("l.d $f26, %0" : : "m" (env[0].__fpregs[2]));
    __asm__ __volatile__ ("l.d $f27, %0" : : "m" (env[0].__fpregs[3]));
    __asm__ __volatile__ ("l.d $f28, %0" : : "m" (env[0].__fpregs[4]));
    __asm__ __volatile__ ("l.d $f29, %0" : : "m" (env[0].__fpregs[5]));
    __asm__ __volatile__ ("l.d $f30, %0" : : "m" (env[0].__fpregs[6]));
    __asm__ __volatile__ ("l.d $f31, %0" : : "m" (env[0].__fpregs[7]));
#else /* O32 || N32 */
    __asm__ __volatile__ ("l.d $f20, %0" : : "m" (env[0].__fpregs[0]));
    __asm__ __volatile__ ("l.d $f22, %0" : : "m" (env[0].__fpregs[1]));
    __asm__ __volatile__ ("l.d $f24, %0" : : "m" (env[0].__fpregs[2]));
    __asm__ __volatile__ ("l.d $f26, %0" : : "m" (env[0].__fpregs[3]));
    __asm__ __volatile__ ("l.d $f28, %0" : : "m" (env[0].__fpregs[4]));
    __asm__ __volatile__ ("l.d $f30, %0" : : "m" (env[0].__fpregs[5]));
#endif /* O32 || N32 */

    /* Get and reconstruct the floating point csr.  */
    __asm__ __volatile__ ("lw $2, %0" : : "m" (env[0].__fpc_csr));
    __asm__ __volatile__ ("ctc1 $2, $31");
#endif

    /* Get the GP. */
#if _MIPS_SIM == _MIPS_SIM_ABI64
    __asm__ __volatile__ ("ld $gp, %0" : : "m" (env[0].__gp));
#else /* O32 || N32 */
    __asm__ __volatile__ ("lw $gp, %0" : : "m" (env[0].__gp));
#endif /* O32 || N32 */

    /* Get the callee-saved registers.  */
#if _MIPS_SIM == _MIPS_SIM_ABI32
    __asm__ __volatile__ ("lw $16, %0" : : "m" (env[0].__regs[0]));
    __asm__ __volatile__ ("lw $17, %0" : : "m" (env[0].__regs[1]));
    __asm__ __volatile__ ("lw $18, %0" : : "m" (env[0].__regs[2]));
    __asm__ __volatile__ ("lw $19, %0" : : "m" (env[0].__regs[3]));
    __asm__ __volatile__ ("lw $20, %0" : : "m" (env[0].__regs[4]));
    __asm__ __volatile__ ("lw $21, %0" : : "m" (env[0].__regs[5]));
    __asm__ __volatile__ ("lw $22, %0" : : "m" (env[0].__regs[6]));
    __asm__ __volatile__ ("lw $23, %0" : : "m" (env[0].__regs[7]));
#else /* N32 || N64 */
    __asm__ __volatile__ ("ld $16, %0" : : "m" (env[0].__regs[0]));
    __asm__ __volatile__ ("ld $17, %0" : : "m" (env[0].__regs[1]));
    __asm__ __volatile__ ("ld $18, %0" : : "m" (env[0].__regs[2]));
    __asm__ __volatile__ ("ld $19, %0" : : "m" (env[0].__regs[3]));
    __asm__ __volatile__ ("ld $20, %0" : : "m" (env[0].__regs[4]));
    __asm__ __volatile__ ("ld $21, %0" : : "m" (env[0].__regs[5]));
    __asm__ __volatile__ ("ld $22, %0" : : "m" (env[0].__regs[6]));
    __asm__ __volatile__ ("ld $23, %0" : : "m" (env[0].__regs[7]));
#endif /* N32 || N64 */

    /* Get the PC.  */
#if _MIPS_SIM == _MIPS_SIM_ABI32
    __asm__ __volatile__ ("lw $25, %0" : : "m" (env[0].__pc));
#elif _MIPS_SIM == _MIPS_SIM_NABI32
    __asm__ __volatile__ ("lw $31, %0" : : "m" (env[0].__pc));
#else /* N64 */
    __asm__ __volatile__ ("ld $31, %0" : : "m" (env[0].__pc));
#endif /* N64 */

    /* Restore the stack pointer and the FP.  They have to be restored
       last and in a single asm as gcc, depending on options used, may
       use either of them to access env.  */
#if _MIPS_SIM == _MIPS_SIM_ABI64
    __asm__ __volatile__ ("ld $29, %0\n\t"
	    "ld $30, %1\n\t" : : "m" (env[0].__sp), "m" (env[0].__fp));
#else /* O32 || N32 */
    __asm__ __volatile__ ("lw $29, %0\n\t"
	    "lw $30, %1\n\t" : : "m" (env[0].__sp), "m" (env[0].__fp));
#endif /* O32 || N32 */

    /* Give setjmp 1 if given a 0, or what they gave us if non-zero.  */
    if (val == 0)
	__asm__ __volatile__ ("li $2, 1");
    else
	__asm__ __volatile__ ("move $2, %0" : : "r" (val));

#if _MIPS_SIM == _MIPS_SIM_ABI32
    __asm__ __volatile__ ("jr $25");
#else /* N32 || N64 */
    __asm__ __volatile__ ("jr $31");
#endif /* N32 || N64 */

    /* Avoid `volatile function does return' warnings.  */
    for (;;);
}
libc_hidden_def(__longjmp)
