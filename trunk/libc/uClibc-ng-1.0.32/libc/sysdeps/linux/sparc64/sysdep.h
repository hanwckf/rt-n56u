/* Copyright (C) 1997-2017 Free Software Foundation, Inc.
   Contributed by Richard Henderson <richard@gnu.ai.mit.edu>, 1997.

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

#ifndef _LINUX_SPARC64_SYSDEP_H
#define _LINUX_SPARC64_SYSDEP_H 1

#include <common/sysdep.h>

#undef SYS_ify
#define SYS_ify(syscall_name) __NR_##syscall_name

#ifdef __ASSEMBLER__

#define SPARC_PIC_THUNK(reg)						\
	.ifndef __sparc_get_pc_thunk.reg;				\
	.section .text.__sparc_get_pc_thunk.reg,"axG",@progbits,__sparc_get_pc_thunk.reg,comdat; \
	.align	 32;							\
	.weak	 __sparc_get_pc_thunk.reg;				\
	.hidden	 __sparc_get_pc_thunk.reg;				\
	.type	 __sparc_get_pc_thunk.reg, #function;			\
__sparc_get_pc_thunk.reg:		   				\
	jmp	%o7 + 8;						\
	 add	%o7, %reg, %##reg;					\
	.previous;							\
	.endif;

/* The "-4" and "+4" offsets against _GLOBAL_OFFSET_TABLE_ are
   critical since they represent the offset from the thunk call to the
   instruction containing the _GLOBAL_OFFSET_TABLE_ reference.
   Therefore these instructions cannot be moved around without
   appropriate adjustments to those offsets.

   Furthermore, these expressions are special in another regard.  When
   the assembler sees a reference to _GLOBAL_OFFSET_TABLE_ inside of
   a %hi() or %lo(), it emits a PC-relative relocation.  This causes
   R_SPARC_HI22 to turn into R_SPARC_PC22, and R_SPARC_LO10 to turn into
   R_SPARC_PC10, respectively.

   Even when v9 we use a call sequence instead of using "rd %pc" because
   RDPC is extremely expensive and incurs a full pipeline flush.  */

#define SPARC_PIC_THUNK_CALL(reg)					\
	sethi	%hi(_GLOBAL_OFFSET_TABLE_-4), %##reg;			\
	call	__sparc_get_pc_thunk.reg;				\
	 or	%##reg, %lo(_GLOBAL_OFFSET_TABLE_+4), %##reg;

#define SETUP_PIC_REG(reg)						\
	SPARC_PIC_THUNK(reg)						\
	SPARC_PIC_THUNK_CALL(reg)

#define SETUP_PIC_REG_LEAF(reg, tmp)					\
	SPARC_PIC_THUNK(reg)						\
	mov	%o7, %##tmp;		      				\
	SPARC_PIC_THUNK_CALL(reg);					\
	mov	%##tmp, %o7;


#define LOADSYSCALL(x) mov __NR_##x, %g1

#define ENTRY(name)                 \
    .align 4;                       \
    .global C_SYMBOL_NAME(name);    \
    .type   name, @function;        \
C_LABEL(name)                       \
    cfi_startproc;

#undef END
#define END(name)                   \
    cfi_endproc;                    \
    .size name, . - name

#define LOC(name) .L##name

#undef PSEUDO
#define PSEUDO(name, syscall_name, args)	\
	.text;					\
ENTRY(name);					\
	LOADSYSCALL(syscall_name);		\
	ta		0x6d;			\
	bcc,pt		%xcc, 1f;		\
	 nop;					\
	SYSCALL_ERROR_HANDLER			\
1:

#undef PSEUDO_NOERRNO
#define	PSEUDO_NOERRNO(name, syscall_name, args)\
	.text;					\
ENTRY(name);					\
	LOADSYSCALL(syscall_name);		\
	ta		0x6d;

#undef PSEUDO_ERRVAL
#define	PSEUDO_ERRVAL(name, syscall_name, args) \
	.text;					\
ENTRY(name);					\
	LOADSYSCALL(syscall_name);		\
	ta		0x6d;

#undef PSEUDO_END
#define PSEUDO_END(name)			\
	END(name)

#ifndef __PIC__
# define SYSCALL_ERROR_HANDLER			\
	mov	%o7, %g1;			\
	call	__syscall_error;		\
	 mov	%g1, %o7;
#else
# define SYSCALL_ERROR_HANDLER		\
0:	SETUP_PIC_REG_LEAF(o2,g1)	\
	sethi	%gdop_hix22(errno), %g1;\
	xor	%g1, %gdop_lox10(errno), %g1;\
	ldx	[%o2 + %g1], %g1, %gdop(errno);\
	st	%o0, [%g1];		\
	jmp	%o7 + 8;		\
	 mov	-1, %o0;
#endif	/* PIC */

#endif	/* __ASSEMBLER__ */

/* This is the offset from the %sp to the backing store above the
   register windows.  So if you poke stack memory directly you add this.  */
#define STACK_BIAS	2047

#endif
