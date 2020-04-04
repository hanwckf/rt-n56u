/*
 *  Copyright (C) 2017 Andes Technology, Inc.
 *  Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <errno.h>
#include <stdarg.h>
#include <sysdep.h>

#include <sys/mman.h>
#include <sys/syscall.h>

#ifdef __NR_mremap
void *mremap (void *__addr, size_t __old_len, size_t __new_len,
                     int __flags, ...);
libc_hidden_proto(mremap)

void *mremap (void *__addr, size_t __old_len, size_t __new_len, int __flags, ...)
{
        unsigned long arg1;
        va_list arg;
        va_start (arg, __flags);
        arg1 = va_arg (arg, int);
        va_end (arg);
        return (void *)INLINE_SYSCALL(mremap,5,__addr,__old_len,__new_len,__flags,arg1);
}
libc_hidden_def (mremap)
#endif
