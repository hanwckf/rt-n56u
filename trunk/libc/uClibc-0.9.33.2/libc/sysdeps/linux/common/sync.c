/* vi: set sw=4 ts=4: */
/*
 * sync syscall for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
# if defined __USE_BSD || defined __USE_UNIX98
#include <sys/types.h>
#include <unistd.h>

void sync(void)
{
	INLINE_SYSCALL(sync, 0);
}
#endif
