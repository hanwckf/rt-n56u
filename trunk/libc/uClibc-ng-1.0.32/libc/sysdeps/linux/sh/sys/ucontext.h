/* Copyright (C) 1999, 2000, 2001 Free Software Foundation, Inc.
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

/* Where is System V/SH ABI?  */

#ifndef _SYS_UCONTEXT_H
#define _SYS_UCONTEXT_H	1

#include <features.h>
#include <signal.h>

/* We need the signal context definitions even if they are not used
   included in <signal.h>.  */
#include <bits/sigcontext.h>


typedef int greg_t;

/* Number of general registers.  */
#define NGREG	16

/* Container for all general registers.  */
typedef greg_t gregset_t[NGREG];

#ifdef __USE_GNU
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
  R8 = 8,
#define R8	R8
  R9 = 9,
#define R9	R9
  R10 = 10,
#define R10	R10
  R11 = 11,
#define R11	R11
  R12 = 12,
#define R12	R12
  R13 = 13,
#define R13	R13
  R14 = 14,
#define R14	R14
  R15 = 15,
#define R15	R15
};
#endif

typedef int freg_t;

/* Number of FPU registers.  */
#define NFPREG	16

/* Structure to describe FPU registers.  */
typedef freg_t fpregset_t[NFPREG];

/* Context to describe whole processor state.  */
typedef struct
  {
    unsigned int oldmask;

    /* CPU registers */
    gregset_t gregs;
    unsigned int pc;
    unsigned int pr;
    unsigned int sr;
    unsigned int gbr;
    unsigned int mach;
    unsigned int macl;

#ifdef __SH4__
    /* FPU registers */
    fpregset_t fpregs;
    fpregset_t xfpregs;
    unsigned int fpscr;
    unsigned int fpul;
    unsigned int ownedfp;
#endif
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
