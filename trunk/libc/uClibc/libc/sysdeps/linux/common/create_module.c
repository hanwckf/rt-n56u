/* vi: set sw=4 ts=4: */
/* Syscalls for uClibc
 *
 * Copyright (C) 2000 by Lineo, inc. and Erik Andersen
 * Copyright (C) 2000,2001 by Erik Andersen <andersen@uclibc.org>
 * Written by Erik Andersen <andersen@uclibc.org>
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

#include <errno.h>
#include <unistd.h>
#include <features.h>
#include <sys/types.h>
#include <sys/syscall.h>


//#define __NR_create_module    127

#ifdef __NR_create_module

#if defined(__i386__) || defined(__m68k__) || defined(__arm__) || defined(__cris__) || defined(__i960__)
#define __NR___create_module  __NR_create_module
#ifdef __STR_NR_create_module
#define __STR_NR___create_module  __STR_NR_create_module
#endif
_syscall2(long, __create_module, const char *, name, size_t, size);
/* By checking the value of errno, we know if we have been fooled 
 * by the syscall2 macro making a very high address look like a 
 * negaitive, so we we fix it up here.  */
unsigned long create_module(const char *name, size_t size)
{
	long ret = __create_module(name, size);

	/* Jump through hoops to fixup error return codes */
	if (ret == -1 && errno > 125) {
		ret = -errno;
		__set_errno(0);
	}
	return ret;
}
#elif defined(__alpha__)
#define __NR___create_module  __NR_create_module
/* Alpha doesn't have the same problem, exactly, but a bug in older
   kernels fails to clear the error flag.  Clear it here explicitly.  */
_syscall4(unsigned long, __create_module, const char *, name,
			size_t, size, size_t, dummy, size_t, err);
unsigned long create_module(const char *name, size_t size)
{
  return __create_module(name, size, 0, 0);
}
#else
/* Sparc, MIPS, etc don't mistake return values for errors. */ 
_syscall2(unsigned long, create_module, const char *, name, size_t, size);
#endif

#else
unsigned long create_module(const char *name, size_t size)
{
	__set_errno(ENOSYS);
	return (unsigned long)-1;
}
#endif

