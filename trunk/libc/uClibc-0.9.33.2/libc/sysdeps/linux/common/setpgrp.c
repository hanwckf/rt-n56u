/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <syscall.h>
#include <unistd.h>


int setpgrp(void)
{
	return setpgid(0,0);
}
