/* vi: set sw=4 ts=4: */
/*
 * mremap() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <unistd.h>

#ifdef __NR_mremap

/* Why do we do this?! */

#define mremap _hidemremap
#include <sys/mman.h>
#undef mremap

void *mremap(void *, size_t, size_t, int, void *);
libc_hidden_proto(mremap)

_syscall5(void *, mremap, void *, old_address, size_t, old_size, size_t,
		  new_size, int, may_move, void *, new_address)
libc_hidden_def(mremap)

#endif
