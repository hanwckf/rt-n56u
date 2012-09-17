/* Copyright (C) 2004 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <tcb-offsets.h>

#undef PSEUDO_PREPARE_ARGS
#define PSEUDO_PREPARE_ARGS						\
	/* Load the current cached pid value across the vfork.  */	\
	rduniq;								\
	ldl	a2, PID_OFFSET(v0);					\
	mov	v0, a1;							\
	/* If the cached value is initialized (nonzero), then write	\
	   back its negation, or INT_MIN, to indicate that the pid	\
	   value is uninitialized in the the child, and in the window	\
	   between here and the point at which we restore the value.  */ \
	ldah	t0, -0x8000;						\
	negl	a2, t1;							\
	cmovne	a2, t1, t0;						\
	stl	t0, PID_OFFSET(v0);

PSEUDO (__vfork, vfork, 0)

	/* If we're back in the parent, restore the saved pid.  */
	beq	v0, 1f
	stl	a2, PID_OFFSET(a1)
1:	ret

PSEUDO_END (__vfork)
libc_hidden_def (__vfork)
weak_alias (__vfork, vfork)
