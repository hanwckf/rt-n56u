/* Copyright (C) 1991,1992,1994-2001,2003,2004,2005,2006,2007, 2009
	Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/*
 *	POSIX Standard: 6.5 File Control Operations	<fcntl.h>
 */

#ifndef	_FCNTL_H
#define	_FCNTL_H	1

#include <features.h>

/* This must be early so <bits/fcntl.h> can define types winningly.  */
__BEGIN_DECLS

/* Get the definitions of O_*, F_*, FD_*: all the
   numbers and flag bits for `open', `fcntl', et al.  */
#include <bits/fcntl.h>

/* For XPG all symbols from <sys/stat.h> should also be available.  */
#ifdef __USE_XOPEN
# include <sys/stat.h>
#endif

#ifdef	__USE_MISC
# ifndef R_OK			/* Verbatim from <unistd.h>.  Ugh.  */
/* Values for the second argument to access.
   These may be OR'd together.  */
#  define R_OK	4		/* Test for read permission.  */
#  define W_OK	2		/* Test for write permission.  */
#  define X_OK	1		/* Test for execute permission.  */
#  define F_OK	0		/* Test for existence.  */
# endif
#endif /* Use misc.  */

/* XPG wants the following symbols.  */
#ifdef __USE_XOPEN		/* <stdio.h> has the same definitions.  */
# define SEEK_SET	0	/* Seek from beginning of file.  */
# define SEEK_CUR	1	/* Seek from current position.  */
# define SEEK_END	2	/* Seek from end of file.  */
#endif	/* XPG */

#ifdef __USE_ATFILE
# define AT_FDCWD		-100	/* Special value used to indicate
					   the *at functions should use the
					   current working directory. */
# define AT_SYMLINK_NOFOLLOW	0x100	/* Do not follow symbolic links.  */
# define AT_REMOVEDIR		0x200	/* Remove directory instead of
					   unlinking file.  */
# define AT_SYMLINK_FOLLOW	0x400	/* Follow symbolic links.  */
# ifdef __USE_GNU
#  define AT_NO_AUTOMOUNT       0x800   /* Suppress terminal automount
                                           traversal.  */
#  define AT_EMPTY_PATH         0x1000  /* Allow empty relative pathname.  */
# endif
# define AT_EACCESS		0x200	/* Test access permitted for
					   effective IDs, not real IDs.  */
#endif

/* Do the file control operation described by CMD on FD.
   The remaining arguments are interpreted depending on CMD.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
#if !defined(__USE_FILE_OFFSET64) || defined(__LP64__)
extern int fcntl (int __fd, int __cmd, ...);
# ifdef _LIBC
extern int __fcntl_nocancel(int, int, long) attribute_hidden;
libc_hidden_proto(fcntl)
# endif
#else
# ifdef __REDIRECT
extern int __REDIRECT (fcntl, (int __fd, int __cmd, ...), fcntl64);
# else
#  define fcntl fcntl64
# endif
#endif
#if defined(__USE_LARGEFILE64) && !defined(__LP64__)
extern int fcntl64 (int __fd, int __cmd, ...);
# ifdef _LIBC
extern int __fcntl64_nocancel(int, int, long) attribute_hidden;
libc_hidden_proto(fcntl64)
# endif
#endif

/* Open FILE and return a new file descriptor for it, or -1 on error.
   OFLAG determines the type of access used.  If O_CREAT is on OFLAG,
   the third argument is taken as a `mode_t', the mode of the created file.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
#ifndef __USE_FILE_OFFSET64
extern int open (const char *__file, int __oflag, ...) __nonnull ((1));
libc_hidden_proto(open)
# ifdef _LIBC
extern int __open2_nocancel(const char *, int) __nonnull ((1)) attribute_hidden;
extern int __open_nocancel(const char *, int, mode_t) __nonnull ((1)) attribute_hidden;
# endif
#else
# ifdef __REDIRECT
extern int __REDIRECT (open, (const char *__file, int __oflag, ...), open64)
     __nonnull ((1));
# else
#  define open open64
# endif
#endif
#ifdef __USE_LARGEFILE64
extern int open64 (const char *__file, int __oflag, ...) __nonnull ((1));
libc_hidden_proto(open64)
#endif

#ifdef __USE_ATFILE
/* Similar to `open' but a relative path name is interpreted relative to
   the directory for which FD is a descriptor.

   NOTE: some other `openat' implementation support additional functionality
   through this interface, especially using the O_XATTR flag.  This is not
   yet supported here.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
# ifndef __USE_FILE_OFFSET64
extern int openat (int __fd, const char *__file, int __oflag, ...)
     __nonnull ((2));
libc_hidden_proto(openat)
# else
#  ifdef __REDIRECT
extern int __REDIRECT (openat, (int __fd, const char *__file, int __oflag,
				...), openat64) __nonnull ((2));
#  else
#   define openat openat64
#  endif
# endif

extern int openat64 (int __fd, const char *__file, int __oflag, ...)
     __nonnull ((2));
#endif

/* Create and open FILE, with mode MODE.  This takes an `int' MODE
   argument because that is what `mode_t' will be widened to.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
#ifndef __USE_FILE_OFFSET64
extern int creat (const char *__file, __mode_t __mode) __nonnull ((1));
#else
# ifdef __REDIRECT
extern int __REDIRECT (creat, (const char *__file, __mode_t __mode),
		       creat64) __nonnull ((1));
# else
#  define creat creat64
# endif
#endif
#ifdef __USE_LARGEFILE64
extern int creat64 (const char *__file, __mode_t __mode) __nonnull ((1));
#endif

#if !defined F_LOCK && (defined __USE_MISC || (defined __USE_XOPEN_EXTENDED \
					       && !defined __USE_POSIX))
/* NOTE: These declarations also appear in <unistd.h>; be sure to keep both
   files consistent.  Some systems have them there and some here, and some
   software depends on the macros being defined without including both.  */

/* `lockf' is a simpler interface to the locking facilities of `fcntl'.
   LEN is always relative to the current file position.
   The CMD argument is one of the following.  */

# define F_ULOCK 0	/* Unlock a previously locked region.  */
# define F_LOCK  1	/* Lock a region for exclusive use.  */
# define F_TLOCK 2	/* Test and lock a region for exclusive use.  */
# define F_TEST  3	/* Test a region for other processes locks.  */

# ifndef __USE_FILE_OFFSET64
extern int lockf (int __fd, int __cmd, __off_t __len);
libc_hidden_proto(lockf)
# else
#  ifdef __REDIRECT
extern int __REDIRECT (lockf, (int __fd, int __cmd, __off64_t __len), lockf64);
#  else
#   define lockf lockf64
#  endif
# endif
# ifdef __USE_LARGEFILE64
extern int lockf64 (int __fd, int __cmd, __off64_t __len);
# endif
#endif

#if defined __USE_XOPEN2K && defined __UCLIBC_HAS_ADVANCED_REALTIME__
/* Advice the system about the expected behaviour of the application with
   respect to the file associated with FD.  */
# ifndef __USE_FILE_OFFSET64
extern int posix_fadvise (int __fd, __off_t __offset, __off_t __len,
			  int __advise) __THROW;
# else
#  ifdef __REDIRECT_NTH
extern int __REDIRECT_NTH (posix_fadvise, (int __fd, __off64_t __offset,
					   __off64_t __len, int __advise),
			   posix_fadvise64);
#  else
#   define posix_fadvise posix_fadvise64
#  endif
# endif
# ifdef __USE_LARGEFILE64
extern int posix_fadvise64 (int __fd, __off64_t __offset, __off64_t __len,
			    int __advise) __THROW;
# endif

#endif

#if defined __UCLIBC_HAS_ADVANCED_REALTIME__

/* Reserve storage for the data of the file associated with FD.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
# ifndef __USE_FILE_OFFSET64
extern int posix_fallocate (int __fd, __off_t __offset, __off_t __len);
# else
#  ifdef __REDIRECT
extern int __REDIRECT (posix_fallocate, (int __fd, __off64_t __offset,
					 __off64_t __len),
		       posix_fallocate64);
#  else
#   define posix_fallocate posix_fallocate64
#  endif
# endif
# ifdef __USE_LARGEFILE64
extern int posix_fallocate64 (int __fd, __off64_t __offset, __off64_t __len);
# endif
#endif

#if (defined __UCLIBC_LINUX_SPECIFIC__ && defined __USE_GNU) || defined _LIBC
/* Reserve storage for the data of a file associated with FD.  This function
   is Linux-specific.  For the portable version, use posix_fallocate().
   Unlike the latter, fallocate can operate in different modes.  The default
   mode = 0 is equivalent to posix_fallocate().

   Note: These declarations are used in posix_fallocate.c and
   posix_fallocate64.c, so we expose them internally.
 */

/* Flags for fallocate's mode.  */
# define FALLOC_FL_KEEP_SIZE            1 /* Don't extend size of file
                                             even if offset + len is
                                             greater than file size.  */
# define FALLOC_FL_PUNCH_HOLE           2 /* Create a hole in the file.  */

# ifndef __USE_FILE_OFFSET64
extern int fallocate (int __fd, int __mode, __off_t __offset, __off_t __len);
# else
#  ifdef __REDIRECT
extern int __REDIRECT (fallocate, (int __fd, int __mode, __off64_t __offset,
				   __off64_t __len),
		       fallocate64);
#  else
#   define fallocate fallocate64
#  endif
# endif
# ifdef __USE_LARGEFILE64
extern int fallocate64 (int __fd, int __mode, __off64_t __offset, __off64_t __len);
# endif
#endif

__END_DECLS

#endif /* fcntl.h  */
