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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

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

/* Number of each register is the `gregset_t' array.  */
enum
{
  R0 = 0,
#define R0	R0
  R1 = 1,
#define R1	R1
  R2 = 2,
#define R2	R2
  R3 = 3,
#define R3	R3
  R4 = 4,
#define R4	R4
  R5 = 5,
#define R5	R5
  R6 = 6,
#define R6	R6
  R7 = 7,
#define R7	R7
  P0 = 8,
#define P0	P0
  P1 = 9,
#define P1	P1
  P2 = 10,
#define P2	P2
  P3 = 11,
#define P3	P3
  P4 = 12,
#define P4	P4
  P5 = 13,
#define P5	P5
  USP = 14,
#define USP	USP
  A0W = 15,
#define A0W	A0W
  A1W = 16,
#define A1W	A1W
  A0X = 17,
#define A0X	A0X
  A1X = 18,
#define A1X	A1X
  ASTAT = 19,
#define ASTAT	ASTAT
  RETS = 20,
#define RETS	RETS
  PC= 21,
#define PC	PC
  RETX = 22,
#define RETX	RETX
  FP = 23,
#define FP	FP
  I0 = 24,
#define I0	I0
  I1 = 25,
#define I1	I1
  I2 = 26,
#define I2	I2
  I3 = 27,
#define I3	I3
  M0 = 28,
#define M0	M0
  M1 = 29,
#define M1	M1
  M2 = 30,
#define M2	M2
  M3 = 31,
#define M3	M3
  L0 = 32,
#define L0	L0
  L1 = 33,
#define L1	L1
  L2 = 34,
#define L2	L2
  L3 = 35,
#define L3	L3
  B_0 = 36,
#define B_0	B_0
  B1 = 37,
#define B1	B1
  B2 = 38,
#define B2	B2
  B3 = 39,
#define B3	B3
  LC0 = 40,
#define LC0	LC0
  LC1 = 41,
#define LC1	LC1
  LT0 = 42,
#define LT0	LT0
  LT1 = 43,
#define LT1	LT1
  LB0 = 44,
#define LB0	LB0
  LB1 = 45,
#define LB1	LB1
  SEQSTAT = 46
#define	SEQSTAT	SEQSTAT
};

/* Context to describe whole processor state.  */
typedef struct
{
  int version;
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
