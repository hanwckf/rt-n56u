/* Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2003.

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

#include "pthreadP.h"

int
pthread_spin_lock (pthread_spinlock_t *lock)
{
  __asm__ __volatile
    ("1: ldstub  [%0], %%g5\n"
     "   brnz,pn %%g5, 2f\n"
     "    membar #StoreLoad | #StoreStore\n"
     ".subsection 2\n"
     "2: ldub    [%0], %%g5\n"
     "   brnz,pt %%g5, 2b\n"
     "    membar #LoadLoad\n"
     "   b,a,pt  %%xcc, 1b\n"
     ".previous"
     : /* no outputs */
     : "r" (lock)
     : "g5", "memory");
  return 0;
}
