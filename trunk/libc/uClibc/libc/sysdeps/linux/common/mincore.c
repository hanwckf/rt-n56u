/* 
 * Distributed under the terms of the GNU General Public License v2
 * $Header: /home/cvsroot/RT288x_SDK/source/lib/libc/sysdeps/linux/common/mincore.c,v 1.1.1.1 2007-01-09 06:46:10 steven Exp $
 *
 * This file provides the mincore() system call to uClibc.
 * 20041215 - <solar@gentoo.org>
 *
 */

#include "syscalls.h"
#include <unistd.h>

#ifdef __NR_mincore
_syscall3(int, mincore, void *, start, size_t, length, unsigned char *, vec);
#endif
