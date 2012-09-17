/* vi: set sw=4 ts=4: */
/*
 * creat() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <fcntl.h>

int creat(const char *file, mode_t mode)
{
	return open(file, O_WRONLY | O_CREAT | O_TRUNC, mode);
}
