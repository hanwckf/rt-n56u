/*
 * getpgrp() for uClibc
 *
 * Copyright (C) 2000-2008 by Erik Andersen <andersen@codepoet.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>

#ifdef __NR_getpgrp
/* According to the manpage the POSIX.1 version is favoured */
_syscall_noerr0(pid_t, getpgrp)
#elif defined __NR_getpgid
pid_t getpgrp(void)
{
	return getpgid(0);
}
#endif
