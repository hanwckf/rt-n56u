/* Copyright (C) 2003, 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Phil Blundell <pb@nexus.co.uk>, 2003.

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
#include <pt-machine.h>
#ifndef __ASSEMBLER__
# include <linuxthreads/internals.h>
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread

/* We push lr onto the stack, so we have to use ldmib instead of ldmia
   to find the saved arguments.  */
# ifdef __PIC__
#  undef DOARGS_5
#  undef DOARGS_6
#  undef DOARGS_7
#  define DOARGS_5 str r4, [sp, $-4]!; ldr r4, [sp, $8];
#  define DOARGS_6 mov ip, sp; stmfd sp!, {r4, r5}; ldmib ip, {r4, r5};
#  define DOARGS_7 mov ip, sp; stmfd sp!, {r4, r5, r6}; ldmib ip, {r4, r5, r6};
# endif

# undef PSEUDO_RET
# define PSEUDO_RET						        \
    ldrcc pc, [sp], $4;						        \
    ldr	lr, [sp], $4;							\
    b PLTJMP(SYSCALL_ERROR)

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				\
  .section ".text";							\
    PSEUDO_PROLOGUE;							\
  ENTRY (name);								\
    SINGLE_THREAD_P;							\
    bne .Lpseudo_cancel;						\
    DO_CALL (syscall_name, args);					\
    cmn r0, $4096;							\
    RETINSTR(cc, lr);							\
    b PLTJMP(SYSCALL_ERROR);						\
  .Lpseudo_cancel:							\
    str lr, [sp, $-4]!;							\
    DOCARGS_##args;	/* save syscall args around CENABLE.  */	\
    CENABLE;								\
    mov ip, r0;		/* put mask in safe place.  */			\
    UNDOCARGS_##args;	/* restore syscall args.  */			\
    swi SYS_ify (syscall_name);	/* do the call.  */			\
    str r0, [sp, $-4]!; /* save syscall return value.  */		\
    mov r0, ip;		/* get mask back.  */				\
    CDISABLE;								\
    ldr r0, [sp], $4;	/* retrieve return value.  */			\
    UNDOC2ARGS_##args;	/* fix register damage.  */			\
    cmn r0, $4096;

# define DOCARGS_0
# define UNDOCARGS_0
# define UNDOC2ARGS_0

# define DOCARGS_1	str r0, [sp, #-4]!;
# define UNDOCARGS_1	ldr r0, [sp], #4;
# define UNDOC2ARGS_1

# define DOCARGS_2	str r1, [sp, #-4]!; str r0, [sp, #-4]!;
# define UNDOCARGS_2	ldr r0, [sp], #4; ldr r1, [sp], #4;
# define UNDOC2ARGS_2

# define DOCARGS_3	str r2, [sp, #-4]!; str r1, [sp, #-4]!; str r0, [sp, #-4]!;
# define UNDOCARGS_3	ldr r0, [sp], #4; ldr r1, [sp], #4; ldr r2, [sp], #4
# define UNDOC2ARGS_3

# define DOCARGS_4	stmfd sp!, {r0-r3}
# define UNDOCARGS_4	ldmfd sp!, {r0-r3}
# define UNDOC2ARGS_4

# define DOCARGS_5	stmfd sp!, {r0-r3}
# define UNDOCARGS_5	ldmfd sp, {r0-r3}; str r4, [sp, #-4]!; ldr r4, [sp, #24]
# define UNDOC2ARGS_5   ldr r4, [sp], #20

# ifdef IS_IN_libpthread
#  define CENABLE	bl PLTJMP(__pthread_enable_asynccancel)
#  define CDISABLE	bl PLTJMP(__pthread_disable_asynccancel)
#  define __local_multiple_threads __pthread_multiple_threads
# else
#  define CENABLE	bl PLTJMP(__libc_enable_asynccancel)
#  define CDISABLE	bl PLTJMP(__libc_disable_asynccancel)
#  define __local_multiple_threads __libc_multiple_threads
# endif

# ifndef __ASSEMBLER__
extern int __local_multiple_threads attribute_hidden;
#  define SINGLE_THREAD_P __builtin_expect (__local_multiple_threads == 0, 1)
# else
#  if !defined __PIC__
#   define SINGLE_THREAD_P						\
  ldr ip, =__local_multiple_threads;					\
  ldr ip, [ip];								\
  teq ip, #0;
#   define PSEUDO_PROLOGUE
#  else
#   define SINGLE_THREAD_P						\
  ldr ip, 1b;								\
2:									\
  ldr ip, [pc, ip];							\
  teq ip, #0;
#   define PSEUDO_PROLOGUE						\
  1:  .word __local_multiple_threads - 2f - 8;
#  endif
# endif

#elif !defined __ASSEMBLER__

/* This code should never be used but we define it anyhow.  */
# define SINGLE_THREAD_P (1)

#endif
