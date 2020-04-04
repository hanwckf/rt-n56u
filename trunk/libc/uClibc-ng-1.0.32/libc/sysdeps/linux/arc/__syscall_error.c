/* Wrapper for setting errno.
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <errno.h>
#include <sys/syscall.h>

int __syscall_error(int err_no)
{
	__set_errno(-err_no);
	return -1;
}
