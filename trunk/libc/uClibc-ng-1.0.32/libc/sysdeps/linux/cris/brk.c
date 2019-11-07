/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include "sysdep.h"

extern void * __curbrk attribute_hidden;
extern int __init_brk (void) attribute_hidden;

int brk(void * end_data_seg)
{
	if (__init_brk () == 0) {
		/*
		 * Notice that we don't need to save/restore the GOT
		 * register since that is not call clobbered by the syscall.
		 */
		__asm__ ("move.d %1,$r10\n\t"
		     "movu.w " STR(__NR_brk) ",$r9\n\t"
		     "break 13\n\t"
		     "move.d $r10, %0"
		     : "=r" (__curbrk)
		     : "g" (end_data_seg)
		     : "r9", "r10");

		if (__curbrk == end_data_seg)
			return 0;
		__set_errno(ENOMEM);
	}
	return -1;

}
libc_hidden_def(brk)
