/* _longjmp_unwind -- Clean up stack frames unwound by longjmp.
   Copyright (C) 2002, 2003 Free Software Foundation, Inc.
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
#include <bits/libc-lock.h>

#ifndef SHARED
weak_extern (__pthread_cleanup_upto);
#endif

void
_longjmp_unwind (jmp_buf env, int val)
{
  __libc_maybe_call2 (pthread_cleanup_upto,
		      (env->__jmpbuf, __builtin_frame_address (0)),
		      (void) 0);
}
