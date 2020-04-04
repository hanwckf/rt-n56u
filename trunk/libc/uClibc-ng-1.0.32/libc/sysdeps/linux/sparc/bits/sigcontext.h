/* Copyright (C) 2000 Free Software Foundation, Inc.
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

#if !defined _SIGNAL_H && !defined _SYS_UCONTEXT_H
# error "Never use <bits/sigcontext.h> directly; include <signal.h> instead."
#endif

#include <bits/wordsize.h>

/* It is quite hard to choose what to put here, because
   Linux/sparc32 had at least 3 totally incompatible
   signal stack layouts.
   This one is for the "new" style signals, which are
   now delivered unless SA_SIGINFO is requested.  */

struct sigcontext
  {
    struct
      {
	unsigned int	psr;
	unsigned int	pc;
	unsigned int	npc;
	unsigned int	y;
	unsigned int	u_regs[16]; /* globals and ins */
      }			si_regs;
    int			si_mask;
  };

