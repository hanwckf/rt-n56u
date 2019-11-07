#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

#ifndef __ASSEMBLER__

#define __SYSCALL_STRING \
	"t 0x10\n\t" \
	"bcc 1f\n\t" \
	"mov %%o0, %0\n\t" \
	"sub %%g0, %%o0, %0\n\t" \
	"1:\n\t"

#define __SYSCALL_CLOBBERS "cc", "memory"

#ifndef NOT_IN_libc
#define DEBUG_SYSCALL(name) { \
      char d[64];\
      write( 2, d, snprintf( d, 64, "syscall %d error %d\n", __NR_##name, _inline_sys_result)); \
}
#else
#define DEBUG_SYSCALL(name) do{} while(0)
#endif

#define INTERNAL_SYSCALL_NCS(sys_num, err, nr, args...) \
(__extension__ \
    ({                                                      \
        unsigned int __res;                                 \
        {                                                   \
            register long __o0 __asm__("o0");               \
            register long __g1 __asm__("g1") = sys_num;     \
            LOAD_ARGS_##nr(args)                            \
            __asm__ __volatile__( __SYSCALL_STRING          \
                : "=r" (__res), "=r" (__o0)                \
                : "1" (__o0) ASM_ARGS_##nr, "r" (__g1)     \
                : __SYSCALL_CLOBBERS );                                   \
        }                                                   \
        (int)__res;                                         \
     }) \
)
#define INTERNAL_SYSCALL_ERROR_P(val, err) \
  ((unsigned int) (val) >= 0xfffff001u)

# define CALL_ERRNO_LOCATION "call   __errno_location;"
#define __CLONE_SYSCALL_STRING						\
	"ta	0x10;"							\
	"bcs	2f;"							\
	" sub	%%o1, 1, %%o1;"						\
	"and	%%o0, %%o1, %%o0;"					\
	"1:"								\
	".subsection 2;"						\
	"2:"								\
	"save	%%sp, -192, %%sp;"					\
	CALL_ERRNO_LOCATION						\
	" nop;"								\
	"st	%%i0, [%%o0];"						\
	"ba	1b;"							\
	" restore %%g0, -1, %%o0;"					\
	".previous;"

#define INLINE_CLONE_SYSCALL(arg1,arg2,arg3,arg4,arg5)			\
({									\
	register long __o0 __asm__ ("o0") = (long)(arg1);		\
	register long __o1 __asm__ ("o1") = (long)(arg2);		\
	register long __o2 __asm__ ("o2") = (long)(arg3);		\
	register long __o3 __asm__ ("o3") = (long)(arg4);		\
	register long __o4 __asm__ ("o4") = (long)(arg5);		\
	register long __g1 __asm__ ("g1") = __NR_clone;			\
	__asm__ __volatile__ (__CLONE_SYSCALL_STRING :			\
			  "=r" (__g1), "=r" (__o0), "=r" (__o1)	:	\
			  "0" (__g1), "1" (__o0), "2" (__o1),		\
			  "r" (__o2), "r" (__o3), "r" (__o4) :		\
			  __SYSCALL_CLOBBERS);				\
	__o0;								\
})

#define LOAD_ARGS_0()
#define ASM_ARGS_0
#define LOAD_ARGS_1(o0) \
    __o0 = (int)o0;     \
    LOAD_ARGS_0()
#define ASM_ARGS_1 ASM_ARGS_0, "r" (__o0)
#define LOAD_ARGS_2(o0, o1)			\
  register int __o1 __asm__ ("o1") = (int) (o1);	\
  LOAD_ARGS_1 (o0)
#define ASM_ARGS_2	ASM_ARGS_1, "r" (__o1)
#define LOAD_ARGS_3(o0, o1, o2)			\
  register int __o2 __asm__ ("o2") = (int) (o2);	\
  LOAD_ARGS_2 (o0, o1)
#define ASM_ARGS_3	ASM_ARGS_2, "r" (__o2)
#define LOAD_ARGS_4(o0, o1, o2, o3)		\
  register int __o3 __asm__ ("o3") = (int) (o3);	\
  LOAD_ARGS_3 (o0, o1, o2)
#define ASM_ARGS_4	ASM_ARGS_3, "r" (__o3)
#define LOAD_ARGS_5(o0, o1, o2, o3, o4)		\
  register int __o4 __asm__ ("o4") = (int) (o4);	\
  LOAD_ARGS_4 (o0, o1, o2, o3)
#define ASM_ARGS_5	ASM_ARGS_4, "r" (__o4)
#define LOAD_ARGS_6(o0, o1, o2, o3, o4, o5)	\
  register int __o5 __asm__ ("o5") = (int) (o5);	\
  LOAD_ARGS_5 (o0, o1, o2, o3, o4)
#define ASM_ARGS_6	ASM_ARGS_5, "r" (__o5)


#endif /* __ASSEMBLER__ */
#endif /* _BITS_SYSCALLS_H */
