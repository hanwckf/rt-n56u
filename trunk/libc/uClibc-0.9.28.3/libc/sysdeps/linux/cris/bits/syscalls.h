#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H

#ifndef _SYSCALL_H
#error "Never use <bits/syscall.h> directly; include <sys/syscall.h> instead."
#endif

/* Include the __NR_<name> definitions. */
#include <bits/sysnum.h>

#if 0
#ifndef __set_errno
#define __set_errno(val) (*__errno_location()) = (val)
#endif
#endif

#undef SYS_ify
#define SYS_ify(syscall_name)   (__NR_##syscall_name)

#ifndef __ASSEMBLER__

#undef _syscall0
#define _syscall0(type,name) \
type name(void) \
{ \
return (type) (INLINE_SYSCALL(name, 0)); \
}

#undef _syscall1
#define _syscall1(type,name,type1,arg1) \
type name(type1 arg1) \
{ \
return (type) (INLINE_SYSCALL(name, 1, arg1)); \
}

#undef _syscall2
#define _syscall2(type,name,type1,arg1,type2,arg2) \
type name(type1 arg1,type2 arg2) \
{ \
return (type) (INLINE_SYSCALL(name, 2, arg1, arg2)); \
}

#undef _syscall3
#define _syscall3(type,name,type1,arg1,type2,arg2,type3,arg3) \
type name(type1 arg1,type2 arg2,type3 arg3) \
{ \
return (type) (INLINE_SYSCALL(name, 3, arg1, arg2, arg3)); \
}

#undef _syscall4
#define _syscall4(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4) \
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4) \
{ \
return (type) (INLINE_SYSCALL(name, 4, arg1, arg2, arg3, arg4)); \
} 

#undef _syscall5
#define _syscall5(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4, \
          type5,arg5) \
type name (type1 arg1,type2 arg2,type3 arg3,type4 arg4,type5 arg5) \
{ \
return (type) (INLINE_SYSCALL(name, 5, arg1, arg2, arg3, arg4, arg5)); \
}

#undef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...)	\
  ({						\
     unsigned long __sys_res;			\
     register unsigned long __res asm ("r10");	\
     LOAD_ARGS_c_##nr (args)			\
     register unsigned long __callno asm ("r9")	\
       = SYS_ify (name);			\
     asm volatile (LOAD_ARGS_asm_##nr (args)	\
		   CHECK_ARGS_asm_##nr		\
		   "break 13"			\
		   : "=r" (__res)		\
		   : ASM_ARGS_##nr (args)	\
		   : ASM_CLOBBER_##nr);		\
     __sys_res = __res;				\
						\
     if (__sys_res >= (unsigned long) -4096)	\
       {					\
	 __set_errno (- __sys_res);		\
	 __sys_res = (unsigned long) -1;	\
       }					\
     __sys_res;					\
   })

#define LOAD_ARGS_c_0()
#define LOAD_ARGS_asm_0()
#define ASM_CLOBBER_0 "memory"
#define ASM_ARGS_0() "r" (__callno)
#define CHECK_ARGS_asm_0

#define LOAD_ARGS_c_1(r10) \
	LOAD_ARGS_c_0()						\
	register unsigned long __r10 __asm__ ("r10") = (unsigned long) (r10);
#define LOAD_ARGS_asm_1(r10) LOAD_ARGS_asm_0 ()
#define ASM_CLOBBER_1 ASM_CLOBBER_0
#define ASM_ARGS_1(r10) ASM_ARGS_0 (), "0" (__r10)
#define CHECK_ARGS_asm_1			\
	".ifnc %0,$r10\n\t"			\
	".err\n\t"				\
	".endif\n\t"

#define LOAD_ARGS_c_2(r10, r11) \
	LOAD_ARGS_c_1(r10)					\
	register unsigned long __r11 __asm__ ("r11") = (unsigned long) (r11);
#define LOAD_ARGS_asm_2(r10, r11) LOAD_ARGS_asm_1 (r10)
#define ASM_CLOBBER_2 ASM_CLOBBER_1
#define ASM_ARGS_2(r10, r11) ASM_ARGS_1 (r10), "r" (__r11)
#define CHECK_ARGS_asm_2			\
	".ifnc %0-%3,$r10-$r11\n\t"		\
	".err\n\t"				\
	".endif\n\t"

#define LOAD_ARGS_c_3(r10, r11, r12) \
	LOAD_ARGS_c_2(r10, r11)					\
	register unsigned long __r12 __asm__ ("r12") = (unsigned long) (r12);
#define LOAD_ARGS_asm_3(r10, r11, r12) LOAD_ARGS_asm_2 (r10, r11)
#define ASM_CLOBBER_3 ASM_CLOBBER_2
#define ASM_ARGS_3(r10, r11, r12) ASM_ARGS_2 (r10, r11), "r" (__r12)
#define CHECK_ARGS_asm_3			\
	".ifnc %0-%3-%4,$r10-$r11-$r12\n\t"	\
	".err\n\t"				\
	".endif\n\t"

#define LOAD_ARGS_c_4(r10, r11, r12, r13) \
	LOAD_ARGS_c_3(r10, r11, r12)				\
	register unsigned long __r13 __asm__ ("r13") = (unsigned long) (r13);
#define LOAD_ARGS_asm_4(r10, r11, r12, r13) LOAD_ARGS_asm_3 (r10, r11, r12)
#define ASM_CLOBBER_4 ASM_CLOBBER_3
#define ASM_ARGS_4(r10, r11, r12, r13) ASM_ARGS_3 (r10, r11, r12), "r" (__r13)
#define CHECK_ARGS_asm_4				\
	".ifnc %0-%3-%4-%5,$r10-$r11-$r12-$r13\n\t"	\
	".err\n\t"					\
	".endif\n\t"

#define LOAD_ARGS_c_5(r10, r11, r12, r13, mof) \
	LOAD_ARGS_c_4(r10, r11, r12, r13)
#define LOAD_ARGS_asm_5(r10, r11, r12, r13, mof) \
	LOAD_ARGS_asm_4 (r10, r11, r12, r13) "move %6,$mof\n\t"
#define ASM_CLOBBER_5 ASM_CLOBBER_4
#define ASM_ARGS_5(r10, r11, r12, r13, mof) \
	ASM_ARGS_4 (r10, r11, r12, r13), "g" (mof)
#define CHECK_ARGS_asm_5 CHECK_ARGS_asm_4

#define LOAD_ARGS_c_6(r10, r11, r12, r13, mof, srp)		\
	LOAD_ARGS_c_5(r10, r11, r12, r13, mof)
#define LOAD_ARGS_asm_6(r10, r11, r12, r13, mof, srp)		\
	LOAD_ARGS_asm_5(r10, r11, r12, r13, mof)		\
	"move %7,$srp\n\t"
#define ASM_CLOBBER_6 ASM_CLOBBER_5, "srp"
#define ASM_ARGS_6(r10, r11, r12, r13, mof, srp) \
	ASM_ARGS_5 (r10, r11, r12, r13, mof), "g" (srp)
#define CHECK_ARGS_asm_6 CHECK_ARGS_asm_5

#endif /* not __ASSEMBLER__ */
#endif /* _BITS_SYSCALLS_H */
