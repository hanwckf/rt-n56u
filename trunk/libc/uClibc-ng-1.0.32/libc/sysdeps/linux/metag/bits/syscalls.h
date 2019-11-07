/*
 * Copyright (C) 2013 Imagination Technologies Ltd.
 *
 * Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 *
 */

#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

/*
  Meta version adapted from the ARM version.
*/

#define SYS_ify(syscall_name)  (__NR_##syscall_name)

#ifdef __ASSEMBLER__

/* Call a given syscall, with arguments loaded. */
#undef DO_CALL
#define DO_CALL(syscall_name, args)		\
  MOV D1Re0, #SYS_ify (syscall_name);		\
  SWITCH #0x440001

#else

#include <errno.h>

#undef INLINE_SYSCALL_NCS
#define INLINE_SYSCALL_NCS(name, nr, args...)				\
(__extension__                                                          \
  ({ unsigned int _inline_sys_result = INTERNAL_SYSCALL_NCS (name, , nr, args); \
    if (unlikely(INTERNAL_SYSCALL_ERROR_P (_inline_sys_result, )))	\
      {									\
	__set_errno (INTERNAL_SYSCALL_ERRNO (_inline_sys_result, ));	\
	_inline_sys_result = (unsigned int) -1;				\
      }									\
    (int) _inline_sys_result; })                                        \
)

#undef INTERNAL_SYSCALL_DECL
#define INTERNAL_SYSCALL_DECL(err) do { } while (0)

#undef INTERNAL_SYSCALL_NCS
#define INTERNAL_SYSCALL_NCS(name, err, nr, args...)			\
(__extension__                                                          \
  ({unsigned int __sys_result;						\
    {									\
      PREP_ARGS_##nr (args);						\
      register int _result __asm__ ("D0Re0"), _nr __asm__ ("D1Re0");	\
      LOAD_ARGS_##nr;							\
      _nr = (name);							\
      __asm__ volatile ("SWITCH  #0x440001 ! syscall " #name		\
			: "=r" (_result)				\
			: "d" (_nr) ASM_ARGS_##nr			\
			: "memory");					\
      __sys_result = _result;						\
    }									\
    (int) __sys_result; })                                              \
)

#undef INTERNAL_SYSCALL_ERROR_P
#define INTERNAL_SYSCALL_ERROR_P(val, err) \
  ((unsigned int) (val) >= 0xfffff001u)

#undef INTERNAL_SYSCALL_ERRNO
#define INTERNAL_SYSCALL_ERRNO(val, err)	(-(val))

#define PREP_ARGS_0()
#define PREP_ARGS_1(a1)                                 \
  int _t1 = (int) (a1);                                 \
  PREP_ARGS_0 ()
#define PREP_ARGS_2(a1, a2)                             \
  int _t2 = (int) (a2);                                 \
  PREP_ARGS_1 (a1)
#define PREP_ARGS_3(a1, a2, a3)                         \
  int _t3 = (int) (a3);                                 \
  PREP_ARGS_2 (a1, a2)
#define PREP_ARGS_4(a1, a2, a3, a4)                     \
  int _t4 = (int) (a4);                                 \
  PREP_ARGS_3 (a1, a2, a3)
#define PREP_ARGS_5(a1, a2, a3, a4, a5)                 \
  int _t5 = (int) (a5);                                 \
  PREP_ARGS_4 (a1, a2, a3, a4)
#define PREP_ARGS_6(a1, a2, a3, a4, a5, a6)             \
  int _t6 = (int) (a6);                                 \
  PREP_ARGS_5 (a1, a2, a3, a4, a5)

#define LOAD_ARGS_0
#define ASM_ARGS_0
#define LOAD_ARGS_1                                     \
  register int _a1 __asm__ ("D1Ar1") = (int) (_t1);     \
  LOAD_ARGS_0
#define ASM_ARGS_1 ASM_ARGS_0, "d" (_a1)
#define LOAD_ARGS_2                                     \
  register int _a2 __asm__ ("D0Ar2") = (int) (_t2);     \
  LOAD_ARGS_1
#define ASM_ARGS_2 ASM_ARGS_1, "d" (_a2)
#define LOAD_ARGS_3                                     \
  register int _a3 __asm__ ("D1Ar3") = (int) (_t3);     \
  LOAD_ARGS_2
#define ASM_ARGS_3 ASM_ARGS_2, "d" (_a3)
#define LOAD_ARGS_4                                     \
  register int _a4 __asm__ ("D0Ar4") = (int) (_t4);     \
  LOAD_ARGS_3
#define ASM_ARGS_4 ASM_ARGS_3, "d" (_a4)
#define LOAD_ARGS_5                                     \
  register int _a5 __asm__ ("D1Ar5") = (int) (_t5);     \
  LOAD_ARGS_4
#define ASM_ARGS_5 ASM_ARGS_4, "d" (_a5)
#define LOAD_ARGS_6                                     \
  register int _a6 __asm__ ("D0Ar6") = (int) (_t6);     \
  LOAD_ARGS_5
#define ASM_ARGS_6 ASM_ARGS_5, "d" (_a6)

#endif /* __ASSEMBLER__ */
#endif /* _BITS_SYSCALLS_H */
