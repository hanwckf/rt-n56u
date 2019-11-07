/*
 * Copyright (C) 2004-2007 Atmel Corporation
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.  See the file "COPYING.LIB" in the main directory of this
 * archive for more details.
 */
#include <features.h>

	.text

	/*
	 * long int syscall(long int sysno, ...)
	 */
	.global	syscall
	.type	syscall, @function
	.align	2
syscall:
	stm	--sp, r3,r5,r6,lr
	sub	lr, sp, -16
	mov	r8, r12
	ldm	lr, r3,r5,r9-r12
	scall
	cp.w	r12, -4095
	brlo	.Ldone

#ifdef __PIC__
	lddpc	r6, .Lgot
.Lgotcalc:
	rsub	r6, pc
# ifdef __UCLIBC_HAS_THREADS__
	rsub	r3, r12, 0
	mcall	r6[__errno_location@got]
	st.w	r12[0], r3
# else
	ld.w	r3, r6[errno@got]
	neg	r12
	st.w	r3[0], r12
# endif
#else
# ifdef __UCLIBC_HAS_THREADS__
	rsub	r3, r12, 0
	mcall	.Lerrno_location
	st.w	r12[0], r3
# else
	lddpc	r3, .Lerrno
	neg	r12
	st.w	r3[0], r12
# endif
#endif
	mov	r12, -1

.Ldone:
	ldm	sp++, r3,r5,r6,pc

	.align	2
#ifdef __PIC__
.Lgot:
	.long	.Lgotcalc - _GLOBAL_OFFSET_TABLE_
#else
# ifdef __UCLIBC_HAS_THREADS__
.Lerrno_location:
	.long	__errno_location
# else
.Lerrno:
	.long	errno
# endif
#endif


	.size	syscall, . - syscall
