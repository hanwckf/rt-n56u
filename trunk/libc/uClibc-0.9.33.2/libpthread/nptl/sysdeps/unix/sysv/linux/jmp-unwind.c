/* Clean up stack frames unwound by longjmp.  Linux version.
   Copyright (C) 1995, 1997, 2002, 2003, 2007 Free Software Foundation, Inc.
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

#include <setjmp.h>
#include <stddef.h>
#include <pthreadP.h>

extern void __pthread_cleanup_upto (__jmp_buf env, char *targetframe);
#pragma weak __pthread_cleanup_upto


void _longjmp_unwind (jmp_buf env, int val);
void
_longjmp_unwind (jmp_buf env, int val)
{
#ifdef SHARED
  if (__libc_pthread_functions_init)
    PTHFCT_CALL (ptr___pthread_cleanup_upto, (env->__jmpbuf,
					      CURRENT_STACK_FRAME));
#else
  if (__pthread_cleanup_upto != NULL)
    __pthread_cleanup_upto (env->__jmpbuf, CURRENT_STACK_FRAME);
#endif
}
