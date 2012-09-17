/* Copyright (C) 1999,2002,2004,2006 Free Software Foundation, Inc.
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

#include <tcb-offsets.h>

/* Save the PID value.  */
#define SAVE_PID \
	movl	%gs:PID, %edx; 						      \
	movl	%edx, %eax;						      \
	negl	%eax;							      \
	jne	1f;							      \
	movl	$0x80000000, %eax;					      \
1:	movl	%eax, %gs:PID

/* Restore the old PID value in the parent.  */
#define RESTORE_PID \
	testl	%eax, %eax;						      \
	je	1f;							      \
	movl	%edx, %gs:PID;						      \
1:


#include <libc/sysdeps/linux/i386/vfork.S>
