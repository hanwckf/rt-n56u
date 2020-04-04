#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

#ifndef __ASSEMBLER__

#include <errno.h>

#define SYS_ify(syscall_name)  (__NR_##syscall_name)

#define INLINE_SYSCALL_NCS(name, nr, args...)				\
(__extension__								\
  ({									\
     unsigned int _inline_sys_result = INTERNAL_SYSCALL_NCS (name, , nr, args);\
     if (unlikely (INTERNAL_SYSCALL_ERROR_P (_inline_sys_result, )))	\
       {								\
	 __set_errno (INTERNAL_SYSCALL_ERRNO (_inline_sys_result, ));	\
	 _inline_sys_result = (unsigned int) -1;			\
       }								\
     (int) _inline_sys_result;						\
   })									\
)

#define INTERNAL_SYSCALL_NCS(name, err, nr, args...)			\
(__extension__ \
  ({unsigned int __internal_sys_result;					\
     {									\
       register int __a1 __asm__ ("gr8");				\
       register int _nr __asm__ ("gr7");				\
       LOAD_ARGS_##nr (args)						\
       _nr = (name);							\
       __asm__ __volatile__ ("tra      gr0,gr0"				\
			     : "=r" (__a1)				\
			     : "r" (_nr) ASM_ARGS_##nr			\
			     : "memory");				\
	       __internal_sys_result = __a1;				\
     }									\
     (int) __internal_sys_result; }) \
)

#define INTERNAL_SYSCALL_ERROR_P(val, err) \
  ((unsigned int) (val) >= 0xfffff001u)

#define LOAD_ARGS_0()
#define ASM_ARGS_0
#define LOAD_ARGS_1(a1)				\
  int __a1tmp = (int) (a1);			\
  LOAD_ARGS_0 ()				\
  __a1 = __a1tmp;
#define ASM_ARGS_1	ASM_ARGS_0, "r" (__a1)
#define LOAD_ARGS_2(a1, a2)			\
  int __a2tmp = (int) (a2);			\
  LOAD_ARGS_1 (a1)				\
  register int __a2 __asm__ ("gr8") = __a2tmp;
#define ASM_ARGS_2	ASM_ARGS_1, "r" (__a2)
#define LOAD_ARGS_3(a1, a2, a3)			\
  int __a3tmp = (int) (a3);			\
  LOAD_ARGS_2 (a1, a2)				\
  register int __a3 __asm__ ("gr9") = __a3tmp;
#define ASM_ARGS_3	ASM_ARGS_2, "r" (__a3)
#define LOAD_ARGS_4(a1, a2, a3, a4)		\
  int __a4tmp = (int) (a4);			\
  LOAD_ARGS_3 (a1, a2, a3)			\
  register int __a4 __asm__ ("gr10") = __a4tmp;
#define ASM_ARGS_4	ASM_ARGS_3, "r" (__a4)
#define LOAD_ARGS_5(a1, a2, a3, a4, a5)		\
  int __v1tmp = (int) (a5);			\
  LOAD_ARGS_4 (a1, a2, a3, a4)			\
  register int __v1 __asm__ ("gr11") = __v1tmp;
#define ASM_ARGS_5	ASM_ARGS_4, "r" (__v1)
#define LOAD_ARGS_6(a1, a2, a3, a4, a5, a6)	\
  int __v2tmp = (int) (a6);			\
  LOAD_ARGS_5 (a1, a2, a3, a4, a5)		\
  register int __v2 __asm__ ("gr12") = __v2tmp;
#define ASM_ARGS_6	ASM_ARGS_5, "r" (__v2)

#endif /* __ASSEMBLER__ */
#endif /* _BITS_SYSCALLS_H */
