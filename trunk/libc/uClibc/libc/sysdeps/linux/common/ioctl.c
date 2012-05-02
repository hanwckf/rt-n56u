/* vi: set sw=4 ts=4: */
/*
 * ioctl() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <stdarg.h>
#include <sys/ioctl.h>

#define __NR___syscall_ioctl __NR_ioctl
_syscall3(int, __syscall_ioctl, int, fd, unsigned long int, request, void *, arg);

int ioctl(int fd, unsigned long int request, ...)
{
    void *arg;
    va_list list;

    va_start(list, request);
    arg = va_arg(list, void *);

    va_end(list);
    return __syscall_ioctl(fd, request, arg);
}

