/* Copyright (C) 2011-2018 Free Software Foundation, Inc.

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

#include <common/sysdep.h>
#include <bits/wordsize.h>
#include <arch/abi.h>

#undef SYS_ify
#define SYS_ify(syscall_name)	__NR_##syscall_name

#if defined __ASSEMBLER__

/* Make use of .size directive.  */
#define ASM_SIZE_DIRECTIVE(name) .size name,.-name;

/* Define an entry point visible from C.  */
#define	ENTRY(name)							      \
  .globl C_SYMBOL_NAME(name);						      \
  .type C_SYMBOL_NAME(name),@function;					      \
  .align 8;								      \
  C_LABEL(name)								      \
  cfi_startproc;							      \

#undef	END
#define END(name)							      \
  cfi_endproc;								      \
  ASM_SIZE_DIRECTIVE(name)

/* Since C identifiers are not normally prefixed with an underscore
   on this system, the asm identifier `syscall_error' intrudes on the
   C name space.  Make sure we use an innocuous name.  */
#define	syscall_error	__syscall_error

/* Local label name for asm code. */
#define L(name)		.L##name

/* Specify the size in bytes of a machine register.  */
#define REGSIZE		8

/* Provide "pointer-oriented" instruction variants.  These differ not
   just for tilepro vs tilegx, but also for tilegx -m64 vs -m32.  */
#if __WORDSIZE == 32
#define ADD_PTR		addx
#define ADDI_PTR	addxi
#define ADDLI_PTR	addxli
#define LD_PTR		ld4s
#define ST_PTR		st4
#define SHL_PTR_ADD	shl2add
#else
#define ADD_PTR		add
#define ADDI_PTR	addi
#define ADDLI_PTR	addli
#define LD_PTR		LD
#define ST_PTR		ST
#define SHL_PTR_ADD	shl3add
#endif

/* The actual implementation of doing a syscall. */
#define DO_CALL(syscall_name, args)                     \
  moveli TREG_SYSCALL_NR_NAME, SYS_ify(syscall_name);	\
  swint1

/* TILE Linux returns the result in r0 (or a negative errno).
   The kernel "owns" the code to decide if a given value is an error,
   and puts errno in r1 if so, or otherwise zero.  */
#define	PSEUDO(name, syscall_name, args)		\
  ENTRY	(name);						\
  DO_CALL(syscall_name, args);				\
  BNEZ r1, 0f

#define ret  jrp lr

#ifndef SHARED
/* For static code, on error jump to __syscall_error directly. */
# define SYSCALL_ERROR_NAME __syscall_error
#elif IS_IN_libc || IS_IN_libpthread
/* Use the internal name for libc/libpthread shared objects. */
# define SYSCALL_ERROR_NAME __GI___syscall_error
#else
/* Otherwise, on error do a full PLT jump. */
# define SYSCALL_ERROR_NAME plt(__syscall_error)
#endif

#undef PSEUDO_END
#define	PSEUDO_END(name)				\
0:							\
  j SYSCALL_ERROR_NAME;					\
  END (name)

#undef PSEUDO_NOERRNO
#define	PSEUDO_NOERRNO(name, syscall_name, args)	\
  ENTRY	(name);						\
  DO_CALL(syscall_name, args)

#define ret_NOERRNO  jrp lr

#undef PSEUDO_END_NOERRNO
#define	PSEUDO_END_NOERRNO(name) \
  END (name)

/* Convenience wrappers. */
#define SYSCALL__(name, args)   PSEUDO (__##name, name, args)
#define SYSCALL(name, args)   PSEUDO (name, name, args)

#endif /* __ASSEMBLER__ */
