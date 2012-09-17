/* Copyright (C) 1992,1993,1995-2000,2002-2006,2007
	Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper, <drepper@gnu.org>, August 1995.

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

#ifndef _LINUX_I386_SYSDEP_H
#define _LINUX_I386_SYSDEP_H 1

#include <sys/syscall.h>
#include <common/sysdep.h>

#ifdef	__ASSEMBLER__

/* Syntactic details of assembler.  */

/* ELF uses byte-counts for .align, most others use log2 of count of bytes.  */
#define ALIGNARG(log2) 1<<log2
/* For ELF we need the `.type' directive to make shared libs work right.  */
#define ASM_TYPE_DIRECTIVE(name,typearg) .type name,typearg;
#define ASM_SIZE_DIRECTIVE(name) .size name,.-name;

/* In ELF C symbols are asm symbols.  */
#undef	NO_UNDERSCORES
#define NO_UNDERSCORES

/* Define an entry point visible from C.

   There is currently a bug in gdb which prevents us from specifying
   incomplete stabs information.  Fake some entries here which specify
   the current source file.  */
#define	ENTRY(name)							      \
  STABS_CURRENT_FILE1("")						      \
  STABS_CURRENT_FILE(name)						      \
  ASM_GLOBAL_DIRECTIVE C_SYMBOL_NAME(name);				      \
  ASM_TYPE_DIRECTIVE (C_SYMBOL_NAME(name),@function)			      \
  .align ALIGNARG(4);							      \
  STABS_FUN(name)							      \
  C_LABEL(name)								      \
  cfi_startproc;							      \
  CALL_MCOUNT

#undef	END
#define END(name)							      \
  cfi_endproc;								      \
  ASM_SIZE_DIRECTIVE(name)						      \
  STABS_FUN_END(name)

#ifdef HAVE_CPP_ASM_DEBUGINFO
/* Disable that goop, because we just pass -g through to the assembler
   and it generates proper line number information directly.  */
# define STABS_CURRENT_FILE1(name)
# define STABS_CURRENT_FILE(name)
# define STABS_FUN(name)
# define STABS_FUN_END(name)
#else
/* Remove the following two lines once the gdb bug is fixed.  */
#define STABS_CURRENT_FILE(name)					      \
  STABS_CURRENT_FILE1 (#name)
#define STABS_CURRENT_FILE1(name)					      \
  1: .stabs name,100,0,0,1b;
/* Emit stabs definition lines.  We use F(0,1) and define t(0,1) as `int',
   the same way gcc does it.  */
#define STABS_FUN(name) STABS_FUN2(name, name##:F(0,1))
#define STABS_FUN2(name, namestr)					      \
  .stabs "int:t(0,1)=r(0,1);-2147483648;2147483647;",128,0,0,0;		      \
  .stabs #namestr,36,0,0,name;
#define STABS_FUN_END(name)						      \
  1: .stabs "",36,0,0,1b-name;
#endif

/* If compiled for profiling, call `mcount' at the start of each function.  */
#ifdef	PROF
/* The mcount code relies on a normal frame pointer being on the stack
   to locate our caller, so push one just for its benefit.  */
#define CALL_MCOUNT \
  pushl %ebp; cfi_adjust_cfa_offset (4); movl %esp, %ebp; \
  cfi_def_cfa_register (ebp); call JUMPTARGET(mcount); \
  popl %ebp; cfi_def_cfa (esp, 4);
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

#undef JUMPTARGET
#ifdef __PIC__
#define JUMPTARGET(name)	name##@PLT
#define SYSCALL_PIC_SETUP \
    pushl %ebx;								      \
    cfi_adjust_cfa_offset (4);						      \
    call 0f;								      \
0:  popl %ebx;								      \
    cfi_adjust_cfa_offset (-4);						      \
    addl $_GLOBAL_OFFSET_TABLE+[.-0b], %ebx;


# define SETUP_PIC_REG(reg) \
  .ifndef __x86.get_pc_thunk.reg;					      \
  .section .gnu.linkonce.t.__x86.get_pc_thunk.reg,"ax",@progbits;	      \
  .globl __x86.get_pc_thunk.reg;					      \
  .hidden __x86.get_pc_thunk.reg;					      \
  .type __x86.get_pc_thunk.reg,@function;				      \
__x86.get_pc_thunk.reg:						      \
  movl (%esp), %e##reg;							      \
  ret;									      \
  .size __x86.get_pc_thunk.reg, . - __x86.get_pc_thunk.reg;		      \
  .previous;								      \
  .endif;								      \
  call __x86.get_pc_thunk.reg

# define LOAD_PIC_REG(reg) \
  SETUP_PIC_REG(reg); addl $_GLOBAL_OFFSET_TABLE_, %e##reg

#else
#define JUMPTARGET(name)	name
#define SYSCALL_PIC_SETUP	/* Nothing.  */
#endif

/* Local label name for asm code. */
#ifndef L
#ifdef HAVE_ELF
#define L(name)		.L##name
#else
#define L(name)		name
#endif
#endif

/* Avoid conflics with thunk section */
#undef __i686
#endif	/* __ASSEMBLER__ */

#ifndef offsetof
# define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

/* For Linux we can use the system call table in the header file
	/usr/include/asm/unistd.h
   of the kernel.  But these symbols do not follow the SYS_* syntax
   so we have to redefine the `SYS_ify' macro here.  */
#undef SYS_ify
#define SYS_ify(syscall_name)	__NR_##syscall_name

#if defined USE_DL_SYSINFO \
    && (!defined NOT_IN_libc || defined IS_IN_libpthread)
# define I386_USE_SYSENTER	1
#else
# undef I386_USE_SYSENTER
#endif

#ifdef __ASSEMBLER__

/* Linux uses a negative return value to indicate syscall errors,
   unlike most Unices, which use the condition codes' carry flag.

   Since version 2.1 the return value of a system call might be
   negative even if the call succeeded.  E.g., the `lseek' system call
   might return a large offset.  Therefore we must not anymore test
   for < 0, but test for a real error by making sure the value in %eax
   is a real error number.  Linus said he will make sure the no syscall
   returns a value in -1 .. -4095 as a valid result so we can savely
   test with -4095.  */

/* We don't want the label for the error handle to be global when we define
   it here.  */
#ifdef __PIC__
# define SYSCALL_ERROR_LABEL 0f
#else
# define SYSCALL_ERROR_LABEL syscall_error
#endif

#undef	PSEUDO
#define	PSEUDO(name, syscall_name, args)				      \
  .text;								      \
  ENTRY (name)								      \
    DO_CALL (syscall_name, args);					      \
    cmpl $-4095, %eax;							      \
    jae SYSCALL_ERROR_LABEL;						      \
  L(pseudo_end):

#undef	PSEUDO_END
#define	PSEUDO_END(name)						      \
  SYSCALL_ERROR_HANDLER							      \
  END (name)

#undef	PSEUDO_NOERRNO
#define	PSEUDO_NOERRNO(name, syscall_name, args)			      \
  .text;								      \
  ENTRY (name)								      \
    DO_CALL (syscall_name, args)

#undef	PSEUDO_END_NOERRNO
#define	PSEUDO_END_NOERRNO(name)					      \
  END (name)

#define ret_NOERRNO ret

/* The function has to return the error code.  */
#undef	PSEUDO_ERRVAL
#define	PSEUDO_ERRVAL(name, syscall_name, args) \
  .text;								      \
  ENTRY (name)								      \
    DO_CALL (syscall_name, args);					      \
    negl %eax

#undef	PSEUDO_END_ERRVAL
#define	PSEUDO_END_ERRVAL(name) \
  END (name)

#define ret_ERRVAL ret

#ifndef __PIC__
# define SYSCALL_ERROR_HANDLER	/* Nothing here; code in sysdep.S is used.  */
#else

# ifdef RTLD_PRIVATE_ERRNO
#  define SYSCALL_ERROR_HANDLER						      \
0:SETUP_PIC_REG(cx);							      \
  addl $_GLOBAL_OFFSET_TABLE_, %ecx;					      \
  xorl %edx, %edx;							      \
  subl %eax, %edx;							      \
  movl %edx, rtld_errno@GOTOFF(%ecx);					      \
  orl $-1, %eax;							      \
  jmp L(pseudo_end);

# elif defined _LIBC_REENTRANT

#  if defined USE___THREAD
#   ifndef NOT_IN_libc
#    define SYSCALL_ERROR_ERRNO __libc_errno
#   else
#    define SYSCALL_ERROR_ERRNO errno
#   endif
#   define SYSCALL_ERROR_HANDLER					      \
0:SETUP_PIC_REG (cx);							      \
  addl $_GLOBAL_OFFSET_TABLE_, %ecx;					      \
  movl SYSCALL_ERROR_ERRNO@GOTNTPOFF(%ecx), %ecx;			      \
  xorl %edx, %edx;							      \
  subl %eax, %edx;							      \
  SYSCALL_ERROR_HANDLER_TLS_STORE (%edx, %ecx);				      \
  orl $-1, %eax;							      \
  jmp L(pseudo_end);
#   ifndef NO_TLS_DIRECT_SEG_REFS
#    define SYSCALL_ERROR_HANDLER_TLS_STORE(src, destoff)		      \
  movl src, %gs:(destoff)
#   else
#    define SYSCALL_ERROR_HANDLER_TLS_STORE(src, destoff)		      \
  addl %gs:0, destoff;							      \
  movl src, (destoff)
#   endif
#  else
#   define SYSCALL_ERROR_HANDLER					      \
0:pushl %ebx;								      \
  cfi_adjust_cfa_offset (4);						      \
  cfi_rel_offset (ebx, 0);						      \
  SETUP_PIC_REG (bx);							      \
  addl $_GLOBAL_OFFSET_TABLE_, %ebx;					      \
  xorl %edx, %edx;							      \
  subl %eax, %edx;							      \
  pushl %edx;								      \
  cfi_adjust_cfa_offset (4);						      \
  call __errno_location@PLT;					              \
  popl %ecx;								      \
  cfi_adjust_cfa_offset (-4);						      \
  popl %ebx;								      \
  cfi_adjust_cfa_offset (-4);						      \
  cfi_restore (ebx);							      \
  movl %ecx, (%eax);							      \
  orl $-1, %eax;							      \
  jmp L(pseudo_end);
/* A quick note: it is assumed that the call to `__errno_location' does
   not modify the stack!  */
#  endif
# else
/* Store (- %eax) into errno through the GOT.  */
#  define SYSCALL_ERROR_HANDLER						      \
0:SETUP_PIC_REG(cx);							      \
  addl $_GLOBAL_OFFSET_TABLE_, %ecx;					      \
  xorl %edx, %edx;							      \
  subl %eax, %edx;							      \
  movl errno@GOT(%ecx), %ecx;						      \
  movl %edx, (%ecx);							      \
  orl $-1, %eax;							      \
  jmp L(pseudo_end);
# endif	/* _LIBC_REENTRANT */
#endif	/* __PIC__ */


/* The original calling convention for system calls on Linux/i386 is
   to use int $0x80.  */
#ifdef I386_USE_SYSENTER
# ifdef SHARED
#  define ENTER_KERNEL call *%gs:SYSINFO_OFFSET
# else
#  define ENTER_KERNEL call *_dl_sysinfo
# endif
#else
# define ENTER_KERNEL int $0x80
#endif

/* Linux takes system call arguments in registers:

	syscall number	%eax	     call-clobbered
	arg 1		%ebx	     call-saved
	arg 2		%ecx	     call-clobbered
	arg 3		%edx	     call-clobbered
	arg 4		%esi	     call-saved
	arg 5		%edi	     call-saved
	arg 6		%ebp	     call-saved

   The stack layout upon entering the function is:

	24(%esp)	Arg# 6
	20(%esp)	Arg# 5
	16(%esp)	Arg# 4
	12(%esp)	Arg# 3
	 8(%esp)	Arg# 2
	 4(%esp)	Arg# 1
	  (%esp)	Return address

   (Of course a function with say 3 arguments does not have entries for
   arguments 4, 5, and 6.)

   The following code tries hard to be optimal.  A general assumption
   (which is true according to the data books I have) is that

	2 * xchg	is more expensive than	pushl + movl + popl

   Beside this a neat trick is used.  The calling conventions for Linux
   tell that among the registers used for parameters %ecx and %edx need
   not be saved.  Beside this we may clobber this registers even when
   they are not used for parameter passing.

   As a result one can see below that we save the content of the %ebx
   register in the %edx register when we have less than 3 arguments
   (2 * movl is less expensive than pushl + popl).

   Second unlike for the other registers we don't save the content of
   %ecx and %edx when we have more than 1 and 2 registers resp.

   The code below might look a bit long but we have to take care for
   the pipelined processors (i586).  Here the `pushl' and `popl'
   instructions are marked as NP (not pairable) but the exception is
   two consecutive of these instruction.  This gives no penalty on
   other processors though.  */

#undef	DO_CALL
#define DO_CALL(syscall_name, args)			      		      \
    PUSHARGS_##args							      \
    DOARGS_##args							      \
    movl $SYS_ify (syscall_name), %eax;					      \
    ENTER_KERNEL							      \
    POPARGS_##args

#define PUSHARGS_0	/* No arguments to push.  */
#define	DOARGS_0	/* No arguments to frob.  */
#define	POPARGS_0	/* No arguments to pop.  */
#define	_PUSHARGS_0	/* No arguments to push.  */
#define _DOARGS_0(n)	/* No arguments to frob.  */
#define	_POPARGS_0	/* No arguments to pop.  */

#define PUSHARGS_1	movl %ebx, %edx; L(SAVEBX1): PUSHARGS_0
#define	DOARGS_1	_DOARGS_1 (4)
#define	POPARGS_1	POPARGS_0; movl %edx, %ebx; L(RESTBX1):
#define	_PUSHARGS_1	pushl %ebx; cfi_adjust_cfa_offset (4); \
			cfi_rel_offset (ebx, 0); L(PUSHBX1): _PUSHARGS_0
#define _DOARGS_1(n)	movl n(%esp), %ebx; _DOARGS_0(n-4)
#define	_POPARGS_1	_POPARGS_0; popl %ebx; cfi_adjust_cfa_offset (-4); \
			cfi_restore (ebx); L(POPBX1):

#define PUSHARGS_2	PUSHARGS_1
#define	DOARGS_2	_DOARGS_2 (8)
#define	POPARGS_2	POPARGS_1
#define _PUSHARGS_2	_PUSHARGS_1
#define	_DOARGS_2(n)	movl n(%esp), %ecx; _DOARGS_1 (n-4)
#define	_POPARGS_2	_POPARGS_1

#define PUSHARGS_3	_PUSHARGS_2
#define DOARGS_3	_DOARGS_3 (16)
#define POPARGS_3	_POPARGS_3
#define _PUSHARGS_3	_PUSHARGS_2
#define _DOARGS_3(n)	movl n(%esp), %edx; _DOARGS_2 (n-4)
#define _POPARGS_3	_POPARGS_2

#define PUSHARGS_4	_PUSHARGS_4
#define DOARGS_4	_DOARGS_4 (24)
#define POPARGS_4	_POPARGS_4
#define _PUSHARGS_4	pushl %esi; cfi_adjust_cfa_offset (4); \
			cfi_rel_offset (esi, 0); L(PUSHSI1): _PUSHARGS_3
#define _DOARGS_4(n)	movl n(%esp), %esi; _DOARGS_3 (n-4)
#define _POPARGS_4	_POPARGS_3; popl %esi; cfi_adjust_cfa_offset (-4); \
			cfi_restore (esi); L(POPSI1):

#define PUSHARGS_5	_PUSHARGS_5
#define DOARGS_5	_DOARGS_5 (32)
#define POPARGS_5	_POPARGS_5
#define _PUSHARGS_5	pushl %edi; cfi_adjust_cfa_offset (4); \
			cfi_rel_offset (edi, 0); L(PUSHDI1): _PUSHARGS_4
#define _DOARGS_5(n)	movl n(%esp), %edi; _DOARGS_4 (n-4)
#define _POPARGS_5	_POPARGS_4; popl %edi; cfi_adjust_cfa_offset (-4); \
			cfi_restore (edi); L(POPDI1):

#define PUSHARGS_6	_PUSHARGS_6
#define DOARGS_6	_DOARGS_6 (40)
#define POPARGS_6	_POPARGS_6
#define _PUSHARGS_6	pushl %ebp; cfi_adjust_cfa_offset (4); \
			cfi_rel_offset (ebp, 0); L(PUSHBP1): _PUSHARGS_5
#define _DOARGS_6(n)	movl n(%esp), %ebp; _DOARGS_5 (n-4)
#define _POPARGS_6	_POPARGS_5; popl %ebp; cfi_adjust_cfa_offset (-4); \
			cfi_restore (ebp); L(POPBP1):

#endif	/* __ASSEMBLER__ */


/* Pointer mangling support.  */
#if defined NOT_IN_libc && defined IS_IN_rtld
/* We cannot use the thread descriptor because in ld.so we use setjmp
   earlier than the descriptor is initialized.  Using a global variable
   is too complicated here since we have no PC-relative addressing mode.  */
#else
# ifdef __ASSEMBLER__
#  define PTR_MANGLE(reg)	xorl %gs:POINTER_GUARD, reg;		      \
				roll $9, reg
#  define PTR_DEMANGLE(reg)	rorl $9, reg;				      \
				xorl %gs:POINTER_GUARD, reg
# else
#  define PTR_MANGLE(var)	__asm__ ("xorl %%gs:%c2, %0\n"		      \
				     "roll $9, %0"			      \
				     : "=r" (var)			      \
				     : "0" (var),			      \
				       "i" (offsetof (tcbhead_t,	      \
						      pointer_guard)))
#  define PTR_DEMANGLE(var)	__asm__ ("rorl $9, %0\n"			      \
				     "xorl %%gs:%c2, %0"		      \
				     : "=r" (var)			      \
				     : "0" (var),			      \
				       "i" (offsetof (tcbhead_t,	      \
						      pointer_guard)))
# endif
#endif

#endif /* linux/i386/sysdep.h */
