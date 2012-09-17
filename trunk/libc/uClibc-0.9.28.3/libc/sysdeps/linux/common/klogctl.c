/* vi: set sw=4 ts=4: */
/*
 * klogctl() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>
#define __NR__syslog		__NR_syslog
static inline _syscall3(int, _syslog, int, type, char *, buf, int, len);
int klogctl(int type, char *buf, int len)
{
	return (_syslog(type, buf, len));
}
