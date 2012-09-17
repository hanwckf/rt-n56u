/* vi: set sw=4 ts=4: */
/*
 * getpriority() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <sys/resource.h>

#define __NR___syscall_getpriority __NR_getpriority
static inline _syscall2(int, __syscall_getpriority,
		__priority_which_t, which, id_t, who);

/* The return value of __syscall_getpriority is biased by this value
 * to avoid returning negative values.  */
#define PZERO 20
int getpriority(enum __priority_which which, id_t who)
{
	int res;

	res = __syscall_getpriority(which, who);
	if (res >= 0)
		res = PZERO - res;
	return res;
}
