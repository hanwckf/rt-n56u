/* Copyright (C) 2006 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <features.h>

#ifdef __UCLIBC_HAS_THREADS_NATIVE__

#include <sysdep.h>

#if defined __thumb__

/* Out-of-line atomic operations that we can't do in Thumb mode.
   This ends up in various libraries where it is needed (and
   a few .a archives where it isn't).  */

	.hidden __thumb_swpb
ENTRY (__thumb_swpb)
	swpb	r0, r0, [r1]
	bx	lr
END (__thumb_swpb)

	.hidden __thumb_swp
ENTRY (__thumb_swp)
	swp	r0, r0, [r1]
	bx	lr
END (__thumb_swp)

	.hidden __thumb_cmpxchg
ENTRY (__thumb_cmpxchg)
	stmdb	sp!, {r4, lr}
	mov	r4, r0
0:	ldr	r3, [r2]
	cmp	r3, r4
	bne	1f
	mov	r0, r4
	mov	r3, #0xffff0fff
	mov	lr, pc
	add	pc, r3, #(0xffff0fc0 - 0xffff0fff)
	bcc	0b
	mov	r3, r4
1:	mov	r0, r3
	ldmia	sp!, {r4, pc}
END (__thumb_cmpxchg)

#endif /* __thumb__ */
#endif /* __UCLIBC_HAS_THREADS_NATIVE__ */

