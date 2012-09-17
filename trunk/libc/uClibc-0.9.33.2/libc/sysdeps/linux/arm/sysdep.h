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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _LINUX_ARM_SYSDEP_H
#define _LINUX_ARM_SYSDEP_H 1

#include <common/sysdep.h>
#include <bits/arm_asm.h>

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
#ifdef __USE_BX__
#define RETINSTR(cond, reg)	\
	bx##cond	reg
#define DO_RET(_reg)		\
	bx _reg
#else
#define RETINSTR(cond, reg)	\
	mov##cond	pc, reg
#define DO_RET(_reg)		\
	mov pc, _reg
#endif
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
  ASM_GLOBAL_DIRECTIVE C_SYMBOL_NAME(name);			\
  ASM_TYPE_DIRECTIVE (C_SYMBOL_NAME(name),function)		\
  .align ALIGNARG(4);						\
  name##:							\
  CALL_MCOUNT

#undef	END
#define END(name)						\
  ASM_SIZE_DIRECTIVE(name)

/* If compiled for profiling, call `mcount' at the start of each function.  */
#ifdef	PROF
#define CALL_MCOUNT			\
	str	lr,[sp, #-4]!	;	\
	bl	PLTJMP(mcount)	;	\
	ldr	lr, [sp], #4	;
#else
#define CALL_MCOUNT		/* Do nothing.  */
#endif

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

#define ret_ERRVAL PSEUDO_RET_NOERRNO

#if defined NOT_IN_libc
# define SYSCALL_ERROR __local_syscall_error
# ifdef RTLD_PRIVATE_ERRNO
#  define SYSCALL_ERROR_HANDLER					\
__local_syscall_error:						\
       ldr     r1, 1f;						\
       rsb     r0, r0, #0;					\
0:     str     r0, [pc, r1];					\
       mvn     r0, #0;						\
       DO_RET(lr);						\
1:     .word C_SYMBOL_NAME(rtld_errno) - 0b - 8;
# else
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
# endif
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

#else /* not __ASSEMBLER__ */
/* Define a macro which expands into the inline wrapper code for a system
   call.  */
#undef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...)					\
  ({ unsigned int _inline_sys_result = INTERNAL_SYSCALL (name, , nr, args);	\
     if (__builtin_expect (INTERNAL_SYSCALL_ERROR_P (_inline_sys_result, ), 0))	\
       {									\
	 __set_errno (INTERNAL_SYSCALL_ERRNO (_inline_sys_result, ));		\
	 _inline_sys_result = (unsigned int) -1;				\
       }									\
     (int) _inline_sys_result; })

#undef INTERNAL_SYSCALL_DECL
#define INTERNAL_SYSCALL_DECL(err) do { } while (0)

#undef INTERNAL_SYSCALL_RAW
#if defined(__thumb__)
/* Hide the use of r7 from the compiler, this would be a lot
 * easier but for the fact that the syscalls can exceed 255.
 * For the moment the LOAD_ARG_7 is sacrificed.
 * We can't use push/pop inside the asm because that breaks
 * unwinding (ie. thread cancellation).
 */
#define INTERNAL_SYSCALL_RAW(name, err, nr, args...)		\
  ({ unsigned int _internal_sys_result;				\
    {								\
      int _sys_buf[2];						\
      register int __a1 __asm__ ("a1");				\
      register int *_v3 __asm__ ("v3") = _sys_buf;		\
      LOAD_ARGS_##nr (args)					\
      *_v3 = (int) (name);					\
      __asm__ __volatile__ ("str	r7, [v3, #4]\n"		\
                    "\tldr      r7, [v3]\n"			\
                    "\tswi      0       @ syscall " #name "\n"	\
                    "\tldr      r7, [v3, #4]"			\
                   : "=r" (__a1)				\
                    : "r" (_v3) ASM_ARGS_##nr			\
                    : "memory");				\
      _internal_sys_result = __a1;				\
    }								\
    (int) _internal_sys_result; })
#elif defined(__ARM_EABI__)
#define INTERNAL_SYSCALL_RAW(name, err, nr, args...)		\
  ({unsigned int _internal_sys_result;				\
     {								\
       register int __a1 __asm__ ("r0"), _nr __asm__ ("r7");	\
       LOAD_ARGS_##nr (args)					\
       _nr = name;						\
       __asm__ __volatile__ ("swi	0x0 @ syscall " #name	\
		     : "=r" (__a1)				\
		     : "r" (_nr) ASM_ARGS_##nr			\
		     : "memory");				\
       _internal_sys_result = __a1;				\
     }								\
     (int) _internal_sys_result; })
#else /* !defined(__ARM_EABI__) */
#define INTERNAL_SYSCALL_RAW(name, err, nr, args...)		\
  ({ unsigned int _internal_sys_result;				\
     {								\
       register int __a1 __asm__ ("a1");			\
       LOAD_ARGS_##nr (args)					\
       __asm__ __volatile__ ("swi	%1 @ syscall " #name	\
		     : "=r" (__a1)				\
		     : "i" (name) ASM_ARGS_##nr			\
		     : "memory");				\
       _internal_sys_result = __a1;				\
     }								\
     (int) _internal_sys_result; })
#endif

#undef INTERNAL_SYSCALL
#define INTERNAL_SYSCALL(name, err, nr, args...)		\
	INTERNAL_SYSCALL_RAW(SYS_ify(name), err, nr, args)

#undef INTERNAL_SYSCALL_ARM
#define INTERNAL_SYSCALL_ARM(name, err, nr, args...)		\
	INTERNAL_SYSCALL_RAW(__ARM_NR_##name, err, nr, args)

#undef INTERNAL_SYSCALL_ERROR_P
#define INTERNAL_SYSCALL_ERROR_P(val, err) \
  ((unsigned int) (val) >= 0xfffff001u)

#undef INTERNAL_SYSCALL_ERRNO
#define INTERNAL_SYSCALL_ERRNO(val, err)	(-(val))

#if defined(__ARM_EABI__)
#undef INTERNAL_SYSCALL_NCS
#define INTERNAL_SYSCALL_NCS(number, err, nr, args...)		\
	INTERNAL_SYSCALL_RAW(number, err, nr, args)
#else
/* We can't implement non-constant syscalls directly since the syscall
   number is normally encoded in the instruction.  So use SYS_syscall.  */
#undef INTERNAL_SYSCALL_NCS
#define INTERNAL_SYSCALL_NCS(number, err, nr, args...)		\
	INTERNAL_SYSCALL_NCS_##nr (number, err, args)

#define INTERNAL_SYSCALL_NCS_0(number, err, args...)		\
	INTERNAL_SYSCALL (syscall, err, 1, number, args)
#define INTERNAL_SYSCALL_NCS_1(number, err, args...)		\
	INTERNAL_SYSCALL (syscall, err, 2, number, args)
#define INTERNAL_SYSCALL_NCS_2(number, err, args...)		\
	INTERNAL_SYSCALL (syscall, err, 3, number, args)
#define INTERNAL_SYSCALL_NCS_3(number, err, args...)		\
	INTERNAL_SYSCALL (syscall, err, 4, number, args)
#define INTERNAL_SYSCALL_NCS_4(number, err, args...)		\
	INTERNAL_SYSCALL (syscall, err, 5, number, args)
#define INTERNAL_SYSCALL_NCS_5(number, err, args...)		\
	INTERNAL_SYSCALL (syscall, err, 6, number, args)
#endif

#endif	/* __ASSEMBLER__ */

/* Pointer mangling is not yet supported for ARM.  */
#define PTR_MANGLE(var) (void) (var)
#define PTR_DEMANGLE(var) (void) (var)

#endif /* linux/arm/sysdep.h */
