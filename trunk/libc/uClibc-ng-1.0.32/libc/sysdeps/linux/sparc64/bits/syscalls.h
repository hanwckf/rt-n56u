/*
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 * ported from GNU C Library
 */

/* Copyright (C) 2000-2017 Free Software Foundation, Inc.

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

#define __SYSCALL_CLOBBERS						\
	"f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7",			\
	"f8", "f9", "f10", "f11", "f12", "f13", "f14", "f15",		\
	"f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23",		\
	"f24", "f25", "f26", "f27", "f28", "f29", "f30", "f31",		\
	"f32", "f34", "f36", "f38", "f40", "f42", "f44", "f46",		\
	"f48", "f50", "f52", "f54", "f56", "f58", "f60", "f62",		\
	"cc", "memory"

#define __SYSCALL_STRING						\
	"ta	0x6d;"							\
	"bcc,pt	%%xcc, 1f;"						\
	" mov	0, %%g1;"						\
	"sub	%%g0, %%o0, %%o0;"					\
	"mov	1, %%g1;"						\
	"1:"

#define INTERNAL_SYSCALL_NCS(name, err, nr, args...) 		\
({                                               		\
            register long __o0 __asm__("o0");            	\
	    long err = name;					\
            LOAD_ARGS_##nr(args)                           	\
								\
            __asm__ __volatile__( __SYSCALL_STRING          	\
                : "=r" (err), "=r" (__o0)                   	\
                : "0" (err), "1" (__o0) ASM_ARGS_##nr       	\
                : __SYSCALL_CLOBBERS );	                    	\
            __o0;					    	\
})

#define INTERNAL_SYSCALL_ERROR_P(val, err) \
  ((unsigned int) (val) >= 0xfffff001u)

#define INLINE_CLONE_SYSCALL(arg1,arg2,arg3,arg4,arg5)			\
({									\
	register long __o0 __asm__ ("o0") = (long)(arg1);		\
	register long __o1 __asm__ ("o1") = (long)(arg2);		\
	register long __o2 __asm__ ("o2") = (long)(arg3);		\
	register long __o3 __asm__ ("o3") = (long)(arg4);		\
	register long __o4 __asm__ ("o4") = (long)(arg5);		\
	register long __g1 __asm__ ("g1") = __NR_clone;			\
	__asm __volatile (__SYSCALL_STRING :				\
			  "=r" (__g1), "=r" (__o0), "=r" (__o1)	:	\
			  "0" (__g1), "1" (__o0), "2" (__o1),		\
			  "r" (__o2), "r" (__o3), "r" (__o4) :		\
			  __SYSCALL_CLOBBERS);				\
	if (INTERNAL_SYSCALL_ERROR_P (__o0, __g1))			\
	  {		     			       		   	\
	    __set_errno (INTERNAL_SYSCALL_ERRNO (__o0, __g1));		\
	    __o0 = -1L;			    				\
	  } 	      							\
	else								\
	  { 	      							\
	    __o0 &= (__o1 - 1);						\
	  } 	    	    						\
	__o0;								\
})

#define LOAD_ARGS_0()
#define ASM_ARGS_0
#define LOAD_ARGS_1(o0) \
    __o0 = (long)o0;     \
    LOAD_ARGS_0()
#define ASM_ARGS_1 ASM_ARGS_0, "r" (__o0)
#define LOAD_ARGS_2(o0, o1)			\
  register long __o1 __asm__ ("o1") = (long) (o1);	\
  LOAD_ARGS_1 (o0)
#define ASM_ARGS_2	ASM_ARGS_1, "r" (__o1)
#define LOAD_ARGS_3(o0, o1, o2)			\
  register long __o2 __asm__ ("o2") = (long) (o2);	\
  LOAD_ARGS_2 (o0, o1)
#define ASM_ARGS_3	ASM_ARGS_2, "r" (__o2)
#define LOAD_ARGS_4(o0, o1, o2, o3)		\
  register long __o3 __asm__ ("o3") = (long) (o3);	\
  LOAD_ARGS_3 (o0, o1, o2)
#define ASM_ARGS_4	ASM_ARGS_3, "r" (__o3)
#define LOAD_ARGS_5(o0, o1, o2, o3, o4)		\
  register long __o4 __asm__ ("o4") = (long) (o4);	\
  LOAD_ARGS_4 (o0, o1, o2, o3)
#define ASM_ARGS_5	ASM_ARGS_4, "r" (__o4)
#define LOAD_ARGS_6(o0, o1, o2, o3, o4, o5)	\
  register long __o5 __asm__ ("o5") = (long) (o5);	\
  LOAD_ARGS_5 (o0, o1, o2, o3, o4)
#define ASM_ARGS_6	ASM_ARGS_5, "r" (__o5)

#endif /* __ASSEMBLER__ */
#endif /* _BITS_SYSCALLS_H */
