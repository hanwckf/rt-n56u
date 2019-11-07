/*
 * Copyright (C) 2016-2017 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Copyright (C) 1999-2013 Free Software Foundation, Inc.
   Contributed by Philip Blundell <philb@gnu.org>, 1999.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sys/ucontext.h>

#define SIGCONTEXT siginfo_t *_si, struct ucontext *
#define SIGCONTEXT_EXTRA_ARGS _si,

#define GET_PC(ctx)	((void *) (ctx)->uc_mcontext.nds32_ipc)
#define GET_FRAME(ctx)	ADVANCE_STACK_FRAME ((void *) ctx->uc_mcontext.nds32_fp)
#define GET_STACK(ctx)	((void *) (ctx)->uc_mcontext.nds32_sp)


#define CALL_SIGHANDLER(handler, signo, ctx) \
  (handler)((signo), SIGCONTEXT_EXTRA_ARGS (ctx))

