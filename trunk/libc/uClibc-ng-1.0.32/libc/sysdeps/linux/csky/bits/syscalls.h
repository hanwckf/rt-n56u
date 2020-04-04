/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

#undef __NR_iopl
#undef __NR_vm86

/*
   Some of the sneaky macros in the code were taken from
   glibc-2.3.2/sysdeps/unix/sysv/linux/arm/sysdep.h
*/

//#include <errno.h>

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

#undef INTERNAL_SYSCALL_NCS
#ifndef __cskyabiv2__
#define INTERNAL_SYSCALL_NCS(name, err, nr, args...)                        \
  ({unsigned int __sys_result;                                          \
     {                                                                  \
       register int _a1 __asm__ ("a0"), _nr __asm__ ("r1");             \
       LOAD_ARGS_##nr (args)                                            \
       _nr = (name);                                             \
       __asm__ __volatile__ ("trap  0 \n\t"                             \
                             : "=r" (_a1)                               \
                             : "r" (_nr) ASM_ARGS_##nr                  \
                             : "memory");                               \
               __sys_result = _a1;                                      \
     }                                                                  \
     (int) __sys_result; })

#else /* __cskyabiv2__ */
#define INTERNAL_SYSCALL_NCS(name, err, nr, args...)                        \
  ({unsigned int __sys_result;                                          \
     {                                                                  \
       register int _a1 __asm__ ("a0"), _nr __asm__ ("r7");             \
       LOAD_ARGS_##nr (args)                                            \
       _nr = (name);                                             \
       __asm__ __volatile__ ("trap  0    \n\t"                          \
                             : "=r" (_a1)                               \
                             : "r" (_nr) ASM_ARGS_##nr                  \
                             : "memory");                               \
               __sys_result = _a1;                                      \
     }                                                                  \
     (int) __sys_result; })
#endif /* __ABI_CSKY_V2__ */

#define INTERNAL_SYSCALL_ERROR_P(val, err) \
  ((unsigned int) (val) >= 0xfffff001u)

#define LOAD_ARGS_0()
#define ASM_ARGS_0
#define LOAD_ARGS_1(a1)                         \
  _a1 = (int) (a1);                             \
  LOAD_ARGS_0 ()
#define ASM_ARGS_1      ASM_ARGS_0, "r" (_a1)
#define LOAD_ARGS_2(a1, a2)                     \
  register int _a2 __asm__ ("a1") = (int) (a2); \
  LOAD_ARGS_1 (a1)
#define ASM_ARGS_2      ASM_ARGS_1, "r" (_a2)
#define LOAD_ARGS_3(a1, a2, a3)                 \
  register int _a3 __asm__ ("a2") = (int) (a3); \
  LOAD_ARGS_2 (a1, a2)
#define ASM_ARGS_3      ASM_ARGS_2, "r" (_a3)
#define LOAD_ARGS_4(a1, a2, a3, a4)             \
  register int _a4 __asm__ ("a3") = (int) (a4); \
  LOAD_ARGS_3 (a1, a2, a3)
#define ASM_ARGS_4      ASM_ARGS_3, "r" (_a4)

#ifndef __cskyabiv2__
#define LOAD_ARGS_5(a1, a2, a3, a4, a5)         \
  register int _v1 __asm__ ("a4") = (int) (a5); \
  LOAD_ARGS_4 (a1, a2, a3, a4)
#define ASM_ARGS_5      ASM_ARGS_4, "r" (_v1)
#define LOAD_ARGS_6(a1, a2, a3, a4, a5, a6)     \
  register int _v2 __asm__ ("a5") = (int) (a6); \
  LOAD_ARGS_5 (a1, a2, a3, a4, a5)
#define ASM_ARGS_6      ASM_ARGS_5, "r" (_v2)
#define LOAD_ARGS_7(a1, a2, a3, a4, a5, a6, a7) \
  register int _v3 __asm__ ("r8") = (int) (a7); \
  LOAD_ARGS_6 (a1, a2, a3, a4, a5, a6)
#define ASM_ARGS_7      ASM_ARGS_6, "r" (_v3)
#else /* __cskyabiv2__ */
#define LOAD_ARGS_5(a1, a2, a3, a4, a5)         \
  register int _v1 __asm__ ("l0") = (int) (a5); \
  LOAD_ARGS_4 (a1, a2, a3, a4)
#define ASM_ARGS_5      ASM_ARGS_4, "r" (_v1)
#define LOAD_ARGS_6(a1, a2, a3, a4, a5, a6)     \
  register int _v2 __asm__ ("l1") = (int) (a6); \
  LOAD_ARGS_5 (a1, a2, a3, a4, a5)
#define ASM_ARGS_6      ASM_ARGS_5, "r" (_v2)
#define LOAD_ARGS_7(a1, a2, a3, a4, a5, a6, a7) \
  register int _v3 __asm__ ("l2") = (int) (a7); \
  LOAD_ARGS_6 (a1, a2, a3, a4, a5, a6)
#define ASM_ARGS_7      ASM_ARGS_6, "r" (_v3)
#endif /* __ABI_CSKY_V2__ */


#endif /* _BITS_SYSCALLS_H */
