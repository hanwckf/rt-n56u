/* Copyright (C) 2011-2014 Free Software Foundation, Inc.

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

#ifndef _LINUX_OR1K_SYSDEP_H
#define _LINUX_OR1K_SYSDEP_H 1

#include <common/sysdep.h>
#include <sys/syscall.h>

/* In order to get __set_errno() definition in INLINE_SYSCALL.  */
#ifndef __ASSEMBLER__
#include <errno.h>
#endif

#undef SYS_ify
#define SYS_ify(syscall_name)	(__NR_##syscall_name)

#ifdef __ASSEMBLER__

/* Local label name for asm code. */
#define L(name)         .L##name

#undef ret_ERRVAL
#define ret_ERRVAL   l.jr r9; l.nop
#define ret_NOERRNO  l.jr r9; l.nop

#undef DO_CALL
#define DO_CALL(syscall_name) \
    l.addi r11, r0, SYS_ify (syscall_name); \
    l.sys 1; \
    l.nop

/* Make use of .size directive.  */
#define ASM_SIZE_DIRECTIVE(name) .size name,.-name;

/* Define an entry point visible from C.  */
#define ENTRY(name)                                                           \
  .globl C_SYMBOL_NAME(name);                                                 \
  .type C_SYMBOL_NAME(name),@function;                                        \
  .align 4;                                                                   \
  C_LABEL(name)                                                               \
  cfi_startproc;                                                              \

#undef  END
#define END(name)                                                             \
  cfi_endproc;                                                                \
  ASM_SIZE_DIRECTIVE(name)

/* Since C identifiers are not normally prefixed with an underscore
   on this system, the asm identifier `syscall_error' intrudes on the
   C name space.  Make sure we use an innocuous name.  */
#define syscall_error   __syscall_error

/* Specify the size in bytes of a machine register.  */
#define REGSIZE         4

#endif	/* __ASSEMBLER__ */

#endif /* linux/or1k/sysdep.h */
