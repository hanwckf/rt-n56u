/* Define the machine-dependent type `jmp_buf'.  MIPS version.
   Copyright (C) 1992,93,95,97,2000 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _BITS_SETJMP_H
#define _BITS_SETJMP_H	1

#if !defined _SETJMP_H && !defined _PTHREAD_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

#include <sgidefs.h>

#if _MIPS_SIM == _MIPS_SIM_ABI32
#define __ptr_size void *
#else
#define __ptr_size long long
#endif

typedef struct
  {
    /* Program counter.  */
    __ptr_size __pc;

    /* Stack pointer.  */
    __ptr_size __sp;

    /* Callee-saved registers s0 through s7.  */
#if _MIPS_SIM == _MIPS_SIM_ABI32
    int __regs[8];
#else
    long long __regs[8];
#endif

    /* The frame pointer.  */
    __ptr_size __fp;

    /* The global pointer.  */
    __ptr_size __gp;

    /* Floating point status register.  */
    int __fpc_csr;

    /* Callee-saved floating point registers.  */
#if _MIPS_SIM == _MIPS_SIM_ABI64
    double __fpregs[8];
#else /* N32 || O32 */
    double __fpregs[6];
#endif /* N32 || O32 */
  } __jmp_buf[1];

#endif	/* bits/setjmp.h */
