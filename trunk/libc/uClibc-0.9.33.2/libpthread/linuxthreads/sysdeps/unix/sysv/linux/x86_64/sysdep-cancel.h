/* Copyright (C) 2002, 2003, 2005 Free Software Foundation, Inc.
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
#include <pt-machine.h>
#ifndef __ASSEMBLER__
# include <linuxthreads/internals.h>
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				      \
  .text;								      \
  ENTRY (name)								      \
    SINGLE_THREAD_P;							      \
    jne L(pseudo_cancel);						      \
    DO_CALL (syscall_name, args);					      \
    cmpq $-4095, %rax;							      \
    jae SYSCALL_ERROR_LABEL;						      \
    ret;								      \
  L(pseudo_cancel):							      \
    /* Save registers that might get destroyed.  */			      \
    SAVESTK_##args							      \
    PUSHARGS_##args							      \
    CENABLE								      \
    /* Restore registers.  */						      \
    POPARGS_##args							      \
    /* The return value from CENABLE is argument for CDISABLE.  */	      \
    movq %rax, (%rsp);							      \
    movl $SYS_ify (syscall_name), %eax;					      \
    syscall;								      \
    movq (%rsp), %rdi;							      \
    /* Save %rax since it's the error code from the syscall.  */	      \
    movq %rax, 8(%rsp);							      \
    CDISABLE								      \
    movq 8(%rsp), %rax;							      \
    RESTSTK_##args							      \
    cmpq $-4095, %rax;							      \
    jae SYSCALL_ERROR_LABEL;						      \
  L(pseudo_end):

# define PUSHARGS_0	/* Nothing.  */
# define PUSHARGS_1	PUSHARGS_0 movq %rdi, 8(%rsp);
# define PUSHARGS_2	PUSHARGS_1 movq %rsi, 16(%rsp);
# define PUSHARGS_3	PUSHARGS_2 movq %rdx, 24(%rsp);
# define PUSHARGS_4	PUSHARGS_3 movq %rcx, 32(%rsp);
# define PUSHARGS_5	PUSHARGS_4 movq %r8, 40(%rsp);
# define PUSHARGS_6	PUSHARGS_5 movq %r9, 48(%rsp);

# define POPARGS_0	/* Nothing.  */
# define POPARGS_1	POPARGS_0 movq 8(%rsp), %rdi;
# define POPARGS_2	POPARGS_1 movq 16(%rsp), %rsi;
# define POPARGS_3	POPARGS_2 movq 24(%rsp), %rdx;
# define POPARGS_4	POPARGS_3 movq 32(%rsp), %r10;
# define POPARGS_5	POPARGS_4 movq 40(%rsp), %r8;
# define POPARGS_6	POPARGS_5 movq 48(%rsp), %r9;

/* We always have to align the stack before calling a function.  */
# define SAVESTK_0	subq $24, %rsp;cfi_adjust_cfa_offset(24);
# define SAVESTK_1	SAVESTK_0
# define SAVESTK_2	SAVESTK_1
# define SAVESTK_3	subq $40, %rsp;cfi_adjust_cfa_offset(40);
# define SAVESTK_4	SAVESTK_3
# define SAVESTK_5	subq $56, %rsp;cfi_adjust_cfa_offset(56);
# define SAVESTK_6	SAVESTK_5

# define RESTSTK_0	addq $24,%rsp;cfi_adjust_cfa_offset(-24);
# define RESTSTK_1	RESTSTK_0
# define RESTSTK_2	RESTSTK_1
# define RESTSTK_3	addq $40, %rsp;cfi_adjust_cfa_offset(-40);
# define RESTSTK_4	RESTSTK_3
# define RESTSTK_5	addq $56, %rsp;cfi_adjust_cfa_offset(-56);
# define RESTSTK_6	RESTSTK_5

# ifdef IS_IN_libpthread
#  define CENABLE	call __pthread_enable_asynccancel;
#  define CDISABLE	call __pthread_disable_asynccancel;
#  define __local_multiple_threads __pthread_multiple_threads
# elif !defined NOT_IN_libc
#  define CENABLE	call __libc_enable_asynccancel;
#  define CDISABLE	call __libc_disable_asynccancel;
#  define __local_multiple_threads __libc_multiple_threads
# else
#  define CENABLE	call __librt_enable_asynccancel@plt;
#  define CDISABLE	call __librt_disable_asynccancel@plt;
# endif

# if defined IS_IN_libpthread || !defined NOT_IN_libc
#  ifndef __ASSEMBLER__
extern int __local_multiple_threads attribute_hidden;
#   define SINGLE_THREAD_P \
  __builtin_expect (__local_multiple_threads == 0, 1)
#  else
#   define SINGLE_THREAD_P cmpl $0, __local_multiple_threads(%rip)
#  endif

# else

#  ifndef __ASSEMBLER__
#   define SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF, \
				   p_header.data.multiple_threads) == 0, 1)
#  else
#   define SINGLE_THREAD_P cmpl $0, %fs:MULTIPLE_THREADS_OFFSET
#  endif

# endif

#elif !defined __ASSEMBLER__

/* This code should never be used but we define it anyhow.  */
# define SINGLE_THREAD_P (1)

#endif
