/*
 * Low-level statistical profiling support function.  Linux/Meta version.
 *
 * Copyright (C) 2013, Imagination Technologies Ltd.
 *
 * Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 *
 * Based on the SH version from the GNU C Library.
 */

#include <signal.h>

static void
profil_counter (int signo, struct sigcontext sc)
{
  profil_count (sc.cbuf.ctx.CurrPC);
}
