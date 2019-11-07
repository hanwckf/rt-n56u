/* Copyright (C) 1992, 1995, 1997, 1999, 2000, 2002, 2003, 2004
   Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _LINUX_MIPS_SYSDEP_H
#define _LINUX_MIPS_SYSDEP_H 1

#include <sgidefs.h>
#include <common/sysdep.h>

/* For Linux we can use the system call table in the header file
   /usr/include/asm/unistd.h
   of the kernel.  But these symbols do not follow the SYS_* syntax
   so we have to redefine the `SYS_ify' macro here.  */

#undef SYS_ify
#ifdef __STDC__
# define SYS_ify(syscall_name)	__NR_##syscall_name
#else
# define SYS_ify(syscall_name)	__NR_/**/syscall_name
#endif

#ifdef __ASSEMBLER__

#include <sys/regdef.h>

#define ENTRY(name) 					\
  .globl name;						\
  .align 2;						\
  .ent name,0;						\
  name##:

#undef END
#define	END(function)					\
		.end	function;			\
		.size	function,.-function

#define ret	j ra ; nop

#undef PSEUDO_END
#define PSEUDO_END(sym) .end sym; .size sym,.-sym

#define PSEUDO_NOERRNO(name, syscall_name, args)	\
  .align 2;						\
  ENTRY(name)						\
  .set noreorder;					\
  li v0, SYS_ify(syscall_name);				\
  syscall

#undef PSEUDO_END_NOERRNO
#define PSEUDO_END_NOERRNO(sym) .end sym; .size sym,.-sym

#define ret_NOERRNO ret

#define PSEUDO_ERRVAL(name, syscall_name, args)		\
  .align 2;						\
  ENTRY(name)						\
  .set noreorder;					\
  li v0, SYS_ify(syscall_name);				\
  syscall

#undef PSEUDO_END_ERRVAL
#define PSEUDO_END_ERRVAL(sym) .end sym; .size sym,.-sym

#define r0	v0
#define r1	v1
/* The mips move insn is d,s.  */
#define MOVE(x,y)	move y , x

#if _MIPS_SIM == _ABIO32
# define L(label) $L ## label
#else
# define L(label) .L ## label
#endif

/* Note that while it's better structurally, going back to call __syscall_error
   can make things confusing if you're debugging---it looks like it's jumping
   backwards into the previous fn.  */

#ifdef __PIC__
#define PSEUDO(name, syscall_name, args) 		\
  .align 2;						\
  99: move a0, v0;					\
  la t9,__syscall_error;				\
  jr t9;						\
  ENTRY(name)						\
  .set noreorder;					\
  .cpload t9;						\
  li v0, SYS_ify(syscall_name);				\
  syscall;						\
  .set reorder;						\
  bne a3, zero, 99b;					\
L(syse1):
#else
#define PSEUDO(name, syscall_name, args) 		\
  .set noreorder;					\
  .align 2;						\
  99: move a0, v0;					\
  j __syscall_error;					\
  nop;							\
  ENTRY(name)						\
  .set noreorder;					\
  li v0, SYS_ify(syscall_name);				\
  syscall;						\
  .set reorder;						\
  bne a3, zero, 99b;					\
L(syse1):
#endif

/* We don't want the label for the error handler to be visible in the symbol
   table when we define it here.  */
#ifdef __PIC__
# define SYSCALL_ERROR_LABEL 99b
#endif

#endif  /* __ASSEMBLER__ */
#endif /* _LINUX_MIPS_SYSDEP_H */
