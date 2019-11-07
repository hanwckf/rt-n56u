#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

#ifndef __ASSEMBLER__

#define INTERNAL_SYSCALL_NCS(name, err, nr, args...)	\
(__extension__ \
 ({							\
	long __res;					\
	__asm__ __volatile__ (				\
		"excpt 0;\n\t"				\
		: "=q0" (__res)				\
		: "qA"  (name) ASMFMT_##nr(args)	\
		: "memory","CC");			\
	__res;						\
  }) \
)
#define ASMFMT_0()

#define ASMFMT_1(arg1) \
	, "q0" ((long)(arg1))

#define ASMFMT_2(arg1, arg2) \
	ASMFMT_1(arg1) \
	, "q1" ((long)(arg2))

#define ASMFMT_3(arg1, arg2, arg3) \
	ASMFMT_2(arg1, arg2) \
	, "q2" ((long)(arg3))

#define ASMFMT_4(arg1, arg2, arg3, arg4) \
	ASMFMT_3(arg1, arg2, arg3) \
	, "q3" ((long)(arg4))

#define ASMFMT_5(arg1, arg2, arg3, arg4, arg5) \
	ASMFMT_4(arg1, arg2, arg3, arg4) \
	, "q4" ((long)(arg5))

#define ASMFMT_6(arg1, arg2, arg3, arg4, arg5, arg6) \
	ASMFMT_5(arg1, arg2, arg3, arg4, arg5) \
	, "q5" ((long)(arg6))

#endif /* __ASSEMBLER__ */
#endif /* _BITS_SYSCALLS_H */
