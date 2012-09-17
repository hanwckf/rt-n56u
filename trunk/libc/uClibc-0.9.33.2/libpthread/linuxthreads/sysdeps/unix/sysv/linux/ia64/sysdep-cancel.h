/* Copyright (C) 2002, 2003 Free Software Foundation, Inc.
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
# include <linuxthreads/internals.h>
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

# ifdef IS_IN_librt
#  define PSEUDO_NLOCAL		6
#  define PSEUDO_SAVE_GP	mov loc5 = gp
#  define PSEUDO_RESTORE_GP	mov gp = loc5
#  define PSEUDO_SAVE_GP_1
#  define PSEUDO_RESTORE_GP_1	mov gp = loc5
# else
#  define PSEUDO_NLOCAL		5
#  define PSEUDO_SAVE_GP
#  define PSEUDO_RESTORE_GP
#  define PSEUDO_SAVE_GP_1	mov loc4 = gp;;
#  define PSEUDO_RESTORE_GP_1	mov gp = loc4
# endif

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				      \
.text;									      \
ENTRY (name)								      \
     adds r14 = MULTIPLE_THREADS_OFFSET, r13;;				      \
     ld4 r14 = [r14];							      \
     mov r15 = SYS_ify(syscall_name);;					      \
     cmp4.ne p6, p7 = 0, r14;						      \
(p6) br.cond.spnt .Lpseudo_cancel;;					      \
     break __BREAK_SYSCALL;;						      \
     cmp.eq p6,p0=-1,r10;						      \
(p6) br.cond.spnt.few __syscall_error;					      \
     ret;;								      \
     .endp name;							      \
     .proc __GC_##name;							      \
     .globl __GC_##name;						      \
     .hidden __GC_##name;						      \
__GC_##name:								      \
.Lpseudo_cancel:							      \
     .prologue;								      \
     .regstk args, PSEUDO_NLOCAL, args, 0;				      \
     .save ar.pfs, loc0;						      \
     alloc loc0 = ar.pfs, args, PSEUDO_NLOCAL, args, 0;			      \
     .save rp, loc1;							      \
     mov loc1 = rp;							      \
     PSEUDO_SAVE_GP;;							      \
     .body;								      \
     CENABLE;;								      \
     PSEUDO_RESTORE_GP;							      \
     mov loc2 = r8;							      \
     COPY_ARGS_##args							      \
     mov r15 = SYS_ify(syscall_name);					      \
     break __BREAK_SYSCALL;;						      \
     mov loc3 = r8;							      \
     mov loc4 = r10;							      \
     mov out0 = loc2;							      \
     CDISABLE;;								      \
     PSEUDO_RESTORE_GP;							      \
     cmp.eq p6,p0=-1,loc4;						      \
(p6) br.cond.spnt.few __syscall_error_##args;				      \
     mov r8 = loc3;							      \
     mov rp = loc1;							      \
     mov ar.pfs = loc0;							      \
.Lpseudo_end:								      \
     ret;								      \
     .endp __GC_##name;							      \
.section .gnu.linkonce.t.__syscall_error_##args, "ax";			      \
     .align 32;								      \
     .proc __syscall_error_##args;					      \
     .global __syscall_error_##args;					      \
     .hidden __syscall_error_##args;					      \
     .size __syscall_error_##args, 64;					      \
__syscall_error_##args:							      \
     .prologue;								      \
     .regstk args, PSEUDO_NLOCAL, args, 0;				      \
     .save ar.pfs, loc0;						      \
     .save rp, loc1;							      \
     .body;								      \
     PSEUDO_SAVE_GP_1;							      \
     br.call.sptk.many b0 = __errno_location;;				      \
     st4 [r8] = loc3;							      \
     PSEUDO_RESTORE_GP_1;						      \
     mov rp = loc1;							      \
     mov r8 = -1;							      \
     mov ar.pfs = loc0

#undef PSEUDO_END
#define PSEUDO_END(name) .endp

# ifdef IS_IN_libpthread
#  define CENABLE	br.call.sptk.many b0 = __pthread_enable_asynccancel
#  define CDISABLE	br.call.sptk.many b0 = __pthread_disable_asynccancel
# elif !defined NOT_IN_libc
#  define CENABLE	br.call.sptk.many b0 = __libc_enable_asynccancel
#  define CDISABLE	br.call.sptk.many b0 = __libc_disable_asynccancel
# else
#  define CENABLE	br.call.sptk.many b0 = __librt_enable_asynccancel
#  define CDISABLE	br.call.sptk.many b0 = __librt_disable_asynccancel
# endif

#define COPY_ARGS_0	/* Nothing */
#define COPY_ARGS_1	COPY_ARGS_0 mov out0 = in0;
#define COPY_ARGS_2	COPY_ARGS_1 mov out1 = in1;
#define COPY_ARGS_3	COPY_ARGS_2 mov out2 = in2;
#define COPY_ARGS_4	COPY_ARGS_3 mov out3 = in3;
#define COPY_ARGS_5	COPY_ARGS_4 mov out4 = in4;
#define COPY_ARGS_6	COPY_ARGS_5 mov out5 = in5;
#define COPY_ARGS_7	COPY_ARGS_6 mov out6 = in6;

# ifndef __ASSEMBLER__
#  define SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF, p_multiple_threads) == 0, 1)
# else
#  define SINGLE_THREAD_P \
  adds r14 = MULTIPLE_THREADS_OFFSET, r13 ;; ld4 r14 = [r14] ;; cmp4.ne p6, p7 = 0, r14
# endif

#elif !defined __ASSEMBLER__

/* This code should never be used but we define it anyhow.  */
# define SINGLE_THREAD_P (1)

#endif
