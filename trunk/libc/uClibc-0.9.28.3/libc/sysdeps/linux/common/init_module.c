/* vi: set sw=4 ts=4: */
/*
 * init_module() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#ifdef __NR_init_module
/* This may have 5 arguments (for old 2.0 kernels) or 2 arguments
 * (for 2.2 and 2.4 kernels).  Use the greatest common denominator,
 * and let the kernel cope with whatever it gets.  It's good at that. */
_syscall5(int, init_module, void *, first, void *, second, void *, third,
		  void *, fourth, void *, fifth);
#else
int init_module(void *first, void *second, void *third, void *fourth, void *fifth)
{
	__set_errno(ENOSYS);
	return -1;
}
#endif

