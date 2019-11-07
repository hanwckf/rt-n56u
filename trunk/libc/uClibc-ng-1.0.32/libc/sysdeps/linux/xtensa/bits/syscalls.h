#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

/*
   Some of the sneaky macros in the code were taken from
   glibc .../sysdeps/unix/sysv/linux/xtensa/sysdep.h
*/

#ifdef __ASSEMBLER__

/* The register layout upon entering the function is:

   return addr	stack ptr	arg0, arg1, arg2, arg3, arg4, arg5
   -----------	---------	----------------------------------
   a0		a1		a2,   a3,   a4,   a5,   a6,   a7

   (Of course a function with say 3 arguments does not have entries for
   arguments 4, 5, and 6.)

   Linux takes system-call arguments in registers.  The ABI and Xtensa
   software conventions require the system-call number in a2.  We move any
   argument that was in a2 to a7, and a7 to a8 if we have all 6 arguments.
   Note that for improved efficiency, we do NOT shift all parameters down
   one register to maintain the original order.

   syscall number		arg0, arg1, arg2, arg3, arg4, arg5
   --------------		----------------------------------
   a2				a6,   a3,   a4,   a5,   a8,   a9

   Upon return, a2 and a3 are clobbered; all other registers are preserved.  */

#undef  DO_CALL
#define DO_CALL(syscall_name, nargs)					      \
	DO_ARGS_##nargs							      \
	movi	a2, SYS_ify (syscall_name);				      \
	syscall

#define DO_ARGS_0
#define DO_ARGS_1	mov a6, a2;
#define DO_ARGS_2	mov a6, a2;
#define DO_ARGS_3	mov a6, a2;
#define DO_ARGS_4	mov a6, a2;
#define DO_ARGS_5	mov a8, a6; mov a6, a2;
#define DO_ARGS_6	mov a9, a7; mov a8, a6; mov a6, a2;

#else /* not __ASSEMBLER__ */

#include <errno.h>

#define STR(s) #s
#define LD_ARG(n,ar)	register int _a##n __asm__ (STR(a##n)) = (int) (ar)

#define LD_ARGS_0()
#define LD_ARGS_1(a0)			LD_ARG(6,a0)
#define LD_ARGS_2(a0,a1)		LD_ARGS_1(a0); LD_ARG(3,a1)
#define LD_ARGS_3(a0,a1,a2)		LD_ARGS_2(a0,a1); LD_ARG(4,a2)
#define LD_ARGS_4(a0,a1,a2,a3)		LD_ARGS_3(a0,a1,a2); LD_ARG(5,a3)
#define LD_ARGS_5(a0,a1,a2,a3,a4)	LD_ARGS_4(a0,a1,a2,a3); LD_ARG(8,a4)
#define LD_ARGS_6(a0,a1,a2,a3,a4,a5)	LD_ARGS_5(a0,a1,a2,a3,a4); LD_ARG(9,a5)

#define ASM_ARGS_0			"r"(_a2)
#define ASM_ARGS_1			ASM_ARGS_0, "r"(_a6)
#define ASM_ARGS_2			ASM_ARGS_1, "r"(_a3)
#define ASM_ARGS_3			ASM_ARGS_2, "r"(_a4)
#define ASM_ARGS_4			ASM_ARGS_3, "r"(_a5)
#define ASM_ARGS_5			ASM_ARGS_4, "r"(_a8)
#define ASM_ARGS_6			ASM_ARGS_5, "r"(_a9)

/* Define a macro which expands into the inline wrapper code for a system
   call.  */

#define INTERNAL_SYSCALL_NCS(name, err, nr, args...)			      \
(__extension__ \
  ({ LD_ARG(2, name);							      \
     LD_ARGS_##nr(args);						      \
     __asm__ __volatile__ ("syscall\n" 						      \
	 	   : "=a" (_a2)						      \
	 	   : ASM_ARGS_##nr					      \
	 	   : "memory");						      \
     (long) _a2;							      \
   }) \
)
#endif /* not __ASSEMBLER__ */
#endif /* _BITS_SYSCALLS_H */
