/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/types.h>
#include <sys/syscall.h>
#include <errno.h>

#define __NR___vfork __NR_vfork
attribute_hidden _syscall0(pid_t, __vfork)
weak_alias(__vfork,vfork)
libc_hidden_weak(vfork)
