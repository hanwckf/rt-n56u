/* system call stubs with cancellation handling.  Linux/MIPS version.
   Copyright (C) 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Demetriou of Broadcom Corporation,
   based on work by Guido Guenther <agx@sigxcpu.org>.

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
#include <sys/asm.h>

#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

#ifdef __PIC__
# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				      \
  .align 2;								      \
  99:									      \
  PTR_LA t9,__syscall_error;					 	      \
  /* manual cpreturn.  */						      \
  REG_L gp, STKOFF_GP(sp);						      \
  RESTORESTK ;								      \
  jr t9;								      \
  ENTRY (name)								      \
    SAVESTK ;								      \
    .cpsetup t9, STKOFF_GP, name ;					      \
    .set reorder;							      \
    SINGLE_THREAD_P(t0);						      \
    bne zero, t0, L(pseudo_cancel);					      \
    .set noreorder;							      \
    li v0, SYS_ify(syscall_name);					      \
    syscall;								      \
    .set reorder;							      \
    bne a3, zero, SYSCALL_ERROR_LABEL;			       		      \
    /* manual cpreturn.  */						      \
    REG_L gp, STKOFF_GP(sp);						      \
    RESTORESTK ;							      \
    ret;								      \
  L(pseudo_cancel):							      \
    REG_S ra, STKOFF_RA(sp);						      \
    PUSHARGS_##args;			/* save syscall args */	      	      \
    CENABLE;								      \
    REG_S v0, STKOFF_SVMSK(sp);		/* save mask */			      \
    POPARGS_##args;			/* restore syscall args */	      \
    .set noreorder;							      \
    li v0, SYS_ify (syscall_name);				      	      \
    syscall;								      \
    .set reorder;							      \
    REG_S v0, STKOFF_SC_V0(sp);		/* save syscall result */             \
    REG_S a3, STKOFF_SC_ERR(sp);	/* save syscall error flag */	      \
    REG_L a0, STKOFF_SVMSK(sp);		/* pass mask as arg1 */		      \
    CDISABLE;								      \
    REG_L a3, STKOFF_SC_ERR(sp);	/* restore syscall error flag */      \
    REG_L ra, STKOFF_RA(sp);		/* restore return address */	      \
    REG_L v0, STKOFF_SC_V0(sp);		/* restore syscall result */          \
    bne a3, zero, SYSCALL_ERROR_LABEL;					      \
    /* manual cpreturn.  */						      \
    REG_L gp, STKOFF_GP(sp);						      \
    RESTORESTK ;							      \
  L(pseudo_end):
#endif

# define PUSHARGS_0	/* nothing to do */
# define PUSHARGS_1	PUSHARGS_0 REG_S a0, STKOFF_A0(sp);
# define PUSHARGS_2	PUSHARGS_1 REG_S a1, STKOFF_A1(sp);
# define PUSHARGS_3	PUSHARGS_2 REG_S a2, STKOFF_A2(sp);
# define PUSHARGS_4	PUSHARGS_3 REG_S a3, STKOFF_A3(sp);
# define PUSHARGS_5	PUSHARGS_4 REG_S a4, STKOFF_A4(sp);
# define PUSHARGS_6	PUSHARGS_5 REG_S a5, STKOFF_A5(sp);

# define POPARGS_0	/* nothing to do */
# define POPARGS_1	POPARGS_0 REG_L a0, STKOFF_A0(sp);
# define POPARGS_2	POPARGS_1 REG_L a1, STKOFF_A1(sp);
# define POPARGS_3	POPARGS_2 REG_L a2, STKOFF_A2(sp);
# define POPARGS_4	POPARGS_3 REG_L a3, STKOFF_A3(sp);
# define POPARGS_5	POPARGS_4 REG_L a4, STKOFF_A4(sp);
# define POPARGS_6	POPARGS_5 REG_L a5, STKOFF_A5(sp);

/* Save an even number of slots.  Should be 0 if an even number of slots
   are used below, or SZREG if an odd number are used.  */
# define STK_PAD	SZREG

/* Place values that we are more likely to use later in this sequence, i.e.
   closer to the SP at function entry.  If you do that, the are more
   likely to already be in your d-cache.  */
# define STKOFF_A5	(STK_PAD)
# define STKOFF_A4	(STKOFF_A5 + SZREG)
# define STKOFF_A3	(STKOFF_A4 + SZREG)
# define STKOFF_A2	(STKOFF_A3 + SZREG)	/* MT and more args.  */
# define STKOFF_A1	(STKOFF_A2 + SZREG)	/* MT and 2 args.  */
# define STKOFF_A0	(STKOFF_A1 + SZREG)	/* MT and 1 arg.  */
# define STKOFF_RA	(STKOFF_A0 + SZREG)	/* Used if MT.  */
# define STKOFF_SC_V0	(STKOFF_RA + SZREG)	/* Used if MT.  */
# define STKOFF_SC_ERR	(STKOFF_SC_V0 + SZREG)	/* Used if MT.  */
# define STKOFF_SVMSK	(STKOFF_SC_ERR + SZREG)	/* Used if MT.  */
# define STKOFF_GP	(STKOFF_SVMSK + SZREG)	/* Always used.  */

# define STKSPACE	(STKOFF_GP + SZREG)
# define SAVESTK 	PTR_SUBU sp, STKSPACE
# define RESTORESTK 	PTR_ADDU sp, STKSPACE

# ifdef IS_IN_libpthread
#  define CENABLE	PTR_LA t9, __pthread_enable_asynccancel; jalr t9;
#  define CDISABLE	PTR_LA t9, __pthread_disable_asynccancel; jalr t9;
#  define __local_multiple_threads __pthread_multiple_threads
# elif defined IS_IN_librt
#  define CENABLE	PTR_LA t9, __librt_enable_asynccancel; jalr t9;
#  define CDISABLE	PTR_LA t9, __librt_disable_asynccancel; jalr t9;
#  define __local_multiple_threads __librt_multiple_threads
# else
#  define CENABLE	PTR_LA t9, __libc_enable_asynccancel; jalr t9;
#  define CDISABLE	PTR_LA t9, __libc_disable_asynccancel; jalr t9;
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
