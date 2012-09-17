/* Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Franz Sirl <Franz.Sirl-kernel@lauterbach.com>, 2003.

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

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				\
  .section ".text";							\
  ENTRY (name)								\
    SINGLE_THREAD_P;							\
    bne- .Lpseudo_cancel;						\
    DO_CALL (SYS_ify (syscall_name));					\
    PSEUDO_RET;								\
  .Lpseudo_cancel:							\
    stdu 1,-128(1);							\
    mflr 9;								\
    std  9,128+16(1);							\
    DOCARGS_##args;	/* save syscall args around CENABLE.  */	\
    CENABLE;								\
    std  3,72(1);	/* store CENABLE return value (MASK).  */	\
    UNDOCARGS_##args;	/* restore syscall args.  */			\
    DO_CALL (SYS_ify (syscall_name));					\
    mfcr 0;		/* save CR/R3 around CDISABLE.  */		\
    std  3,64(1);								\
    std  0,8(1);							\
    ld   3,72(1);	/* pass MASK to CDISABLE.  */			\
    CDISABLE;								\
    ld   9,128+16(1);							\
    ld   0,8(1);	/* restore CR/R3. */				\
    ld   3,64(1);								\
    mtlr 9;								\
    mtcr 0;								\
    addi 1,1,128;

# define DOCARGS_0
# define UNDOCARGS_0

# define DOCARGS_1	std 3,80(1); DOCARGS_0
# define UNDOCARGS_1	ld 3,80(1); UNDOCARGS_0

# define DOCARGS_2	std 4,88(1); DOCARGS_1
# define UNDOCARGS_2	ld 4,88(1); UNDOCARGS_1

# define DOCARGS_3	std 5,96(1); DOCARGS_2
# define UNDOCARGS_3	ld 5,96(1); UNDOCARGS_2

# define DOCARGS_4	std 6,104(1); DOCARGS_3
# define UNDOCARGS_4	ld 6,104(1); UNDOCARGS_3

# define DOCARGS_5	std 7,112(1); DOCARGS_4
# define UNDOCARGS_5	ld 7,112(1); UNDOCARGS_4

# define DOCARGS_6	std 8,120(1); DOCARGS_5
# define UNDOCARGS_6	ld 8,120(1); UNDOCARGS_5

# ifdef IS_IN_libpthread
#  define CENABLE	bl JUMPTARGET(__pthread_enable_asynccancel)
#  define CDISABLE	bl JUMPTARGET(__pthread_disable_asynccancel)
#  define __local_multiple_threads __pthread_multiple_threads
# elif !defined NOT_IN_libc
#  define CENABLE	bl JUMPTARGET(__libc_enable_asynccancel)
#  define CDISABLE	bl JUMPTARGET(__libc_disable_asynccancel)
#  define __local_multiple_threads __libc_multiple_threads
# else
#  define CENABLE	bl JUMPTARGET(__librt_enable_asynccancel); nop
#  define CDISABLE	bl JUMPTARGET(__librt_disable_asynccancel); nop
#  define __local_multiple_threads __librt_multiple_threads
# endif

# ifdef HAVE_TLS_SUPPORT
#  ifndef __ASSEMBLER__
#   define SINGLE_THREAD_P						\
  __builtin_expect (THREAD_GETMEM (THREAD_SELF, p_multiple_threads) == 0, 1)
#  else
#   define SINGLE_THREAD_P						\
  lwz 10,MULTIPLE_THREADS_OFFSET(13);					\
  cmpwi 10,0
#  endif
# else /* !HAVE_TLS_SUPPORT */
#  ifndef __ASSEMBLER__
extern int __local_multiple_threads
#   if !defined NOT_IN_libc || defined IS_IN_libpthread
  attribute_hidden;
#   else
  ;
#   endif
#   define SINGLE_THREAD_P __builtin_expect (__local_multiple_threads == 0, 1)
#  else
#   define SINGLE_THREAD_P						\
	.section	".toc","aw";					\
.LC__local_multiple_threads:;						\
	.tc __local_multiple_threads[TC],__local_multiple_threads;	\
  .previous;								\
  ld    10,.LC__local_multiple_threads@toc(2);				\
  lwz   10,0(10);							\
  cmpwi 10,0
#  endif
# endif

#elif !defined __ASSEMBLER__

/* This code should never be used but we define it anyhow.  */
# define SINGLE_THREAD_P (1)

#endif
