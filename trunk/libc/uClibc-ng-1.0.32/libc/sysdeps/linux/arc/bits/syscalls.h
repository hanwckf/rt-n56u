/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 *
 */
#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
#error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

#ifndef __ASSEMBLER__

#include <errno.h>

/*
 * Fine tuned code for errno handling in syscall wrappers.
 *
 * 1. __syscall_error(raw_syscall_ret_val) is used to set the errno (vs.
 *    the typical __set_errno). This helps elide the generated code for
 *    GOT fetch for __errno_location pointer etc, in each wrapper.
 *
 * 2. The call to above is also disguised in inline asm. This elides
 *    unconditional save/restore of a few callee regs which gcc almost
 *    always generates if the call is exposed
 *
 * 3. The function can't be hidden because wrappers from librt et all also
 *    call it. However hidden is not really needed to bypass PLT for
 *    intra-libc calls as the branch insn w/o @plt is sufficient.
 */

#ifdef IS_IN_rtld
/* ldso doesn't have real errno */
#define ERRNO_ERRANDS(_sys_result)
#else /* !IS_IN_rtld */
extern int __syscall_error (int);
#ifndef IS_IN_libc
/* Inter-libc callers use PLT */
#define CALL_ERRNO_SETTER   "bl   __syscall_error@plt    \n\t"
#else
/* intra-libc callers, despite PIC can bypass PLT */
#define CALL_ERRNO_SETTER   "bl   __syscall_error    \n\t"
#endif

#define ERRNO_ERRANDS(_sys_result)          \
        __asm__ volatile (                  \
        "st.a blink, [sp, -4] \n\t"         \
        CALL_ERRNO_SETTER                   \
        "ld.ab blink, [sp, 4] \n\t"         \
        :"+r" (_sys_result)                 \
        :                                   \
        :"r1","r2","r3","r4","r5","r6",     \
         "r7","r8","r9","r10","r11","r12"   \
        );

#endif /* IS_IN_rtld */

/* -1 to -1023 as valid error values will suffice for some time */
#define INTERNAL_SYSCALL_ERROR_P(val, err)		\
	((unsigned int) (val) > (unsigned int) -1024)

/*
 * Standard sycall wrapper
 *  -Gets syscall name (conv to __NR_xxx)
 *  -sets errno, return success/error-codes
 */
#define INLINE_SYSCALL(name, nr_args, args...)				\
({									\
	register int __res __asm__("r0");				\
	__res = INTERNAL_SYSCALL_NCS(__NR_##name, , nr_args, args);	\
	if (__builtin_expect (INTERNAL_SYSCALL_ERROR_P ((__res), ), 0))	\
	{								\
		ERRNO_ERRANDS(__res);					\
	}								\
	__res;								\
})

/* variant of INLINE_SYSCALL, gets syscall number
 */
#define INLINE_SYSCALL_NCS(num, nr_args, args...)			\
({									\
	register int __res __asm__("r0");				\
	__res = INTERNAL_SYSCALL_NCS(num, , nr_args, args);		\
	if (__builtin_expect (INTERNAL_SYSCALL_ERROR_P ((__res), ), 0))	\
	{								\
		ERRNO_ERRANDS(__res);					\
	}								\
	__res;								\
})

/*-------------------------------------------------------------------------
 * Mechanics of Trap - specific to ARC700
 *
 * Note the memory clobber is not strictly needed for intended semantics of
 * the inline asm. However some of the cases, such as old-style 6 arg mmap
 * gcc was generating code for inline syscall ahead of buffer packing needed
 * for syscall itself.
 *-------------------------------------------------------------------------*/

#ifdef __A7__
#define ARC_TRAP_INSN	"trap0		\n\t"
#elif defined(__HS__)
#define ARC_TRAP_INSN	"trap_s 0	\n\t"
#endif

#define INTERNAL_SYSCALL_NCS(nm, err, nr_args, args...)	\
({							\
	/* Per ABI, r0 is 1st arg and return reg */	\
	register int __ret __asm__("r0");		\
	register int _sys_num __asm__("r8");		\
							\
	LOAD_ARGS_##nr_args (nm, args)			\
							\
        __asm__ volatile (				\
		ARC_TRAP_INSN				\
		: "+r" (__ret)				\
		: "r"(_sys_num) ASM_ARGS_##nr_args	\
		: "memory");				\
                                                        \
	__ret;						\
})

/* Macros for setting up inline __asm__ input regs */
#define ASM_ARGS_0
#define ASM_ARGS_1	ASM_ARGS_0, "r" (__ret)
#define ASM_ARGS_2	ASM_ARGS_1, "r" (_arg2)
#define ASM_ARGS_3	ASM_ARGS_2, "r" (_arg3)
#define ASM_ARGS_4	ASM_ARGS_3, "r" (_arg4)
#define ASM_ARGS_5	ASM_ARGS_4, "r" (_arg5)
#define ASM_ARGS_6	ASM_ARGS_5, "r" (_arg6)
#define ASM_ARGS_7	ASM_ARGS_6, "r" (_arg7)

/* Macros for converting sys-call wrapper args into sys call args */
#define LOAD_ARGS_0(nm, arg)					\
	_sys_num = (int) (nm);					\

#define LOAD_ARGS_1(nm, arg1) 					\
	__ret = (int) (arg1);					\
	LOAD_ARGS_0 (nm, arg1)

/*
 * Note that the use of _tmpX might look superflous, however it is needed
 * to ensure that register variables are not clobbered if arg happens to be
 * a function call itself. e.g. sched_setaffinity() calling getpid() for arg2
 *
 * Also this specific order of recursive calling is important to segregate
 * the tmp args evaluation (function call case described above) and assigment
 * of register variables
 */
#define LOAD_ARGS_2(nm, arg1, arg2)				\
	int _tmp2 = (int) (arg2);				\
	LOAD_ARGS_1 (nm, arg1)					\
	register int _arg2 __asm__ ("r1") = _tmp2;

#define LOAD_ARGS_3(nm, arg1, arg2, arg3)			\
	int _tmp3 = (int) (arg3);				\
	LOAD_ARGS_2 (nm, arg1, arg2)				\
	register int _arg3 __asm__ ("r2") = _tmp3;

#define LOAD_ARGS_4(nm, arg1, arg2, arg3, arg4)			\
	int _tmp4 = (int) (arg4);				\
	LOAD_ARGS_3 (nm, arg1, arg2, arg3)			\
	register int _arg4 __asm__ ("r3") = _tmp4;

#define LOAD_ARGS_5(nm, arg1, arg2, arg3, arg4, arg5)		\
	int _tmp5 = (int) (arg5);				\
	LOAD_ARGS_4 (nm, arg1, arg2, arg3, arg4)		\
	register int _arg5 __asm__ ("r4") = _tmp5;

#define LOAD_ARGS_6(nm,  arg1, arg2, arg3, arg4, arg5, arg6)	\
	int _tmp6 = (int) (arg6);				\
	LOAD_ARGS_5 (nm, arg1, arg2, arg3, arg4, arg5)		\
	register int _arg6 __asm__ ("r5") = _tmp6;

#define LOAD_ARGS_7(nm, arg1, arg2, arg3, arg4, arg5, arg6, arg7)\
	int _tmp7 = (int) (arg7);				\
	LOAD_ARGS_6 (nm, arg1, arg2, arg3, arg4, arg5, arg6)	\
	register int _arg7 __asm__ ("r6") = _tmp7;

#else

#ifdef __A7__
#define ARC_TRAP_INSN	trap0
#elif defined(__HS__)
#define ARC_TRAP_INSN	trap_s 0
#endif

#endif /* __ASSEMBLER__ */

#endif /* _BITS_SYSCALLS_H */
