/* Uncancelable versions of cancelable interfaces.  Linux version.
   Copyright (C) 2003, 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

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

#include <sys/types.h>
#include <sysdep.h>

/* Uncancelable open.  */
#define open_not_cancel(name, flags, mode) \
   INLINE_SYSCALL (open, 3, (const char *) (name), (flags), (mode))
#define open_not_cancel_2(name, flags) \
   INLINE_SYSCALL (open, 2, (const char *) (name), (flags))

/* Uncancelable openat.  */
#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt
extern int __openat_nocancel (int fd, const char *fname, int oflag,
			      mode_t mode) attribute_hidden;
extern int __openat64_nocancel (int fd, const char *fname, int oflag,
				mode_t mode) attribute_hidden;
#else
# define __openat_nocancel(fd, fname, oflag, mode) \
  openat (fd, fname, oflag, mode)
# define __openat64_nocancel(fd, fname, oflag, mode) \
  openat64 (fd, fname, oflag, mode)
#endif

#define openat_not_cancel(fd, fname, oflag, mode) \
  __openat_nocancel (fd, fname, oflag, mode)
#define openat_not_cancel_3(fd, fname, oflag) \
  __openat_nocancel (fd, fname, oflag, 0)
#define openat64_not_cancel(fd, fname, oflag, mode) \
  __openat64_nocancel (fd, fname, oflag, mode)
#define openat64_not_cancel_3(fd, fname, oflag) \
  __openat64_nocancel (fd, fname, oflag, 0)

/* Uncancelable close.  */
#define close_not_cancel(fd) \
  INLINE_SYSCALL (close, 1, fd)
#define close_not_cancel_no_status(fd) \
  (void) ({ INTERNAL_SYSCALL_DECL (err);				      \
	    INTERNAL_SYSCALL (close, err, 1, (fd)); })

/* Uncancelable read.  */
#define read_not_cancel(fd, buf, n) \
  INLINE_SYSCALL (read, 3, (fd), (buf), (n))

/* Uncancelable write.  */
#define write_not_cancel(fd, buf, n) \
  INLINE_SYSCALL (write, 3, (fd), (buf), (n))

/* Uncancelable writev.  */
#define writev_not_cancel_no_status(fd, iov, n) \
  (void) ({ INTERNAL_SYSCALL_DECL (err);				      \
	    INTERNAL_SYSCALL (writev, err, 3, (fd), (iov), (n)); })

/* Uncancelable fcntl.  */
#define fcntl_not_cancel(fd, cmd, val) \
  __fcntl_nocancel (fd, cmd, val)

/* Uncancelable waitpid.  */
#ifdef __NR_waitpid
# define waitpid_not_cancel(pid, stat_loc, options) \
  INLINE_SYSCALL (waitpid, 3, pid, stat_loc, options)
#else
# define waitpid_not_cancel(pid, stat_loc, options) \
  INLINE_SYSCALL (wait4, 4, pid, stat_loc, options, NULL)
#endif

/* Uncancelable pause.  */
#ifdef __NR_pause
# define pause_not_cancel() \
  INLINE_SYSCALL (pause, 0)
#else
# define pause_not_cancel() \
  __pause_nocancel ()
#endif

/* Uncancelable nanosleep.  */
#ifdef __NR_nanosleep
# define nanosleep_not_cancel(requested_time, remaining) \
  INLINE_SYSCALL (nanosleep, 2, requested_time, remaining)
#else
# define nanosleep_not_cancel(requested_time, remaining) \
  __nanosleep_nocancel (requested_time, remaining)
#endif

/* Uncancelable sigsuspend.  */
#define sigsuspend_not_cancel(set) \
  __sigsuspend_nocancel (set)
