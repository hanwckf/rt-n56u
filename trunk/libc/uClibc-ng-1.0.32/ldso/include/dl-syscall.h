/*
 * Copyright (C) 2000-2005 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Lesser General Public License version 2.1 or later.
 */

#ifndef _DL_SYSCALL_H
#define _DL_SYSCALL_H

/* We can't use the real errno in ldso, since it has not yet
 * been dynamicly linked in yet. */
#include "sys/syscall.h"
extern int _dl_errno;
#undef __set_errno
#define __set_errno(X) {(_dl_errno) = (X);}

/* Pull in the arch specific syscall implementation */
#include <dl-syscalls.h>
/*  For MAP_ANONYMOUS -- differs between platforms */
#define _SYS_MMAN_H 1
#include <bits/mman.h>

#ifdef __ARCH_HAS_DEPRECATED_SYSCALLS__
/* Pull in whatever this particular arch's kernel thinks the kernel version of
 * struct stat should look like.  It turns out that each arch has a different
 * opinion on the subject, and different kernel revs use different names... */
#define kernel_stat stat
#include <bits/kernel_stat.h>
#include <bits/kernel_types.h>

/* Protection bits.  */
#define	S_ISUID		04000	/* Set user ID on execution.  */
#define	S_ISGID		02000	/* Set group ID on execution.  */

#else
/* 1. common-generic ABI doesn't need kernel_stat translation
 * 3. S_IS?ID already provided by stat.h
 */
#include <sys/stat.h>
#endif


/* Here are the definitions for some syscalls that are used
   by the dynamic linker.  The idea is that we want to be able
   to call these before the errno symbol is dynamicly linked, so
   we use our own version here.  Note that we cannot assume any
   dynamic linking at all, so we cannot return any error codes.
   We just punt if there is an error. */
#define __NR__dl_exit __NR_exit
static __always_inline attribute_noreturn __cold void _dl_exit(int status)
{
	INLINE_SYSCALL(_dl_exit, 1, status);
#if __GNUC_PREREQ(4, 5)
	__builtin_unreachable(); /* shut up warning: 'noreturn' function does return*/
#else
	while (1);
#endif
}

#define __NR__dl_close __NR_close
static __always_inline _syscall1(int, _dl_close, int, fd)

#if defined __NR_openat && !defined __NR_open
static __always_inline int _dl_open(const char *fn,
						int flags, __kernel_mode_t mode)
{
	return INLINE_SYSCALL(openat, 4, AT_FDCWD, fn, flags, mode);
}

#elif defined __NR_open
# define __NR__dl_open __NR_open
static __always_inline _syscall3(int, _dl_open, const char *, fn, int, flags,
                        __kernel_mode_t, mode)
#endif

#define __NR__dl_write __NR_write
static __always_inline _syscall3(unsigned long, _dl_write, int, fd,
                        const void *, buf, unsigned long, count)

#define __NR__dl_read __NR_read
static __always_inline _syscall3(unsigned long, _dl_read, int, fd,
                        const void *, buf, unsigned long, count)

#define __NR__dl_mprotect __NR_mprotect
static __always_inline _syscall3(int, _dl_mprotect, const void *, addr,
                        unsigned long, len, int, prot)

#if defined __NR_fstatat64 && !defined __NR_stat
# define __NR__dl_fstatat64 __NR_fstatat64
static __always_inline _syscall4(int, _dl_fstatat64, int, fd, const char *,
				 fn, struct stat *, stat, int, flags)

static __always_inline int _dl_stat(const char *file_name,
                        struct stat *buf)
{
	return _dl_fstatat64(AT_FDCWD, file_name, buf, 0);
}
#elif defined __NR_newfstatat && !defined __NR_stat
# define __NR__dl_newfstatat __NR_newfstatat
static __always_inline _syscall4(int, _dl_newfstatat, int, fd, const char *,
				 fn, struct stat *, stat, int, flags)

static __always_inline int _dl_stat(const char *file_name,
                        struct stat *buf)
{
	return _dl_newfstatat(AT_FDCWD, file_name, buf, 0);
}
#elif defined __NR_stat
# define __NR__dl_stat __NR_stat
static __always_inline _syscall2(int, _dl_stat, const char *, file_name,
                        struct stat *, buf)
#endif

#if defined __NR_fstat64 && !defined __NR_fstat
# define __NR__dl_fstat __NR_fstat64
#elif defined __NR_fstat
# define __NR__dl_fstat __NR_fstat
#endif
static __always_inline _syscall2(int, _dl_fstat, int, fd, struct stat *, buf)

#define __NR__dl_munmap __NR_munmap
static __always_inline _syscall2(int, _dl_munmap, void *, start, unsigned long, length)

#ifdef __NR_getxuid
# define __NR_getuid __NR_getxuid
#endif
#define __NR__dl_getuid __NR_getuid
static __always_inline _syscall0(uid_t, _dl_getuid)

#ifndef __NR_geteuid
# define __NR_geteuid __NR_getuid
#endif
#define __NR__dl_geteuid __NR_geteuid
static __always_inline _syscall0(uid_t, _dl_geteuid)

#ifdef __NR_getxgid
# define __NR_getgid __NR_getxgid
#endif
#define __NR__dl_getgid __NR_getgid
static __always_inline _syscall0(gid_t, _dl_getgid)

#ifndef __NR_getegid
# define __NR_getegid __NR_getgid
#endif
#define __NR__dl_getegid __NR_getegid
static __always_inline _syscall0(gid_t, _dl_getegid)

#ifdef __NR_getxpid
# define __NR_getpid __NR_getxpid
#endif
#define __NR__dl_getpid __NR_getpid
static __always_inline _syscall0(gid_t, _dl_getpid)

#if defined __NR_readlinkat
# define __NR__dl_readlink __NR_readlinkat
static __always_inline _syscall4(int, _dl_readlink, int, id, const char *, path,
						char *, buf, size_t, bufsiz)
#endif

#ifdef __NR_pread64
#define __NR___syscall_pread __NR_pread64
#ifdef __UCLIBC_SYSCALL_ALIGN_64BIT__
static __always_inline _syscall6(ssize_t, __syscall_pread, int, fd, void *, buf, size_t, dummy,
			size_t, count, off_t, offset_hi, off_t, offset_lo)

static __always_inline ssize_t
_dl_pread(int fd, void *buf, size_t count, off_t offset)
{
	return __syscall_pread(fd, buf, count, 0, __LONG_LONG_PAIR((offset >> 32), (offset & 0xffffffff)));
}
#else
static __always_inline _syscall5(ssize_t, __syscall_pread, int, fd, void *, buf,
			size_t, count, off_t, offset_hi, off_t, offset_lo)

static __always_inline ssize_t
_dl_pread(int fd, void *buf, size_t count, off_t offset)
{
	return __syscall_pread(fd, buf, count, __LONG_LONG_PAIR(offset >> 31, offset));
}
#endif
#elif defined __NR_pread
#define __NR___syscall_pread __NR_pread
static __always_inline _syscall5(ssize_t, __syscall_pread, int, fd, void *, buf,
			size_t, count, off_t, offset_hi, off_t, offset_lo)

static __always_inline ssize_t
_dl_pread(int fd, void *buf, size_t count, off_t offset)
{
	return __syscall_pread(fd, buf, count, __LONG_LONG_PAIR(offset >> 31, offset));
}
#endif

#ifdef __UCLIBC_HAS_SSP__
# include <sys/time.h>
# define __NR__dl_gettimeofday __NR_gettimeofday
static __always_inline _syscall2(int, _dl_gettimeofday, struct timeval *, tv,
# ifdef __USE_BSD
                        struct timezone *
# else
                        void *
# endif
						, tz)
#endif

/* Some architectures always use 12 as page shift for mmap2() eventhough the
 * real PAGE_SHIFT != 12.  Other architectures use the same value as
 * PAGE_SHIFT...
 */
#ifndef MMAP2_PAGE_SHIFT
# define MMAP2_PAGE_SHIFT 12
#endif

#define MAP_FAILED ((void *) -1)
#define _dl_mmap_check_error(X) (((void *)X) == MAP_FAILED)

static __always_inline
void *_dl_mmap(void *addr, unsigned long size, int prot,
               int flags, int fd, unsigned long offset)
{
#if defined(__UCLIBC_MMAP_HAS_6_ARGS__) && defined(__NR_mmap)
	/* first try mmap(), syscall6() style */
	return (void *)INLINE_SYSCALL(mmap, 6, addr, size, prot, flags, fd, offset);

#elif defined(__NR_mmap2) && !defined (__mcoldfire__)
	/* then try mmap2() */
	unsigned long shifted;

	if (offset & ((1 << MMAP2_PAGE_SHIFT) - 1))
		return MAP_FAILED;

	/* gcc needs help with putting things onto the stack */
	shifted = offset >> MMAP2_PAGE_SHIFT;
	return (void *)INLINE_SYSCALL(mmap2, 6, addr, size, prot, flags, fd, shifted);

#elif defined(__NR_mmap)
	/* finally, fall back to mmap(), syscall1() style */
	unsigned long buffer[6];
	buffer[0] = (unsigned long) addr;
	buffer[1] = (unsigned long) size;
	buffer[2] = (unsigned long) prot;
	buffer[3] = (unsigned long) flags;
	buffer[4] = (unsigned long) fd;
	buffer[5] = (unsigned long) offset;
	return (void *)INLINE_SYSCALL(mmap, 1, buffer);
#else
# error "Your architecture doesn't seem to provide mmap() !?"
#endif
}

#endif /* _DL_SYSCALL_H */
