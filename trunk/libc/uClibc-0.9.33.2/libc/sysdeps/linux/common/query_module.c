/* vi: set sw=4 ts=4: */
/*
 * query_module() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
int query_module(const char *name attribute_unused, int which attribute_unused,
				 void *buf attribute_unused, size_t bufsize attribute_unused, size_t * ret attribute_unused);
#ifdef __NR_query_module
_syscall5(int, query_module, const char *, name, int, which,
		  void *, buf, size_t, bufsize, size_t *, ret)
#endif
