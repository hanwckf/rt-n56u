/* Copyright (C) 1993, 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>
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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <search.h>

/* The non-reentrant version use a global space for storing the table.  */
static struct hsearch_data htab;


/* Define the non-reentrant function using the reentrant counterparts.  */
ENTRY *hsearch (ENTRY item, ACTION action)
{
  ENTRY *result;

  (void) hsearch_r (item, action, &result, &htab);

  return result;
}


int hcreate (size_t nel)
{
  return hcreate_r (nel, &htab);
}


/* void __hdestroy (void) */
void hdestroy (void)
{
  hdestroy_r (&htab);
}
/* weak_alias (__hdestroy, hdestroy) */

/* Make sure the table is freed if we want to free everything before
   exiting.  */
/* text_set_element (__libc_subfreeres, __hdestroy); */
