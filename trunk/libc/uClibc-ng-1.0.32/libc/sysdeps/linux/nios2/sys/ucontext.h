/* Copyright (C) 1998, 1999 Free Software Foundation, Inc.
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

#ifndef _SYS_UCONTEXT_H
#define _SYS_UCONTEXT_H	1

#include <features.h>
#include <signal.h>
/*
 * Location of the users' stored registers relative to R0.
 * Usage is as an index into a gregset_t array or as u.u_ar0[XX].
 */
#define REG_PSR (0)
#define REG_PC  (1)
#define REG_SPARE   (2)
#define REG_WVALID  (3)
#define REG_G1  (4)
#define REG_G2  (5)
#define REG_G3  (6)
#define REG_G4  (7)
#define REG_G5  (8)
#define REG_G6  (9)
#define REG_G7  (10)
#define REG_O0  (11)
#define REG_O1  (12)
#define REG_O2  (13)
#define REG_O3  (14)
#define REG_O4  (15)
#define REG_O5  (16)
#define REG_O6  (17)
#define REG_O7  (18)
#define REG_GLOBALS (19)

/*
 * A gregset_t is defined as an array type for compatibility with the reference
 * source. This is important due to differences in the way the C language
 * treats arrays and structures as parameters.
 *
 * Note that NGREG is really (sizeof (struct regs) / sizeof (greg_t)),
 * but that the ABI defines it absolutely to be 21 (resp. 19).
 */

#define NGREG   20
typedef int greg_t;

typedef greg_t  gregset_t[NGREG];

/*
 * The following structures define how a register window can appear on the
 * stack. This structure is available (when required) through the `gwins'
 * field of an mcontext (nested within ucontext). NIOS_MAXWINDOW is the
 * maximum number of outstanding register windows defined in the NIOS
 * architecture (*not* implementation).
 */
#define NIOS_MAXREGWINDOW	31	/* max windows in NIOS arch. */
struct  rwindow
  {
    greg_t rw_local[8];			/* locals */
    greg_t rw_in[8];			/* ins */
  };

#define rw_fp   rw_in[6]		/* frame pointer */
#define rw_rtn  rw_in[7]		/* return address */

typedef struct gwindows
  {
    int            wbcnt;
    int           *spbuf[NIOS_MAXREGWINDOW];
    struct rwindow wbuf[NIOS_MAXREGWINDOW];
  } gwindows_t;

typedef struct
  {
    gregset_t   gregs;		/* general register set */
    gwindows_t  *gwins;		/* POSSIBLE pointer to register windows */
  } mcontext_t;


/* Userlevel context.  */
typedef struct ucontext
  {
    unsigned long   uc_flags;
    struct ucontext *uc_link;
    __sigset_t	    uc_sigmask;
    stack_t         uc_stack;
    mcontext_t      uc_mcontext;
  } ucontext_t;

#endif /* sys/ucontext.h */
