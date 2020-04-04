/* syscall for META/uClibc
 *
 * Copyright (C) 2013 Imagination Technologies
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <features.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/syscall.h>

long syscall(long sysnum,
	     long arg1, long arg2, long arg3,
	     long arg4, long arg5, long arg6)
{

	register long __call __asm__ ("D1Re0") = sysnum;
	register long __res __asm__ ("D0Re0");
	register long __a __asm__ ("D1Ar1") = arg1;
	register long __b __asm__ ("D0Ar2") = arg2;
	register long __c __asm__ ("D1Ar3") = arg3;
	register long __d __asm__ ("D0Ar4") = arg4;
	register long __e __asm__ ("D1Ar5") = arg5;
	register long __f __asm__ ("D0Ar6") = arg6;


	__asm__ __volatile__ ("SWITCH  #0x440001"
			      : "=d" (__res)
			      : "d" (__call), "d" (__a), "d" (__b),
				"d" (__c), "d" (__d), "d" (__e) , "d" (__f)
			      : "memory");

	if(__res >= (unsigned long) -4095) {
		long err = __res;
		(*__errno_location()) = (-err);
		__res = (unsigned long) -1;
	}
	return (long) __res;
}
