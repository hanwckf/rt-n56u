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

/* We want an #include_next, but we are the main source file.
   So, #include ourselves and in that incarnation we can use #include_next.  */
#ifndef INCLUDED_SELF
# define INCLUDED_SELF
# include <vfork.S>
#else

# include <tcb-offsets.h>

# define SAVE_PID \
	movl	%fs:PID, %esi;						      \
	movl	$0x80000000, %ecx;					      \
	movl	%esi, %edx;						      \
	negl	%edx;							      \
	cmove	%ecx, %edx;						      \
	movl	%edx, %fs:PID

# define RESTORE_PID \
	testq	%rax, %rax;						      \
	je	1f;							      \
	movl	%esi, %fs:PID;						      \
1:

# include_next <vfork.S>
#endif
