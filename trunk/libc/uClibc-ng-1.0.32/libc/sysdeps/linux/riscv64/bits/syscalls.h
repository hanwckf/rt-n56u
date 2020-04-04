/*
 * Copyright (C) 2018 by Waldemar Brodkorb <wbx@uclibc-ng.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 * ported from GNU C Library
 */

#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

#define INTERNAL_SYSCALL_NCS(name, err, nr, args...)	\
  ({ long _sys_result;					\
     {							\
	register long int _a7 __asm__ ("a7");		\
	LOAD_ARGS_##nr (args)				\
	_a7 = (name);					\
							\
        __asm__ volatile (				\
		"scall\n\t"				\
		: "=r" (_a0) 				\
		: "r"(_a7) ASM_ARGS_##nr  		\
		: "memory"); 				\
	_sys_result = _a0;				\
     } 							\
     _sys_result; 					\
  })

/* Macros for setting up inline __asm__ input regs */
# define ASM_ARGS_0
# define ASM_ARGS_1	, "r" (_a0)
# define ASM_ARGS_2	ASM_ARGS_1, "r" (_a1)
# define ASM_ARGS_3	ASM_ARGS_2, "r" (_a2)
# define ASM_ARGS_4	ASM_ARGS_3, "r" (_a3)
# define ASM_ARGS_5	ASM_ARGS_4, "r" (_a4)
# define ASM_ARGS_6	ASM_ARGS_5, "r" (_a5)
# define ASM_ARGS_7	ASM_ARGS_6, "r" (_a6)

/* Macros for converting sys-call wrapper args into sys call args */
# define LOAD_ARGS_0()				\
  register long _a0 __asm__ ("a0");
# define LOAD_ARGS_1(a0)			\
  long _a0tmp;					\
  LOAD_ARGS_0 ()				\
  _a0tmp = (long) (a0);				\
  _a0 = _a0tmp;
# define LOAD_ARGS_2(a0, a1)			\
  register long _a1 __asm__ ("a1");		\
  long _a1tmp;					\
  LOAD_ARGS_1 (a0)				\
  _a1tmp = (long) (a1);				\
  _a1 = _a1tmp;
# define LOAD_ARGS_3(a0, a1, a2)		\
  register long _a2 __asm__ ("a2");		\
  long _a2tmp;					\
  LOAD_ARGS_2 (a0, a1)				\
  _a2tmp = (long) (a2);				\
  _a2 = _a2tmp;
# define LOAD_ARGS_4(a0, a1, a2, a3)		\
  register long _a3 __asm__ ("a3");		\
  long _a3tmp;					\
  LOAD_ARGS_3 (a0, a1, a2)			\
  _a3tmp = (long) (a3);				\
  _a3 = _a3tmp;
# define LOAD_ARGS_5(a0, a1, a2, a3, a4)	\
  register long _a4 __asm__ ("a4");		\
  long _a4tmp;					\
  LOAD_ARGS_4 (a0, a1, a2, a3)			\
  _a4tmp = (long) (a4);				\
  _a4 = _a4tmp;
# define LOAD_ARGS_6(a0, a1, a2, a3, a4, a5)	\
  register long _a5 __asm__ ("a5");		\
  long _a5tmp;					\
  LOAD_ARGS_5 (a0, a1, a2, a3, a4)		\
  _a5tmp = (long) (a5);				\
  _a5 = _a5tmp;
# define LOAD_ARGS_7(a0, a1, a2, a3, a4, a5, a6)\
  register long _a6 __asm__ ("a6");		\
  long _a6tmp;					\
  LOAD_ARGS_6 (a0, a1, a2, a3, a4, a5)		\
  _a6tmp = (long) (a6);				\
  _a6 = _a6tmp;

#endif
