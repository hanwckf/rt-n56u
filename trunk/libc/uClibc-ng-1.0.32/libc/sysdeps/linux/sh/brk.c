/* From libc-5.3.12 */
/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>

extern void * __curbrk attribute_hidden;
extern int __init_brk (void) attribute_hidden;
extern void *_brk(void *ptr) attribute_hidden;

int brk(void * end_data_seg)
{
    if (__init_brk () == 0)
    {
		__curbrk = _brk(end_data_seg);
		if (__curbrk == end_data_seg)
			return 0;
		__set_errno(ENOMEM);
    }
    return -1;
}
libc_hidden_def(brk)
