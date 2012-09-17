/* Copyright (C) 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2003.

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

#include <sysdep.h>
#include <tls.h>
#ifndef __ASSEMBLER__
# include <linuxthreads/internals.h>
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

# if !defined NOT_IN_libc || defined IS_IN_libpthread

#  define PSEUDO_CANCEL(name, syscall_name, args)			      \
L(pseudo_cancel):							      \
	STM_##args							      \
	stm	%r12,%r15,48(%r15);					      \
	lr	%r14,%r15;						      \
	ahi	%r15,-96;						      \
	st	%r14,0(%r15);						      \
	basr    %r13,0;							      \
0:	l	%r1,1f-0b(%r13);					      \
	bas	%r14,0(%r1,%r13);					      \
	lr	%r0,%r2;						      \
	LM_##args							      \
	DO_CALL(syscall_name, args);					      \
	l	%r1,2f-0b(%r13);					      \
	lr	%r12,%r2;						      \
	lr	%r2,%r0;						      \
	bas	%r14,0(%r1,%r13);					      \
	lr	%r2,%r12;						      \
	lm	%r12,%r15,48+96(%r15);					      \
	j	L(pseudo_check);					      \
1:	.long	CENABLE-0b;						      \
2:	.long	CDISABLE-0b;

# else /* !libc.so && !libpthread.so */

#  define PSEUDO_CANCEL(name, syscall_name, args)			      \
L(pseudo_cancel):							      \
	STM_##args							      \
	stm	%r11,%r15,44(%r15);					      \
	lr	%r14,%r15;						      \
	ahi	%r15,-96;						      \
	st	%r14,0(%r15);						      \
	basr    %r13,0;							      \
0:	l	%r12,3f-0b(%r13);					      \
	l	%r1,1f-0b(%r13);					      \
	la	%r12,0(%r12,%r13);					      \
	bas	%r14,0(%r1,%r13);					      \
	lr	%r0,%r2;						      \
	LM_##args							      \
	DO_CALL(syscall_name, args);					      \
	l	%r1,2f-0b(%r13);					      \
	lr	%r11,%r2;						      \
	lr	%r2,%r0;						      \
	bas	%r14,0(%r1,%r13);					      \
	lr	%r2,%r11;						      \
	lm	%r11,%r15,44+96(%r15);					      \
	j	L(pseudo_check);					      \
1:	.long	CENABLE@PLT-0b;						      \
2:	.long	CDISABLE@PLT-0b;					      \
3:	.long	_GLOBAL_OFFSET_TABLE_-0b;

# endif

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				      \
	.text;								      \
PSEUDO_CANCEL(name, syscall_name, args)					      \
ENTRY(name)								      \
	SINGLE_THREAD_P(%r1)						      \
	jne	L(pseudo_cancel);					      \
	DO_CALL(syscall_name, args);					      \
L(pseudo_check):							      \
	lhi	%r4,-4095;						      \
	clr	%r2,%r4;						      \
	jnl	SYSCALL_ERROR_LABEL;					      \
L(pseudo_end):

# ifdef IS_IN_libpthread
#  define CENABLE	__pthread_enable_asynccancel
#  define CDISABLE	__pthread_disable_asynccancel
# elif !defined NOT_IN_libc
#  define CENABLE	__libc_enable_asynccancel
#  define CDISABLE	__libc_disable_asynccancel
# else
#  define CENABLE	__librt_enable_asynccancel
#  define CDISABLE	__librt_disable_asynccancel
# endif

#define STM_0		/* Nothing */
#define STM_1		st %r2,8(%r15);
#define STM_2		stm %r2,%r3,8(%r15);
#define STM_3		stm %r2,%r4,8(%r15);
#define STM_4		stm %r2,%r5,8(%r15);
#define STM_5		stm %r2,%r5,8(%r15);

#define LM_0		/* Nothing */
#define LM_1		l %r2,8+96(%r15);
#define LM_2		lm %r2,%r3,8+96(%r15);
#define LM_3		lm %r2,%r4,8+96(%r15);
#define LM_4		lm %r2,%r5,8+96(%r15);
#define LM_5		lm %r2,%r5,8+96(%r15);

# ifndef __ASSEMBLER__
#  define SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF,				      \
				   p_header.data.multiple_threads) == 0, 1)
# else
#  define SINGLE_THREAD_P(reg) \
	ear	reg,%a0;						      \
	icm	reg,15,MULTIPLE_THREADS_OFFSET(reg);
# endif

#elif !defined __ASSEMBLER__

/* This code should never be used but we define it anyhow.  */
# define SINGLE_THREAD_P (1)

#endif
