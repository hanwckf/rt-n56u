/* Copyright (C) 1999, 2000, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Jes Sorensen, <Jes.Sorensen@cern.ch>, April 1999.
   Based on code originally written by David Mosberger-Tang

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

#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H

#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

#ifndef __ASSEMBLER__

#include <errno.h>

#undef IA64_USE_NEW_STUB

#define __IA64_BREAK_SYSCALL	0x100000

/* mostly taken from glibc sysdeps/unix/sysv/linux/ia64/sysdep.h */
#define BREAK_INSN_1(num) "break " #num ";;\n\t"
#define BREAK_INSN(num) BREAK_INSN_1(num)

/* On IA-64 we have stacked registers for passing arguments.  The
   "out" registers end up being the called function's "in"
   registers.

   Also, since we have plenty of registers we have two return values
   from a syscall.  r10 is set to -1 on error, whilst r8 contains the
   (non-negative) errno on error or the return value on success.
 */

# define DO_INLINE_SYSCALL_NCS(name, nr, args...)		\
    LOAD_ARGS_##nr (args)					\
    register long _r8 __asm__ ("r8");				\
    register long _r10 __asm__ ("r10");				\
    register long _r15 __asm__ ("r15") = name;			\
    long _retval;						\
    LOAD_REGS_##nr						\
    __asm__ __volatile__ (BREAK_INSN (__IA64_BREAK_SYSCALL)	\
		      : "=r" (_r8), "=r" (_r10), "=r" (_r15)	\
			ASM_OUTARGS_##nr			\
		      : "2" (_r15) ASM_ARGS_##nr		\
		      : "memory" ASM_CLOBBERS_##nr);		\
    _retval = _r8;

#define INLINE_SYSCALL_NCS(name, nr, args...)		\
(__extension__						\
  ({							\
    DO_INLINE_SYSCALL_NCS (name, nr, args)		\
    if (unlikely (_r10 == -1))				\
      {							\
	__set_errno (_retval);				\
	_retval = -1;					\
      }							\
    _retval;						\
   })							\
)

#define INTERNAL_SYSCALL_DECL(err) long int err

#define INTERNAL_SYSCALL_NCS(name, err, nr, args...)	\
(__extension__ \
  ({							\
    DO_INLINE_SYSCALL_NCS (name, nr, args)		\
    err = _r10;						\
    _retval;						\
   }) \
)
#define INTERNAL_SYSCALL_ERROR_P(val, err)	(err == -1)

#define INTERNAL_SYSCALL_ERRNO(val, err)	(val)

#define LOAD_ARGS_0()
#define LOAD_REGS_0
#define LOAD_ARGS_1(a1)					\
  long _arg1 = (long) (a1);				\
  LOAD_ARGS_0 ()
#define LOAD_REGS_1					\
  register long _out0 __asm__ ("out0") = _arg1;		\
  LOAD_REGS_0
#define LOAD_ARGS_2(a1, a2)				\
  long _arg2 = (long) (a2);				\
  LOAD_ARGS_1 (a1)
#define LOAD_REGS_2					\
  register long _out1 __asm__ ("out1") = _arg2;		\
  LOAD_REGS_1
#define LOAD_ARGS_3(a1, a2, a3)				\
  long _arg3 = (long) (a3);				\
  LOAD_ARGS_2 (a1, a2)
#define LOAD_REGS_3					\
  register long _out2 __asm__ ("out2") = _arg3;		\
  LOAD_REGS_2
#define LOAD_ARGS_4(a1, a2, a3, a4)			\
  long _arg4 = (long) (a4);				\
  LOAD_ARGS_3 (a1, a2, a3)
#define LOAD_REGS_4					\
  register long _out3 __asm__ ("out3") = _arg4;		\
  LOAD_REGS_3
#define LOAD_ARGS_5(a1, a2, a3, a4, a5)			\
  long _arg5 = (long) (a5);				\
  LOAD_ARGS_4 (a1, a2, a3, a4)
#define LOAD_REGS_5					\
  register long _out4 __asm__ ("out4") = _arg5;		\
  LOAD_REGS_4
#define LOAD_ARGS_6(a1, a2, a3, a4, a5, a6)		\
  long _arg6 = (long) (a6);	    			\
  LOAD_ARGS_5 (a1, a2, a3, a4, a5)
#define LOAD_REGS_6					\
  register long _out5 __asm__ ("out5") = _arg6;		\
  LOAD_REGS_5

#define ASM_OUTARGS_0
#define ASM_OUTARGS_1	ASM_OUTARGS_0, "=r" (_out0)
#define ASM_OUTARGS_2	ASM_OUTARGS_1, "=r" (_out1)
#define ASM_OUTARGS_3	ASM_OUTARGS_2, "=r" (_out2)
#define ASM_OUTARGS_4	ASM_OUTARGS_3, "=r" (_out3)
#define ASM_OUTARGS_5	ASM_OUTARGS_4, "=r" (_out4)
#define ASM_OUTARGS_6	ASM_OUTARGS_5, "=r" (_out5)

#define ASM_ARGS_0
#define ASM_ARGS_1	ASM_ARGS_0, "3" (_out0)
#define ASM_ARGS_2	ASM_ARGS_1, "4" (_out1)
#define ASM_ARGS_3	ASM_ARGS_2, "5" (_out2)
#define ASM_ARGS_4	ASM_ARGS_3, "6" (_out3)
#define ASM_ARGS_5	ASM_ARGS_4, "7" (_out4)
#define ASM_ARGS_6	ASM_ARGS_5, "8" (_out5)

#define ASM_CLOBBERS_0	ASM_CLOBBERS_1, "out0"
#define ASM_CLOBBERS_1	ASM_CLOBBERS_2, "out1"
#define ASM_CLOBBERS_2	ASM_CLOBBERS_3, "out2"
#define ASM_CLOBBERS_3	ASM_CLOBBERS_4, "out3"
#define ASM_CLOBBERS_4	ASM_CLOBBERS_5, "out4"
#define ASM_CLOBBERS_5	ASM_CLOBBERS_6, "out5"
#define ASM_CLOBBERS_6	ASM_CLOBBERS_6_COMMON , "b7"
#define ASM_CLOBBERS_6_COMMON	, "out6", "out7",			\
  /* Non-stacked integer registers, minus r8, r10, r15.  */		\
  "r2", "r3", "r9", "r11", "r12", "r13", "r14", "r16", "r17", "r18",	\
  "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27",	\
  "r28", "r29", "r30", "r31",						\
  /* Predicate registers.  */						\
  "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15",	\
  /* Non-rotating fp registers.  */					\
  "f6", "f7", "f8", "f9", "f10", "f11", "f12", "f13", "f14", "f15",	\
  /* Branch registers.  */						\
  "b6"

#endif /* __ASSEMBLER__ */
#endif /* _BITS_SYSCALLS_H */
