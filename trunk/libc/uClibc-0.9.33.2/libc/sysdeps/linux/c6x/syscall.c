/*
 *  syscall.c
 *
 *  Port on Texas Instruments TMS320C6x architecture
 *
 *  Copyright (C) 2006, 2010 Texas Instruments Incorporated
 *  Author: Thomas Charleux (thomas.charleux@jaluna.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <features.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdarg.h>

long int syscall (long int __sysno, ...)
{
	register long no __asm__("B0");
	register long a __asm__("A4");
	register long b __asm__("B4");
	register long c __asm__("A6");
	register long d __asm__("B6");
	register long e __asm__("A8");
	register long f __asm__("B8");
	long __res;
	va_list ap;

	va_start( ap, __sysno);
	a = va_arg( ap, long);
	b = va_arg( ap, long);
	c = va_arg( ap, long);
	d = va_arg( ap, long);
	e = va_arg( ap, long);
	f = va_arg( ap, long);
	va_end( ap );

	no = __sysno;

	__asm__ __volatile__ ("SWE" : "=a" (a) : "a" (a), "b" (b), "a" (c), "b" (d), "a" (e), "b" (f), "b" (no)
			      : "memory", "cc");

	__res = a;
	__SYSCALL_RETURN (long);
}
