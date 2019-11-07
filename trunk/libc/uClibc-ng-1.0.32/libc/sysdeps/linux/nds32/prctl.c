/*
 *  Copyright (C) 2017 Andes Technology, Inc.
 *  Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <errno.h>
#include <stdarg.h>
#include <sysdep.h>
#include <sys/prctl.h>
#include <sys/syscall.h>

int prctl (int __option, ...)
{
	unsigned long arg1,arg2,arg3,arg4;
	va_list arg;
	va_start (arg, __option);
	arg1 = va_arg (arg, unsigned long);
	arg2 = va_arg (arg, unsigned long);
	arg3 = va_arg (arg, unsigned long);
	arg4 = va_arg (arg, unsigned long);
	va_end (arg);
	return INLINE_SYSCALL(prctl,5,__option,arg1,arg2,arg3,arg4);
}
