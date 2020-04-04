/*
 * unshare() for uClibc
 *
 * Copyright (C) 2011 Henning Heinold <heinold@inf.fu-berlin.de>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sched.h>

#if defined __NR_unshare
_syscall1(int, unshare, int, flags)
#endif
