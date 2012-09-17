/* Copyright (C) 1991,92,94,95,96,97,98,99,2000,2001 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

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

/* Do the file control operation described by CMD on FD.
   The remaining arguments are interpreted depending on CMD.  */
#ifndef __USE_FILE_OFFSET64
extern int fcntl (int __fd, int __cmd, ...) __THROW;
#else
# ifdef __REDIRECT
extern int __REDIRECT (fcntl, (int __fd, int __cmd, ...) __THROW,
		       fcntl64);
# else
#  define fcntl fcntl64
# endif
#endif

/* Open FILE and return a new file descriptor for it, or -1 on error.
   OFLAG determines the type of access used.  If O_CREAT is on OFLAG,
   the third argument is taken as a `mode_t', the mode of the created file.  */
#ifndef __USE_FILE_OFFSET64
extern int open (__const char *__file, int __oflag, ...) __THROW;
#else
# ifdef __REDIRECT
extern int __REDIRECT (open, (__const char *__file, int __oflag, ...) __THROW,
		       open64);
# else
#  define open open64
# endif
#endif
#ifdef __USE_LARGEFILE64
extern int open64 (__const char *__file, int __oflag, ...) __THROW;
#endif

/* Create and open FILE, with mode MODE.
   This takes an `int' MODE argument because that is
   what `mode_t' will be widened to.  */
#ifndef __USE_FILE_OFFSET64
extern int creat (__const char *__file, __mode_t __mode) __THROW;
#else
# ifdef __REDIRECT
extern int __REDIRECT (creat, (__const char *__file, __mode_t __mode) __THROW,
		       creat64);
# else
#  define creat creat64
# endif
#endif
#ifdef __USE_LARGEFILE64
extern int creat64 (__const char *__file, __mode_t __mode) __THROW;
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
extern int lockf (int __fd, int __cmd, __off_t __len) __THROW;
# else
# ifdef __REDIRECT
extern int __REDIRECT (lockf, (int __fd, int __cmd, __off64_t __len) __THROW,
		       lockf64);
# else
#  define lockf lockf64
# endif
# endif
# ifdef __USE_LARGEFILE64
extern int lockf64 (int __fd, int __cmd, __off64_t __len) __THROW;
# endif
#endif


#if 0

/* FIXME -- uClibc should probably implement these... */

#ifdef __USE_XOPEN2K
/* Advice the system about the expected behaviour of the application with
   respect to the file associated with FD.  */
# ifndef __USE_FILE_OFFSET64
extern int posix_fadvise (int __fd, __off_t __offset, size_t __len,
			  int __advise) __THROW;
# else
# ifdef __REDIRECT
extern int __REDIRECT (posix_fadvise, (int __fd, __off64_t __offset,
				       size_t __len, int __advise) __THROW,
		       posix_fadvise64);
# else
#  define posix_fadvise posix_fadvise64
# endif
# endif
# ifdef __USE_LARGEFILE64
extern int posix_fadvise64 (int __fd, __off64_t __offset, size_t __len,
			    int __advise) __THROW;
# endif


/* Reserve storage for the data of the file associated with FD.  */
# ifndef __USE_FILE_OFFSET64
extern int posix_fallocate (int __fd, __off_t __offset, size_t __len) __THROW;
# else
# ifdef __REDIRECT
extern int __REDIRECT (posix_fallocate, (int __fd, __off64_t __offset,
					 size_t __len) __THROW,
		       posix_fallocate64);
# else
#  define posix_fallocate posix_fallocate64
# endif
# endif
# ifdef __USE_LARGEFILE64
extern int posix_fallocate64 (int __fd, __off64_t __offset, size_t __len)
     __THROW;
# endif
#endif
#endif

__END_DECLS

#endif /* fcntl.h  */
