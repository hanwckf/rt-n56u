/* Copyright (C) 1992, 1995, 1996 Free Software Foundation, Inc.
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

#include <features.h>
#include <stddef.h>
#include <search.h>

#ifdef L_insque

/* Insert ELEM into a doubly-linked list, after PREV.  */

void
insque (void *elem, void *prev)
{
  if (prev == NULL)
    {
      ((struct qelem *) elem)->q_forw = NULL;
      ((struct qelem *) elem)->q_back = NULL;
    }
  else
    {
      struct qelem *next = ((struct qelem *) prev)->q_forw;
      ((struct qelem *) prev)->q_forw = (struct qelem *) elem;
      if (next != NULL)
        next->q_back = (struct qelem *) elem;
      ((struct qelem *) elem)->q_forw = next;
      ((struct qelem *) elem)->q_back = (struct qelem *) prev;
    }
}

#endif

#ifdef L_remque
/* Unlink ELEM from the doubly-linked list that it is in.  */

void
remque (void *elem)
{
  struct qelem *next = ((struct qelem *) elem)->q_forw;
  struct qelem *prev = ((struct qelem *) elem)->q_back;
  if (next != NULL)
    next->q_back = prev;
  if (prev != NULL)
    prev->q_forw = (struct qelem *) next;
}

#endif
