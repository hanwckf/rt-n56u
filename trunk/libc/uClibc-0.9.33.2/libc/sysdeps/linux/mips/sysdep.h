/* Copyright (C) 1992, 1995, 1997, 1999, 2000, 2002, 2003, 2004
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Brendan Kehoe (brendan@zen.org).

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

#ifndef _LINUX_MIPS_SYSDEP_H
#define _LINUX_MIPS_SYSDEP_H 1

#include <sgidefs.h>
#include <common/sysdep.h>

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

#ifdef __ASSEMBLER__

#include <sys/regdef.h>

#define ENTRY(name) 					\
  .globl name;						\
  .align 2;						\
  .ent name,0;						\
  name##:

#undef END
#define	END(function)					\
		.end	function;			\
		.size	function,.-function

#define ret	j ra ; nop

#undef PSEUDO_END
#define PSEUDO_END(sym) .end sym; .size sym,.-sym

#define PSEUDO_NOERRNO(name, syscall_name, args)	\
  .align 2;						\
  ENTRY(name)						\
  .set noreorder;					\
  li v0, SYS_ify(syscall_name);				\
  syscall

#undef PSEUDO_END_NOERRNO
#define PSEUDO_END_NOERRNO(sym) .end sym; .size sym,.-sym

#define ret_NOERRNO ret

#define PSEUDO_ERRVAL(name, syscall_name, args)		\
  .align 2;						\
  ENTRY(name)						\
  .set noreorder;					\
  li v0, SYS_ify(syscall_name);				\
  syscall

#undef PSEUDO_END_ERRVAL
#define PSEUDO_END_ERRVAL(sym) .end sym; .size sym,.-sym

#define ret_ERRVAL ret

#define r0	v0
#define r1	v1
/* The mips move insn is d,s.  */
#define MOVE(x,y)	move y , x

#if _MIPS_SIM == _ABIO32
# define L(label) $L ## label
#else
# define L(label) .L ## label
#endif

/* Note that while it's better structurally, going back to call __syscall_error
   can make things confusing if you're debugging---it looks like it's jumping
   backwards into the previous fn.  */

#ifdef __PIC__
#define PSEUDO(name, syscall_name, args) 		\
  .align 2;						\
  99: move a0, v0;					\
  la t9,__syscall_error;				\
  jr t9;						\
  ENTRY(name)						\
  .set noreorder;					\
  .cpload t9;						\
  li v0, SYS_ify(syscall_name);				\
  syscall;						\
  .set reorder;						\
  bne a3, zero, 99b;					\
L(syse1):
#else
#define PSEUDO(name, syscall_name, args) 		\
  .set noreorder;					\
  .align 2;						\
  99: move a0, v0;					\
  j __syscall_error;					\
  nop;							\
  ENTRY(name)						\
  .set noreorder;					\
  li v0, SYS_ify(syscall_name);				\
  syscall;						\
  .set reorder;						\
  bne a3, zero, 99b;					\
L(syse1):
#endif

/* We don't want the label for the error handler to be visible in the symbol
   table when we define it here.  */
#ifdef __PIC__
# define SYSCALL_ERROR_LABEL 99b
#endif

#else   /* ! __ASSEMBLER__ */

/* Define a macro which expands into the inline wrapper code for a system
   call.  */
#undef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...)				\
  ({ INTERNAL_SYSCALL_DECL(err);					\
     long result_var = INTERNAL_SYSCALL (name, err, nr, args);		\
     if ( INTERNAL_SYSCALL_ERROR_P (result_var, err) )			\
       {								\
	 __set_errno (INTERNAL_SYSCALL_ERRNO (result_var, err));	\
	 result_var = -1L;						\
       }								\
     result_var; })

#undef INTERNAL_SYSCALL_DECL
#define INTERNAL_SYSCALL_DECL(err) long err attribute_unused

#undef INTERNAL_SYSCALL_ERROR_P
#define INTERNAL_SYSCALL_ERROR_P(val, err)   ((long) (err))

#undef INTERNAL_SYSCALL_ERRNO
#define INTERNAL_SYSCALL_ERRNO(val, err)     (val)

#undef INTERNAL_SYSCALL
#define INTERNAL_SYSCALL(name, err, nr, args...) \
	internal_syscall##nr (, "li\t$2, %2\t\t\t# " #name "\n\t",	\
			      "i" (SYS_ify (name)), err, args)

#undef INTERNAL_SYSCALL_NCS
#define INTERNAL_SYSCALL_NCS(number, err, nr, args...) \
	internal_syscall##nr (= number, , "r" (__v0), err, args)
#undef internal_syscall0
#define internal_syscall0(ncs_init, cs_init, input, err, dummy...)	\
({									\
	long _sys_result;						\
									\
	{								\
	register long __v0 __asm__("$2") ncs_init;			\
	register long __a3 __asm__("$7");				\
	__asm__ __volatile__ (						\
	".set\tnoreorder\n\t"						\
	cs_init								\
	"syscall\n\t"							\
	".set reorder"							\
	: "=r" (__v0), "=r" (__a3)					\
	: input								\
	: __SYSCALL_CLOBBERS);						\
	err = __a3;							\
	_sys_result = __v0;						\
	}								\
	_sys_result;							\
})

#undef internal_syscall1
#define internal_syscall1(ncs_init, cs_init, input, err, arg1)		\
({									\
	long _sys_result;						\
									\
	{								\
	register long __v0 __asm__("$2") ncs_init;			\
	register long __a0 __asm__("$4") = (long) arg1;			\
	register long __a3 __asm__("$7");				\
	__asm__ __volatile__ (						\
	".set\tnoreorder\n\t"						\
	cs_init								\
	"syscall\n\t"							\
	".set reorder"							\
	: "=r" (__v0), "=r" (__a3)					\
	: input, "r" (__a0)						\
	: __SYSCALL_CLOBBERS);						\
	err = __a3;							\
	_sys_result = __v0;						\
	}								\
	_sys_result;							\
})

#undef internal_syscall2
#define internal_syscall2(ncs_init, cs_init, input, err, arg1, arg2)	\
({									\
	long _sys_result;						\
									\
	{								\
	register long __v0 __asm__("$2") ncs_init;			\
	register long __a0 __asm__("$4") = (long) arg1;			\
	register long __a1 __asm__("$5") = (long) arg2;			\
	register long __a3 __asm__("$7");				\
	__asm__ __volatile__ (						\
	".set\tnoreorder\n\t"						\
	cs_init								\
	"syscall\n\t"							\
	".set\treorder"							\
	: "=r" (__v0), "=r" (__a3)					\
	: input, "r" (__a0), "r" (__a1)					\
	: __SYSCALL_CLOBBERS);						\
	err = __a3;							\
	_sys_result = __v0;						\
	}								\
	_sys_result;							\
})

#undef internal_syscall3
#define internal_syscall3(ncs_init, cs_init, input, err, arg1, arg2, arg3)\
({									\
	long _sys_result;						\
									\
	{								\
	register long __v0 __asm__("$2") ncs_init;			\
	register long __a0 __asm__("$4") = (long) arg1;			\
	register long __a1 __asm__("$5") = (long) arg2;			\
	register long __a2 __asm__("$6") = (long) arg3;			\
	register long __a3 __asm__("$7");				\
	__asm__ __volatile__ (						\
	".set\tnoreorder\n\t"						\
	cs_init								\
	"syscall\n\t"							\
	".set\treorder"							\
	: "=r" (__v0), "=r" (__a3)					\
	: input, "r" (__a0), "r" (__a1), "r" (__a2)			\
	: __SYSCALL_CLOBBERS);						\
	err = __a3;							\
	_sys_result = __v0;						\
	}								\
	_sys_result;							\
})

#undef internal_syscall4
#define internal_syscall4(ncs_init, cs_init, input, err, arg1, arg2, arg3, arg4)\
({									\
	long _sys_result;						\
									\
	{								\
	register long __v0 __asm__("$2") ncs_init;			\
	register long __a0 __asm__("$4") = (long) arg1;			\
	register long __a1 __asm__("$5") = (long) arg2;			\
	register long __a2 __asm__("$6") = (long) arg3;			\
	register long __a3 __asm__("$7") = (long) arg4;			\
	__asm__ __volatile__ (						\
	".set\tnoreorder\n\t"						\
	cs_init								\
	"syscall\n\t"							\
	".set\treorder"							\
	: "=r" (__v0), "+r" (__a3)					\
	: input, "r" (__a0), "r" (__a1), "r" (__a2)			\
	: __SYSCALL_CLOBBERS);						\
	err = __a3;							\
	_sys_result = __v0;						\
	}								\
	_sys_result;							\
})

/* We need to use a frame pointer for the functions in which we
   adjust $sp around the syscall, or debug information and unwind
   information will be $sp relative and thus wrong during the syscall.  As
   of GCC 3.4.3, this is sufficient.  */
#define FORCE_FRAME_POINTER alloca (4)

#undef internal_syscall5
#define internal_syscall5(ncs_init, cs_init, input, err, arg1, arg2, arg3, arg4, arg5)\
({									\
	long _sys_result;						\
									\
	FORCE_FRAME_POINTER;						\
	{								\
	register long __v0 __asm__("$2") ncs_init;			\
	register long __a0 __asm__("$4") = (long) arg1;			\
	register long __a1 __asm__("$5") = (long) arg2;			\
	register long __a2 __asm__("$6") = (long) arg3;			\
	register long __a3 __asm__("$7") = (long) arg4;			\
	__asm__ __volatile__ (						\
	".set\tnoreorder\n\t"						\
	"subu\t$29, 32\n\t"						\
	"sw\t%6, 16($29)\n\t"						\
	cs_init								\
	"syscall\n\t"							\
	"addiu\t$29, 32\n\t"						\
	".set\treorder"							\
	: "=r" (__v0), "+r" (__a3)					\
	: input, "r" (__a0), "r" (__a1), "r" (__a2),			\
	  "r" ((long)arg5)						\
	: __SYSCALL_CLOBBERS);						\
	err = __a3;							\
	_sys_result = __v0;						\
	}								\
	_sys_result;							\
})

#undef internal_syscall6
#define internal_syscall6(ncs_init, cs_init, input, err, arg1, arg2, arg3, arg4, arg5, arg6)\
({									\
	long _sys_result;						\
									\
	FORCE_FRAME_POINTER;						\
	{								\
	register long __v0 __asm__("$2") ncs_init;			\
	register long __a0 __asm__("$4") = (long) arg1;			\
	register long __a1 __asm__("$5") = (long) arg2;			\
	register long __a2 __asm__("$6") = (long) arg3;			\
	register long __a3 __asm__("$7") = (long) arg4;			\
	__asm__ __volatile__ (						\
	".set\tnoreorder\n\t"						\
	"subu\t$29, 32\n\t"						\
	"sw\t%6, 16($29)\n\t"						\
	"sw\t%7, 20($29)\n\t"						\
	cs_init								\
	"syscall\n\t"							\
	"addiu\t$29, 32\n\t"						\
	".set\treorder"							\
	: "=r" (__v0), "+r" (__a3)					\
	: input, "r" (__a0), "r" (__a1), "r" (__a2),			\
	  "r" ((long)arg5), "r" ((long)arg6)				\
	: __SYSCALL_CLOBBERS);						\
	err = __a3;							\
	_sys_result = __v0;						\
	}								\
	_sys_result;							\
})

#undef internal_syscall7
#define internal_syscall7(ncs_init, cs_init, input, err, arg1, arg2, arg3, arg4, arg5, arg6, arg7)\
({									\
	long _sys_result;						\
									\
	FORCE_FRAME_POINTER;						\
	{								\
	register long __v0 __asm__("$2") ncs_init;			\
	register long __a0 __asm__("$4") = (long) arg1;			\
	register long __a1 __asm__("$5") = (long) arg2;			\
	register long __a2 __asm__("$6") = (long) arg3;			\
	register long __a3 __asm__("$7") = (long) arg4;			\
	__asm__ __volatile__ (						\
	".set\tnoreorder\n\t"						\
	"subu\t$29, 32\n\t"						\
	"sw\t%6, 16($29)\n\t"						\
	"sw\t%7, 20($29)\n\t"						\
	"sw\t%8, 24($29)\n\t"						\
	cs_init								\
	"syscall\n\t"							\
	"addiu\t$29, 32\n\t"						\
	".set\treorder"							\
	: "=r" (__v0), "+r" (__a3)					\
	: input, "r" (__a0), "r" (__a1), "r" (__a2),			\
	  "r" ((long)arg5), "r" ((long)arg6), "r" ((long)arg7)		\
	: __SYSCALL_CLOBBERS);						\
	err = __a3;							\
	_sys_result = __v0;						\
	}								\
	_sys_result;							\
})

#undef __SYSCALL_CLOBBERS
#define __SYSCALL_CLOBBERS "$1", "$3", "$8", "$9", "$10", "$11", "$12", "$13", \
	"$14", "$15", "$24", "$25", "memory"

/* Pointer mangling is not yet supported for MIPS.  */
#define PTR_MANGLE(var) (void) (var)
#define PTR_DEMANGLE(var) (void) (var)

#endif  /* __ASSEMBLER__ */
#endif /* _LINUX_MIPS_SYSDEP_H */
