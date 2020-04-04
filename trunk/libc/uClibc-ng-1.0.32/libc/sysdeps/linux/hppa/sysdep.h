/* Assembler macros for PA-RISC.
   Copyright (C) 1999-2018 Free Software Foundation, Inc.
   Contributed by Ulrich Drepper, <drepper@cygnus.com>, August 1999.
   Linux/PA-RISC changes by Philipp Rumpf, <prumpf@tux.org>, March 2000.

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

#ifndef _LINUX_HPPA_SYSDEP_H
#define _LINUX_HPPA_SYSDEP_H 1

#include <common/sysdep.h>
#include <sys/syscall.h>

/* In order to get __set_errno() definition in INLINE_SYSCALL.  */
#ifndef __ASSEMBLER__
#include <errno.h>
#endif

#undef ASM_LINE_SEP
#define ASM_LINE_SEP !

#undef SYS_ify
#define SYS_ify(syscall_name)	(__NR_##syscall_name)

/* The vfork, fork, and clone syscalls clobber r19
 * and r21. We list r21 as either clobbered or as an
 * input to a 6-argument syscall. We must save and
 * restore r19 in both PIC and non-PIC cases.
 */
/* WARNING: TREG must be a callee saves register so
   that it doesn't have to be restored after a call
   to another function */
#define TREG 4
#define SAVE_PIC(SREG) \
	copy %r19, SREG
#define LOAD_PIC(LREG) \
	copy LREG , %r19
/* Inline assembly defines */
#define TREG_ASM "%r4" /* Cant clobber r3, it holds framemarker */
#define SAVE_ASM_PIC	"       copy %%r19, %" TREG_ASM "\n"
#define LOAD_ASM_PIC	"       copy %" TREG_ASM ", %%r19\n"
#define CLOB_TREG	TREG_ASM ,
#define PIC_REG_DEF	register unsigned long __r19 asm("r19");
#define PIC_REG_USE	, "r" (__r19)

#ifdef __ASSEMBLER__

/* Syntactic details of assembler.  */

#define ALIGNARG(log2) log2

/* For Linux we can use the system call table in the header file
	/usr/include/asm/unistd.h
   of the kernel.  But these symbols do not follow the SYS_* syntax
   so we have to redefine the `SYS_ify' macro here.  */
#undef SYS_ify
#define SYS_ify(syscall_name)	__NR_##syscall_name

/* ELF-like local names start with `.L'.  */
#undef L
#define L(name)	.L##name

/* Linux uses a negative return value to indicate syscall errors,
   unlike most Unices, which use the condition codes' carry flag.

   Since version 2.1 the return value of a system call might be
   negative even if the call succeeded.  E.g., the `lseek' system call
   might return a large offset.  Therefore we must not anymore test
   for < 0, but test for a real error by making sure the value in %eax
   is a real error number.  Linus said he will make sure the no syscall
   returns a value in -1 .. -4095 as a valid result so we can safely
   test with -4095.  */

/* We don't want the label for the error handle to be global when we define
   it here.  */
/*#ifdef PIC
# define SYSCALL_ERROR_LABEL 0f
#else
# define SYSCALL_ERROR_LABEL syscall_error
#endif*/

/* Argument manipulation from the stack for preparing to
   make a syscall */

#define DOARGS_0 /* nothing */
#define DOARGS_1 /* nothing */
#define DOARGS_2 /* nothing */
#define DOARGS_3 /* nothing */
#define DOARGS_4 /* nothing */
#define DOARGS_5 ldw -52(%sp), %r22		ASM_LINE_SEP
#define DOARGS_6 DOARGS_5 ldw -56(%sp), %r21	ASM_LINE_SEP

#define UNDOARGS_0 /* nothing */
#define UNDOARGS_1 /* nothing */
#define UNDOARGS_2 /* nothing */
#define UNDOARGS_3 /* nothing */
#define UNDOARGS_4 /* nothing */
#define UNDOARGS_5 /* nothing */
#define UNDOARGS_6 /* nothing */

/* Define an entry point visible from C.

   There is currently a bug in gdb which prevents us from specifying
   incomplete stabs information.  Fake some entries here which specify
   the current source file.  */
#undef ENTRY
#define	ENTRY(name)							\
	.text						ASM_LINE_SEP	\
	.align ALIGNARG(4)				ASM_LINE_SEP	\
	.export C_SYMBOL_NAME(name)			ASM_LINE_SEP	\
	.type	C_SYMBOL_NAME(name),@function		ASM_LINE_SEP	\
	cfi_startproc					ASM_LINE_SEP	\
	C_LABEL(name)					ASM_LINE_SEP	\
	.PROC						ASM_LINE_SEP	\
	.CALLINFO FRAME=64,CALLS,SAVE_RP,ENTRY_GR=3	ASM_LINE_SEP	\
	.ENTRY						ASM_LINE_SEP	\
	/* SAVE_RP says we do */			ASM_LINE_SEP	\
	stw %rp, -20(%sr0,%sp)				ASM_LINE_SEP	\
	.cfi_offset 2, -20				ASM_LINE_SEP	\
	/*FIXME: Call mcount? (carefull with stack!) */

/* Some syscall wrappers do not call other functions, and
   hence are classified as leaf, so add NO_CALLS for gdb */
#define	ENTRY_LEAF(name)						\
	.text						ASM_LINE_SEP	\
	.align ALIGNARG(4)				ASM_LINE_SEP	\
	.export C_SYMBOL_NAME(name)			ASM_LINE_SEP	\
	.type	C_SYMBOL_NAME(name),@function		ASM_LINE_SEP	\
	cfi_startproc					ASM_LINE_SEP	\
	C_LABEL(name)					ASM_LINE_SEP	\
	.PROC						ASM_LINE_SEP	\
	.CALLINFO FRAME=64,NO_CALLS,SAVE_RP,ENTRY_GR=3	ASM_LINE_SEP	\
	.ENTRY						ASM_LINE_SEP	\
	/* SAVE_RP says we do */			ASM_LINE_SEP	\
	stw %rp, -20(%sr0,%sp)				ASM_LINE_SEP	\
	.cfi_offset 2, -20				ASM_LINE_SEP	\
	/*FIXME: Call mcount? (carefull with stack!) */

#undef	END
#define END(name)							\
	.EXIT						ASM_LINE_SEP	\
	.PROCEND					ASM_LINE_SEP	\
	cfi_endproc					ASM_LINE_SEP	\
.size	C_SYMBOL_NAME(name), .-C_SYMBOL_NAME(name)	ASM_LINE_SEP

/* If compiled for profiling, call `mcount' at the start
   of each function. No, don't bother.  gcc will put the
   call in for us.  */
#define CALL_MCOUNT		/* Do nothing.  */

/* syscall wrappers consist of
	#include <sysdep.h>
	PSEUDO(...)
	ret
	PSEUDO_END(...)

   which means
	ENTRY(name)
	DO_CALL(...)
	bv,n 0(2)
*/

#undef PSEUDO
#define	PSEUDO(name, syscall_name, args)			\
  ENTRY (name)					ASM_LINE_SEP	\
  /* If necc. load args from stack */		ASM_LINE_SEP	\
  DOARGS_##args					ASM_LINE_SEP	\
  DO_CALL (syscall_name, args)			ASM_LINE_SEP	\
  UNDOARGS_##args				ASM_LINE_SEP

#define ret \
  /* Return value set by ERRNO code */		ASM_LINE_SEP	\
  bv,n 0(2)					ASM_LINE_SEP

#undef	PSEUDO_END
#define	PSEUDO_END(name)					\
  END (name)

/* We don't set the errno on the return from the syscall */
#define	PSEUDO_NOERRNO(name, syscall_name, args)		\
  ENTRY_LEAF (name)				ASM_LINE_SEP	\
  DOARGS_##args					ASM_LINE_SEP	\
  DO_CALL_NOERRNO (syscall_name, args)		ASM_LINE_SEP	\
  UNDOARGS_##args				ASM_LINE_SEP

#define ret_NOERRNO ret

#undef	PSEUDO_END_NOERRNO
#define	PSEUDO_END_NOERRNO(name)				\
  END (name)

/* This has to return the error value */
#undef  PSEUDO_ERRVAL
#define PSEUDO_ERRVAL(name, syscall_name, args)			\
  ENTRY_LEAF (name)				ASM_LINE_SEP	\
  DOARGS_##args					ASM_LINE_SEP	\
  DO_CALL_ERRVAL (syscall_name, args)		ASM_LINE_SEP	\
  UNDOARGS_##args				ASM_LINE_SEP

#define ret_ERRVAL ret

#undef	PSEUDO_END_ERRVAL
#define PSEUDO_END_ERRVAL(name)					\
	END(name)

#undef JUMPTARGET
#define JUMPTARGET(name)	name
#define SYSCALL_PIC_SETUP	/* Nothing.  */


/* FIXME: This comment is not true.
 * All the syscall assembly macros rely on finding the appropriate
   SYSCALL_ERROR_LABEL or rather HANDLER. */

/* int * __errno_location(void) so you have to store your value
   into the return address! */
#define DEFAULT_SYSCALL_ERROR_HANDLER			\
	.import __errno_location,code	ASM_LINE_SEP	\
	/* branch to errno handler */	ASM_LINE_SEP	\
	bl __errno_location,%rp		ASM_LINE_SEP

/* Here are the myriad of configuration options that the above can
   work for... what we've done is provide the framework for future
   changes if required to each section */

#ifdef __PIC__
#  if defined _LIBC_REENTRANT
#   define SYSCALL_ERROR_HANDLER DEFAULT_SYSCALL_ERROR_HANDLER
#  else /* !_LIBC_REENTRANT */
#   define SYSCALL_ERROR_HANDLER DEFAULT_SYSCALL_ERROR_HANDLER
#  endif /* _LIBC_REENTRANT */
#else
# ifndef _LIBC_REENTRANT
#  define SYSCALL_ERROR_HANDLER DEFAULT_SYSCALL_ERROR_HANDLER
# else
#  define SYSCALL_ERROR_HANDLER DEFAULT_SYSCALL_ERROR_HANDLER
# endif
#endif


/* Linux takes system call arguments in registers:
	syscall number	gr20
	arg 1		gr26
	arg 2		gr25
	arg 3		gr24
	arg 4		gr23
	arg 5		gr22
	arg 6		gr21

   The compiler calls us by the C convention:
	syscall number	in the DO_CALL macro
	arg 1		gr26
	arg 2		gr25
	arg 3		gr24
	arg 4		gr23
	arg 5		-52(sp)
	arg 6		-56(sp)

   gr22 and gr21 are caller-saves, so we can just load the arguments
   there and generally be happy. */

/* the cmpb...no_error code below inside DO_CALL
 * is intended to mimic the if (__sys_res...)
 * code inside INLINE_SYSCALL
 */
#define NO_ERROR -0x1000

#undef	DO_CALL
#define DO_CALL(syscall_name, args)				\
	/* Create a frame */			ASM_LINE_SEP	\
	stwm TREG, 64(%sp)			ASM_LINE_SEP	\
	.cfi_def_cfa_offset -64			ASM_LINE_SEP	\
	.cfi_offset TREG, 0			ASM_LINE_SEP	\
	stw %sp, -4(%sp)			ASM_LINE_SEP	\
	stw %r19, -32(%sp)			ASM_LINE_SEP	\
	.cfi_offset 19, 32			ASM_LINE_SEP	\
	/* Save r19 */				ASM_LINE_SEP	\
	SAVE_PIC(TREG)				ASM_LINE_SEP	\
	/* Do syscall, delay loads # */		ASM_LINE_SEP	\
	ble  0x100(%sr2,%r0)			ASM_LINE_SEP	\
	ldi SYS_ify (syscall_name), %r20	ASM_LINE_SEP	\
	ldi NO_ERROR,%r1			ASM_LINE_SEP	\
	cmpb,>>=,n %r1,%ret0,L(pre_end)		ASM_LINE_SEP	\
	/* Restore r19 from TREG */		ASM_LINE_SEP	\
	LOAD_PIC(TREG) /* delay */		ASM_LINE_SEP	\
	SYSCALL_ERROR_HANDLER			ASM_LINE_SEP	\
	/* Use TREG for temp storage */		ASM_LINE_SEP	\
	copy %ret0, TREG /* delay */		ASM_LINE_SEP	\
	/* OPTIMIZE: Don't reload r19 */	ASM_LINE_SEP	\
	/* do a -1*syscall_ret0 */		ASM_LINE_SEP	\
	sub %r0, TREG, TREG			ASM_LINE_SEP	\
	/* Store into errno location */		ASM_LINE_SEP	\
	stw TREG, 0(%sr0,%ret0)			ASM_LINE_SEP	\
	/* return -1 as error */		ASM_LINE_SEP	\
	ldo -1(%r0), %ret0			ASM_LINE_SEP	\
L(pre_end):					ASM_LINE_SEP	\
	/* Restore our frame, restoring TREG */	ASM_LINE_SEP	\
	ldwm -64(%sp), TREG			ASM_LINE_SEP	\
	/* Restore return pointer */		ASM_LINE_SEP	\
	ldw -20(%sp),%rp			ASM_LINE_SEP

/* We do nothing with the return, except hand it back to someone else */
#undef  DO_CALL_NOERRNO
#define DO_CALL_NOERRNO(syscall_name, args)			\
	/* No need to store r19 */		ASM_LINE_SEP	\
	ble  0x100(%sr2,%r0)                    ASM_LINE_SEP    \
	ldi SYS_ify (syscall_name), %r20        ASM_LINE_SEP    \
	/* Caller will restore r19 */		ASM_LINE_SEP

/* Here, we return the ERRVAL in assembly, note we don't call the
   error handler function, but we do 'negate' the return _IF_
   it's an error. Not sure if this is the right semantic. */

#undef	DO_CALL_ERRVAL
#define DO_CALL_ERRVAL(syscall_name, args)			\
	/* No need to store r19 */		ASM_LINE_SEP	\
	ble  0x100(%sr2,%r0)			ASM_LINE_SEP	\
	ldi SYS_ify (syscall_name), %r20	ASM_LINE_SEP	\
	/* Caller will restore r19 */		ASM_LINE_SEP	\
	ldi NO_ERROR,%r1			ASM_LINE_SEP	\
	cmpb,>>=,n %r1,%ret0,0f			ASM_LINE_SEP	\
	sub %r0, %ret0, %ret0			ASM_LINE_SEP	\
0:						ASM_LINE_SEP


#else

/* GCC has to be warned that a syscall may clobber all the ABI
   registers listed as "caller-saves", see page 8, Table 2
   in section 2.2.6 of the PA-RISC RUN-TIME architecture
   document. However! r28 is the result and will conflict with
   the clobber list so it is left out. Also the input arguments
   registers r20 -> r26 will conflict with the list so they
   are treated specially. Although r19 is clobbered by the syscall
   we cannot say this because it would violate ABI, thus we say
   TREG is clobbered and use that register to save/restore r19
   across the syscall. */

#define CALL_CLOB_REGS	"%r1", "%r2", CLOB_TREG \
			"%r20", "%r29", "%r31"

#endif	/* __ASSEMBLER__ */

#endif /* _LINUX_HPPA_SYSDEP_H */
