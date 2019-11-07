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

/* H8/300 compliant context switching support.  */

#ifndef _SYS_UCONTEXT_H
#define _SYS_UCONTEXT_H	1

#include <features.h>
#include <signal.h>

typedef int greg_t;

/* Number of general registers.  */
#define NFPREG	8

/* Container for all general registers.  */
typedef greg_t gregset_t[NFPREG];

#ifdef __USE_GNU
/* Number of each register is the `gregset_t' array.  */
enum
{
  ER0 = 0,
#define ER0	ER0
  ER1 = 1,
#define ER1	ER1
  ER2 = 2,
#define ER2	ER2
  ER3 = 3,
#define ER3	ER3
  ER4 = 4,
#define ER4	ER4
  ER5 = 5,
#define ER5	ER5
  ER6 = 6,
#define ER6	ER6
  ER7 = 7,
#define ER7	ER7
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

#endif

