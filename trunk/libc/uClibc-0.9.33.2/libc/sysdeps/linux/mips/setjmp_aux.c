/* Copyright (C) 1996, 1997, 2000 Free Software Foundation, Inc.
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
#include <sgidefs.h>
#include <sys/asm.h>

/* This function is only called via the assembly language routine
   __sigsetjmp, which arranges to pass in the stack pointer and the frame
   pointer.  We do things this way because it's difficult to reliably
   access them in C.  */

extern int __sigjmp_save (sigjmp_buf, int);

int
#if _MIPS_SIM == _MIPS_SIM_ABI64
__sigsetjmp_aux (jmp_buf env, int savemask, long sp, long fp)
#else /* O32 || N32 */
__sigsetjmp_aux (jmp_buf env, int savemask, int sp, int fp)
#endif /* O32 || N32 */
{
#if defined __UCLIBC_HAS_FLOATS__ && ! defined __UCLIBC_HAS_SOFT_FLOAT__
  /* Store the floating point callee-saved registers...  */
#if _MIPS_SIM == _MIPS_SIM_ABI64
  __asm__ __volatile__ ("s.d $f24, %0" : : "m" (env[0].__jmpbuf[0].__fpregs[0]));
  __asm__ __volatile__ ("s.d $f25, %0" : : "m" (env[0].__jmpbuf[0].__fpregs[1]));
  __asm__ __volatile__ ("s.d $f26, %0" : : "m" (env[0].__jmpbuf[0].__fpregs[2]));
  __asm__ __volatile__ ("s.d $f27, %0" : : "m" (env[0].__jmpbuf[0].__fpregs[3]));
  __asm__ __volatile__ ("s.d $f28, %0" : : "m" (env[0].__jmpbuf[0].__fpregs[4]));
  __asm__ __volatile__ ("s.d $f29, %0" : : "m" (env[0].__jmpbuf[0].__fpregs[5]));
  __asm__ __volatile__ ("s.d $f30, %0" : : "m" (env[0].__jmpbuf[0].__fpregs[6]));
  __asm__ __volatile__ ("s.d $f31, %0" : : "m" (env[0].__jmpbuf[0].__fpregs[7]));
#else /* O32 || N32 */
  __asm__ __volatile__ ("s.d $f20, %0" : : "m" (env[0].__jmpbuf[0].__fpregs[0]));
  __asm__ __volatile__ ("s.d $f22, %0" : : "m" (env[0].__jmpbuf[0].__fpregs[1]));
  __asm__ __volatile__ ("s.d $f24, %0" : : "m" (env[0].__jmpbuf[0].__fpregs[2]));
  __asm__ __volatile__ ("s.d $f26, %0" : : "m" (env[0].__jmpbuf[0].__fpregs[3]));
  __asm__ __volatile__ ("s.d $f28, %0" : : "m" (env[0].__jmpbuf[0].__fpregs[4]));
  __asm__ __volatile__ ("s.d $f30, %0" : : "m" (env[0].__jmpbuf[0].__fpregs[5]));
#endif /* O32 || N32 */
#endif

  /* .. and the PC;  */
#if _MIPS_SIM == _MIPS_SIM_ABI64
  __asm__ __volatile__ ("sd $31, %0" : : "m" (env[0].__jmpbuf[0].__pc));
#else
  __asm__ __volatile__ ("sw $31, %0" : : "m" (env[0].__jmpbuf[0].__pc));
#endif

  /* .. and the stack pointer;  */
  env[0].__jmpbuf[0].__sp = (void *) sp;

  /* .. and the FP; it'll be in s8. */
  env[0].__jmpbuf[0].__fp = (void *) fp;

  /* .. and the GP; */
#if _MIPS_SIM == _MIPS_SIM_ABI64
  __asm__ __volatile__ ("sd $gp, %0" : : "m" (env[0].__jmpbuf[0].__gp));
#else
  __asm__ __volatile__ ("sw $gp, %0" : : "m" (env[0].__jmpbuf[0].__gp));
#endif

  /* .. and the callee-saved registers; */
#if (_MIPS_SIM == _MIPS_SIM_ABI32)
  __asm__ __volatile__ ("sw $16, %0" : : "m" (env[0].__jmpbuf[0].__regs[0]));
  __asm__ __volatile__ ("sw $17, %0" : : "m" (env[0].__jmpbuf[0].__regs[1]));
  __asm__ __volatile__ ("sw $18, %0" : : "m" (env[0].__jmpbuf[0].__regs[2]));
  __asm__ __volatile__ ("sw $19, %0" : : "m" (env[0].__jmpbuf[0].__regs[3]));
  __asm__ __volatile__ ("sw $20, %0" : : "m" (env[0].__jmpbuf[0].__regs[4]));
  __asm__ __volatile__ ("sw $21, %0" : : "m" (env[0].__jmpbuf[0].__regs[5]));
  __asm__ __volatile__ ("sw $22, %0" : : "m" (env[0].__jmpbuf[0].__regs[6]));
  __asm__ __volatile__ ("sw $23, %0" : : "m" (env[0].__jmpbuf[0].__regs[7]));
#else	/* N32 || N64 */
  __asm__ __volatile__ ("sd $16, %0" : : "m" (env[0].__jmpbuf[0].__regs[0]));
  __asm__ __volatile__ ("sd $17, %0" : : "m" (env[0].__jmpbuf[0].__regs[1]));
  __asm__ __volatile__ ("sd $18, %0" : : "m" (env[0].__jmpbuf[0].__regs[2]));
  __asm__ __volatile__ ("sd $19, %0" : : "m" (env[0].__jmpbuf[0].__regs[3]));
  __asm__ __volatile__ ("sd $20, %0" : : "m" (env[0].__jmpbuf[0].__regs[4]));
  __asm__ __volatile__ ("sd $21, %0" : : "m" (env[0].__jmpbuf[0].__regs[5]));
  __asm__ __volatile__ ("sd $22, %0" : : "m" (env[0].__jmpbuf[0].__regs[6]));
  __asm__ __volatile__ ("sd $23, %0" : : "m" (env[0].__jmpbuf[0].__regs[7]));
#endif	/* N32 || N64 */

#if defined __UCLIBC_HAS_FLOATS__ && ! defined __UCLIBC_HAS_SOFT_FLOAT__
  /* .. and finally get and reconstruct the floating point csr.  */
  __asm__ ("cfc1 %0, $31" : "=r" (env[0].__jmpbuf[0].__fpc_csr));
#endif

  /* Save the signal mask if requested.  */
  return __sigjmp_save (env, savemask);
}
