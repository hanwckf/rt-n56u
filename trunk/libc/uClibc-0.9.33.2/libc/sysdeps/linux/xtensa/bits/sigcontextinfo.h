/* Copyright (C) 2003, 2007 Free Software Foundation, Inc.
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
   Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* Also see register-dump.h, where we spill live registers to the
   stack so that we can trace the stack backward.  */

#define SIGCONTEXT unsigned long _info, ucontext_t *
#define SIGCONTEXT_EXTRA_ARGS _info,

/* ANDing with 0x3fffffff clears the window-size bits.
   Assumes TASK_SIZE = 0x40000000.  */

#define GET_PC(ctx)	((void *) (ctx->uc_mcontext.sc_pc & 0x3fffffff))
#define GET_FRAME(ctx)	((void *) ctx->uc_mcontext.sc_a[1])
#define GET_STACK(ctx)	((void *) ctx->uc_mcontext.sc_a[1])
#define CALL_SIGHANDLER(handler, signo, ctx) \
  (handler)((signo), SIGCONTEXT_EXTRA_ARGS (ctx))

