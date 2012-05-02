/* Wrapper for setting errno.
   Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
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

#include <errno.h>
#include <features.h>

/* This routine is jumped to by all the syscall handlers, to stash
 * an error number into errno.  */

/* This version uses a lot of magic and relies heavily on x86 
 * calling convention ... The advantage is that this is the same 
 * size as the previous __syscall_error() but all the .S functions
 * need just one instruction.
 *
 * Local .S files have to set %eax to the negative errno value 
 * and then jump to this function.  The neglected return to caller 
 * and return value of -1 is taken care of here so we don't have to 
 * worry about it in the .S functions.
 *
 * We have to stash the errno from %eax in a local stack var because 
 * __set_errno will prob call a function thus clobbering %eax on us.
 */
int attribute_hidden __syscall_error(void)
{
	register int edx asm("%edx");
	asm("mov %eax, %edx");
	asm("negl %edx");
	__set_errno(edx);
	return -1;
}
