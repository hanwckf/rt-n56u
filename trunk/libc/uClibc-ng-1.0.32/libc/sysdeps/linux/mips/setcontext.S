/* Set current context.
   Copyright (C) 2009 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Maciej W. Rozycki <macro@codesourcery.com>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <sys/asm.h>
#include <sys/fpregdef.h>
#include <sys/regdef.h>

#include "ucontext_i.h"

/* int setcontext (const ucontext_t *ucp) */

	.text
LOCALSZ = 0
ARGSZ = 0
MASK = 0x00000000
#ifdef __PIC__
LOCALSZ = 1						/* save gp */
#endif
#if _MIPS_SIM != _ABIO32
ARGSZ = 1						/* save a0 */
# ifdef __PIC__
MASK = 0x10000000
# endif
#endif
FRAMESZ = (((ARGSZ + LOCALSZ) * SZREG) + ALSZ) & ALMASK
GPOFF = FRAMESZ - ((ARGSZ + 1) * SZREG)
#if _MIPS_SIM != _ABIO32
A0OFF = FRAMESZ - (1 * SZREG)				/* callee-allocated */
#else
A0OFF = FRAMESZ + (0 * SZREG)				/* caller-allocated */
#endif

NESTED (__setcontext, FRAMESZ, ra)
	.mask	MASK, -(ARGSZ * SZREG)
	.fmask	0x00000000, 0

#ifdef __PIC__
	SETUP_GP
#endif

	PTR_ADDIU sp, -FRAMESZ

#ifdef __PIC__
	SETUP_GP64 (GPOFF, __setcontext)
	SAVE_GP (GPOFF)
#endif

	/* Check for the magic flag.  */
	li	v0, 1
	REG_L	v1, (0 * SZREG + MCONTEXT_GREGS)(a0)	/* zero */
	bne	v0, v1, 98f

	REG_S	a0, A0OFF(sp)

/* rt_sigprocmask (SIG_SETMASK, &ucp->uc_sigmask, NULL, _NSIG8) */
	li	a3, _NSIG8
	move	a2, zero
	PTR_ADDU a1, a0, UCONTEXT_SIGMASK
	li	a0, SIG_SETMASK

	li	v0, SYS_ify (rt_sigprocmask)
	syscall
	bnez	a3, 99f

	REG_L	v0, A0OFF(sp)

#ifdef __mips_hard_float
# if _MIPS_SIM == _ABI64
	l.d	fs0, (24 * SZREG + MCONTEXT_FPREGS)(v0)
	l.d	fs1, (25 * SZREG + MCONTEXT_FPREGS)(v0)
	l.d	fs2, (26 * SZREG + MCONTEXT_FPREGS)(v0)
	l.d	fs3, (27 * SZREG + MCONTEXT_FPREGS)(v0)
	l.d	fs4, (28 * SZREG + MCONTEXT_FPREGS)(v0)
	l.d	fs5, (29 * SZREG + MCONTEXT_FPREGS)(v0)
	l.d	fs6, (30 * SZREG + MCONTEXT_FPREGS)(v0)
	l.d	fs7, (31 * SZREG + MCONTEXT_FPREGS)(v0)

# else  /* _MIPS_SIM != _ABI64 */
	l.d	fs0, (20 * SZREG + MCONTEXT_FPREGS)(v0)
	l.d	fs1, (22 * SZREG + MCONTEXT_FPREGS)(v0)
	l.d	fs2, (24 * SZREG + MCONTEXT_FPREGS)(v0)
	l.d	fs3, (26 * SZREG + MCONTEXT_FPREGS)(v0)
	l.d	fs4, (28 * SZREG + MCONTEXT_FPREGS)(v0)
	l.d	fs5, (30 * SZREG + MCONTEXT_FPREGS)(v0)

# endif /* _MIPS_SIM != _ABI64 */

	lw	v1, MCONTEXT_FPC_CSR(v0)
	ctc1	v1, fcr31
#endif /* __mips_hard_float */

	/* Note the contents of argument registers will be random
	   unless makecontext() has been called.  */
	REG_L	a0, (4 * SZREG + MCONTEXT_GREGS)(v0)
	REG_L	a1, (5 * SZREG + MCONTEXT_GREGS)(v0)
	REG_L	a2, (6 * SZREG + MCONTEXT_GREGS)(v0)
	REG_L	a3, (7 * SZREG + MCONTEXT_GREGS)(v0)
#if _MIPS_SIM != _ABIO32
	REG_L	a4, (8 * SZREG + MCONTEXT_GREGS)(v0)
	REG_L	a5, (9 * SZREG + MCONTEXT_GREGS)(v0)
	REG_L	a6, (10 * SZREG + MCONTEXT_GREGS)(v0)
	REG_L	a7, (11 * SZREG + MCONTEXT_GREGS)(v0)
#endif

	REG_L	s0, (16 * SZREG + MCONTEXT_GREGS)(v0)
	REG_L	s1, (17 * SZREG + MCONTEXT_GREGS)(v0)
	REG_L	s2, (18 * SZREG + MCONTEXT_GREGS)(v0)
	REG_L	s3, (19 * SZREG + MCONTEXT_GREGS)(v0)
	REG_L	s4, (20 * SZREG + MCONTEXT_GREGS)(v0)
	REG_L	s5, (21 * SZREG + MCONTEXT_GREGS)(v0)
	REG_L	s6, (22 * SZREG + MCONTEXT_GREGS)(v0)
	REG_L	s7, (23 * SZREG + MCONTEXT_GREGS)(v0)
#if ! defined (__PIC__) || _MIPS_SIM != _ABIO32
	REG_L	gp, (28 * SZREG + MCONTEXT_GREGS)(v0)
#endif
	REG_L	sp, (29 * SZREG + MCONTEXT_GREGS)(v0)
	REG_L	fp, (30 * SZREG + MCONTEXT_GREGS)(v0)
	REG_L	ra, (31 * SZREG + MCONTEXT_GREGS)(v0)
	REG_L	t9, MCONTEXT_PC(v0)

	move	v0, zero
	jr	t9

98:
	/* This is a context obtained from a signal handler.
	   Perform a full restore by pushing the context
	   passed onto a simulated signal frame on the stack
	   and call the signal return syscall as if a signal
	   handler exited normally.  */
	PTR_ADDIU sp, -((RT_SIGFRAME_SIZE + ALSZ) & ALMASK)

	/* Only ucontext is referred to from rt_sigreturn,
	   copy it.  */
	PTR_ADDIU t1, sp, RT_SIGFRAME_UCONTEXT
	li	t3, ((UCONTEXT_SIZE + SZREG - 1) / SZREG) - 1
0:
	REG_L	t2, (a0)
	PTR_ADDIU a0, SZREG
	REG_S	t2, (t1)
	PTR_ADDIU t1, SZREG
	.set	noreorder
	bgtz	t3, 0b
	 addiu	t3, -1
	.set	reorder

/* rt_sigreturn () -- no arguments, sp points to struct rt_sigframe.  */
	li	v0, SYS_ify (rt_sigreturn)
	syscall

	/* Restore the stack and fall through to the error
	   path.  Successful rt_sigreturn never returns to
	   its calling place.  */
	PTR_ADDIU sp, ((RT_SIGFRAME_SIZE + ALSZ) & ALMASK)
99:
#ifdef __PIC__
	PTR_LA	t9, JUMPTARGET (__syscall_error)
	RESTORE_GP64
	PTR_ADDIU sp, FRAMESZ
	jr	t9

#else  /* ! __PIC__ */

	j	JUMPTARGET (__syscall_error)
#endif /* ! __PIC__ */
PSEUDO_END (__setcontext)

weak_alias (__setcontext, setcontext)
