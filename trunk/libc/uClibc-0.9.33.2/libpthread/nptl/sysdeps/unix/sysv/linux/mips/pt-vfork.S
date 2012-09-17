/* Copyright (C) 2005 Free Software Foundation, Inc.
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

#include <features.h>
#include <tls.h>

/* Save the PID value.  */
#define SAVE_PID \
	READ_THREAD_POINTER(v1);	/* Get the thread pointer.  */	\
	lw	a2, PID_OFFSET(v1);	/* Load the saved PID.  */	\
	subu	a2, $0, a2;		/* Negate it.  */		\
	sw	a2, PID_OFFSET(v1);	/* Store the temporary PID.  */

/* Restore the old PID value in the parent.  */
#define RESTORE_PID \
	beqz	v0, 1f;			/* If we are the parent... */	\
	READ_THREAD_POINTER(v1);	/* Get the thread pointer.  */	\
	lw	a2, PID_OFFSET(v1);	/* Load the saved PID.  */	\
	subu	a2, $0, a2;		/* Re-negate it.  */		\
	sw	a2, PID_OFFSET(v1);	/* Restore the PID.  */		\
1:

#include <../../../../../../../libc/sysdeps/linux/mips/vfork.S>
