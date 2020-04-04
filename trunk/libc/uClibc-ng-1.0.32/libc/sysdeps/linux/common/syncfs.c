/*
 * Copyright (C) 2015 Bartosz Golaszewski <bartekgola@gmail.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#if defined(__NR_syncfs) && __USE_GNU
#include <unistd.h>
_syscall1(int, syncfs, int, fd)
#endif
