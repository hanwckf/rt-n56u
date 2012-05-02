/* Copyright 2003 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation; either version 2.1 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#define _GNU_SOURCE
#include <link.h>

extern int __attribute__((__weak__))
__dl_iterate_phdr (int (*callback) (struct dl_phdr_info *info,
				    size_t size, void *data),
		   void *data);

/* Define it as a pointer, such that we get a pointer to the global
   function descriptor, that won't be optimized away by the
   linker.  */
static int (*ptr) (int (*callback) (struct dl_phdr_info *info,
				    size_t size, void *data),
		   void *data) = __dl_iterate_phdr;

int
dl_iterate_phdr (int (*callback) (struct dl_phdr_info *info,
				  size_t size, void *data),
		 void *data)
{
  if (ptr)
    return ptr (callback, data);

  return 0;
}
