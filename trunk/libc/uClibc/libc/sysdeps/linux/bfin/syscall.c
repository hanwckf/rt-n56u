/* vi: set sw=4 ts=4: */
/* syscall for blackfin/uClibc
 *
 * Copyright (C) 2004 by Analog Devices Inc.
 * Copyright (C) 2002 by Erik Andersen <andersen@uclibc.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <features.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/syscall.h>

long syscall(long sysnum, long a, long b, long c, long d, long e)
{
	int _r0 = 0;
    asm volatile(
	    "p0 = %1;"		/*SysCall Number*/
	    "r0 = %2;"
	    "r1 = %3;"
	    "r2 = %4;"
		"r3 = %6;"
		"r4 = %5;"
		"excpt 0;"		/*Call the System Call*/
		"%0 = r0;"		/*Store the result of syscall*/
	    : "=r"(_r0)
	    : "r"(sysnum), "r"(a), "r"(b),
	      "r"(c), "r"(d), "r"(e)
	    : "memory");

    if(_r0 >=(unsigned long) -4095) {
	(*__errno_location())=(-_r0);
	_r0=(unsigned long) -1;
    }
    return (long) _r0;
}
