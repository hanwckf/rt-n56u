/* vi: set sw=4 ts=4: */
/*
 * prctl() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <stdarg.h>
/* psm: including sys/prctl.h would depend on kernel headers */

#ifdef __NR_prctl
extern int prctl (int __option, ...);
int prctl (int __option, ...)
{
	register long no __asm__("B0");
	register long a __asm__("A4");
	register long b __asm__("B4");
	register long c __asm__("A6");
	register long d __asm__("B6");
	register long e __asm__("A8");
	int __res;
	va_list ap;

	va_start( ap, __option);
	a = __option;
	b = va_arg( ap, long);
	c = va_arg( ap, long);
	d = va_arg( ap, long);
	e = va_arg( ap, long);
	va_end( ap );

	no = __NR_prctl;

	__asm__ __volatile__ ("SWE" : "=a" (a) : "a" (a), "b" (b), "a" (c), "b" (d), "a" (e), "b" (no)
			      : "memory", "cc");

	__res = a;
	__SYSCALL_RETURN (int);
}
#endif
