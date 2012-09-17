/*
 * syscall() library function
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

long syscall(long sysnum, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6)
{
	return INLINE_SYSCALL_NCS(sysnum, 6, arg1, arg2, arg3, arg4, arg5, arg6);
}
