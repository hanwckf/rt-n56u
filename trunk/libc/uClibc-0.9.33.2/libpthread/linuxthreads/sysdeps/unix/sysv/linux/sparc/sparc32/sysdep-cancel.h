/* Copyright (C) 2002, 2003, 2004 Free Software Foundation, Inc.
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

#include <tls.h>
#ifndef __ASSEMBLER__
# include <linuxthreads/internals.h>
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				      \
	.text;								      \
ENTRY(name)								      \
	ld [%g7 + MULTIPLE_THREADS_OFFSET], %g1;			      \
	cmp %g1, 0;							      \
	bne 1f;								      \
	 mov SYS_ify(syscall_name), %g1;				      \
	ta 0x10;							      \
	bcs __syscall_error_handler;					      \
	 nop;								      \
	.subsection 2;							      \
1:	save %sp, -96, %sp;						      \
	CENABLE;							      \
	 nop;								      \
	mov %o0, %l0;							      \
	COPY_ARGS_##args						      \
	mov SYS_ify(syscall_name), %g1;					      \
	ta 0x10;							      \
	bcs __syscall_error_handler2;					      \
	 mov %o0, %l1;							      \
	CDISABLE;							      \
	 mov %l0, %o0;							      \
	jmpl %i7 + 8, %g0;						      \
	 restore %g0, %l1, %o0;						      \
	.previous;							      \
	SYSCALL_ERROR_HANDLER						      \
	SYSCALL_ERROR_HANDLER2

#define SYSCALL_ERROR_HANDLER2						      \
SYSCALL_ERROR_HANDLER_ENTRY(__syscall_error_handler2)			      \
	.global __errno_location;					      \
        .type   __errno_location,@function;				      \
	CDISABLE;							      \
	 mov	%l0, %o0;						      \
	call	__errno_location;					      \
	 nop;								      \
	st	%l1, [%o0];						      \
	jmpl	%i7 + 8, %g0;						      \
	 restore %g0, -1, %o0;						      \
	.previous;

# ifdef IS_IN_libpthread
#  define CENABLE	call __pthread_enable_asynccancel
#  define CDISABLE	call __pthread_disable_asynccancel
# elif !defined NOT_IN_libc
#  define CENABLE	call __libc_enable_asynccancel
#  define CDISABLE	call __libc_disable_asynccancel
# else
#  define CENABLE	call __librt_enable_asynccancel
#  define CDISABLE	call __librt_disable_asynccancel
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
				   p_header.data.multiple_threads) == 0, 1)
# else
#  define SINGLE_THREAD_P ld [%g7 + MULTIPLE_THREADS_OFFSET], %g1
# endif

#elif !defined __ASSEMBLER__

/* This code should never be used but we define it anyhow.  */
# define SINGLE_THREAD_P (1)

#endif
