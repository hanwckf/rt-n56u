/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */
/*
 * Based in part on the files
 *		./sysdeps/unix/sysv/linux/pwrite.c,
 *		./sysdeps/unix/sysv/linux/pread.c,
 *		sysdeps/posix/pread.c
 *		sysdeps/posix/pwrite.c
 * from GNU libc 2.2.5, but reworked considerably...
 */

#include <sys/syscall.h>
#include <unistd.h>
#include <stdint.h>
#include <endian.h>

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
#include <sysdep-cancel.h>
#else
#define SINGLE_THREAD_P 1
#endif


#ifdef __NR_pread64             /* Newer kernels renamed but it's the same.  */
# ifdef __NR_pread
#  error "__NR_pread and __NR_pread64 both defined???"
# endif
# define __NR_pread __NR_pread64
#endif

#ifdef __NR_pread
extern __typeof(pread) __libc_pread;
# define __NR___syscall_pread __NR_pread
static __inline__ _syscall6(ssize_t, __syscall_pread, int, fd, void *, buf,
		size_t, count, int, dummy, off_t, offset_hi, off_t, offset_lo)

ssize_t __libc_pread(int fd, void *buf, size_t count, off_t offset)
{
	if (SINGLE_THREAD_P)
		return(__syscall_pread(fd,buf,count,0,__LONG_LONG_PAIR(offset >> 31,offset)));

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	int oldtype = LIBC_CANCEL_ASYNC ();
	ssize_t result = __syscall_pread(fd,buf,count,0,__LONG_LONG_PAIR(offset >> 31,offset));
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif
}
weak_alias(__libc_pread,pread)

# ifdef __UCLIBC_HAS_LFS__
extern __typeof(pread64) __libc_pread64;
ssize_t __libc_pread64(int fd, void *buf, size_t count, off64_t offset)
{
	uint32_t low = offset & 0xffffffff;
	uint32_t high = offset >> 32;

	if (SINGLE_THREAD_P)
		return __syscall_pread(fd, buf, count, 0, __LONG_LONG_PAIR (high, low));

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	int oldtype = LIBC_CANCEL_ASYNC ();
	ssize_t result = __syscall_pread(fd, buf, count, 0, __LONG_LONG_PAIR (high, low));
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif
}
weak_alias(__libc_pread64,pread64)
# endif /* __UCLIBC_HAS_LFS__  */
#endif /* __NR_pread */

/**********************************************************************/

#ifdef __NR_pwrite64            /* Newer kernels renamed but it's the same.  */
# ifdef __NR_pwrite
#  error "__NR_pwrite and __NR_pwrite64 both defined???"
# endif
# define __NR_pwrite __NR_pwrite64
#endif

#ifdef __NR_pwrite
extern __typeof(pwrite) __libc_pwrite;
# define __NR___syscall_pwrite __NR_pwrite
static __inline__ _syscall6(ssize_t, __syscall_pwrite, int, fd, const void *, buf,
		size_t, count, int, dummy, off_t, offset_hi, off_t, offset_lo)

ssize_t __libc_pwrite(int fd, const void *buf, size_t count, off_t offset)
{
	if (SINGLE_THREAD_P)
		return __syscall_pwrite(fd,buf,count,0,__LONG_LONG_PAIR(offset >> 31,offset));

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	int oldtype = LIBC_CANCEL_ASYNC ();
	ssize_t result = __syscall_pwrite(fd,buf,count,0,__LONG_LONG_PAIR(offset >> 31,offset));
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif

}
weak_alias(__libc_pwrite,pwrite)

# ifdef __UCLIBC_HAS_LFS__
extern __typeof(pwrite64) __libc_pwrite64;
ssize_t __libc_pwrite64(int fd, const void *buf, size_t count, off64_t offset)
{
	uint32_t low = offset & 0xffffffff;
	uint32_t high = offset >> 32;

	if (SINGLE_THREAD_P)
		return __syscall_pwrite(fd, buf, count, 0, __LONG_LONG_PAIR (high, low));

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	int oldtype = LIBC_CANCEL_ASYNC ();
	ssize_t result = __syscall_pwrite(fd, buf, count, 0, __LONG_LONG_PAIR (high, low));
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif
}
weak_alias(__libc_pwrite64,pwrite64)
# endif /* __UCLIBC_HAS_LFS__  */
#endif /* __NR_pwrite */
