/* BSD `setjmp' entry point to `sigsetjmp (..., 1)'.  i386 version.
   Copyright (C) 1995, 1996, 1997, 2000, 2001 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

.global setjmp
.type   setjmp,%function
setjmp:
	movl 4   (%esp), %eax
	/* Save registers.  */
	movl %ebx, (0 *4)(%eax)
	movl %esi, (1 *4)(%eax)
	movl %edi, (2 *4)(%eax)
	/* Save SP as it will be after we return.  */
	leal 4   (%esp), %ecx
	movl %ecx, (4 *4)(%eax)
	/* Save PC we are returning to now.  */
	movl 0 (%esp), %ecx
	movl %ecx, (5 *4)(%eax)
	/* Save caller's frame pointer.  */
	movl %ebp, (3 *4)(%eax)  

	/* Call __sigjmp_save.  */
	pushl $1
	pushl 8(%esp)
#ifdef	__PIC__
	/* We cannot use the PLT, because it requires that %ebx be set, but
	   we can't save and restore our caller's value.  Instead, we do an
	   indirect jump through the GOT, using for the temporary register
	   %ecx, which is call-clobbered.  */
	call here2
here2:	popl %ecx
	addl $_GLOBAL_OFFSET_TABLE_+[.-here2], %ecx
	movl    __sigjmp_save    @GOT  (%ecx), %ecx
	call *%ecx
#else
	call __sigjmp_save
#endif
	popl %ecx
	popl %edx
	ret
.size setjmp,.-setjmp
