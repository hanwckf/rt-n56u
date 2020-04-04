/* Copyright (C) 1997, 1999, 2001 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* System V/blackfin ABI compliant context switching support.  */

#ifndef _SYS_UCONTEXT_H
#define _SYS_UCONTEXT_H	1

#include <features.h>
#include <signal.h>

/* Type for general register.  */
typedef int greg_t;

/* Number of general registers.  */
#define NGREG	47

/* Container for all general registers.  */
typedef greg_t gregset_t[NGREG];

/* There is no user thread context implementation for bfin, avoid
   clashing with gcc symbols, see:
   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=47779
*/
#if 0
/* Number of each register is the `gregset_t' array.  */
enum
{
  REG_R0 = 0,
#define REG_R0	REG_R0
  REG_R1 = 1,
#define REG_R1	REG_R1
  REG_R2 = 2,
#define REG_R2	REG_R2
  REG_R3 = 3,
#define REG_R3	REG_R3
  REG_R4 = 4,
#define REG_R4	REG_R4
  REG_R5 = 5,
#define REG_R5	REG_R5
  REG_R6 = 6,
#define REG_R6	REG_R6
  REG_R7 = 7,
#define REG_R7	REG_R7
  REG_P0 = 8,
#define REG_P0	REG_P0
  REG_P1 = 9,
#define REG_P1	REG_P1
  REG_P2 = 10,
#define REG_P2	REG_P2
  REG_P3 = 11,
#define REG_P3	REG_P3
  REG_P4 = 12,
#define REG_P4	REG_P4
  REG_P5 = 13,
#define REG_P5	REG_P5
  REG_USP = 14,
#define REG_USP	REG_USP
  REG_A0W = 15,
#define REG_A0W	REG_A0W
  REG_A1W = 16,
#define REG_A1W	REG_A1W
  REG_A0X = 17,
#define REG_A0X	REG_A0X
  REG_A1X = 18,
#define REG_A1X	REG_A1X
  REG_ASTAT = 19,
#define REG_ASTAT	REG_ASTAT
  REG_RETS = 20,
#define REG_RETS	REG_RETS
  REG_PC= 21,
#define REG_PC	REG_PC
  REG_RETX = 22,
#define REG_RETX	REG_RETX
  REG_FP = 23,
#define REG_FP	REG_FP
  REG_I0 = 24,
#define REG_I0	REG_I0
  REG_I1 = 25,
#define REG_I1	REG_I1
  REG_I2 = 26,
#define REG_I2	REG_I2
  REG_I3 = 27,
#define REG_I3	REG_I3
  REG_M0 = 28,
#define REG_M0	REG_M0
  REG_M1 = 29,
#define REG_M1	REG_M1
  REG_M2 = 30,
#define REG_M2	REG_M2
  REG_M3 = 31,
#define REG_M3	REG_M3
  REG_L0 = 32,
#define REG_L0	REG_L0
  REG_L1 = 33,
#define REG_L1	REG_L1
  REG_L2 = 34,
#define REG_L2	REG_L2
  REG_L3 = 35,
#define REG_L3	REG_L3
  REG_B_0 = 36,
#define REG_B0	REG_B0
  REG_B1 = 37,
#define REG_B1	REG_B1
  REG_B2 = 38,
#define REG_B2	REG_B2
  REG_B3 = 39,
#define REG_B3	REG_B3
  REG_LC0 = 40,
#define REG_LC0	REG_LC0
  REG_LC1 = 41,
#define REG_LC1	REG_LC1
  REG_LT0 = 42,
#define REG_LT0	REG_LT0
  REG_LT1 = 43,
#define REG_LT1	REG_LT1
  REG_LB0 = 44,
#define REG_LB0	REG_LB0
  REG_LB1 = 45,
#define REG_LB1	REG_LB1
  REG_SEQSTAT = 46
#define	REG_SEQSTAT	REG_SEQSTAT
};
#endif

/* Context to describe whole processor state.  */
typedef struct
{
  gregset_t gregs;
} mcontext_t;


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
