/* Copyright (C) 1997, 1998, 2000 Free Software Foundation, Inc.
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

/* Don't rely on this, the interface is currently messed up and may need to
   be broken to be fixed.  */
#ifndef _SYS_UCONTEXT_H
#define _SYS_UCONTEXT_H	1

#include <features.h>
#include <signal.h>

/* We need the signal context definitions even if they are not used
   included in <signal.h>.  */
#include <bits/sigcontext.h>


/* Type for general register.  */
typedef unsigned long int greg_t;

/* Number of general registers.  */
#define NGREG	37
#define NFPREG	33

/* Container for all general registers.  */
/* gregset_t must be an array.  The below declared array corresponds to:
typedef struct gregset {
	greg_t	g_regs[32];
	greg_t	g_hi;
	greg_t	g_lo;
	greg_t	g_pad[3];
} gregset_t;  */
typedef greg_t gregset_t[NGREG];

/* Container for all FPU registers.  */
typedef struct fpregset {
	union {
		double	fp_dregs[32];
		struct {
			float		_fp_fregs;
			unsigned int	_fp_pad;
		} fp_fregs[32];
	} fp_r;
	unsigned int	fp_csr;
	unsigned int	fp_pad;
} fpregset_t;


/* Context to describe whole processor state.  */
typedef struct
  {
    gregset_t gregs;
    fpregset_t fpregs;
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
