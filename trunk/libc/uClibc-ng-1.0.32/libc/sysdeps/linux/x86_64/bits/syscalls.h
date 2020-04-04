#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

/*
   Some of the sneaky macros in the code were taken from
   glibc-2.2.5/sysdeps/unix/sysv/linux/x86_64/sysdep.h
*/

#ifndef __ASSEMBLER__

#include <errno.h>

/* The Linux/x86-64 kernel expects the system call parameters in
   registers according to the following table:

    syscall number	rax
    arg 1		rdi
    arg 2		rsi
    arg 3		rdx
    arg 4		r10
    arg 5		r8
    arg 6		r9

    The Linux kernel uses and destroys internally these registers:
    return address from
    syscall		rcx
    additionally clobered: r12-r15,rbx,rbp
    eflags from syscall	r11

    Normal function call, including calls to the system call stub
    functions in the libc, get the first six parameters passed in
    registers and the seventh parameter and later on the stack.  The
    register use is as follows:

     system call number	in the DO_CALL macro
     arg 1		rdi
     arg 2		rsi
     arg 3		rdx
     arg 4		rcx
     arg 5		r8
     arg 6		r9

    We have to take care that the stack is aligned to 16 bytes.  When
    called the stack is not aligned since the return address has just
    been pushed.


    Syscalls of more than 6 arguments are not supported.  */

#undef	DO_CALL
#define DO_CALL(syscall_name, args)		\
    DOARGS_##args				\
    movq $SYS_ify (syscall_name), %rax;		\
    syscall;

#define DOARGS_0 /* nothing */
#define DOARGS_1 /* nothing */
#define DOARGS_2 /* nothing */
#define DOARGS_3 /* nothing */
#define DOARGS_4 movq %rcx, %r10;
#define DOARGS_5 DOARGS_4
#define DOARGS_6 DOARGS_5

/* Define a macro which expands inline into the wrapper code for a system
   call.  */
#define INTERNAL_SYSCALL_NCS(name, err, nr, args...) \
(__extension__ \
  ({									      \
    unsigned long resultvar;						      \
    LOAD_ARGS_##nr (args)						      \
    LOAD_REGS_##nr							      \
    __asm__ __volatile__ (						      \
    "syscall\n\t"							      \
    : "=a" (resultvar)							      \
    : "0" (name) ASM_ARGS_##nr : "memory", "cc", "r11", "cx");		      \
    (long) resultvar;							      \
   }) \
)
#define LOAD_ARGS_0()
#define LOAD_REGS_0
#define ASM_ARGS_0

#define LOAD_ARGS_1(a1)					\
  long int __arg1 = (long) (a1);			\
  LOAD_ARGS_0 ()
#define LOAD_REGS_1					\
  register long int __a1 __asm__ ("rdi") = __arg1;	\
  LOAD_REGS_0
#define ASM_ARGS_1	ASM_ARGS_0, "r" (__a1)

#define LOAD_ARGS_2(a1, a2)				\
  long int __arg2 = (long) (a2);			\
  LOAD_ARGS_1 (a1)
#define LOAD_REGS_2					\
  register long int __a2 __asm__ ("rsi") = __arg2;	\
  LOAD_REGS_1
#define ASM_ARGS_2	ASM_ARGS_1, "r" (__a2)

#define LOAD_ARGS_3(a1, a2, a3)				\
  long int __arg3 = (long) (a3);			\
  LOAD_ARGS_2 (a1, a2)
#define LOAD_REGS_3					\
  register long int __a3 __asm__ ("rdx") = __arg3;	\
  LOAD_REGS_2
#define ASM_ARGS_3	ASM_ARGS_2, "r" (__a3)

#define LOAD_ARGS_4(a1, a2, a3, a4)			\
  long int __arg4 = (long) (a4);			\
  LOAD_ARGS_3 (a1, a2, a3)
#define LOAD_REGS_4					\
  register long int __a4 __asm__ ("r10") = __arg4;	\
  LOAD_REGS_3
#define ASM_ARGS_4	ASM_ARGS_3, "r" (__a4)

#define LOAD_ARGS_5(a1, a2, a3, a4, a5)			\
  long int __arg5 = (long) (a5);			\
  LOAD_ARGS_4 (a1, a2, a3, a4)
#define LOAD_REGS_5					\
  register long int __a5 __asm__ ("r8") = __arg5;	\
  LOAD_REGS_4
#define ASM_ARGS_5	ASM_ARGS_4, "r" (__a5)

#define LOAD_ARGS_6(a1, a2, a3, a4, a5, a6)		\
  long int __arg6 = (long) (a6);			\
  LOAD_ARGS_5 (a1, a2, a3, a4, a5)
#define LOAD_REGS_6					\
  register long int __a6 __asm__ ("r9") = __arg6;	\
  LOAD_REGS_5
#define ASM_ARGS_6	ASM_ARGS_5, "r" (__a6)

#endif /* __ASSEMBLER__ */
#endif /* _BITS_SYSCALLS_H */
