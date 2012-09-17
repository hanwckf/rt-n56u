/* vi: set sw=4 ts=4: */
/*
 * adjtimex() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/timex.h>


_syscall1(int, adjtimex, struct timex *, buf)
libc_hidden_def(adjtimex)
#if defined __UCLIBC_NTP_LEGACY__
strong_alias(adjtimex,ntp_adjtime)
#endif
