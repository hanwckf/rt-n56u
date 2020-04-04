/*
 * setgroups() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#ifdef __USE_BSD
#include <grp.h>

#if defined(__NR_setgroups32)
# undef __NR_setgroups
# define __NR_setgroups __NR_setgroups32
_syscall2(int, setgroups, size_t, size, const gid_t *, list)

#elif __WORDSIZE == 64
_syscall2(int, setgroups, size_t, size, const gid_t *, list)

#else
# include <errno.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/types.h>

# define __NR___syscall_setgroups __NR_setgroups
static __always_inline _syscall2(int, __syscall_setgroups,
				 size_t, size, const __kernel_gid_t *, list)

int setgroups(size_t size, const gid_t *groups)
{
	if (size > (size_t) sysconf(_SC_NGROUPS_MAX)) {
ret_error:
		__set_errno(EINVAL);
		return -1;
	} else {
		size_t i;
		__kernel_gid_t *kernel_groups = NULL;

		if (size) {
			kernel_groups = (__kernel_gid_t *)malloc(sizeof(*kernel_groups) * size);
			if (kernel_groups == NULL)
				goto ret_error;
		}

		for (i = 0; i < size; i++) {
			kernel_groups[i] = (groups)[i];
			if (groups[i] != (gid_t) ((__kernel_gid_t) groups[i])) {
				goto ret_error;
			}
		}

		i = __syscall_setgroups(size, kernel_groups);
		free(kernel_groups);
		return i;
	}
}
#endif

libc_hidden_def(setgroups)
#endif
