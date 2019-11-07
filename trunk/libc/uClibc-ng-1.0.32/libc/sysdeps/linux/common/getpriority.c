/*
 * getpriority() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/resource.h>


#define __NR___syscall_getpriority __NR_getpriority
static __inline__ _syscall2(int, __syscall_getpriority,
		__priority_which_t, which, id_t, who)

/* The return value of __syscall_getpriority is biased by this value
 * to avoid returning negative values.  */
#define PZERO 20
int getpriority(__priority_which_t which, id_t who)
{
	int res;

	res = __syscall_getpriority(which, who);
	if (res >= 0)
		res = PZERO - res;
	return res;
}
libc_hidden_def(getpriority)
