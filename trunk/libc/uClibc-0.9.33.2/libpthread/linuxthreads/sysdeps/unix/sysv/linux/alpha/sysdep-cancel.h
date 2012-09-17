/* Copyright (C) 2003 Free Software Foundation, Inc.
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
#ifndef __ASSEMBLER__
# include <linuxthreads/internals.h>
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread

# ifdef PROF
#  define PSEUDO_PROF				\
	.set noat;				\
	lda	AT, _mcount;			\
	jsr	AT, (AT), _mcount;		\
	.set at
# else
#  define PSEUDO_PROF
# endif

/* ??? Assumes that nothing comes between PSEUDO and PSEUDO_END
   besides "ret".  */

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)			\
	.globl name;						\
	.align 4;						\
	.type name, @function;					\
	.usepv name, std;					\
	cfi_startproc;						\
__LABEL(name)							\
	ldgp	gp, 0(pv);					\
	PSEUDO_PROF;						\
	PSEUDO_PREPARE_ARGS					\
	SINGLE_THREAD_P(t0);					\
	bne	t0, $pseudo_cancel;				\
	lda	v0, SYS_ify(syscall_name);			\
	call_pal PAL_callsys;					\
	bne	a3, SYSCALL_ERROR_LABEL;			\
__LABEL($pseudo_ret)						\
	.subsection 2;						\
__LABEL($pseudo_cancel)						\
	subq	sp, 64, sp;					\
	cfi_def_cfa_offset(64);					\
	stq	ra, 0(sp);					\
	cfi_offset(ra, -64);					\
	SAVE_ARGS_##args;					\
	CENABLE;						\
	LOAD_ARGS_##args;					\
	lda	v0, SYS_ify(syscall_name);			\
	call_pal PAL_callsys;					\
	stq	v0, 8(sp);					\
	bne	a3, $multi_error;				\
	CDISABLE;						\
	ldq	ra, 0(sp);					\
	ldq	v0, 8(sp);					\
	addq	sp, 64, sp;					\
	cfi_remember_state;					\
	cfi_restore(ra);					\
	cfi_def_cfa_offset(0);					\
	ret;							\
	cfi_restore_state;					\
__LABEL($multi_error)						\
	CDISABLE;						\
	ldq	ra, 0(sp);					\
	ldq	v0, 8(sp);					\
	addq	sp, 64, sp;					\
	cfi_restore(ra);					\
	cfi_def_cfa_offset(0);					\
__LABEL($syscall_error)						\
	SYSCALL_ERROR_HANDLER;					\
	.previous

# undef PSEUDO_END
# define PSEUDO_END(sym)					\
	.subsection 2;						\
	cfi_endproc;						\
	.size sym, .-sym

# define SAVE_ARGS_0	/* Nothing.  */
# define SAVE_ARGS_1	SAVE_ARGS_0; stq a0, 8(sp)
# define SAVE_ARGS_2	SAVE_ARGS_1; stq a1, 16(sp)
# define SAVE_ARGS_3	SAVE_ARGS_2; stq a2, 24(sp)
# define SAVE_ARGS_4	SAVE_ARGS_3; stq a3, 32(sp)
# define SAVE_ARGS_5	SAVE_ARGS_4; stq a4, 40(sp)
# define SAVE_ARGS_6	SAVE_ARGS_5; stq a5, 48(sp)

# define LOAD_ARGS_0	/* Nothing.  */
# define LOAD_ARGS_1	LOAD_ARGS_0; ldq a0, 8(sp)
# define LOAD_ARGS_2	LOAD_ARGS_1; ldq a1, 16(sp)
# define LOAD_ARGS_3	LOAD_ARGS_2; ldq a2, 24(sp)
# define LOAD_ARGS_4	LOAD_ARGS_3; ldq a3, 32(sp)
# define LOAD_ARGS_5	LOAD_ARGS_4; ldq a4, 40(sp)
# define LOAD_ARGS_6	LOAD_ARGS_5; ldq a5, 48(sp)

# ifdef IS_IN_libpthread
#  define __local_enable_asynccancel	__pthread_enable_asynccancel
#  define __local_disable_asynccancel	__pthread_disable_asynccancel
#  define __local_multiple_threads	__pthread_multiple_threads
# else
#  define __local_enable_asynccancel	__libc_enable_asynccancel
#  define __local_disable_asynccancel	__libc_disable_asynccancel
#  define __local_multiple_threads	__libc_multiple_threads
# endif

# ifdef __PIC__
#  define CENABLE	bsr ra, __local_enable_asynccancel !samegp
#  define CDISABLE	bsr ra, __local_disable_asynccancel !samegp
# else
#  define CENABLE	jsr ra, __local_enable_asynccancel; ldgp ra, 0(gp)
#  define CDISABLE	jsr ra, __local_disable_asynccancel; ldgp ra, 0(gp)
# endif

# ifndef __ASSEMBLER__
extern int __local_multiple_threads attribute_hidden;
#   define SINGLE_THREAD_P \
  __builtin_expect (__local_multiple_threads == 0, 1)
# elif defined(__PIC__)
#  define SINGLE_THREAD_P(reg)  ldl reg, __local_multiple_threads(gp) !gprel
# else
#  define SINGLE_THREAD_P(reg)					\
	ldah	reg, __local_multiple_threads(gp) !gprelhigh;	\
	ldl	reg, __local_multiple_threads(reg) !gprellow
# endif

#elif !defined __ASSEMBLER__

/* This code should never be used but we define it anyhow.  */
# define SINGLE_THREAD_P (1)

#endif
