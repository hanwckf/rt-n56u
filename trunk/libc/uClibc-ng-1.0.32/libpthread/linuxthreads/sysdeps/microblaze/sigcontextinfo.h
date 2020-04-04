/*
 * sysdeps/microblaze/sigcontextinfo.h -- microblaze-specific pthread signal definitions
 *
 *  Copyright (C) 2002  NEC Electronics Corporation
 *  Copyright (C) 2002  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 */

#include <signal.h>

#define SIGCONTEXT struct sigcontext *
#define SIGCONTEXT_EXTRA_ARGS
