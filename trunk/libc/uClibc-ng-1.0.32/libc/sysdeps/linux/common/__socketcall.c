/*
 * __socketcall() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

/* At the time of this writing,
 * several arches provide the individual calls and do _not_ go through
 * this demuxer.
 *
 * Verify all arches supported by your kernel before you remove the
 * guard below!
 */
#ifdef __NR_socketcall
#include <sys/socket.h>

#define __NR___socketcall __NR_socketcall
_syscall2(int, __socketcall, int, call, unsigned long *, args)

#endif
