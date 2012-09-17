/* Copyright (C) 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Guido Guenther <agx@sigxcpu.org>, 2003.

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

#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

#ifdef __PIC__
# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				      \
  .align 2;								      \
  99: move a0, v0; 							      \
  la t9,__syscall_error;						      \
  jr t9;								      \
  ENTRY (name)								      \
    .set noreorder;							      \
    .cpload t9;								      \
    .set reorder;							      \
    SINGLE_THREAD_P(t0);						      \
    bne zero, t0, L(pseudo_cancel);					      \
    .set noreorder;							      \
    li v0, SYS_ify(syscall_name);					      \
    syscall;								      \
    .set reorder;							      \
    bne a3, zero, SYSCALL_ERROR_LABEL;			       		      \
    ret;								      \
  L(pseudo_cancel):							      \
    SAVESTK_##args;						              \
    sw ra, 28(sp);							      \
    sw gp, 32(sp);							      \
    PUSHARGS_##args;			/* save syscall args */	      	      \
    CENABLE;								      \
    lw gp, 32(sp);							      \
    sw v0, 44(sp);			/* save mask */			      \
    POPARGS_##args;			/* restore syscall args */	      \
    .set noreorder;							      \
    li v0, SYS_ify (syscall_name);				      	      \
    syscall;								      \
    .set reorder;							      \
    sw v0, 36(sp);			/* save syscall result */             \
    sw a3, 40(sp);			/* save syscall error flag */	      \
    lw a0, 44(sp);			/* pass mask as arg1 */		      \
    CDISABLE;								      \
    lw gp, 32(sp);							      \
    lw v0, 36(sp);			/* restore syscall result */          \
    lw a3, 40(sp);			/* restore syscall error flag */      \
    lw ra, 28(sp);			/* restore return address */	      \
    RESTORESTK;							              \
    bne a3, zero, SYSCALL_ERROR_LABEL;					      \
  L(pseudo_end):
#endif

# define PUSHARGS_0	/* nothing to do */
# define PUSHARGS_1	PUSHARGS_0 sw a0, 0(sp);
# define PUSHARGS_2	PUSHARGS_1 sw a1, 4(sp);
# define PUSHARGS_3	PUSHARGS_2 sw a2, 8(sp);
# define PUSHARGS_4	PUSHARGS_3 sw a3, 12(sp);
# define PUSHARGS_5	PUSHARGS_4 /* handeld by SAVESTK_## */
# define PUSHARGS_6	PUSHARGS_5
# define PUSHARGS_7	PUSHARGS_6

# define POPARGS_0	/* nothing to do */
# define POPARGS_1	POPARGS_0 lw a0, 0(sp);
# define POPARGS_2	POPARGS_1 lw a1, 4(sp);
# define POPARGS_3	POPARGS_2 lw a2, 8(sp);
# define POPARGS_4	POPARGS_3 lw a3, 12(sp);
# define POPARGS_5	POPARGS_4 /* args already in new stackframe */
# define POPARGS_6	POPARGS_5
# define POPARGS_7	POPARGS_6


# define STKSPACE	48
# define SAVESTK_0 	subu sp, STKSPACE
# define SAVESTK_1      SAVESTK_0
# define SAVESTK_2      SAVESTK_1
# define SAVESTK_3      SAVESTK_2
# define SAVESTK_4      SAVESTK_3
# define SAVESTK_5      lw t0, 16(sp);		\
			subu sp, STKSPACE;	\
			sw t0, 16(sp)

# define SAVESTK_6      lw t0, 16(sp);		\
			lw t1, 20(sp);		\
			subu sp, STKSPACE;	\
			sw t0, 16(sp);		\
			sw t1, 20(sp)

# define SAVESTK_7      lw t0, 16(sp);		\
			lw t1, 20(sp);		\
			lw t2, 24(sp);		\
			subu sp, STKSPACE;	\
			sw t0, 16(sp);		\
			sw t1, 20(sp);		\
			sw t2, 24(sp)

# define RESTORESTK 	addu sp, STKSPACE


# ifdef IS_IN_libpthread
#  define CENABLE	la t9, __pthread_enable_asynccancel; jalr t9;
#  define CDISABLE	la t9, __pthread_disable_asynccancel; jalr t9;
#  define __local_multiple_threads __pthread_multiple_threads
# elif defined IS_IN_librt
#  define CENABLE	la t9, __librt_enable_asynccancel; jalr t9;
#  define CDISABLE	la t9, __librt_disable_asynccancel; jalr t9;
#  define __local_multiple_threads __librt_multiple_threads
# else
#  define CENABLE	la t9, __libc_enable_asynccancel; jalr t9;
#  define CDISABLE	la t9, __libc_disable_asynccancel; jalr t9;
#  define __local_multiple_threads __libc_multiple_threads
# endif

# ifndef __ASSEMBLER__
extern int __local_multiple_threads attribute_hidden;
#  define SINGLE_THREAD_P __builtin_expect (__local_multiple_threads == 0, 1)
# else
#  define SINGLE_THREAD_P(reg) lw reg, __local_multiple_threads
#endif

#elif !defined __ASSEMBLER__

/* This code should never be used but we define it anyhow.  */
# define SINGLE_THREAD_P (1)

#endif
