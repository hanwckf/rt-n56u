#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H

#ifndef _SYSCALL_H
#error "Never use <bits/syscall.h> directly; include <sys/syscall.h> instead."
#endif

#ifndef __ASSEMBLER__

#define INTERNAL_SYSCALL_NCS(name, err, nr, args...)	\
(__extension__ \
  ({						\
     register unsigned long __res __asm__ ("r10");	\
     LOAD_ARGS_c_##nr(args)			\
     register unsigned long __callno __asm__ ("r9")	\
       = name;					\
     __asm__ __volatile__ (LOAD_ARGS_asm_##nr (args)	\
		   CHECK_ARGS_asm_##nr		\
		   "break 13"			\
		   : "=r" (__res)		\
		   : ASM_ARGS_##nr (args)	\
		   : ASM_CLOBBER_##nr);		\
     __res;					\
   }) \
)
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
