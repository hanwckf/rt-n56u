/* Copyright (C) 1999, 2000, 2001 Free Software Foundation, Inc.
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
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* Where is System V/SH ABI?  */

#ifndef _SYS_UCONTEXT_H
#define _SYS_UCONTEXT_H	1

#include <features.h>
#include <signal.h>

/* We need the signal context definitions even if they are not used
   included in <signal.h>.  */
#include <bits/sigcontext.h>


typedef long long greg_t;

/* Number of general registers.  */
#define NGREG	64

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
  R16 = 16,
#define R16	R16
  R17 = 17,
#define R17	R17
  R18 = 18,
#define R18	R18
  R19 = 19,
#define R19	R19
  R20 = 20,
#define R20	R20
  R21 = 21,
#define R21	R21
  R22 = 22,
#define R22	R22
  R23 = 23,
#define R23	R23
  R24 = 24,
#define R24	R24
  R25 = 25,
#define R25	R25
  R26 = 26,
#define R26	R26
  R27 = 27,
#define R27	R27
  R28 = 28,
#define R28	R28
  R29 = 29,
#define R29	R29
  R30 = 30,
#define R30	R30
  R31 = 31,
#define R31	R31
  R32 = 32,
#define R32	R32
  R33 = 33,
#define R33	R33
  R34 = 34,
#define R34	R34
  R35 = 35,
#define R35	R35
  R36 = 36,
#define R36	R36
  R37 = 37,
#define R37	R37
  R38 = 38,
#define R38	R38
  R39 = 39,
#define R39	R39
  R40 = 40,
#define R40	R40
  R41 = 41,
#define R41	R41
  R42 = 42,
#define R42	R42
  R43 = 43,
#define R43	R43
  R44 = 44,
#define R44	R44
  R45 = 45,
#define R45	R45
  R46 = 46,
#define R46	R46
  R47 = 47,
#define R47	R47
  R48 = 48,
#define R48	R48
  R49 = 49,
#define R49	R49
  R50 = 50,
#define R50	R50
  R51 = 51,
#define R51	R51
  R52 = 52,
#define R52	R52
  R53 = 53,
#define R53	R53
  R54 = 54,
#define R54	R54
  R55 = 55,
#define R55	R55
  R56 = 56,
#define R56	R56
  R57 = 57,
#define R57	R57
  R58 = 58,
#define R58	R58
  R59 = 59,
#define R59	R59
  R60 = 60,
#define R60	R60
  R61 = 61,
#define R61	R61
  R62 = 62,
#define R62	R62
  R63 = 63,
#define R63	R63
};
#endif

typedef int freg_t;

/* Number of FPU registers.  */
#define NFPREG	32

/* Structure to describe FPU registers.  */
typedef freg_t fpregset_t[NFPREG];

/* Context to describe whole processor state.  */
typedef struct
  {
    gregset_t gregs;
    fpregset_t fpregs;
    unsigned long long sc_tregs[8];
    unsigned long long sc_pc;
    unsigned long long sc_sr;
    unsigned long long sc_fpscr;
    
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
