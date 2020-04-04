/* Assembler macros for Xtensa processors.
   Copyright (C) 2001, 2007 Free Software Foundation, Inc.
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

#ifndef _LINUX_XTENSA_SYSDEP_H
#define _LINUX_XTENSA_SYSDEP_H 1

#include <common/sysdep.h>
#include <sys/syscall.h>

#ifdef __ASSEMBLER__

#define ALIGNARG(log2) 1 << log2
#define ASM_TYPE_DIRECTIVE(name, typearg) .type name, typearg
#define ASM_SIZE_DIRECTIVE(name) .size name, . - name

#if defined(__XTENSA_WINDOWED_ABI__)
#define abi_entry(reg, frame_size) entry reg, frame_size
#define abi_ret retw
#elif defined(__XTENSA_CALL0_ABI__)
#define abi_entry(reg, frame_size)
#define abi_ret ret
#else
#error Unsupported Xtensa ABI
#endif


#define	ENTRY(name)							\
  .globl C_SYMBOL_NAME(name);				\
  ASM_TYPE_DIRECTIVE (C_SYMBOL_NAME(name), @function);			\
  .align ALIGNARG(2);							\
  LITERAL_POSITION;							\
  C_LABEL(name)								\
  abi_entry(sp, FRAMESIZE);

#define	HIDDEN_ENTRY(name)						\
  .globl C_SYMBOL_NAME(name);				\
  .hidden C_SYMBOL_NAME(name);						\
  ASM_TYPE_DIRECTIVE (C_SYMBOL_NAME(name), @function);			\
  .align ALIGNARG(2);							\
  LITERAL_POSITION;							\
  C_LABEL(name)								\
  abi_entry(sp, FRAMESIZE);

#undef END
#define END(name) ASM_SIZE_DIRECTIVE(name)

/* Local label name for asm code. */
#ifndef L
# ifdef HAVE_ELF
#  define L(name)       .L##name
# else
#  define L(name)       name
# endif
#endif

/* Define a macro for this directive so it can be removed in a few places.  */
#define LITERAL_POSITION .literal_position

#undef JUMPTARGET
#ifdef __PIC__
/* The "@PLT" suffix is currently a no-op for non-shared linking, but
   it doesn't hurt to use it conditionally for PIC code in case that
   changes someday.  */
#define JUMPTARGET(name) name##@PLT
#else
#define JUMPTARGET(name) name
#endif

#ifndef FRAMESIZE
#if defined(__XTENSA_WINDOWED_ABI__)
#define FRAMESIZE 16
#elif defined(__XTENSA_CALL0_ABI__)
#define FRAMESIZE 0
#else
#error Unsupported Xtensa ABI
#endif
#endif

/* Linux uses a negative return value to indicate syscall errors,
   unlike most Unices, which use the condition codes' carry flag.

   Since version 2.1 the return value of a system call might be
   negative even if the call succeeded.  E.g., the `lseek' system call
   might return a large offset.  Therefore we must not anymore test
   for < 0, but test for a real error by making sure the value in a2
   is a real error number.  Linus said he will make sure the no syscall
   returns a value in -1 .. -4095 as a valid result so we can safely
   test with -4095.  */

/* We don't want the label for the error handler to be global when we define
   it here.  */
#define SYSCALL_ERROR_LABEL 0f

#undef  PSEUDO
#define	PSEUDO(name, syscall_name, args)				      \
  .text;								      \
  ENTRY (name)								      \
	DO_CALL	(syscall_name, args);					      \
	movi	a4, -4095;						      \
	bgeu	a2, a4, SYSCALL_ERROR_LABEL;				      \
  .Lpseudo_end:

#undef	PSEUDO_END
#define	PSEUDO_END(name)						      \
  SYSCALL_ERROR_HANDLER							      \
  END (name)

#undef	PSEUDO_NOERRNO
#define	PSEUDO_NOERRNO(name, syscall_name, args)			      \
  .text;								      \
  ENTRY (name)								      \
	DO_CALL	(syscall_name, args)

#undef	PSEUDO_END_NOERRNO
#define	PSEUDO_END_NOERRNO(name)					      \
  END (name)

#undef	ret_NOERRNO
#define ret_NOERRNO abi_ret

/* The function has to return the error code.  */
#undef	PSEUDO_ERRVAL
#define	PSEUDO_ERRVAL(name, syscall_name, args)				      \
  .text;								      \
  ENTRY (name)								      \
	DO_CALL	(syscall_name, args);					      \
	neg	a2, a2

#undef	PSEUDO_END_ERRVAL
#define	PSEUDO_END_ERRVAL(name)						      \
  END (name)

#undef	ret_ERRVAL
#define ret_ERRVAL abi_ret

#if defined _LIBC_REENTRANT
# if defined USE___THREAD
#   define SYSCALL_ERROR_ERRNO errno
#  define SYSCALL_ERROR_HANDLER						      \
0:	rur	a4, THREADPTR;						      \
	movi	a3, SYSCALL_ERROR_ERRNO@TPOFF;				      \
	neg	a2, a2;							      \
	add	a4, a4, a3;						      \
	s32i	a2, a4, 0;						      \
	movi	a2, -1;							      \
	j	.Lpseudo_end;
# else /* !USE___THREAD */

#if defined(__XTENSA_WINDOWED_ABI__)
#  define SYSCALL_ERROR_HANDLER						      \
0:	neg	a2, a2;							      \
	mov	a6, a2;							      \
	movi	a4, __errno_location@PLT;				      \
	callx4	a4;						              \
	s32i	a2, a6, 0;						      \
	movi	a2, -1;							      \
	j	.Lpseudo_end;
#elif defined(__XTENSA_CALL0_ABI__)
#  define SYSCALL_ERROR_HANDLER						      \
0:	neg	a2, a2;							      \
	addi	a1, a1, -16;						      \
	s32i	a0, a1, 0;						      \
	s32i	a2, a1, 4;						      \
	movi	a0, __errno_location@PLT;				      \
	callx0	a0;						              \
	l32i	a0, a1, 0;						      \
	l32i	a3, a1, 4;						      \
	addi	a1, a1, 16;						      \
	s32i	a3, a2, 0;						      \
	movi	a2, -1;							      \
	j	.Lpseudo_end;
#else
#error Unsupported Xtensa ABI
#endif

# endif /* !USE___THREAD */
#else /* !_LIBC_REENTRANT */
#define SYSCALL_ERROR_HANDLER						      \
0:	movi	a4, errno;						      \
	neg	a2, a2;							      \
	s32i	a2, a4, 0;						      \
	movi	a2, -1;							      \
	j	.Lpseudo_end;
#endif /* _LIBC_REENTRANT */

#endif	/* __ASSEMBLER__ */
#endif	/* _LINUX_XTENSA_SYSDEP_H */
