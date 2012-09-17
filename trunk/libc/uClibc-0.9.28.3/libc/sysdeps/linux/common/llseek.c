/* vi: set sw=4 ts=4: */
/*
 * llseek/lseek64 syscall for uClibc
 *
 * Copyright (C) 2002 by Erik Andersen <andersen@codepoet.org>
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
 *
 */

#define _GNU_SOURCE
#define _LARGEFILE64_SOURCE
#include <features.h>
#undef __OPTIMIZE__
/* We absolutely do _NOT_ want interfaces silently
 *  *  * renamed under us or very bad things will happen... */
#ifdef __USE_FILE_OFFSET64
# undef __USE_FILE_OFFSET64
#endif
#include <errno.h>
#include <sys/types.h>
#include <sys/syscall.h>


#if defined __NR__llseek && defined __UCLIBC_HAS_LFS__

#ifndef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...) __syscall_llseek (args)
#define __NR___syscall_llseek __NR__llseek
static inline _syscall5(int, __syscall_llseek, int, fd, off_t, offset_hi, 
		off_t, offset_lo, loff_t *, result, int, whence);
#endif

loff_t __libc_lseek64(int fd, loff_t offset, int whence)
{
	loff_t result;
	return(loff_t)(INLINE_SYSCALL (_llseek, 5, fd, (off_t) (offset >> 32), 
				(off_t) (offset & 0xffffffff), &result, whence) ?: result);
}
#else
extern __off_t __libc_lseek(int fildes, off_t offset, int whence);
loff_t __libc_lseek64(int fd, loff_t offset, int whence)
{
	return(loff_t)(__libc_lseek(fd, (off_t) (offset & 0xffffffff), whence));
}
#endif
weak_alias(__libc_lseek64, _llseek);
weak_alias(__libc_lseek64, llseek);
weak_alias(__libc_lseek64, lseek64);

