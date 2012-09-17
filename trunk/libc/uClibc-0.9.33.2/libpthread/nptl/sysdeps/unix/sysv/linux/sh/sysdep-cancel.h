/* Copyright (C) 2003, 2004, 2005, 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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
#include <sysdep.h>
#ifndef __ASSEMBLER__
# include <pthreadP.h>
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

# define _IMM12 #-12
# define _IMM16 #-16
# define _IMP16 #16
# undef PSEUDO
# define PSEUDO(name, syscall_name, args) \
  .text; \
  ENTRY (name); \
  .Lpseudo_start: \
    SINGLE_THREAD_P; \
    bf .Lpseudo_cancel; \
    .type __##syscall_name##_nocancel,@function; \
    .globl __##syscall_name##_nocancel; \
    __##syscall_name##_nocancel: \
    DO_CALL (syscall_name, args); \
    mov r0,r1; \
    mov _IMM12,r2; \
    shad r2,r1; \
    not r1,r1; \
    tst r1,r1; \
    bt .Lsyscall_error; \
    bra .Lpseudo_end; \
     nop; \
    .size __##syscall_name##_nocancel,.-__##syscall_name##_nocancel; \
 .Lpseudo_cancel: \
    sts.l pr,@-r15; \
    cfi_adjust_cfa_offset (4); \
    cfi_rel_offset (pr, 0); \
    add _IMM16,r15; \
    cfi_adjust_cfa_offset (16); \
    SAVE_ARGS_##args; \
    CENABLE; \
    LOAD_ARGS_##args; \
    add _IMP16,r15; \
    cfi_adjust_cfa_offset (-16); \
    lds.l @r15+,pr; \
    cfi_adjust_cfa_offset (-4); \
    cfi_restore (pr); \
    DO_CALL(syscall_name, args); \
    SYSCALL_INST_PAD; \
    sts.l pr,@-r15; \
    cfi_adjust_cfa_offset (4); \
    cfi_rel_offset (pr, 0); \
    mov.l r0,@-r15; \
    cfi_adjust_cfa_offset (4); \
    cfi_rel_offset (r0, 0); \
    CDISABLE; \
    mov.l @r15+,r0; \
    cfi_adjust_cfa_offset (-4); \
    lds.l @r15+,pr; \
    cfi_adjust_cfa_offset (-4); \
    cfi_restore (pr); \
    mov r0,r1; \
    mov _IMM12,r2; \
    shad r2,r1; \
    not r1,r1; \
    tst r1,r1; \
    bf .Lpseudo_end; \
 .Lsyscall_error: \
    SYSCALL_ERROR_HANDLER; \
 .Lpseudo_end:

# undef PSEUDO_END
# define PSEUDO_END(sym) \
  END (sym)

# define SAVE_ARGS_0	/* Nothing.  */
# define SAVE_ARGS_1	SAVE_ARGS_0; mov.l r4,@(0,r15); cfi_offset (r4,-4)
# define SAVE_ARGS_2	SAVE_ARGS_1; mov.l r5,@(4,r15); cfi_offset (r5,-8)
# define SAVE_ARGS_3	SAVE_ARGS_2; mov.l r6,@(8,r15); cfi_offset (r6,-12)
# define SAVE_ARGS_4	SAVE_ARGS_3; mov.l r7,@(12,r15); cfi_offset (r7,-16)
# define SAVE_ARGS_5	SAVE_ARGS_4
# define SAVE_ARGS_6	SAVE_ARGS_5

# define LOAD_ARGS_0	/* Nothing.  */
# define LOAD_ARGS_1	LOAD_ARGS_0; mov.l @(0,r15),r4
# define LOAD_ARGS_2	LOAD_ARGS_1; mov.l @(4,r15),r5
# define LOAD_ARGS_3	LOAD_ARGS_2; mov.l @(8,r15),r6
# define LOAD_ARGS_4	LOAD_ARGS_3; mov.l @(12,r15),r7
# define LOAD_ARGS_5	LOAD_ARGS_4
# define LOAD_ARGS_6	LOAD_ARGS_5

# ifdef IS_IN_libpthread
#  define __local_enable_asynccancel	__pthread_enable_asynccancel
#  define __local_disable_asynccancel	__pthread_disable_asynccancel
# elif !defined NOT_IN_libc
#  define __local_enable_asynccancel	__libc_enable_asynccancel
#  define __local_disable_asynccancel	__libc_disable_asynccancel
# elif defined IS_IN_librt
#  define __local_enable_asynccancel	__librt_enable_asynccancel
#  define __local_disable_asynccancel	__librt_disable_asynccancel
# else
#  error Unsupported library
# endif

# define CENABLE \
	mov.l 1f,r0; \
	bsrf r0; \
	 nop; \
     0: bra 2f; \
	 mov r0,r2; \
	.align 2; \
     1: .long __local_enable_asynccancel - 0b; \
     2:

# define CDISABLE \
	mov.l 1f,r0; \
	bsrf r0; \
	 mov r2,r4; \
     0: bra 2f; \
	 nop; \
	.align 2; \
     1: .long __local_disable_asynccancel - 0b; \
     2:

# ifndef __ASSEMBLER__
#  define SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF, \
				   header.multiple_threads) == 0, 1)
# else
#  define SINGLE_THREAD_P \
	stc gbr,r0; \
	mov.w 0f,r1; \
	sub r1,r0; \
	mov.l @(MULTIPLE_THREADS_OFFSET,r0),r0; \
	bra 1f; \
	 tst r0,r0; \
     0: .word TLS_PRE_TCB_SIZE; \
     1:

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
