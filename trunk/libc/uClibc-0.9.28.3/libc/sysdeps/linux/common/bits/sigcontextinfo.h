/* Copyright (C) 1998, 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

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

/* In general we cannot provide any information.  */
#define SIGCONTEXT struct sigcontext *
#define SIGCONTEXT_EXTRA_ARGS
#define GET_PC(ctx)	((void *) 0)
#define GET_FRAME(ctx)	((void *) 0)
#define GET_STACK(ctx)	((void *) 0)
#define CALL_SIGHANDLER(handler, signo, ctx) \
  (handler)((signo), SIGCONTEXT_EXTRA_ARGS (ctx))
