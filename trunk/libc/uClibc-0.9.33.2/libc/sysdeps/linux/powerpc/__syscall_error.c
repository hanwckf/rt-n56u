/* Wrapper for setting errno.
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <errno.h>
#include <features.h>

/* This routine is jumped to by all the syscall handlers, to stash
 * an error number into errno.  */
int __syscall_error(int err_no) attribute_hidden;
int __syscall_error(int err_no)
{
	__set_errno(err_no);
	return -1;
}
