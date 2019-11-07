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

/*
 * I don't know if we can be absolutely certain that the fn and arg
 * parameters are preserved when returning as the child. If the
 * compiler stores them in registers (r0-r7), they should be.
 */
int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg, ...)
{
	register int (*_fn)(void *arg) = fn;
	register void *_arg = arg;
	int err;

	/* Sanity check the arguments */
	err = -EINVAL;
	if (!fn)
		goto syscall_error;
	if (!child_stack)
		goto syscall_error;

	err = INLINE_SYSCALL(clone, 2, flags, child_stack);
	if (err < 0)
		goto syscall_error;
	else if (err != 0)
		return err;

	_exit(_fn(_arg));

syscall_error:
	__set_errno (-err);
	return -1;
}
