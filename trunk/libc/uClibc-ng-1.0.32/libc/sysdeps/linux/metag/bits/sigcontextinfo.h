/*
 * Copyright (C) 2013, Imagination Technologies Ltd.
 *
 * Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 *
 */

#define SIGCONTEXT struct sigcontext
#define SIGCONTEXT_EXTRA_ARGS
#define GET_PC(ctx)	((void *) ctx.cbuf.ctx.CurrPC)
#define GET_FRAME(ctx)	((void *) ctx.cbuf.ctx.AX[1].U0)
#define GET_STACK(ctx)	((void *) ctx.cbuf.ctx.AX[0].U0)
#define CALL_SIGHANDLER(handler, signo, ctx) \
  (handler)((signo), SIGCONTEXT_EXTRA_ARGS (ctx))
