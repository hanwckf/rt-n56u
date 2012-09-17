/* Copyright (C) 2002,2003,2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#ifndef LOCK_PREFIX
# ifdef UP
#  define LOCK_PREFIX	/* nothing */
# else
#  define LOCK_PREFIX	"lock;"
# endif
#endif


int
pthread_spin_lock (
     pthread_spinlock_t *lock)
{
  __asm__ ("\n"
       "1:\t" LOCK_PREFIX "decl %0\n\t"
       "jne 2f\n\t"
       ".subsection 1\n\t"
       ".align 16\n"
       "2:\trep; nop\n\t"
       "cmpl $0, %0\n\t"
       "jg 1b\n\t"
       "jmp 2b\n\t"
       ".previous"
       : "=m" (*lock)
       : "m" (*lock));

  return 0;
}
