/* Copyright (C) 2003, 2004, 2005, 2009 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <tcb-offsets.h>
#ifndef __ASSEMBLER__
# include <pthreadP.h>
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

/* NOTE: We do mark syscalls with unwind annotations, for the benefit of
   cancellation; but they're really only accurate at the point of the
   syscall.  The ARM unwind directives are not rich enough without adding
   a custom personality function.  */

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				\
  .section ".text";							\
    PSEUDO_PROLOGUE;							\
  .type __##syscall_name##_nocancel,%function;				\
  .globl __##syscall_name##_nocancel;					\
  __##syscall_name##_nocancel:						\
    cfi_sections(.debug_frame);						\
    cfi_startproc;							\
    DO_CALL (syscall_name, args);					\
    PSEUDO_RET;								\
    cfi_endproc;							\
  .size __##syscall_name##_nocancel,.-__##syscall_name##_nocancel;	\
  ENTRY (name);								\
    SINGLE_THREAD_P;							\
    DOARGS_##args;							\
    bne .Lpseudo_cancel;						\
    cfi_remember_state;							\
    DO_CALL (syscall_name, 0);						\
    UNDOARGS_##args;							\
    cmn r0, $4096;							\
    PSEUDO_RET;								\
    cfi_restore_state;							\
  .Lpseudo_cancel:							\
    .fnstart;								\
    DOCARGS_##args;	/* save syscall args etc. around CENABLE.  */	\
    CENABLE;								\
    mov ip, r0;		/* put mask in safe place.  */			\
    UNDOCARGS_##args;	/* restore syscall args.  */			\
    ldr r7, =SYS_ify (syscall_name);					\
    swi 0x0;		/* do the call.  */				\
    .fnend;		/* Past here we can't easily unwind.  */	\
    mov r7, r0;		/* save syscall return value.  */		\
    mov r0, ip;		/* get mask back.  */				\
    CDISABLE;								\
    mov r0, r7;		/* retrieve return value.  */			\
    RESTORE_LR_##args;							\
    UNDOARGS_##args;							\
    cmn r0, $4096

/* DOARGS pushes four bytes on the stack for five arguments, eight bytes for
   six arguments, and nothing for fewer.  In order to preserve doubleword
   alignment, sometimes we must save an extra register.  */

# define RESTART_UNWIND \
  .fnend; \
  .fnstart; \
  .save {r7, lr}

# define DOCARGS_0 \
  stmfd sp!, {r7, lr}; \
  cfi_adjust_cfa_offset (8); \
  cfi_rel_offset (r7, 0); \
  cfi_rel_offset (lr, 4); \
  .save {r7, lr}
# define UNDOCARGS_0
# define RESTORE_LR_0 \
  ldmfd sp!, {r7, lr}; \
  cfi_adjust_cfa_offset (-8); \
  cfi_restore (r7); \
  cfi_restore (lr)

# define DOCARGS_1 \
  stmfd sp!, {r0, r1, r7, lr}; \
  cfi_adjust_cfa_offset (16); \
  cfi_rel_offset (r7, 8); \
  cfi_rel_offset (lr, 12); \
  .save {r7, lr}; \
  .pad #8
# define UNDOCARGS_1 \
  ldr r0, [sp], #8; \
  cfi_adjust_cfa_offset (-8); \
  RESTART_UNWIND
# define RESTORE_LR_1 \
  RESTORE_LR_0

# define DOCARGS_2 \
  stmfd sp!, {r0, r1, r7, lr}; \
  cfi_adjust_cfa_offset (16); \
  cfi_rel_offset (r7, 8); \
  cfi_rel_offset (lr, 12); \
  .save {r7, lr}; \
  .pad #8
# define UNDOCARGS_2 \
  ldmfd sp!, {r0, r1}; \
  cfi_adjust_cfa_offset (-8); \
  RESTART_UNWIND
# define RESTORE_LR_2 \
  RESTORE_LR_0

# define DOCARGS_3 \
  stmfd sp!, {r0, r1, r2, r3, r7, lr}; \
  cfi_adjust_cfa_offset (24); \
  cfi_rel_offset (r7, 16); \
  cfi_rel_offset (lr, 20); \
  .save {r7, lr}; \
  .pad #16
# define UNDOCARGS_3 \
  ldmfd sp!, {r0, r1, r2, r3}; \
  cfi_adjust_cfa_offset (-16); \
  RESTART_UNWIND
# define RESTORE_LR_3 \
  RESTORE_LR_0

# define DOCARGS_4 \
  stmfd sp!, {r0, r1, r2, r3, r7, lr}; \
  cfi_adjust_cfa_offset (24); \
  cfi_rel_offset (r7, 16); \
  cfi_rel_offset (lr, 20); \
  .save {r7, lr}; \
  .pad #16
# define UNDOCARGS_4 \
  ldmfd sp!, {r0, r1, r2, r3}; \
  cfi_adjust_cfa_offset (-16); \
  RESTART_UNWIND
# define RESTORE_LR_4 \
  RESTORE_LR_0

/* r4 is only stmfd'ed for correct stack alignment.  */
# define DOCARGS_5 \
  .save {r4}; \
  stmfd sp!, {r0, r1, r2, r3, r4, r7, lr}; \
  cfi_adjust_cfa_offset (28); \
  cfi_rel_offset (r7, 20); \
  cfi_rel_offset (lr, 24); \
  .save {r7, lr}; \
  .pad #20
# define UNDOCARGS_5 \
  ldmfd sp!, {r0, r1, r2, r3}; \
  cfi_adjust_cfa_offset (-16); \
  .fnend; \
  .fnstart; \
  .save {r4}; \
  .save {r7, lr}; \
  .pad #4
# define RESTORE_LR_5 \
  ldmfd sp!, {r4, r7, lr}; \
  cfi_adjust_cfa_offset (-12); \
  /* r4 will be marked as restored later.  */ \
  cfi_restore (r7); \
  cfi_restore (lr)

# define DOCARGS_6 \
  .save {r4, r5}; \
  stmfd sp!, {r0, r1, r2, r3, r7, lr}; \
  cfi_adjust_cfa_offset (24); \
  cfi_rel_offset (r7, 16); \
  cfi_rel_offset (lr, 20); \
  .save {r7, lr}; \
  .pad #16
# define UNDOCARGS_6 \
  ldmfd sp!, {r0, r1, r2, r3}; \
  cfi_adjust_cfa_offset (-16); \
  .fnend; \
  .fnstart; \
  .save {r4, r5}; \
  .save {r7, lr}
# define RESTORE_LR_6 \
  RESTORE_LR_0

# ifdef IS_IN_libpthread
#  define CENABLE	bl PLTJMP(__pthread_enable_asynccancel)
#  define CDISABLE	bl PLTJMP(__pthread_disable_asynccancel)
#  define __local_multiple_threads __pthread_multiple_threads
# elif !defined NOT_IN_libc
#  define CENABLE	bl PLTJMP(__libc_enable_asynccancel)
#  define CDISABLE	bl PLTJMP(__libc_disable_asynccancel)
#  define __local_multiple_threads __libc_multiple_threads
# elif defined IS_IN_librt
#  define CENABLE	bl PLTJMP(__librt_enable_asynccancel)
#  define CDISABLE	bl PLTJMP(__librt_disable_asynccancel)
# else
#  error Unsupported library
# endif

# if defined IS_IN_libpthread || !defined NOT_IN_libc
#  ifndef __ASSEMBLER__
extern int __local_multiple_threads attribute_hidden;
#   define SINGLE_THREAD_P __builtin_expect (__local_multiple_threads == 0, 1)
#  else
#   define SINGLE_THREAD_P						\
  ldr ip, 1b;								\
2:									\
  ldr ip, [pc, ip];							\
  teq ip, #0;
#   define PSEUDO_PROLOGUE						\
  1:  .word __local_multiple_threads - 2f - 8;
#  endif
# else
/*  There is no __local_multiple_threads for librt, so use the TCB.  */
#  ifndef __ASSEMBLER__
#   define SINGLE_THREAD_P						\
  __builtin_expect (THREAD_GETMEM (THREAD_SELF,				\
				   header.multiple_threads) == 0, 1)
#  else
#   define PSEUDO_PROLOGUE
#   define SINGLE_THREAD_P						\
  stmfd	sp!, {r0, lr};							\
  cfi_adjust_cfa_offset (8);						\
  cfi_rel_offset (lr, 4);						\
  bl	__aeabi_read_tp;						\
  ldr	ip, [r0, #MULTIPLE_THREADS_OFFSET];				\
  ldmfd	sp!, {r0, lr};							\
  cfi_adjust_cfa_offset (-8);						\
  cfi_restore (lr);							\
  teq	ip, #0
#   define SINGLE_THREAD_P_PIC(x) SINGLE_THREAD_P
#  endif
# endif

#elif !defined __ASSEMBLER__

/* For rtld, et cetera.  */
# define SINGLE_THREAD_P 1
# define NO_CANCELLATION 1

#endif

#ifndef __ASSEMBLER__
# define RTLD_SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF, \
				   header.multiple_threads) == 0, 1)
#endif
