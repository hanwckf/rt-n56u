/* vi: set sw=4 ts=4: */
/* Allocate memory on a page boundary.
   Copyright (C) 1991, 1992 Free Software Foundation, Inc.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.

   The author may be reached (Email) at the address mike@@ai.mit.edu,
   or (US mail) as Mike Haertel c/o Free Software Foundation.  */

#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>

static size_t pagesize;

__ptr_t valloc (size_t size)
{
	if (pagesize == 0)
		pagesize = getpagesize ();

	return memalign(pagesize, size);
}
