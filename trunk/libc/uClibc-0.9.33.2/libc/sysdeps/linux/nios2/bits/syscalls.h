#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

#ifndef __ASSEMBLER__

#include <errno.h>
#include <asm/traps.h>

#define __syscall_return(type, res)					\
	do {								\
		if (unlikely(INTERNAL_SYSCALL_ERROR_P(res, ))) {	\
			__set_errno(INTERNAL_SYSCALL_ERRNO(res, ));	\
			res = (unsigned long) -1;			\
		}							\
		return (type) (res);					\
	} while (0)

#define INTERNAL_SYSCALL_NCS(name, err, nr, args...)			\
(__extension__ \
	({								\
		long __res;						\
		__asm__ __volatile__ (					\
			"movi    r2,    %2\n\t"   /* TRAP_ID_SYSCALL */ \
			"movi    r3,    %1\n\t"   /* __NR_##name     */	\
			ASM_ARGS_##nr					\
			"trap\n\t"					\
			"mov     %0,    r2\n\t"   /* syscall return  */	\
			:   "=r" (__res)          /* %0              */	\
			:   "i" (name)            /* %1              */	\
			  , "i" (TRAP_ID_SYSCALL) /* %2              */	\
			  MAP_ARGS_##nr (args)    /* %3-%8           */	\
			:   "r2"					\
			  , "r3"					\
			  CLOB_ARGS_##nr          /* Clobbered       */ \
		);							\
		__res;							\
	}) \
)

#define INTERNAL_SYSCALL_ERROR_P(val, err)	\
	((unsigned long)(val) >= (unsigned long)(-125))

#define ASM_ARGS_0
#define MAP_ARGS_0()
#define CLOB_ARGS_0

#define ASM_ARGS_1		\
	"mov     r4,    %3\n\t"
#define MAP_ARGS_1(a)		\
	, "r" ((long) a)
#define CLOB_ARGS_1		\
	, "r4"

#define ASM_ARGS_2		\
	ASM_ARGS_1		\
	"mov     r5,    %4\n\t"
#define MAP_ARGS_2(a, b)	\
	MAP_ARGS_1(a)		\
	, "r" ((long) b)
#define CLOB_ARGS_2		\
	CLOB_ARGS_1		\
	, "r5"

#define ASM_ARGS_3		\
	ASM_ARGS_2		\
	"mov     r6,    %5\n\t"
#define MAP_ARGS_3(a, b, c)	\
	MAP_ARGS_2(a, b)	\
	, "r" ((long) c)
#define CLOB_ARGS_3		\
	CLOB_ARGS_2		\
	, "r6"

#define ASM_ARGS_4		\
	ASM_ARGS_3		\
	"mov     r7,    %6\n\t"
#define MAP_ARGS_4(a, b, c, d)	\
	MAP_ARGS_3(a, b, c)	\
	, "r" ((long) d)
#define CLOB_ARGS_4		\
	CLOB_ARGS_3		\
	, "r7"

#define ASM_ARGS_5		\
	ASM_ARGS_4		\
	"mov     r8,    %7\n\t"
#define MAP_ARGS_5(a, b, c, d, e)	\
	MAP_ARGS_4(a, b, c, d)		\
	, "r" ((long) e)
#define CLOB_ARGS_5		\
	CLOB_ARGS_4		\
	, "r8"

#define ASM_ARGS_6		\
	ASM_ARGS_5		\
	"mov     r9,    %8\n\t"
#define MAP_ARGS_6(a, b, c, d, e, f)	\
	MAP_ARGS_5(a, b, c, d, e)	\
	, "r" ((long) f)
#define CLOB_ARGS_6		\
	CLOB_ARGS_5		\
	, "r9"

#endif /* __ASSEMBLER__ */
#endif /* _BITS_SYSCALLS_H */
