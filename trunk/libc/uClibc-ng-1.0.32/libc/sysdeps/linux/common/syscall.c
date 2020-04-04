/*
 * syscall() library function
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <stdarg.h>
#include <sys/syscall.h>
#include <unistd.h>

long syscall(long sysnum, ...)
{

	unsigned long arg1, arg2, arg3, arg4, arg5, arg6;
	va_list arg;

	va_start (arg, sysnum);
	arg1 = va_arg (arg, unsigned long);
	arg2 = va_arg (arg, unsigned long);
	arg3 = va_arg (arg, unsigned long);
	arg4 = va_arg (arg, unsigned long);
	arg5 = va_arg (arg, unsigned long);
	arg6 = va_arg (arg, unsigned long);
	va_end (arg);

        __asm__ volatile ( "" ::: "memory" );
	return INLINE_SYSCALL_NCS(sysnum, 6, arg1, arg2, arg3, arg4, arg5, arg6);
}
