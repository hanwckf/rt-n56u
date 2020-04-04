/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

#include <sysdep.h>
PSEUDO_ERRVAL(__vfork, vfork, 0)
PSEUDO_END_ERRVAL(__vfork)
weak_alias(__vfork, vfork)
libc_hidden_weak(vfork)

