/*
 * prctl syscall for AVR32 Linux.
 *
 * Copyright (C) 2010 Atmel Corporation
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License. See the file "COPYING.LIB" in the main directory of this
 * archive for more details.
 */
#include <sys/syscall.h>
#include <sys/prctl.h>
#include <stdarg.h>

#ifdef __NR_prctl
#define __NR___syscall_prctl	__NR_prctl
static inline _syscall5(int, __syscall_prctl, int, option, long, arg2,
		long, arg3, long, arg4, long, arg5);

int prctl(int __option, ...)
{
	long arg2;
	long arg3;
	long arg4;
	long arg5;
	va_list ap;

	va_start(ap, __option);
	arg2 = va_arg(ap, long);
	arg3 = va_arg(ap, long);
	arg4 = va_arg(ap, long);
	arg5 = va_arg(ap, long);
	va_end(ap);

	return INLINE_SYSCALL(prctl, 5, __option, arg2, arg3, arg4, arg5);
}
#endif
