/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <features.h>
#define SIGCONTEXT struct sigcontext *
#define SIGCONTEXT_EXTRA_ARGS

#define GET_PC(ctx)	((void *) ctx->regs.scratch.ret)
#define GET_FRAME(ctx)	((void *) ctx->regs.scratch.fp)
#define GET_STACK(ctx)	((void *) ctx->regs.scratch.sp)
#define CALL_SIGHANDLER(handler, signo, ctx) \
  (handler)((signo), SIGCONTEXT_EXTRA_ARGS (ctx))
