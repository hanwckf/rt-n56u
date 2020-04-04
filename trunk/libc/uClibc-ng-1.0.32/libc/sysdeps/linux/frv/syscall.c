/* syscall for frv/uClibc
 *
 * Copyright (C) 2004 by Alexandre Oliva <aoliva@redhat.com>
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
 * along with this program; see the file COPYING.LIB.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <features.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/syscall.h>


long syscall(long sysnum, long arg1, long arg2, long arg3,
	     long arg4, long arg5, long arg6)
{
  register unsigned long __scnum __asm__ ("gr7") = (sysnum);
  register unsigned long __sc0 __asm__ ("gr8") = (unsigned long) (arg1);
  register unsigned long __sc1 __asm__ ("gr9") = (unsigned long) (arg2);
  register unsigned long __sc2 __asm__ ("gr10") = (unsigned long) (arg3);
  register unsigned long __sc3 __asm__ ("gr11") = (unsigned long) (arg4);
  register unsigned long __sc4 __asm__ ("gr12") = (unsigned long) (arg5);
  register unsigned long __sc5 __asm__ ("gr13") = (unsigned long) (arg6);
  __asm__ __volatile__ ("tra	gr0,gr0"
			: "+r" (__sc0)
			: "r" (__scnum), "r" (__sc1), "r" (__sc2),
			"r" (__sc3), "r" (__sc4), "r" (__sc5));
  return (long) __sc0;
}
