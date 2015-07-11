/* Copyright (C) 1992, 1993, 1994, 1997, 1998, 1999, 2000, 2002, 2003
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Brendan Kehoe (brendan@zen.org).

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

#include <sys/asm.h>
#include <sysdep.h>
#include <bits/errno.h>

#ifdef _LIBC_REENTRANT

LOCALSZ= 3
FRAMESZ= (((NARGSAVE+LOCALSZ)*SZREG)+ALSZ)&ALMASK
RAOFF= FRAMESZ-(1*SZREG)
GPOFF= FRAMESZ-(2*SZREG)
V0OFF= FRAMESZ-(3*SZREG)

ENTRY(__syscall_error)
#ifdef __PIC__
	.set noat
	SETUP_GPX (AT)
	.set at
#endif
	PTR_SUBU sp, FRAMESZ
	.set noat
	SETUP_GPX64(GPOFF,AT)
	.set at
#ifdef __PIC__
	SAVE_GP(GPOFF)
#endif
	REG_S	a0, V0OFF(sp)
	REG_S	ra, RAOFF(sp)

	/* Find our per-thread errno address  */
	jal	__errno_location

	/* Store the error value.  */
	REG_L	t0, V0OFF(sp)
	sw	t0, 0(v0)

	/* And just kick back a -1.  */
	REG_L	ra, RAOFF(sp)
	RESTORE_GP64
	PTR_ADDU sp, FRAMESZ
	li	v0, -1
	j	ra
	END(__syscall_error)

#else /* __LIBC_REENTRANT */


ENTRY(__syscall_error)
#ifdef __PIC__
	.set noat
	SETUP_GPX (AT)
	.set at
#endif
	SETUP_GPX64 (t9, AT)

	/* Store it in errno... */
	sw v0, errno

	/* And just kick back a -1.  */
	li v0, -1

	RESTORE_GP64
	j ra
	END(__syscall_error)
#endif  /* _LIBC_REENTRANT*/
