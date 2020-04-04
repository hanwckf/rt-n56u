/*
 * libc/sysdeps/linux/bfin/clone.c -- `clone' syscall for linux/blackfin
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sched.h>
#include <errno.h>
#include <sys/syscall.h>

int
clone (int (*fn)(void *arg), void *child_stack, int flags, void *arg, ...)
{
	long rval = -1;

	if (fn && child_stack) {

		__asm__ __volatile__ (
			"excpt 0;"	/* Call sys_clone */
			"cc = r0 == 0;"
			"if !cc jump 1f;"	/* if (rval != 0) skip to parent */
			"r0 = %4;"
			"p0 = %5;"
			"fp = 0;"
#ifdef __BFIN_FDPIC__
			"p1 = [p0];"
			"p3 = [p0 + 4];"
			"call (p1);"	/* Call cloned function */
#else
			"call (p0);"	/* Call cloned function */
#endif
			"p0 = %6;"
			"excpt 0;"	/* Call sys_exit */
			"1: nop;"
			: "=q0" (rval)
			: "qA" (__NR_clone), "q1" (child_stack), "q0" (flags), "a" (arg), "a" (fn), "i" (__NR_exit)
			: "CC");

	} else
		__set_errno(EINVAL);

	return rval;
}
