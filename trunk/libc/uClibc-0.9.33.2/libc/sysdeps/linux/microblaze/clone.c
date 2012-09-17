/*
 * Copyright (C) 2004 Atmel Corporation
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.  See the file "COPYING.LIB" in the main directory of this
 * archive for more details.
 */
#include <sched.h>
#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>

int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg, ...)
{
	int rval = -EINVAL;
	if (fn && child_stack)
		rval = INTERNAL_SYSCALL(clone, 0, 2, flags, child_stack);

	if (rval == 0)
	{
		int exitCode = fn(arg);
		rval = INTERNAL_SYSCALL(exit, 0, 1, exitCode);
	}

	return rval;
}

#ifdef __NR_clone2
int
__clone2(int (*fn)(void *arg), void *child_stack, size_t stack_size,
	 int flags, void *arg, ...)
{
	int rval = -EINVAL;
	if (fn && child_stack)
	{
		rval = INTERNAL_SYSCALL(clone2, 0, 3, flags, child_stack, stack_size);
	}

	if (rval == 0)
	{
		int exitCode = fn(arg);
		rval = INTERNAL_SYSCALL(exit, 0, 1, exitCode);
	}

	return rval;
}
#endif
