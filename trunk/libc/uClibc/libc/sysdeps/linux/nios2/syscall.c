/* 
 * libc/sysdeps/linux/nios2/syscall.c -- generic syscall function for linux/nios2
 *
 * Copyright (C) 2004 Microtronix Datacom Ltd.
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


long syscall(long sysnum, long a, long b, long c, long d, long e, long f)
{
    register long _r2 asm("r2")=(long)TRAP_ID_SYSCALL;
    register long _r3 asm("r3")=(long)sysnum;
   
    register long _r4 asm("r4")=(long)(a);
    register long _r5 asm("r5")=(long)(b);
    register long _r6 asm("r6")=(long)(c);
    register long _r7 asm("r7")=(long)(d);
    register long _r8 asm("r8")=(long)(e);
    register long _r9 asm("r9")=(long)(f);
    asm volatile(
	    "trap "
	    : "=r"(_r2), "=r"(_r3)
	    : "0"(_r2), "1"(_r3),
	      "r"(_r4), "r"(_r5), "r"(_r6), "r"(_r7), "r"(_r8), "r"(_r9)
	    : "memory");

  __syscall_return (long, _r2);
}

