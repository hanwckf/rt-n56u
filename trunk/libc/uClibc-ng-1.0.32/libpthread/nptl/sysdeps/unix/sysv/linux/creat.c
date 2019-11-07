/*
 * Copyright (C) 2017 Waldemar Brodkorb <wbx@uclibc-ng.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <fcntl.h>
#include <sys/types.h>
#include <cancel.h>

int
creat (const char *file, mode_t mode)
{
# ifdef __NR_creat
  return _syscall2(int, __NC(creat), const char*, file, mode_t, mode)
# else
  return __open (file, O_WRONLY | O_CREAT | O_TRUNC, mode);
# endif
}

CANCELLABLE_SYSCALL(int, creat, (const char *file, mode_t mode), (file, mode))

lt_libc_hidden(creat)
