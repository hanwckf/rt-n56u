/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>

extern __typeof(vfork) __vfork attribute_hidden;

#ifdef __NR_vfork

# define __NR___vfork __NR_vfork
_syscall0(pid_t, __vfork)

weak_alias(__vfork,vfork)
libc_hidden_weak(vfork)

#elif defined __ARCH_USE_MMU__ && defined __NR_fork

/* Trivial implementation for arches that lack vfork */

pid_t __vfork(void)
{
    return fork();
}

weak_alias(__vfork,vfork)
libc_hidden_weak(vfork)

#endif
