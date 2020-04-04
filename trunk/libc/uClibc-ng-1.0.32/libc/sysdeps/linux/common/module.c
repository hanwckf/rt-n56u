/*
 * init_module()/delete_module() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#ifdef __NR_init_module
int init_module(void *first, void *second, void *third, void *fourth, void *fifth);
/* This may have 5 arguments (for old 2.0 kernels) or 2 arguments
 * (for 2.2 and 2.4 kernels).  Use the greatest common denominator,
 * and let the kernel cope with whatever it gets.  It's good at that. */
_syscall5(int, init_module, void *, first, void *, second, void *, third,
	  void *, fourth, void *, fifth)
#endif

#ifdef __NR_delete_module
int delete_module(const char *name, unsigned int flags);
_syscall2(int, delete_module, const char *, name, unsigned int, flags)
#endif
