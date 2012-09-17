/* vi: set sw=4 ts=4: */
/*
 * create_module syscall for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <errno.h>
#include <unistd.h>
#include <features.h>
#include <sys/types.h>
#include <sys/syscall.h>

#ifdef __NR_create_module

unsigned long create_module(const char *name, size_t size);

#if defined(__UCLIBC_BROKEN_CREATE_MODULE__)
# define __NR___create_module  __NR_create_module
static __inline__ _syscall2(long, __create_module, const char *, name, size_t, size)
/* By checking the value of errno, we know if we have been fooled
 * by the syscall2 macro making a very high address look like a
 * negative, so we we fix it up here.  */
unsigned long create_module(const char *name, size_t size)
{
	long ret = __create_module(name, size);

	/* Jump through hoops to fixup error return codes */
	if (ret == -1 && errno > 125) {
		ret = -errno;
		__set_errno(0);
	}
	return ret;
}
#elif defined(__UCLIBC_SLIGHTLY_BROKEN_CREATE_MODULE__)
# define __NR___create_module  __NR_create_module
/* Alpha doesn't have the same problem, exactly, but a bug in older
   kernels fails to clear the error flag.  Clear it here explicitly.  */
static __inline__ _syscall4(unsigned long, __create_module, const char *, name,
			size_t, size, size_t, dummy, size_t, err)
unsigned long create_module(const char *name, size_t size)
{
	return __create_module(name, size, 0, 0);
}
#else
/* Sparc, MIPS, etc don't mistake return values for errors. */
_syscall2(unsigned long, create_module, const char *, name, size_t, size)
#endif

#endif
