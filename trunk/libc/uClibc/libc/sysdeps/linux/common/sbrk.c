/* Copyright (C) 1991, 1995, 1996, 1997, 2000 Free Software Foundation, Inc.
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

#include <unistd.h>
#include <errno.h>

/* Defined in brk.c.  */
extern void *__curbrk;
extern int brk (void *addr);


/* Extend the process's data space by INCREMENT.
   If INCREMENT is negative, shrink data space by - INCREMENT.
   Return start of new space allocated, or -1 for errors.  */
void * sbrk (intptr_t increment)
{
    void *oldbrk;

    if (__curbrk == NULL)
	if (brk (0) < 0)		/* Initialize the break.  */
	    return (void *) -1;

    if (increment == 0)
	return __curbrk;

    oldbrk = __curbrk;
    if (brk (oldbrk + increment) < 0)
	return (void *) -1;

    return oldbrk;
}

