/* Define POSIX options for Linux.
   Copyright (C) 1996,1997,1998,1999,2000,2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef	_POSIX_OPT_H
#define	_POSIX_OPT_H	1

/* Job control is supported.  */
#define	_POSIX_JOB_CONTROL	1

/* Processes have a saved set-user-ID and a saved set-group-ID.  */
#define	_POSIX_SAVED_IDS	1

/* Priority scheduling is supported.  */
#define	_POSIX_PRIORITY_SCHEDULING	1

/* Synchronizing file data is supported.  */
#define	_POSIX_SYNCHRONIZED_IO	1

/* The fsync function is present.  */
#define	_POSIX_FSYNC	1

/* Mapping of files to memory is supported.  */
#define	_POSIX_MAPPED_FILES	1

/* Locking of all memory is supported.  */
#ifdef __ARCH_HAS_MMU__
# define	_POSIX_MEMLOCK	1
#else
# undef		_POSIX_MEMLOCK
#endif

/* Locking of ranges of memory is supported.  */
#ifdef __ARCH_HAS_MMU__
# define	_POSIX_MEMLOCK_RANGE	1
#else
# define	_POSIX_MEMLOCK_RANGE
#endif

/* Setting of memory protections is supported.  */
#ifdef __ARCH_HAS_MMU__
# define	_POSIX_MEMORY_PROTECTION	1
#else
# undef		_POSIX_MEMORY_PROTECTION
#endif

/* Implementation supports `poll' function.  */
#define	_POSIX_POLL	1

/* Implementation supports `select' and `pselect' functions.  */
#define	_POSIX_SELECT	1

/* Only root can change owner of file.  */
#define	_POSIX_CHOWN_RESTRICTED	1

/* `c_cc' member of 'struct termios' structure can be disabled by
   using the value _POSIX_VDISABLE.  */
#define	_POSIX_VDISABLE	'\0'

/* Filenames are not silently truncated.  */
#define	_POSIX_NO_TRUNC	1

/* X/Open realtime support is available.  */
#define _XOPEN_REALTIME	1

/* X/Open realtime thread support is available.  */
#ifdef __UCLIBC_HAS_THREADS__
# define _XOPEN_REALTIME_THREADS	1
#else
# undef _XOPEN_REALTIME_THREADS
#endif

/* XPG4.2 shared memory is supported.  */
#define	_XOPEN_SHM	1

/* Tell we have POSIX threads.  */
#ifdef __UCLIBC_HAS_THREADS__
# define _POSIX_THREADS	1
#else
# undef _POSIX_THREADS
#endif

/* We have the reentrant functions described in POSIX.  */
#ifdef __UCLIBC_HAS_THREADS__
# define _POSIX_REENTRANT_FUNCTIONS      1
# define _POSIX_THREAD_SAFE_FUNCTIONS	1
#else
# undef _POSIX_REENTRANT_FUNCTIONS
# undef _POSIX_THREAD_SAFE_FUNCTIONS
#endif

/* We provide priority scheduling for threads.  */
#define	_POSIX_THREAD_PRIORITY_SCHEDULING	1

/* We support user-defined stack sizes.  */
#define _POSIX_THREAD_ATTR_STACKSIZE	1

/* We support user-defined stacks.  */
#define _POSIX_THREAD_ATTR_STACKADDR	1

/* We support POSIX.1b semaphores, but only the non-shared form for now.  */
#ifdef __UCLIBC_HAS_THREADS__
# define _POSIX_SEMAPHORES	1
#else
# undef _POSIX_SEMAPHORES
#endif

/* Real-time signals are supported.  */
#define _POSIX_REALTIME_SIGNALS	1

/* We support asynchronous I/O.  */
#define _POSIX_ASYNCHRONOUS_IO	1
#define _POSIX_ASYNC_IO		1
/* Alternative name for Unix98.  */
#define _LFS_ASYNCHRONOUS_IO	1

/* The LFS support in asynchronous I/O is also available.  */
#ifdef __UCLIBC_HAS_LFS__
# define _LFS64_ASYNCHRONOUS_IO	1
#else
# undef _LFS64_ASYNCHRONOUS_IO
#endif

/* The rest of the LFS is also available.  */
#ifdef __UCLIBC_HAS_LFS__
# define _LFS_LARGEFILE		1
# define _LFS64_LARGEFILE	1
# define _LFS64_STDIO		1
#else
# undef _LFS_LARGEFILE
# undef _LFS64_LARGEFILE
# undef _LFS64_STDIO
#endif

/* POSIX shared memory objects are implemented.  */
#define _POSIX_SHARED_MEMORY_OBJECTS	1

/* GNU libc provides regular expression handling.  */
#ifdef __UCLIBC_HAS_REGEX__
# define _POSIX_REGEXP	1
#else
# undef _POSIX_REGEXP
#endif

#if defined(__i386__)
/* CPU-time clocks supported.  */
#define _POSIX_CPUTIME 200912L

/* We support the clock also in threads.  */
#define _POSIX_THREAD_CPUTIME  200912L
#endif

/* Reader/Writer locks are available.  */
#define _POSIX_READER_WRITER_LOCKS	200912L

/* We have a POSIX shell.  */
#define _POSIX_SHELL	1

/* We support the Timeouts option.  */
#define _POSIX_TIMEOUTS	200912L

/* We support spinlocks.  */
#define _POSIX_SPIN_LOCKS	200912L

/* The `spawn' function family is supported.  */
#define _POSIX_SPAWN	200912L

/* We have POSIX timers.  */
#define _POSIX_TIMERS	1

/* The barrier functions are available.  */
#define _POSIX_BARRIERS	200912L

/* POSIX message queues are not yet supported.  */
#undef	_POSIX_MESSAGE_PASSING

#endif /* posix_opt.h */
