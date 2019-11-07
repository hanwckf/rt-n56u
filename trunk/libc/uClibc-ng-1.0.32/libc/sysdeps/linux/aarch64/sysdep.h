/* Copyright (C) 2005-2016 Free Software Foundation, Inc.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation; either version 2.1 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with the GNU C Library; if not, see
<http://www.gnu.org/licenses/>.  */

#ifndef _LINUX_AARCH64_SYSDEP_H
#define _LINUX_AARCH64_SYSDEP_H 1

#include <common/sysdep.h>
#include <sys/syscall.h>

/* In order to get __set_errno() definition in INLINE_SYSCALL.  */
#ifndef __ASSEMBLER__
#include <errno.h>
#endif

/* For Linux we can use the system call table in the header file
/usr/include/asm/unistd.h
of the kernel.  But these symbols do not follow the SYS_* syntax
so we have to redefine the `SYS_ify' macro here.  */
#undef SYS_ify
#define SYS_ify(syscall_name)	(__NR_##syscall_name)

#ifdef __ASSEMBLER__

#define ASM_SIZE_DIRECTIVE(name) .size name,.-name

/* Local label name for asm code.  */
#ifndef L
# define L(name)         .L##name
#endif

/* Define an entry point visible from C.  */
#define ENTRY(name)						\
  .globl C_SYMBOL_NAME(name);					\
  .type C_SYMBOL_NAME(name),%function;				\
  .align 4;							\
  C_LABEL(name)							\
  cfi_startproc;

/* Define an entry point visible from C.  */
#define ENTRY_ALIGN(name, align)				\
  .globl C_SYMBOL_NAME(name);					\
  .type C_SYMBOL_NAME(name),%function;				\
  .p2align align;						\
  C_LABEL(name)							\
  cfi_startproc;

#undef	END
#define END(name)						\
  cfi_endproc;							\
  ASM_SIZE_DIRECTIVE(name)

/* Linux uses a negative return value to indicate syscall errors,
unlike most Unices, which use the condition codes' carry flag.

Since version 2.1 the return value of a system call might be
negative even if the call succeeded.  E.g., the `lseek' system call
might return a large offset.  Therefore we must not anymore test
for < 0, but test for a real error by making sure the value in R0
is a real error number.  Linus said he will make sure the no syscall
returns a value in -1 .. -4095 as a valid result so we can safely
test with -4095.  */

# undef	PSEUDO
# define PSEUDO(name, syscall_name, args)			\
.text;								\
ENTRY (name);							\
DO_CALL (syscall_name, args);					\
cmn x0, #4095;							\
b.cs .Lsyscall_error;

# undef	PSEUDO_END
# define PSEUDO_END(name)					\
SYSCALL_ERROR_HANDLER						\
END (name)

# undef	PSEUDO_NOERRNO
# define PSEUDO_NOERRNO(name, syscall_name, args)		\
.text;								\
ENTRY (name);							\
DO_CALL (syscall_name, args);

# undef	PSEUDO_END_NOERRNO
# define PSEUDO_END_NOERRNO(name)				\
END (name)

# define ret_NOERRNO ret

/* The function has to return the error code.  */
# undef	PSEUDO_ERRVAL
# define PSEUDO_ERRVAL(name, syscall_name, args)		\
.text;								\
ENTRY (name)							\
DO_CALL (syscall_name, args);					\
neg x0, x0

# undef	PSEUDO_END_ERRVAL
# define PSEUDO_END_ERRVAL(name) \
END (name)

# define ret_ERRVAL ret

#if defined NOT_IN_libc
# define SYSCALL_ERROR  .Lsyscall_error
# define SYSCALL_ERROR_HANDLER					\
.Lsyscall_error:						\
	adrp	x1, :gottprel:errno;				\
	neg	w2, w0;						\
	ldr	x1, [x1, :gottprel_lo12:errno];			\
	mrs	x3, tpidr_el0;					\
	mov	x0, -1;						\
	str	w2, [x1, x3];					\
	ret;
#else
# define SYSCALL_ERROR __syscall_error
# define SYSCALL_ERROR_HANDLER                                  \
.Lsyscall_error:                                                \
	b	__syscall_error;
#endif

# undef	DO_CALL
# define DO_CALL(syscall_name, args)				\
mov x8, SYS_ify (syscall_name);					\
svc 0

#endif	/* __ASSEMBLER__ */

#endif /* linux/aarch64/sysdep.h */
