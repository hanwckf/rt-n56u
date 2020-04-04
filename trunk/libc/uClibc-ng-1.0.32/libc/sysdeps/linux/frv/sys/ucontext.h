/* Copyright (C) 1999, 2000, 2001, 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   see <http://www.gnu.org/licenses/>.  */

#ifndef _SYS_UCONTEXT_H
#define _SYS_UCONTEXT_H	1

#include <features.h>
#include <signal.h>

/* We need the signal context definitions even if they are not used
   included in <signal.h>.  */
#include <bits/sigcontext.h>


typedef unsigned long greg_t;

/* Number of general registers.  */
#define NGREG	(10+2+64)

/* Container for all general registers.  */
typedef greg_t gregset_t[NGREG];

#ifdef __USE_GNU
/* Number of each register is the `gregset_t' array.  */
enum
{
  REG_PSR = 0,
#define REG_PSR	REG_PSR
  REG_ISR = 1,
#define REG_ISR	REG_ISR
  REG_CCR = 2,
#define REG_CCR	REG_CCR
  REG_CCCR = 3,
#define REG_CCCR	REG_CCCR
  REG_LR = 4,
#define REG_LR	REG_LR
  REG_LCR = 5,
#define REG_LCR	REG_LCR
  REG_PC = 6,
#define REG_PC	REG_PC
  REG___STATUS = 7,
#define REG___STATUS	REG___STATUS
  REG_SYSCALLNO = 8,
#define REG_SYSCALLNO	REG_SYSCALLNO
  REG_ORIG_GR8 = 9,
#define REG_ORIG_GR8	REG_ORIG_GR8
  REG_GNER0 = 10,
#define REG_GNER0	REG_GNER0
  REG_GNER1 = 11,
#define REG_GNER1	REG_GNER1
  REG_GR0 = 12,
#define REG_GR0	REG_GR0
  REG_GR1 = 13,
#define REG_GR1	REG_GR1
  REG_GR2 = 14,
#define REG_GR2	REG_GR2
  REG_GR3 = 15,
#define REG_GR3	REG_GR3
  REG_GR4 = 16,
#define REG_GR4	REG_GR4
  REG_GR5 = 17,
#define REG_GR5	REG_GR5
  REG_GR6 = 18,
#define REG_GR6	REG_GR6
  REG_GR7 = 19,
#define REG_GR7	REG_GR7
  REG_GR8 = 20,
#define REG_GR8	REG_GR8
  REG_GR9 = 21,
#define REG_GR9	REG_GR9
  REG_GR10 = 22,
#define REG_GR10	REG_GR10
  REG_GR11 = 23,
#define REG_GR11	REG_GR11
  REG_GR12 = 24,
#define REG_GR12	REG_GR12
  REG_GR13 = 25,
#define REG_GR13	REG_GR13
  REG_GR14 = 26,
#define REG_GR14	REG_GR14
  REG_GR15 = 27,
#define REG_GR15	REG_GR15
  REG_GR16 = 28,
#define REG_GR16	REG_GR16
  REG_GR17 = 29,
#define REG_GR17	REG_GR17
  REG_GR18 = 30,
#define REG_GR18	REG_GR18
  REG_GR19 = 31,
#define REG_GR19	REG_GR19
  REG_GR20 = 32,
#define REG_GR20	REG_GR20
  REG_GR21 = 33,
#define REG_GR21	REG_GR21
  REG_GR22 = 34,
#define REG_GR22	REG_GR22
  REG_GR23 = 35,
#define REG_GR23	REG_GR23
  REG_GR24 = 36,
#define REG_GR24	REG_GR24
  REG_GR25 = 37,
#define REG_GR25	REG_GR25
  REG_GR26 = 38,
#define REG_GR26	REG_GR26
  REG_GR27 = 39,
#define REG_GR27	REG_GR27
  REG_GR28 = 40,
#define REG_GR28	REG_GR28
  REG_GR29 = 41,
#define REG_GR29	REG_GR29
  REG_GR30 = 42,
#define REG_GR30	REG_GR30
  REG_GR31 = 43,
#define REG_GR31	REG_GR31
  REG_GR32 = 44,
#define REG_GR32	REG_GR32
  REG_GR33 = 45,
#define REG_GR33	REG_GR33
  REG_GR34 = 46,
#define REG_GR34	REG_GR34
  REG_GR35 = 47,
#define REG_GR35	REG_GR35
  REG_GR36 = 48,
#define REG_GR36	REG_GR36
  REG_GR37 = 49,
#define REG_GR37	REG_GR37
  REG_GR38 = 50,
#define REG_GR38	REG_GR38
  REG_GR39 = 51,
#define REG_GR39	REG_GR39
  REG_GR40 = 52,
#define REG_GR40	REG_GR40
  REG_GR41 = 53,
#define REG_GR41	REG_GR41
  REG_GR42 = 54,
#define REG_GR42	REG_GR42
  REG_GR43 = 55,
#define REG_GR43	REG_GR43
  REG_GR44 = 56,
#define REG_GR44	REG_GR44
  REG_GR45 = 57,
#define REG_GR45	REG_GR45
  REG_GR46 = 58,
#define REG_GR46	REG_GR46
  REG_GR47 = 59,
#define REG_GR47	REG_GR47
  REG_GR48 = 60,
#define REG_GR48	REG_GR48
  REG_GR49 = 61,
#define REG_GR49	REG_GR49
  REG_GR50 = 62,
#define REG_GR50	REG_GR50
  REG_GR51 = 63,
#define REG_GR51	REG_GR51
  REG_GR52 = 64,
#define REG_GR52	REG_GR52
  REG_GR53 = 65,
#define REG_GR53	REG_GR53
  REG_GR54 = 66,
#define REG_GR54	REG_GR54
  REG_GR55 = 67,
#define REG_GR55	REG_GR55
  REG_GR56 = 68,
#define REG_GR56	REG_GR56
  REG_GR57 = 69,
#define REG_GR57	REG_GR57
  REG_GR58 = 70,
#define REG_GR58	REG_GR58
  REG_GR59 = 71,
#define REG_GR59	REG_GR59
  REG_GR60 = 72,
#define REG_GR60	REG_GR60
  REG_GR61 = 73,
#define REG_GR61	REG_GR61
  REG_GR62 = 74,
#define REG_GR62	REG_GR62
  REG_GR63 = 75,
#define REG_GR63	REG_GR63
};
#endif

typedef unsigned long freg_t;

/* Number of FPU registers.  */
#define NFPREG	(64+2+2+8+2+1)

#ifdef __USE_GNU
/* Number of each register is the `gregset_t' array.  */
enum
{
  REG_FR0 = 0,
#define REG_FR0	REG_FR0
  REG_FR1 = 1,
#define REG_FR1	REG_FR1
  REG_FR2 = 2,
#define REG_FR2	REG_FR2
  REG_FR3 = 3,
#define REG_FR3	REG_FR3
  REG_FR4 = 4,
#define REG_FR4	REG_FR4
  REG_FR5 = 5,
#define REG_FR5	REG_FR5
  REG_FR6 = 6,
#define REG_FR6	REG_FR6
  REG_FR7 = 7,
#define REG_FR7	REG_FR7
  REG_FR8 = 8,
#define REG_FR8	REG_FR8
  REG_FR9 = 9,
#define REG_FR9	REG_FR9
  REG_FR10 = 10,
#define REG_FR10	REG_FR10
  REG_FR11 = 11,
#define REG_FR11	REG_FR11
  REG_FR12 = 12,
#define REG_FR12	REG_FR12
  REG_FR13 = 13,
#define REG_FR13	REG_FR13
  REG_FR14 = 14,
#define REG_FR14	REG_FR14
  REG_FR15 = 15,
#define REG_FR15	REG_FR15
  REG_FR16 = 16,
#define REG_FR16	REG_FR16
  REG_FR17 = 17,
#define REG_FR17	REG_FR17
  REG_FR18 = 18,
#define REG_FR18	REG_FR18
  REG_FR19 = 19,
#define REG_FR19	REG_FR19
  REG_FR20 = 20,
#define REG_FR20	REG_FR20
  REG_FR21 = 21,
#define REG_FR21	REG_FR21
  REG_FR22 = 22,
#define REG_FR22	REG_FR22
  REG_FR23 = 23,
#define REG_FR23	REG_FR23
  REG_FR24 = 24,
#define REG_FR24	REG_FR24
  REG_FR25 = 25,
#define REG_FR25	REG_FR25
  REG_FR26 = 26,
#define REG_FR26	REG_FR26
  REG_FR27 = 27,
#define REG_FR27	REG_FR27
  REG_FR28 = 28,
#define REG_FR28	REG_FR28
  REG_FR29 = 29,
#define REG_FR29	REG_FR29
  REG_FR30 = 30,
#define REG_FR30	REG_FR30
  REG_FR31 = 31,
#define REG_FR31	REG_FR31
  REG_FR32 = 32,
#define REG_FR32	REG_FR32
  REG_FR33 = 33,
#define REG_FR33	REG_FR33
  REG_FR34 = 34,
#define REG_FR34	REG_FR34
  REG_FR35 = 35,
#define REG_FR35	REG_FR35
  REG_FR36 = 36,
#define REG_FR36	REG_FR36
  REG_FR37 = 37,
#define REG_FR37	REG_FR37
  REG_FR38 = 38,
#define REG_FR38	REG_FR38
  REG_FR39 = 39,
#define REG_FR39	REG_FR39
  REG_FR40 = 40,
#define REG_FR40	REG_FR40
  REG_FR41 = 41,
#define REG_FR41	REG_FR41
  REG_FR42 = 42,
#define REG_FR42	REG_FR42
  REG_FR43 = 43,
#define REG_FR43	REG_FR43
  REG_FR44 = 44,
#define REG_FR44	REG_FR44
  REG_FR45 = 45,
#define REG_FR45	REG_FR45
  REG_FR46 = 46,
#define REG_FR46	REG_FR46
  REG_FR47 = 47,
#define REG_FR47	REG_FR47
  REG_FR48 = 48,
#define REG_FR48	REG_FR48
  REG_FR49 = 49,
#define REG_FR49	REG_FR49
  REG_FR50 = 50,
#define REG_FR50	REG_FR50
  REG_FR51 = 51,
#define REG_FR51	REG_FR51
  REG_FR52 = 52,
#define REG_FR52	REG_FR52
  REG_FR53 = 53,
#define REG_FR53	REG_FR53
  REG_FR54 = 54,
#define REG_FR54	REG_FR54
  REG_FR55 = 55,
#define REG_FR55	REG_FR55
  REG_FR56 = 56,
#define REG_FR56	REG_FR56
  REG_FR57 = 57,
#define REG_FR57	REG_FR57
  REG_FR58 = 58,
#define REG_FR58	REG_FR58
  REG_FR59 = 59,
#define REG_FR59	REG_FR59
  REG_FR60 = 60,
#define REG_FR60	REG_FR60
  REG_FR61 = 61,
#define REG_FR61	REG_FR61
  REG_FR62 = 62,
#define REG_FR62	REG_FR62
  REG_FR63 = 63,
#define REG_FR63	REG_FR63
  REG_FNER0 = 64,
#define REG_FNER0	REG_FNER0
  REG_FNER1 = 65,
#define REG_FNER1	REG_FNER1
  REG_MSR0 = 66,
#define REG_MSR0	REG_MSR0
  REG_MSR1 = 67,
#define REG_MSR1	REG_MSR1
  REG_ACC0 = 68,
#define REG_ACC0	REG_ACC0
  REG_ACC1 = 69,
#define REG_ACC1	REG_ACC1
  REG_ACC2 = 70,
#define REG_ACC2	REG_ACC2
  REG_ACC3 = 71,
#define REG_ACC3	REG_ACC3
  REG_ACC4 = 72,
#define REG_ACC4	REG_ACC4
  REG_ACC5 = 73,
#define REG_ACC5	REG_ACC5
  REG_ACC6 = 74,
#define REG_ACC6	REG_ACC6
  REG_ACC7 = 75,
#define REG_ACC7	REG_ACC7
  REG_ACCG0123 = 76,
#define REG_ACCG0123	REG_ACCG0123
  REG_ACCG4567 = 77,
#define REG_ACCG4567	REG_ACCG4567
  REG_FSR0 = 78,
#define REG_FSR0	REG_FSR0
};
#endif

/* Structure to describe FPU registers.  */
typedef freg_t fpregset_t[NFPREG];

/* Context to describe whole processor state.  */
typedef struct
  {
    gregset_t gregs;
    fpregset_t fpregs;
    void *extension;
    unsigned long sc_oldmask; 	/* old sigmask */
  } __attribute__((aligned(8))) mcontext_t;

#ifdef __USE_GNU
struct kernel_user_int_regs
{
  /* integer registers
   * - up to gr[31] mirror pt_regs in the kernel
   */
  unsigned long		psr;		/* Processor Status Register */
  unsigned long		isr;		/* Integer Status Register */
  unsigned long		ccr;		/* Condition Code Register */
  unsigned long		cccr;		/* Condition Code for Conditional Insns Register */
  unsigned long		lr;		/* Link Register */
  unsigned long		lcr;		/* Loop Count Register */
  unsigned long		pc;		/* Program Counter Register */
  unsigned long		__status;	/* exception status */
  unsigned long		syscallno;	/* syscall number or -1 */
  unsigned long		orig_gr8;	/* original syscall arg #1 */
  unsigned long		gner[2];

  union {
    unsigned long	tbr;
    unsigned long	gr[64];
  };
};

struct kernel_user_fpmedia_regs
{
  /* FP/Media registers */
  unsigned long	fr[64];
  unsigned long	fner[2];
  unsigned long	msr[2];
  unsigned long	acc[8];
  unsigned char	accg[8];
  unsigned long	fsr[1];
};

struct kernel_user_context
{
  struct kernel_user_int_regs i;
  struct kernel_user_fpmedia_regs f;

  /* we provide a context extension so that we can save the regs for CPUs that
   * implement many more of Fujitsu's lavish register spec
   */
  void			*extension;

  /* This is not part of the kernel's struct user_context, but
     rather of the enclosing struct sigcontext, but we add it
     here to parallel mcontext_t, just for completeness.  */
  unsigned long sc_oldmask; 	/* old sigmask */
} __attribute__((aligned(8)));

/* This union enables alias-safe casts from mcontext_t* to the union
   type, that can then be dereferenced as_aliases.  */
union kmcontext_t
{
  mcontext_t as_regsets;
  /* as_aliases is actually missing sc_oldmask, that is present in
     mcontext_t.  */
  struct kernel_user_context as_aliases;
};
#endif

/* Userlevel context.  */
typedef struct ucontext
  {
    unsigned long int uc_flags;
    struct ucontext *uc_link;
    stack_t uc_stack;
    mcontext_t uc_mcontext;
    __sigset_t uc_sigmask;
  } ucontext_t;

#endif /* sys/ucontext.h */
