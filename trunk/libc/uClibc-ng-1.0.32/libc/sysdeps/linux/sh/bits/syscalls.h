#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

/* The Linux kernel uses different trap numbers on sh-2.  */
#if defined __sh2__ || defined __SH2A__
# define __SH_SYSCALL_TRAP_BASE 0x20
#else
# define __SH_SYSCALL_TRAP_BASE 0x10
#endif

#ifndef __ASSEMBLER__

#include <errno.h>

#define SYSCALL_INST_STR(x)	"trapa #"__stringify(__SH_SYSCALL_TRAP_BASE + x)"\n\t"
#define SYSCALL_INST_STR0	SYSCALL_INST_STR(0)
#define SYSCALL_INST_STR1	SYSCALL_INST_STR(1)
#define SYSCALL_INST_STR2	SYSCALL_INST_STR(2)
#define SYSCALL_INST_STR3	SYSCALL_INST_STR(3)
#define SYSCALL_INST_STR4	SYSCALL_INST_STR(4)
#define SYSCALL_INST_STR5	SYSCALL_INST_STR(5)
#define SYSCALL_INST_STR6	SYSCALL_INST_STR(6)

# ifdef NEED_SYSCALL_INST_PAD
#  define SYSCALL_INST_PAD "\
	or r0,r0; or r0,r0; or r0,r0; or r0,r0; or r0,r0"
# else
#  define SYSCALL_INST_PAD
# endif

#define ASMFMT_0
#define ASMFMT_1 \
	, "r" (r4)
#define ASMFMT_2 \
	, "r" (r4), "r" (r5)
#define ASMFMT_3 \
	, "r" (r4), "r" (r5), "r" (r6)
#define ASMFMT_4 \
	, "r" (r4), "r" (r5), "r" (r6), "r" (r7)
#define ASMFMT_5 \
	, "r" (r4), "r" (r5), "r" (r6), "r" (r7), "0" (r0)
#define ASMFMT_6 \
	, "r" (r4), "r" (r5), "r" (r6), "r" (r7), "0" (r0), "r" (r1)
#define ASMFMT_7 \
	, "r" (r4), "r" (r5), "r" (r6), "r" (r7), "0" (r0), "r" (r1), "r" (r2)

#define SUBSTITUTE_ARGS_0()
#define SUBSTITUTE_ARGS_1(arg1) \
	long int _arg1 = (long int) (arg1);				      \
	register long int r4 __asm__ ("%r4") = (long int) (_arg1)
#define SUBSTITUTE_ARGS_2(arg1, arg2) \
	long int _arg1 = (long int) (arg1);				      \
	long int _arg2 = (long int) (arg2);				      \
	register long int r4 __asm__ ("%r4") = (long int) (_arg1);		      \
	register long int r5 __asm__ ("%r5") = (long int) (_arg2)
#define SUBSTITUTE_ARGS_3(arg1, arg2, arg3) \
	long int _arg1 = (long int) (arg1);				      \
	long int _arg2 = (long int) (arg2);				      \
	long int _arg3 = (long int) (arg3);				      \
	register long int r4 __asm__ ("%r4") = (long int) (_arg1);		      \
	register long int r5 __asm__ ("%r5") = (long int) (_arg2);		      \
	register long int r6 __asm__ ("%r6") = (long int) (_arg3)
#define SUBSTITUTE_ARGS_4(arg1, arg2, arg3, arg4) \
	long int _arg1 = (long int) (arg1);				      \
	long int _arg2 = (long int) (arg2);				      \
	long int _arg3 = (long int) (arg3);				      \
	long int _arg4 = (long int) (arg4);				      \
	register long int r4 __asm__ ("%r4") = (long int) (_arg1);		      \
	register long int r5 __asm__ ("%r5") = (long int) (_arg2);		      \
	register long int r6 __asm__ ("%r6") = (long int) (_arg3);		      \
	register long int r7 __asm__ ("%r7") = (long int) (_arg4)
#define SUBSTITUTE_ARGS_5(arg1, arg2, arg3, arg4, arg5) \
	long int _arg1 = (long int) (arg1);				      \
	long int _arg2 = (long int) (arg2);				      \
	long int _arg3 = (long int) (arg3);				      \
	long int _arg4 = (long int) (arg4);				      \
	long int _arg5 = (long int) (arg5);				      \
	register long int r4 __asm__ ("%r4") = (long int) (_arg1);		      \
	register long int r5 __asm__ ("%r5") = (long int) (_arg2);		      \
	register long int r6 __asm__ ("%r6") = (long int) (_arg3);		      \
	register long int r7 __asm__ ("%r7") = (long int) (_arg4);		      \
	register long int r0 __asm__ ("%r0") = (long int) (_arg5)
#define SUBSTITUTE_ARGS_6(arg1, arg2, arg3, arg4, arg5, arg6) \
	long int _arg1 = (long int) (arg1);				      \
	long int _arg2 = (long int) (arg2);				      \
	long int _arg3 = (long int) (arg3);				      \
	long int _arg4 = (long int) (arg4);				      \
	long int _arg5 = (long int) (arg5);				      \
	long int _arg6 = (long int) (arg6);				      \
	register long int r4 __asm__ ("%r4") = (long int)(_arg1);		      \
	register long int r5 __asm__ ("%r5") = (long int) (_arg2);		      \
	register long int r6 __asm__ ("%r6") = (long int) (_arg3);		      \
	register long int r7 __asm__ ("%r7") = (long int) (_arg4);		      \
	register long int r0 __asm__ ("%r0") = (long int) (_arg5);		      \
	register long int r1 __asm__ ("%r1") = (long int) (_arg6)
#define SUBSTITUTE_ARGS_7(arg1, arg2, arg3, arg4, arg5, arg6, arg7) \
	long int _arg1 = (long int) (arg1);				      \
	long int _arg2 = (long int) (arg2);				      \
	long int _arg3 = (long int) (arg3);				      \
	long int _arg4 = (long int) (arg4);				      \
	long int _arg5 = (long int) (arg5);				      \
	long int _arg6 = (long int) (arg6);				      \
	long int _arg7 = (long int) (arg7);				      \
	register long int r4 __asm__ ("%r4") = (long int) (_arg1);		      \
	register long int r5 __asm__ ("%r5") = (long int) (_arg2);		      \
	register long int r6 __asm__ ("%r6") = (long int) (_arg3);		      \
	register long int r7 __asm__ ("%r7") = (long int) (_arg4);		      \
	register long int r0 __asm__ ("%r0") = (long int) (_arg5);		      \
	register long int r1 __asm__ ("%r1") = (long int) (_arg6);		      \
	register long int r2 __asm__ ("%r2") = (long int) (_arg7)

/* The _NCS variant allows non-constant syscall numbers.  */
#define INTERNAL_SYSCALL_NCS(name, err, nr, args...) \
(__extension__ \
  ({															\
    unsigned long int resultvar;								\
    register long int r3 __asm__ ("%r3") = (name);				\
    SUBSTITUTE_ARGS_##nr(args);									\
    __asm__ __volatile__ (SYSCALL_INST_STR##nr SYSCALL_INST_PAD	\
		  : "=z" (resultvar)									\
		  : "r" (r3) ASMFMT_##nr								\
		  : "memory", "t"											\
    );															\
    (int) resultvar;											\
   }) \
)
#define INTERNAL_SYSCALL_ERROR_P(val, err) \
  ((unsigned int) (val) >= 0xfffff001u)

#endif /* __ASSEMBLER__ */
#endif /* _BITS_SYSCALLS_H */

