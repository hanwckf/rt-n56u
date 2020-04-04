/* Copyright (C) 1995, 1996, 1997, 2002 Free Software Foundation, Inc.
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

#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#ifdef __UCLIBC_HAS_THREADS_NATIVE__
#include <sysdep-cancel.h>
#endif

libc_hidden_proto(ioctl)

extern __typeof(tcdrain) __libc_tcdrain;
/* Wait for pending output to be written on FD.  */
int __libc_tcdrain (int fd)
{
#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	if (SINGLE_THREAD_P)
		/* With an argument of 1, TCSBRK for output to be drain.  */
		return INLINE_SYSCALL (ioctl, 3, fd, TCSBRK, 1);

	int oldtype = LIBC_CANCEL_ASYNC ();

	/* With an argument of 1, TCSBRK for output to be drain.  */
	int result = INLINE_SYSCALL (ioctl, 3, fd, TCSBRK, 1);

	LIBC_CANCEL_RESET (oldtype);

	return result;
#else
	return ioctl(fd, TCSBRK, 1);
#endif
}
weak_alias(__libc_tcdrain,tcdrain)
