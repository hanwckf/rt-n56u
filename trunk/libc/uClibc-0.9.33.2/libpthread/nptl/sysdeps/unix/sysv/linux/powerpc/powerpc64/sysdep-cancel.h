/* Cancellable system call stubs.  Linux/PowerPC64 version.
   Copyright (C) 2003, 2004, 2006 Free Software Foundation, Inc.
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
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA
   02110-1301 USA.  */

#include <sysdep.h>
#include <tls.h>
#ifndef __ASSEMBLER__
# include <pthreadP.h>
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

# ifdef HAVE_ASM_GLOBAL_DOT_NAME
#  define DASHDASHPFX(str) .__##str
# else
#  define DASHDASHPFX(str) __##str
# endif

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				\
  .section ".text";							\
  ENTRY (name)								\
    SINGLE_THREAD_P;							\
    bne- .Lpseudo_cancel;						\
  .type DASHDASHPFX(syscall_name##_nocancel),@function;			\
  .globl DASHDASHPFX(syscall_name##_nocancel);				\
  DASHDASHPFX(syscall_name##_nocancel):					\
    DO_CALL (SYS_ify (syscall_name));					\
    PSEUDO_RET;								\
  .size DASHDASHPFX(syscall_name##_nocancel),.-DASHDASHPFX(syscall_name##_nocancel);	\
  .Lpseudo_cancel:							\
    stdu 1,-128(1);							\
    cfi_adjust_cfa_offset (128);					\
    mflr 9;								\
    std  9,128+16(1);							\
    cfi_offset (lr, 16);						\
    DOCARGS_##args;	/* save syscall args around CENABLE.  */	\
    CENABLE;								\
    std  3,72(1);	/* store CENABLE return value (MASK).  */	\
    UNDOCARGS_##args;	/* restore syscall args.  */			\
    DO_CALL (SYS_ify (syscall_name));					\
    mfcr 0;		/* save CR/R3 around CDISABLE.  */		\
    std  3,64(1);							\
    std  0,8(1);							\
    ld   3,72(1);	/* pass MASK to CDISABLE.  */			\
    CDISABLE;								\
    ld   9,128+16(1);							\
    ld   0,8(1);	/* restore CR/R3. */				\
    ld   3,64(1);							\
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
# elif !defined NOT_IN_libc
#  define CENABLE	bl JUMPTARGET(__libc_enable_asynccancel)
#  define CDISABLE	bl JUMPTARGET(__libc_disable_asynccancel)
# elif defined IS_IN_librt
#  define CENABLE	bl JUMPTARGET(__librt_enable_asynccancel)
#  define CDISABLE	bl JUMPTARGET(__librt_disable_asynccancel)
# else
#  error Unsupported library
# endif

# ifndef __ASSEMBLER__
#  define SINGLE_THREAD_P						\
  __builtin_expect (THREAD_GETMEM (THREAD_SELF,				\
				   header.multiple_threads) == 0, 1)
# else
#   define SINGLE_THREAD_P						\
  lwz   10,MULTIPLE_THREADS_OFFSET(13);				\
  cmpwi 10,0
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
