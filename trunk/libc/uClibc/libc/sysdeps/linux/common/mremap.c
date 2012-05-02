/* vi: set sw=4 ts=4: */
/*
 * mremap() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <unistd.h>
#include <sys/mman.h>
_syscall4(__ptr_t, mremap, __ptr_t, old_address, size_t, old_size, size_t,
		  new_size, int, may_move);
