/* Assembler macros for ARM.
   Copyright (C) 1997, 1998, 2003 Free Software Foundation, Inc.
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

#ifndef _LINUX_ARM_SYSDEP_H
#define _LINUX_ARM_SYSDEP_H 1

#include <common/sysdep.h>
#include <bits/arm_bx.h>
#include <sys/syscall.h>
/* For Linux we can use the system call table in the header file
	/usr/include/asm/unistd.h
   of the kernel.  But these symbols do not follow the SYS_* syntax
   so we have to redefine the `SYS_ify' macro here.  */
#undef SYS_ify
#define SWI_BASE  (0x900000)
#define SYS_ify(syscall_name)	(__NR_##syscall_name)

#ifdef	__ASSEMBLER__

/* Syntactic details of assembler.  */

#define ALIGNARG(log2) log2
/* For ELF we need the `.type' directive to make shared libs work right.  */
#define ASM_TYPE_DIRECTIVE(name,typearg) .type name,%##typearg;
#define ASM_SIZE_DIRECTIVE(name) .size name,.-name

/* In ELF C symbols are asm symbols.  */
#undef	NO_UNDERSCORES
#define NO_UNDERSCORES

#define PLTJMP(_x)	_x##(PLT)

/* APCS-32 doesn't preserve the condition codes across function call. */
#ifdef __APCS_32__
#define LOADREGS(cond, base, reglist...)\
	ldm##cond	base,reglist
#define RETINSTR(cond, reg) \
	BXC(cond, reg)
#define DO_RET(_reg)		\
	BX(_reg)
#else  /* APCS-26 */
#define LOADREGS(cond, base, reglist...)	\
	ldm##cond	base,reglist^
#define RETINSTR(cond, reg)	\
	mov##cond##s	pc, reg
#define DO_RET(_reg)		\
	movs pc, _reg
#endif

/* Define an entry point visible from C.  */
#define	ENTRY(name)						\
  .globl C_SYMBOL_NAME(name);			\
  ASM_TYPE_DIRECTIVE (C_SYMBOL_NAME(name),function)		\
  .align ALIGNARG(4);						\
  name##:

#undef	END
#define END(name)						\
  ASM_SIZE_DIRECTIVE(name)

#ifdef	NO_UNDERSCORES
/* Since C identifiers are not normally prefixed with an underscore
   on this system, the asm identifier `syscall_error' intrudes on the
   C name space.  Make sure we use an innocuous name.  */
#define	syscall_error	__syscall_error
#define mcount		_mcount
#endif
/* Linux uses a negative return value to indicate syscall errors,
   unlike most Unices, which use the condition codes' carry flag.

   Since version 2.1 the return value of a system call might be
   negative even if the call succeeded.  E.g., the `lseek' system call
   might return a large offset.  Therefore we must not anymore test
   for < 0, but test for a real error by making sure the value in R0
   is a real error number.  Linus said he will make sure the no syscall
   returns a value in -1 .. -4095 as a valid result so we can safely
   test with -4095.  */

#undef	PSEUDO
#define	PSEUDO(name, syscall_name, args)				\
  .text;								\
  ENTRY (name);								\
    DO_CALL (syscall_name, args);					\
    cmn r0, $4096;

#define PSEUDO_RET							\
    RETINSTR(cc, lr);							\
    b PLTJMP(SYSCALL_ERROR)
#undef ret
#define ret PSEUDO_RET

#undef	PSEUDO_END
#define	PSEUDO_END(name)						\
  SYSCALL_ERROR_HANDLER							\
  END (name)

#undef	PSEUDO_NOERRNO
#define	PSEUDO_NOERRNO(name, syscall_name, args)			\
  .text;								\
  ENTRY (name);								\
    DO_CALL (syscall_name, args);

#define PSEUDO_RET_NOERRNO						\
    DO_RET (lr);

#undef ret_NOERRNO
#define ret_NOERRNO PSEUDO_RET_NOERRNO

#undef	PSEUDO_END_NOERRNO
#define	PSEUDO_END_NOERRNO(name)					\
  END (name)

/* The function has to return the error code.  */
#undef	PSEUDO_ERRVAL
#define	PSEUDO_ERRVAL(name, syscall_name, args) \
  .text;								\
  ENTRY (name)								\
    DO_CALL (syscall_name, args);					\
    rsb r0, r0, #0

#undef	PSEUDO_END_ERRVAL
#define	PSEUDO_END_ERRVAL(name) \
  END (name)

#if defined NOT_IN_libc
# define SYSCALL_ERROR __local_syscall_error
#  define SYSCALL_ERROR_HANDLER					\
__local_syscall_error:						\
	str	lr, [sp, #-4]!;					\
	str	r0, [sp, #-4]!;					\
	bl	PLTJMP(C_SYMBOL_NAME(__errno_location)); 	\
	ldr	r1, [sp], #4;					\
	rsb	r1, r1, #0;					\
	str	r1, [r0];					\
	mvn	r0, #0;						\
	ldr	pc, [sp], #4;
#else
# define SYSCALL_ERROR_HANDLER	/* Nothing here; code in sysdep.S is used.  */
# define SYSCALL_ERROR __syscall_error
#endif

/* Linux takes system call args in registers:
	syscall number	in the SWI instruction
	arg 1		r0
	arg 2		r1
	arg 3		r2
	arg 4		r3
	arg 5		r4	(this is different from the APCS convention)
	arg 6		r5
	arg 7		r6

   The compiler is going to form a call by coming here, through PSEUDO, with
   arguments
	syscall number	in the DO_CALL macro
	arg 1		r0
	arg 2		r1
	arg 3		r2
	arg 4		r3
	arg 5		[sp]
	arg 6		[sp+4]
	arg 7		[sp+8]

   We need to shuffle values between R4..R6 and the stack so that the
   caller's v1..v3 and stack frame are not corrupted, and the kernel
   sees the right arguments.

*/
#if __ARM_ARCH > 6 || defined (__ARM_ARCH_6K__) || defined (__ARM_ARCH_6ZK__)
# define ARCH_HAS_HARD_TP
#endif

# ifdef __thumb2__
#  define NEGOFF_ADJ_BASE(R, OFF)	add R, R, $OFF
#  define NEGOFF_ADJ_BASE2(D, S, OFF)	add D, S, $OFF
#  define NEGOFF_OFF1(R, OFF)		[R]
#  define NEGOFF_OFF2(R, OFFA, OFFB)	[R, $((OFFA) - (OFFB))]
# else
#  define NEGOFF_ADJ_BASE(R, OFF)
#  define NEGOFF_ADJ_BASE2(D, S, OFF)	mov D, S
#  define NEGOFF_OFF1(R, OFF)		[R, $OFF]
#  define NEGOFF_OFF2(R, OFFA, OFFB)	[R, $OFFA]
# endif

# ifdef ARCH_HAS_HARD_TP
/* If the cpu has cp15 available, use it.  */
#  define GET_TLS(TMP)		mrc p15, 0, r0, c13, c0, 3
# else
/* At this generic level we have no tricks to pull.  Call the ABI routine.  */
#  define GET_TLS(TMP)					\
	push	{ r1, r2, r3, lr };			\
	cfi_remember_state;				\
	cfi_adjust_cfa_offset (16);			\
	cfi_rel_offset (r1, 0);				\
	cfi_rel_offset (r2, 4);				\
	cfi_rel_offset (r3, 8);				\
	cfi_rel_offset (lr, 12);			\
	bl	__aeabi_read_tp;			\
	pop	{ r1, r2, r3, lr };			\
	cfi_restore_state
# endif /* ARCH_HAS_HARD_TP */




#undef	DO_CALL
#if defined(__ARM_EABI__)
#define DO_CALL(syscall_name, args)		\
    DOARGS_##args				\
    mov ip, r7;					\
    ldr r7, =SYS_ify (syscall_name);		\
    swi 0x0;					\
    mov r7, ip;					\
    UNDOARGS_##args
#else
#define DO_CALL(syscall_name, args)		\
    DOARGS_##args				\
    swi SYS_ify (syscall_name); 		\
    UNDOARGS_##args
#endif

#define DOARGS_0 /* nothing */
#define DOARGS_1 /* nothing */
#define DOARGS_2 /* nothing */
#define DOARGS_3 /* nothing */
#define DOARGS_4 /* nothing */
#define DOARGS_5 str r4, [sp, $-4]!; ldr r4, [sp, $4];
#define DOARGS_6 mov ip, sp; stmfd sp!, {r4, r5}; ldmia ip, {r4, r5};
#define DOARGS_7 mov ip, sp; stmfd sp!, {r4, r5, r6}; ldmia ip, {r4, r5, r6};

#define UNDOARGS_0 /* nothing */
#define UNDOARGS_1 /* nothing */
#define UNDOARGS_2 /* nothing */
#define UNDOARGS_3 /* nothing */
#define UNDOARGS_4 /* nothing */
#define UNDOARGS_5 ldr r4, [sp], $4;
#define UNDOARGS_6 ldmfd sp!, {r4, r5};
#define UNDOARGS_7 ldmfd sp!, {r4, r5, r6};

#endif	/* __ASSEMBLER__ */
#endif /* linux/arm/sysdep.h */
