/* Copyright (C) 1995, 1996, 1997, 1998, 2001, 2002, 2005
   Free Software Foundation, Inc.

   Copyright (C) 2010 Embecosm Limited

   Contributor  Jeremy Bennett <jeremy.bennett@embecosm.com>
   Contributor  Joern Rennecke <joern.rennecke@embecosm.com>
   With other contributions from the OpenCores community.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   In addition to the permissions in the GNU Lesser General Public
   License, the Free Software Foundation gives you unlimited
   permission to link the compiled version of this file with other
   programs, and to distribute those programs without any restriction
   coming from the use of this file. (The GNU Lesser General Public
   License restrictions do apply in other respects; for example, they
   cover modification of the file, and distribution when not linked
   into another program.)

   Note that people who make modified versions of this file are not
   obligated to grant this special exception for their modified
   versions; it is their choice whether to do so. The GNU Lesser
   General Public License gives permission to release a modified
   version without this exception; this exception also makes it
   possible to release a modified version which carries forward this
   exception.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

/* Stick in a dummy reference to main(), so that if an application
 * is linking when the main() function is in a static library (.a)
 * we can be sure that main() actually gets linked in */
.type main,%function

/* This is the canonical entry point, usually the first thing in the text
   segment.

   sp		The stack contains the arguments and environment:
		0(sp)			argc
		4(sp)			argv[0]
		...
		(4*argc)(sp)		NULL
		(4*(argc+1))(sp)	envp[0]
		...
					NULL
*/

#include <features.h>

.text
	.globl	_start
	.type	_start,%function

.global _start
_start:

#ifdef __PIC__
	/* Obtain a pointer to .got in r16 */
	l.jal	.LPC0
#ifndef __OR1K_NODELAY__
	l.nop
#endif
.LPC0:
	l.movhi	r16, gotpchi(_GLOBAL_OFFSET_TABLE_+(.-.LPC0))
	l.ori	r16, r16, gotpclo(_GLOBAL_OFFSET_TABLE_+(.-.LPC0))
	l.add	r16, r16, r9
#endif

	/* Push stack limit onto the stack */
	/* Provides highest stack address to user code (as stack grows
	 * downwards
	 * This is the seventh argument to __uClibc_main and thus needs to
	 * be passed on the stack
	 */
	l.sw  -4(r1),r1

	/* Take values for argc and argv off the stack.
	 * These will be passed as arguments two and three to __uClibc_main
	 * and thus go in registers r4 and r5, respectively
	 */
	l.lwz  r4,0(r1)
	l.addi r5,r1,4

#ifdef __PIC__
	l.lwz	r3,got(main)(r16)
#else
	l.movhi r3,hi(main)
	l.ori   r3,r3,lo(main)
#endif

	/* Put the constructor and destructor initialization as args four and
	   five into r6 and r7 */
#ifdef __PIC__
	l.lwz	r6, got(_init)(r16)
	l.lwz	r7, got(_fini)(r16)
#else
	l.movhi	r6,hi(_init)
	l.ori	r6,r6,lo(_init)
	l.movhi	r7,hi(_fini)
	l.ori	r7,r7,lo(_fini)
#endif

	/* rtld_fini = NULL */
	l.movhi	r8, 0

	/* Adjust stack to account for a total of 7 args (i.e. the last one is
	   on the stack. */
	l.addi  r1,r1,-4

	/* Clear the frame pointer and link register since this is the
	 * outermost frame.
	 */
	l.add r2,r0,r0
	l.add r9,r0,r0

	/* Let the libc call main and exit with its return code.  */
#ifdef __PIC__
	l.j	plt(__uClibc_main)
#else
	l.j	__uClibc_main
#endif
	 l.nop

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start
