#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

#ifndef __ASSEMBLER__

#include <errno.h>

#define ASMFMT_0
#define ASMFMT_1 , "g"(er1)
#define ASMFMT_2 , "g"(er1), "g"(er2)
#define ASMFMT_3 , "g"(er1), "g"(er2), "g"(er3)
#define ASMFMT_4 , "g"(er1), "g"(er2), "g"(er3), "g"(er4)
#define ASMFMT_5 , "g"(er1), "g"(er2), "g"(er3), "g"(er4), "g"(er5)
#define ASMFMT_6 , "g"(er1), "g"(er2), "g"(er3), "g"(er4), "m"(er5), "m"(er6)

#define SUBSTITUTE_ARGS_0() do {} while(0);
#define SUBSTITUTE_ARGS_1(arg1)			\
	register long int er1 __asm__("er1") = (long int)arg1;
#define SUBSTITUTE_ARGS_2(arg1, arg2)		\
	register long int er1 __asm__("er1") = (long int)arg1; \
	register long int er2 __asm__("er2") = (long int)arg2;
#define SUBSTITUTE_ARGS_3(arg1, arg2, arg3)	\
	register long int er1 __asm__("er1") = (long int)arg1; \
	register long int er2 __asm__("er2") = (long int)arg2; \
	register long int er3 __asm__("er3") = (long int)arg3;
#define SUBSTITUTE_ARGS_4(arg1, arg2, arg3, arg4)	     \
	register long int er1 __asm__("er1") = (long int)arg1; \
	register long int er2 __asm__("er2") = (long int)arg2; \
	register long int er3 __asm__("er3") = (long int)arg3; \
	register long int er4 __asm__("er4") = (long int)arg4;
#define SUBSTITUTE_ARGS_5(arg1, arg2, arg3, arg4, arg5)   \
	register long int er1 __asm__("er1") = (long int)arg1; \
	register long int er2 __asm__("er2") = (long int)arg2; \
	register long int er3 __asm__("er3") = (long int)arg3; \
	register long int er4 __asm__("er4") = (long int)arg4; \
	register long int er5 __asm__("er5") = (long int)arg5;
#define SUBSTITUTE_ARGS_6(arg1, arg2, arg3, arg4, arg5, arg6)	\
	register long int er1 __asm__("er1") = (long int)arg1; \
	register long int er2 __asm__("er2") = (long int)arg2; \
	register long int er3 __asm__("er3") = (long int)arg3; \
	register long int er4 __asm__("er4") = (long int)arg4; \
	long int er5 = (long int)arg5; \
	long int er6 = (long int)arg6;

#define LOAD_ARGS_0
#define LOAD_ARGS_1
#define LOAD_ARGS_2
#define LOAD_ARGS_3
#define LOAD_ARGS_4
#define LOAD_ARGS_5
#define LOAD_ARGS_6	"mov.l er5,@-sp\n\tmov.l %6,er5\n\t" \
			"mov.l er6,@-sp\n\tmov.l %7,er6\n\t"

#define RESTORE_ARGS_0
#define RESTORE_ARGS_1
#define RESTORE_ARGS_2
#define RESTORE_ARGS_3
#define RESTORE_ARGS_4
#define RESTORE_ARGS_5
#define RESTORE_ARGS_6	"mov.l @sp+,er6\n\tmov.l @sp+,er5"

/* The _NCS variant allows non-constant syscall numbers.  */
#define INTERNAL_SYSCALL_NCS(name, err, nr, args...) \
(__extension__ \
  ({							\
    register long int er0 __asm__ ("er0");		\
    SUBSTITUTE_ARGS_##nr(args)				\
    __asm__ __volatile__ ( 				\
		LOAD_ARGS_##nr				\
		"mov.l %1,er0\n\t"			\
		"trapa #0\n\t"				\
		RESTORE_ARGS_##nr			\
		: "=r" (er0)				\
		: "ir" (name) ASMFMT_##nr		\
		: "memory"				\
    );							\
    (int) er0;						\
   }) \
)

#endif /* __ASSEMBLER__ */
#endif /* _BITS_SYSCALLS_H */
