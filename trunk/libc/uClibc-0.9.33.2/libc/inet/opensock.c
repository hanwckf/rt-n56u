/* Copyright (C) 1999, 2001, 2002 Free Software Foundation, Inc.
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

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <features.h>
#include <libc-internal.h>

/* Return a socket of any type.  The socket can be used in subsequent
   ioctl calls to talk to the kernel.  */
int __opensock(void) attribute_hidden;
int
__opensock(void)
{
	int fd = -1;
#ifdef __UCLIBC_HAS_IPV6__
	fd = socket(AF_INET6, SOCK_DGRAM, 0);
#endif
#ifdef __UCLIBC_HAS_IPV4__
	if (fd < 0)
		fd = socket(AF_INET, SOCK_DGRAM, 0);
#endif
	return fd;
}
