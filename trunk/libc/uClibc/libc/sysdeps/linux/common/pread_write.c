/* vi: set sw=4 ts=4:
 *
 * Copyright (C) 2002 by Erik Andersen <andersen@uclibc.org>
 * Based in part on the files
 *		./sysdeps/unix/sysv/linux/pwrite.c,
 *		./sysdeps/unix/sysv/linux/pread.c, 
 *		sysdeps/posix/pread.c
 *		sysdeps/posix/pwrite.c
 * from GNU libc 2.2.5, but reworked considerably...
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
#include <unistd.h>
#include <stdint.h>

#ifdef __NR_pread

#define __NR___syscall_pread __NR_pread 
static inline _syscall5(ssize_t, __syscall_pread, int, fd, void *, buf, 
		size_t, count, off_t, offset_hi, off_t, offset_lo);

ssize_t __libc_pread(int fd, void *buf, size_t count, off_t offset)
{ 
	return(__syscall_pread(fd,buf,count,__LONG_LONG_PAIR (offset >> 31, offset)));
}
weak_alias (__libc_pread, pread)

#if defined __UCLIBC_HAS_LFS__ 
ssize_t __libc_pread64(int fd, void *buf, size_t count, off64_t offset)
{ 
    uint32_t low = offset & 0xffffffff;
    uint32_t high = offset >> 32;
	return(__syscall_pread(fd, buf, count, __LONG_LONG_PAIR (high, low)));
}
weak_alias (__libc_pread64, pread64)
#endif /* __UCLIBC_HAS_LFS__  */

#endif /* __NR_pread */


#ifdef __NR_pwrite

#define __NR___syscall_pwrite __NR_pwrite 
static inline _syscall5(ssize_t, __syscall_pwrite, int, fd, const void *, buf, 
		size_t, count, off_t, offset_hi, off_t, offset_lo);

ssize_t __libc_pwrite(int fd, const void *buf, size_t count, off_t offset)
{ 
	return(__syscall_pwrite(fd,buf,count,__LONG_LONG_PAIR (offset >> 31, offset)));
}
weak_alias (__libc_pwrite, pwrite)

#if defined __UCLIBC_HAS_LFS__ 
ssize_t __libc_pwrite64(int fd, const void *buf, size_t count, off64_t offset)
{ 
    uint32_t low = offset & 0xffffffff;
    uint32_t high = offset >> 32;
	return(__syscall_pwrite(fd, buf, count, __LONG_LONG_PAIR (high, low)));
}
weak_alias (__libc_pwrite64, pwrite64)
#endif /* __UCLIBC_HAS_LFS__  */
#endif /* __NR_pwrite */



#if ! defined __NR_pread || ! defined __NR_pwrite
static ssize_t __fake_pread_write(int fd, void *buf, 
		size_t count, off_t offset, int do_pwrite)
{
	int save_errno;
	ssize_t result;
	off_t old_offset;

	/* Since we must not change the file pointer preserve the 
	 * value so that we can restore it later.  */
	if ((old_offset=lseek(fd, 0, SEEK_CUR)) == (off_t) -1)
		return -1;

	/* Set to wanted position.  */
	if (lseek (fd, offset, SEEK_SET) == (off_t) -1)
		return -1;

	if (do_pwrite==1) {
		/* Write the data.  */
		result = write(fd, buf, count);
	} else {
		/* Read the data.  */
		result = read(fd, buf, count);
	}

	/* Now we have to restore the position.  If this fails we 
	 * have to return this as an error.  */
	save_errno = errno;
	if (lseek(fd, old_offset, SEEK_SET) == (off_t) -1)
	{
		if (result == -1)
			__set_errno(save_errno);
		return -1;
	}
	__set_errno(save_errno);
	return(result);
}

#if defined __UCLIBC_HAS_LFS__ 
static ssize_t __fake_pread_write64(int fd, void *buf, 
		size_t count, off64_t offset, int do_pwrite)
{
	int save_errno;
	ssize_t result;
	off64_t old_offset;

	/* Since we must not change the file pointer preserve the 
	 * value so that we can restore it later.  */
	if ((old_offset=lseek64(fd, 0, SEEK_CUR)) == (off64_t) -1)
		return -1;

	/* Set to wanted position.  */
	if (lseek64(fd, offset, SEEK_SET) == (off64_t) -1) 
		return -1;                               

	if (do_pwrite==1) {
		/* Write the data.  */
		result = write(fd, buf, count);
	} else {
		/* Read the data.  */
		result = read(fd, buf, count);
	}

	/* Now we have to restore the position. */
	save_errno = errno;
	if (lseek64 (fd, old_offset, SEEK_SET) == (off64_t) -1) {
		if (result == -1)
			__set_errno (save_errno);
		return -1;
	}
	__set_errno (save_errno);
	return result;
}
#endif /* __UCLIBC_HAS_LFS__  */
#endif /*  ! defined __NR_pread || ! defined __NR_pwrite */

#ifndef __NR_pread
ssize_t __libc_pread(int fd, void *buf, size_t count, off_t offset)
{
	return(__fake_pread_write(fd, buf, count, offset, 0));
}
weak_alias (__libc_pread, pread)

#if defined __UCLIBC_HAS_LFS__ 
ssize_t __libc_pread64(int fd, void *buf, size_t count, off64_t offset)
{ 
	return(__fake_pread_write64(fd, buf, count, offset, 0));
}
weak_alias (__libc_pread64, pread64)
#endif /* __UCLIBC_HAS_LFS__  */
#endif /* ! __NR_pread */


#ifndef __NR_pwrite
ssize_t __libc_pwrite(int fd, const void *buf, size_t count, off_t offset)
{
	/* we won't actually be modifying the buffer, 
	 *just cast it to get rid of warnings */
	return(__fake_pread_write(fd, (void*)buf, count, offset, 1));
}
weak_alias (__libc_pwrite, pwrite)

#if defined __UCLIBC_HAS_LFS__ 
ssize_t __libc_pwrite64(int fd, const void *buf, size_t count, off64_t offset)
{ 
	return(__fake_pread_write64(fd, (void*)buf, count, offset, 1));
}
weak_alias (__libc_pwrite64, pwrite64)
#endif /* __UCLIBC_HAS_LFS__  */
#endif /* ! __NR_pwrite */

