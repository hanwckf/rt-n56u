/* Copyright (C) 2002, 2003, 2004, 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2002.

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
# include <pthreadP.h>
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)	\
	.text;					\
	.globl		__syscall_error;	\
ENTRY(name)					\
	ld [%g7 + MULTIPLE_THREADS_OFFSET], %g1;\
	brnz,pn %g1, 1f;			\
.type	__##syscall_name##_nocancel,@function;	\
.globl	__##syscall_name##_nocancel;		\
__##syscall_name##_nocancel:			\
	 mov SYS_ify(syscall_name), %g1;	\
	ta 0x6d;				\
	bcc,pt %xcc, 8f;			\
	 mov %o7, %g1;				\
	call __syscall_error;			\
	 mov %g1, %o7;				\
8:	jmpl %o7 + 8, %g0;			\
	 nop;					\
.size	__##syscall_name##_nocancel,.-__##syscall_name##_nocancel;\
1:	save %sp, -192, %sp;			\
	cfi_def_cfa_register(%fp);		\
	cfi_window_save;			\
	cfi_register(%o7, %i7);			\
	CENABLE;				\
	 nop;					\
	mov %o0, %l0;				\
	COPY_ARGS_##args			\
	mov SYS_ify(syscall_name), %g1;		\
	ta 0x6d;				\
	bcc,pt %xcc, 1f;			\
	 mov %o0, %l1;				\
	CDISABLE;				\
	 mov %l0, %o0;				\
	call __syscall_error;			\
	 mov %l1, %o0;				\
	ba,pt %xcc, 2f;				\
	 mov -1, %l1;				\
1:	CDISABLE;				\
	 mov %l0, %o0;				\
2:	jmpl %i7 + 8, %g0;			\
	 restore %g0, %l1, %o0;

# ifdef IS_IN_libpthread
#  define CENABLE	call __pthread_enable_asynccancel
#  define CDISABLE	call __pthread_disable_asynccancel
# elif !defined NOT_IN_libc
#  define CENABLE	call __libc_enable_asynccancel
#  define CDISABLE	call __libc_disable_asynccancel
# elif defined IS_IN_librt
#  define CENABLE	call __librt_enable_asynccancel
#  define CDISABLE	call __librt_disable_asynccancel
# else
#  error Unsupported library
# endif

#define COPY_ARGS_0	/* Nothing */
#define COPY_ARGS_1	COPY_ARGS_0 mov %i0, %o0;
#define COPY_ARGS_2	COPY_ARGS_1 mov %i1, %o1;
#define COPY_ARGS_3	COPY_ARGS_2 mov %i2, %o2;
#define COPY_ARGS_4	COPY_ARGS_3 mov %i3, %o3;
#define COPY_ARGS_5	COPY_ARGS_4 mov %i4, %o4;
#define COPY_ARGS_6	COPY_ARGS_5 mov %i5, %o5;

# ifndef __ASSEMBLER__
#  define SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF,				      \
				   header.multiple_threads) == 0, 1)
# else
#  define SINGLE_THREAD_P ld [%g7 + MULTIPLE_THREADS_OFFSET], %g1
# endif

#elif !defined __ASSEMBLER__

# define SINGLE_THREAD_P (1)
# define NO_CANCELLATION 1

#endif

#ifndef __ASSEMBLER__
# define RTLD_SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF, \
				   header.multiple_threads) == 0, 1)
#endif
