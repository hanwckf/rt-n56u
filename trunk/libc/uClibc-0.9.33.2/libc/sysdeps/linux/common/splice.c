/* vi: set sw=4 ts=4: */
/*
 * splice() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#if defined __NR_splice && defined __UCLIBC_HAS_LFS__ && defined __USE_GNU
# include <fcntl.h>

_syscall6(ssize_t, splice, int, __fdin, off64_t *, __offin, int, __fdout,
	  off64_t *, __offout, size_t, __len, unsigned int, __flags)
#endif
