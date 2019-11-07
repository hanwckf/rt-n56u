/*
 * Copyright (C) 2016 by Waldemar Brodkorb <wbx@uclibc-ng.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 * ported from GNU libc
 */

/* Copyright (C) 2005-2016 Free Software Foundation, Inc.

This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation; either version 2.1 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with the GNU C Library; if not, see
<http://www.gnu.org/licenses/>.  */

#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

#ifndef __ASSEMBLER__
#include <errno.h>

/* For Linux we can use the system call table in the header file
 * 	/usr/include/asm/unistd.h
 * 	   of the kernel.  But these symbols do not follow the SYS_* syntax
 * 	      so we have to redefine the `SYS_ify' macro here.  */
#undef SYS_ify
#define SYS_ify(syscall_name)	(__NR_##syscall_name)

/* Define a macro which expands into the inline wrapper code for a system
   call.  */
# undef INLINE_SYSCALL
# define INLINE_SYSCALL(name, nr, args...)				\
  ({ unsigned long _sys_result = INTERNAL_SYSCALL (name, , nr, args);	\
     if (__builtin_expect (INTERNAL_SYSCALL_ERROR_P (_sys_result, ), 0))\
       {								\
	 __set_errno (INTERNAL_SYSCALL_ERRNO (_sys_result, ));		\
	 _sys_result = (unsigned long) -1;				\
       }								\
     (long) _sys_result; })

# undef INTERNAL_SYSCALL_DECL
# define INTERNAL_SYSCALL_DECL(err) do { } while (0)

#define INTERNAL_SYSCALL_RAW(name, err, nr, args...)	\
  ({ long _sys_result;					\
     {							\
	LOAD_ARGS_##nr (args)				\
	register long _x8 __asm__ ("x8") = (name);	\
        __asm__ volatile (				\
		"svc       0       // syscall " # name  \
		: "=r" (_x0) : "r"(_x8) ASM_ARGS_##nr : "memory"); \
	_sys_result = _x0;				\
     } 							\
     _sys_result; })

# undef INTERNAL_SYSCALL
# define INTERNAL_SYSCALL(name, err, nr, args...)		\
	INTERNAL_SYSCALL_RAW(SYS_ify(name), err, nr, args)

# undef INTERNAL_SYSCALL_AARCH64
# define INTERNAL_SYSCALL_AARCH64(name, err, nr, args...)	\
	INTERNAL_SYSCALL_RAW(__ARM_NR_##name, err, nr, args)

# undef INTERNAL_SYSCALL_ERROR_P
# define INTERNAL_SYSCALL_ERROR_P(val, err) \
   ((unsigned long) (val) >= (unsigned long) -4095)
 
# undef INTERNAL_SYSCALL_ERRNO
# define INTERNAL_SYSCALL_ERRNO(val, err)       (-(val))

/* Macros for setting up inline __asm__ input regs */
# define ASM_ARGS_0
# define ASM_ARGS_1	, "r" (_x0)
# define ASM_ARGS_2	ASM_ARGS_1, "r" (_x1)
# define ASM_ARGS_3	ASM_ARGS_2, "r" (_x2)
# define ASM_ARGS_4	ASM_ARGS_3, "r" (_x3)
# define ASM_ARGS_5	ASM_ARGS_4, "r" (_x4)
# define ASM_ARGS_6	ASM_ARGS_5, "r" (_x5)
# define ASM_ARGS_7	ASM_ARGS_6, "r" (_x6)

/* Macros for converting sys-call wrapper args into sys call args */
# define LOAD_ARGS_0()				\
  register long _x0 __asm__ ("x0");
# define LOAD_ARGS_1(x0)			\
  long _x0tmp;					\
  LOAD_ARGS_0 ()				\
  _x0tmp = (long) (x0);				\
  _x0 = _x0tmp;
# define LOAD_ARGS_2(x0, x1)			\
  register long _x1 __asm__ ("x1");		\
  long _x1tmp;					\
  LOAD_ARGS_1 (x0)				\
  _x1tmp = (long) (x1);				\
  _x1 = _x1tmp;
# define LOAD_ARGS_3(x0, x1, x2)		\
  register long _x2 __asm__ ("x2");		\
  long _x2tmp;					\
  LOAD_ARGS_2 (x0, x1)				\
  _x2tmp = (long) (x2);				\
  _x2 = _x2tmp;
# define LOAD_ARGS_4(x0, x1, x2, x3)		\
  register long _x3 __asm__ ("x3");		\
  long _x3tmp;					\
  LOAD_ARGS_3 (x0, x1, x2)			\
  _x3tmp = (long) (x3);				\
  _x3 = _x3tmp;
# define LOAD_ARGS_5(x0, x1, x2, x3, x4)	\
  register long _x4 __asm__ ("x4");		\
  long _x4tmp;					\
  LOAD_ARGS_4 (x0, x1, x2, x3)			\
  _x4tmp = (long) (x4);				\
  _x4 = _x4tmp;
# define LOAD_ARGS_6(x0, x1, x2, x3, x4, x5)	\
  register long _x5 __asm__ ("x5");		\
  long _x5tmp;					\
  LOAD_ARGS_5 (x0, x1, x2, x3, x4)		\
  _x5tmp = (long) (x5);				\
  _x5 = _x5tmp;
# define LOAD_ARGS_7(x0, x1, x2, x3, x4, x5, x6)\
  register long _x6 __asm__ ("x6");		\
  long _x6tmp;					\
  LOAD_ARGS_6 (x0, x1, x2, x3, x4, x5)		\
  _x6tmp = (long) (x6);				\
  _x6 = _x6tmp;

# undef INTERNAL_SYSCALL_NCS
# define INTERNAL_SYSCALL_NCS(number, err, nr, args...)	\
		INTERNAL_SYSCALL_RAW (number, err, nr, args)

#endif /* ! __ASSEMBLER__  */
#endif /* _BITS_SYSCALLS_H */
