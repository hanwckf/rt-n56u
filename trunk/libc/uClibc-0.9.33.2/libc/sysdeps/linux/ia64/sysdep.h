/* Copyright (C) 1999, 2000, 2002, 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Jes Sorensen, <Jes.Sorensen@cern.ch>, April 1999.
   Based on code originally written by David Mosberger-Tang

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

#ifndef _LINUX_IA64_SYSDEP_H
#define _LINUX_IA64_SYSDEP_H 1

#include <features.h>
#include <asm/unistd.h>

#ifdef __ASSEMBLER__

/* Macros to help writing .prologue directives in assembly code.  */
#define ASM_UNW_PRLG_RP			0x8
#define ASM_UNW_PRLG_PFS		0x4
#define ASM_UNW_PRLG_PSP		0x2
#define ASM_UNW_PRLG_PR			0x1
#define ASM_UNW_PRLG_GRSAVE(ninputs)	(32+(ninputs))

#ifdef	__STDC__
#define C_LABEL(name)		name :
#else
#define C_LABEL(name)		name/**/:
#endif

#define CALL_MCOUNT

#define ENTRY(name)				\
	.text;					\
	.align 32;				\
	.proc C_SYMBOL_NAME(name);		\
	.global C_SYMBOL_NAME(name);		\
	C_LABEL(name)				\
	CALL_MCOUNT

#define LEAF(name)				\
  .text;					\
  .align 32;					\
  .proc C_SYMBOL_NAME(name);			\
  .global name;					\
  C_LABEL(name)

/* Mark the end of function SYM.  */
#undef END
#define END(sym)	.endp C_SYMBOL_NAME(sym)

/* For Linux we can use the system call table in the header file
	/usr/include/asm/unistd.h
   of the kernel.  But these symbols do not follow the SYS_* syntax
   so we have to redefine the `SYS_ify' macro here.  */
#undef SYS_ify
#ifdef __STDC__
# define SYS_ify(syscall_name)	__NR_##syscall_name
#else
# define SYS_ify(syscall_name)	__NR_/**/syscall_name
#endif

/* Linux uses a negative return value to indicate syscall errors, unlike
   most Unices, which use the condition codes' carry flag.

   Since version 2.1 the return value of a system call might be negative
   even if the call succeeded.  E.g., the `lseek' system call might return
   a large offset.  Therefore we must not anymore test for < 0, but test
   for a real error by making sure the value in %d0 is a real error
   number.  Linus said he will make sure the no syscall returns a value
   in -1 .. -4095 as a valid result so we can savely test with -4095.  */

/* We don't want the label for the error handler to be visible in the symbol
   table when we define it here.  */
#define SYSCALL_ERROR_LABEL __syscall_error

#undef PSEUDO
#define	PSEUDO(name, syscall_name, args)	\
  ENTRY(name)					\
    DO_CALL (SYS_ify(syscall_name));		\
	cmp.eq p6,p0=-1,r10;			\
(p6)	br.cond.spnt.few __syscall_error;

#define DO_CALL_VIA_BREAK(num)			\
	mov r15=num;				\
	break __BREAK_SYSCALL

#ifdef IA64_USE_NEW_STUB
# ifdef SHARED
#  define DO_CALL(num)				\
	.prologue;				\
	adds r2 = SYSINFO_OFFSET, r13;;		\
	ld8 r2 = [r2];				\
	.save ar.pfs, r11;			\
	mov r11 = ar.pfs;;			\
	.body;					\
	mov r15 = num;				\
	mov b7 = r2;				\
	br.call.sptk.many b6 = b7;;		\
	.restore sp;				\
	mov ar.pfs = r11;			\
	.prologue;				\
	.body
# else /* !SHARED */
#  define DO_CALL(num)				\
	.prologue;				\
	mov r15 = num;				\
	movl r2 = _dl_sysinfo;;			\
	ld8 r2 = [r2];				\
	.save ar.pfs, r11;			\
	mov r11 = ar.pfs;;			\
	.body;					\
	mov b7 = r2;				\
	br.call.sptk.many b6 = b7;;		\
	.restore sp;				\
	mov ar.pfs = r11;			\
	.prologue;				\
	.body
# endif
#else
# define DO_CALL(num)				DO_CALL_VIA_BREAK(num)
#endif

#undef PSEUDO_END
#define PSEUDO_END(name)	.endp C_SYMBOL_NAME(name);

#undef PSEUDO_NOERRNO
#define	PSEUDO_NOERRNO(name, syscall_name, args)	\
  ENTRY(name)						\
    DO_CALL (SYS_ify(syscall_name));

#undef PSEUDO_END_NOERRNO
#define PSEUDO_END_NOERRNO(name)	.endp C_SYMBOL_NAME(name);

#undef PSEUDO_ERRVAL
#define	PSEUDO_ERRVAL(name, syscall_name, args)	\
  ENTRY(name)					\
    DO_CALL (SYS_ify(syscall_name));		\
	cmp.eq p6,p0=-1,r10;			\
(p6)	mov r10=r8;


#undef PSEUDO_END_ERRVAL
#define PSEUDO_END_ERRVAL(name)	.endp C_SYMBOL_NAME(name);

#undef END
#define END(name)						\
	.size	C_SYMBOL_NAME(name), . - C_SYMBOL_NAME(name) ;	\
	.endp	C_SYMBOL_NAME(name)

#define ret			br.ret.sptk.few b0
#define ret_NOERRNO		ret
#define ret_ERRVAL		ret

#endif /* not __ASSEMBLER__ */

#endif /* linux/ia64/sysdep.h */
