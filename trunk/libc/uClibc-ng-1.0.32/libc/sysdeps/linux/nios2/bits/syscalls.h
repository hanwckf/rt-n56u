#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

#ifdef __ASSEMBLER__

#undef DO_CALL
#define DO_CALL(syscall_name, args) \
    DOARGS_##args                   \
    movi r2, SYS_ify(syscall_name);  \
    trap;

#define DOARGS_0 /* nothing */
#define DOARGS_1 /* nothing */
#define DOARGS_2 /* nothing */
#define DOARGS_3 /* nothing */
#define DOARGS_4 /* nothing */
#define DOARGS_5 ldw r8, 0(sp);
#define DOARGS_6 ldw r9, 4(sp); ldw r8, 0(sp);

#else

#include <errno.h>

#undef INTERNAL_SYSCALL_DECL
#define INTERNAL_SYSCALL_DECL(err) unsigned int err __attribute__((unused))

#undef INTERNAL_SYSCALL_ERROR_P
#define INTERNAL_SYSCALL_ERROR_P(val, err) ((void) (val), (unsigned int) (err))

#undef INTERNAL_SYSCALL_ERRNO
#define INTERNAL_SYSCALL_ERRNO(val, err)   ((void) (err), val)

#undef INTERNAL_SYSCALL_NCS
#define INTERNAL_SYSCALL_NCS(name, err, nr, args...)            \
  ({ unsigned int _sys_result;                                  \
     {                                                          \
       LOAD_ARGS_##nr (args)					\
       LOAD_REGS_##nr						\
       register int _r2 __asm__ ("r2") = (int)(name);           \
       register int _sys_err __asm__ ("r7");                        \
       __asm__ volatile ("trap"                                 \
                     : "+r" (_r2), "=r" (_sys_err)              \
                     : ASM_ARGS_##nr				\
                     : "memory");                               \
       _sys_result = _r2;                                       \
       err = _sys_err;						\
     }                                                          \
     (int) _sys_result; })

#define LOAD_ARGS_0()
#define LOAD_REGS_0
#define ASM_ARGS_0
#define LOAD_ARGS_1(a1)				\
  LOAD_ARGS_0 ()				\
  int __arg1 = (int) (a1);
#define LOAD_REGS_1				\
  register int _r4 __asm__ ("r4") = __arg1;	\
  LOAD_REGS_0
#define ASM_ARGS_1                  "r" (_r4)
#define LOAD_ARGS_2(a1, a2)			\
  LOAD_ARGS_1 (a1)				\
  int __arg2 = (int) (a2);
#define LOAD_REGS_2				\
  register int _r5 __asm__ ("r5") = __arg2;	\
  LOAD_REGS_1
#define ASM_ARGS_2      ASM_ARGS_1, "r" (_r5)
#define LOAD_ARGS_3(a1, a2, a3)			\
  LOAD_ARGS_2 (a1, a2)				\
  int __arg3 = (int) (a3);
#define LOAD_REGS_3				\
  register int _r6 __asm__ ("r6") = __arg3;	\
  LOAD_REGS_2
#define ASM_ARGS_3      ASM_ARGS_2, "r" (_r6)
#define LOAD_ARGS_4(a1, a2, a3, a4)		\
  LOAD_ARGS_3 (a1, a2, a3)			\
  int __arg4 = (int) (a4);
#define LOAD_REGS_4				\
  register int _r7 __asm__ ("r7") = __arg4;	\
  LOAD_REGS_3
#define ASM_ARGS_4      ASM_ARGS_3, "r" (_r7)
#define LOAD_ARGS_5(a1, a2, a3, a4, a5)		\
  LOAD_ARGS_4 (a1, a2, a3, a4)			\
  int __arg5 = (int) (a5);
#define LOAD_REGS_5				\
  register int _r8 __asm__ ("r8") = __arg5;	\
  LOAD_REGS_4
#define ASM_ARGS_5      ASM_ARGS_4, "r" (_r8)
#define LOAD_ARGS_6(a1, a2, a3, a4, a5, a6)	\
  LOAD_ARGS_5 (a1, a2, a3, a4, a5)		\
  int __arg6 = (int) (a6);
#define LOAD_REGS_6			    \
  register int _r9 __asm__ ("r9") = __arg6;     \
  LOAD_REGS_5
#define ASM_ARGS_6      ASM_ARGS_5, "r" (_r9)

#endif /* __ASSEMBLER__ */
#endif /* _BITS_SYSCALLS_H */
