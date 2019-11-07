/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */
/*
 * Never include this file directly; use <unistd.h> instead.
 */

#ifndef	_BITS_UCLIBC_POSIX_OPT_H
#define	_BITS_UCLIBC_POSIX_OPT_H	1

/* This file works correctly only if posix_opt.h is the NPTL version */
#ifndef _POSIX_THREADS
# error posix_opt.h was incorrectly updated, use the NPTL version from glibc
#endif

/* change first options based on what glibc does */

#ifndef __UCLIBC_HAS_THREADS_NATIVE__
# undef _POSIX_THREAD_PROCESS_SHARED
# define _POSIX_THREAD_PROCESS_SHARED	-1
# undef _POSIX_CLOCK_SELECTION
# define _POSIX_CLOCK_SELECTION		-1
# undef _POSIX_THREAD_PRIO_INHERIT
# define _POSIX_THREAD_PRIO_INHERIT	-1
# undef _POSIX_THREAD_PRIO_PROTECT
# define _POSIX_THREAD_PRIO_PROTECT	-1
# undef _POSIX_THREAD_ROBUST_PRIO_INHERIT
# undef _POSIX_THREAD_ROBUST_PRIO_PROTECT
#endif

/* this has to be adapted to uClibc, not all are thread related */
#ifndef __UCLIBC_HAS_THREADS__
# undef _XOPEN_REALTIME_THREADS
# undef _POSIX_THREADS
# undef _POSIX_REENTRANT_FUNCTIONS
# undef _POSIX_THREAD_SAFE_FUNCTIONS
# undef _POSIX_THREAD_PRIORITY_SCHEDULING
# undef _POSIX_THREAD_ATTR_STACKSIZE
# undef _POSIX_THREAD_ATTR_STACKADDR
# undef _POSIX_THREAD_PRIO_INHERIT
# undef _POSIX_THREAD_PRIO_PROTECT
# undef _POSIX_SEMAPHORES
# undef _POSIX_ASYNCHRONOUS_IO
# undef _POSIX_ASYNC_IO
# undef _LFS_ASYNCHRONOUS_IO
# undef _POSIX_PRIORITIZED_IO
# undef _LFS64_ASYNCHRONOUS_IO
# undef _POSIX_CPUTIME
# undef _POSIX_THREAD_CPUTIME
# undef _POSIX_READER_WRITER_LOCKS
# undef _POSIX_TIMEOUTS
# undef _POSIX_SPIN_LOCKS
# undef _POSIX_BARRIERS
# undef _POSIX_MESSAGE_PASSING
# undef _POSIX_THREAD_PROCESS_SHARED
# undef _POSIX_CLOCK_SELECTION
# undef _POSIX_ADVISORY_INFO
/*# undef _POSIX_RAW_SOCKETS*/
/*# undef _POSIX2_CHAR_TERM*/
# undef _POSIX_SPORADIC_SERVER
# undef _POSIX_THREAD_SPORADIC_SERVER
/*# undef _POSIX_TRACE
# undef _POSIX_TRACE_EVENT_FILTER
# undef _POSIX_TRACE_INHERIT
# undef _POSIX_TRACE_LOG
# undef _POSIX_TYPED_MEMORY_OBJECTS*/
#endif

/* were in earlier version, used by sysconf */
#define	_POSIX_POLL	1
#define	_POSIX_SELECT	1

/* disable independently unsupported features */
#undef _POSIX_TRACE
#undef _POSIX_TRACE_EVENT_FILTER
#undef _POSIX_TRACE_INHERIT
#undef _POSIX_TRACE_LOG
#undef _POSIX_TYPED_MEMORY_OBJECTS
#undef _POSIX_SPAWN

#if 0 /* does uClibc support these? */
# undef _POSIX_ASYNCHRONOUS_IO
# undef _POSIX_ASYNC_IO
# undef _LFS_ASYNCHRONOUS_IO
# undef _POSIX_PRIORITIZED_IO
# undef _LFS64_ASYNCHRONOUS_IO
# undef _POSIX_MESSAGE_PASSING
#endif

/* change options based on uClibc config options */

#if 0 /*ndef __UCLIBC_HAS_POSIX_TIMERS__*/
# undef _POSIX_TIMERS
# undef _POSIX_THREAD_CPUTIME
#endif

#if 0 /*ndef __UCLIBC_HAS_POSIX_BARRIERS__*/
# undef _POSIX_BARRIERS
#endif

#if 0 /*ndef __UCLIBC_HAS_POSIX_SPINLOCKS__*/
# undef _POSIX_SPIN_LOCKS
#endif

#ifndef __ARCH_USE_MMU__
# undef _POSIX_MEMLOCK
# undef _POSIX_MEMLOCK_RANGE
# undef _POSIX_MEMORY_PROTECTION
#endif

#ifndef __UCLIBC_HAS_REALTIME__
# undef _POSIX_SEMAPHORES
#endif

#ifndef __UCLIBC_HAS_REGEX__
# undef _POSIX_REGEXP
#endif

#ifndef __UCLIBC_HAS_IPV6__
# undef _POSIX_IPV6
#endif

#ifndef __UCLIBC_HAS_SOCKET__
# undef _POSIX_RAW_SOCKETS
#endif

#endif /* bits/uClibc_posix_opt.h */
