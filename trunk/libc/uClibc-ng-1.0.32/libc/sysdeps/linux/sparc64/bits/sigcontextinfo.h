/* Copyright (C) 1999-2017 Free Software Foundation, Inc.
   Contributed by Jakub Jelinek <jj@ultra.linux.cz>, 1999.

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

#ifndef STACK_BIAS
#define STACK_BIAS 2047
#endif
#define SIGCONTEXT struct sigcontext *
#define SIGCONTEXT_EXTRA_ARGS
#define GET_PC(__ctx)	((void *) ((__ctx)->sigc_regs.tpc))
#define ADVANCE_STACK_FRAME(__next) \
	((void *) (((unsigned long *) (((unsigned long int) (__next))     \
					   + STACK_BIAS))+14))
#define GET_STACK(__ctx)	((void *) ((__ctx)->sigc_regs.u_regs[14]))
#define GET_FRAME(__ctx)	ADVANCE_STACK_FRAME (GET_STACK (__ctx))
#define CALL_SIGHANDLER(handler, signo, ctx) \
  (handler)((signo), SIGCONTEXT_EXTRA_ARGS (ctx))
