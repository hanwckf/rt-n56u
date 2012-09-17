#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
# error "Never use <bits/syscall.h> directly; include <sys/syscall.h> instead."
#endif

#ifndef __ASSEMBLER__

#include <errno.h>

#define SYS_ify(syscall_name)  (__NR_##syscall_name)

#undef _syscall_return
#define _syscall_return(type)							\
	do {									\
		if ((unsigned long) (_sc_ret) >= (unsigned long) (-125)) {	\
			__set_errno(-_sc_ret);					\
			_sc_ret = -1;						\
		}								\
										\
		return (type) (_sc_ret);					\
	} while (0)

#define _syscall_clobbers		\
	"r1",  "r2",  "r3",  "r4",	\
	"r5",  "r6",  "r7",  "r8",	\
	"r9", "r10", "r11"

#ifdef _syscall0
#	undef _syscall0
#endif
#define _syscall0(type, name)						\
type name (void)							\
{									\
	register long _sc_0 __asm__("r0") = SYS_ify (name);		\
	long _sc_ret;							\
									\
	__asm__ __volatile__ (						\
	"	pushl	%%ap		\n"				\
	"	pushl	$0x0		\n"				\
	"	movl	%%sp, %%ap	\n"				\
	"	chmk	%%r0		\n"				\
	"	addl2	$4, %%sp	\n"				\
	"	movl	(%%sp)+, %%ap	\n"				\
	: "=r" (_sc_0)							\
	: "0" (_sc_0)							\
	: _syscall_clobbers);						\
									\
	_sc_ret = _sc_0;						\
	_syscall_return (type);						\
}

#ifdef _syscall1
#	undef _syscall1
#endif
#define _syscall1(type, name, type1, arg1)				\
type name (type1 arg1)							\
{									\
	register long _sc_0 __asm__("r0") = SYS_ify (name);		\
	long _sc_ret;							\
									\
	__asm__ __volatile__ (						\
	"	pushl	%%ap		\n"				\
	"	pushl	%2		\n"				\
	"	pushl	$0x1		\n"				\
	"	movl	%%sp, %%ap	\n"				\
	"	chmk	%%r0		\n"				\
	"	addl2	$8, %%sp	\n"				\
	"	movl	(%%sp)+, %%ap	\n"				\
	: "=r" (_sc_0)							\
	: "0" (_sc_0),							\
	  "m" (arg1)							\
	: _syscall_clobbers);						\
									\
	_sc_ret = _sc_0;						\
	_syscall_return (type);						\
}

#ifdef _syscall2
#	undef _syscall2
#endif
#define _syscall2(type, name, type1, arg1, type2, arg2)			\
type name (type1 arg1,							\
	   type2 arg2)							\
{									\
	register long _sc_0 __asm__("r0") = SYS_ify (name);		\
	long _sc_ret;							\
									\
	__asm__ __volatile__ (						\
	"	pushl	%%ap		\n"				\
	"	pushl	%3		\n"				\
	"	pushl	%2		\n"				\
	"	pushl	$0x2		\n"				\
	"	movl	%%sp, %%ap	\n"				\
	"	chmk	%%r0		\n"				\
	"	addl2	$12, %%sp	\n"				\
	"	movl	(%%sp)+, %%ap	\n"				\
	: "=r" (_sc_0)							\
	: "0" (_sc_0),							\
	  "m" (arg1),							\
	  "m" (arg2)							\
	: _syscall_clobbers);						\
									\
	_sc_ret = _sc_0;						\
	_syscall_return (type);						\
}

#ifdef _syscall3
#	undef _syscall3
#endif
#define _syscall3(type, name, type1, arg1, type2, arg2, type3, arg3)	\
type name (type1 arg1,							\
	   type2 arg2,							\
	   type3 arg3)							\
{									\
	register long _sc_0 __asm__("r0") = SYS_ify (name);		\
	long _sc_ret;							\
									\
	__asm__ __volatile__ (						\
	"	pushl	%%ap		\n"				\
	"	pushl	%4		\n"				\
	"	pushl	%3		\n"				\
	"	pushl	%2		\n"				\
	"	pushl	$0x3		\n"				\
	"	movl	%%sp, %%ap	\n"				\
	"	chmk	%%r0		\n"				\
	"	addl2	$16, %%sp	\n"				\
	"	movl	(%%sp)+, %%ap	\n"				\
	: "=r" (_sc_0)							\
	: "0" (_sc_0),							\
	  "m" (arg1),							\
	  "m" (arg2),							\
	  "m" (arg3)							\
	: _syscall_clobbers);						\
									\
	_sc_ret = _sc_0;						\
	_syscall_return (type);						\
}

#ifdef _syscall4
#	undef _syscall4
#endif
#define _syscall4(type, name, type1, arg1, type2, arg2, type3, arg3,	\
		type4, arg4)						\
type name (type1 arg1,							\
	   type2 arg2,							\
	   type3 arg3,							\
	   type4 arg4)							\
{									\
	register long _sc_0 __asm__("r0") = SYS_ify (name);		\
	long _sc_ret;							\
									\
	__asm__ __volatile__ (						\
	"	pushl	%%ap		\n"				\
	"	pushl	%5		\n"				\
	"	pushl	%4		\n"				\
	"	pushl	%3		\n"				\
	"	pushl	%2		\n"				\
	"	pushl	$0x4		\n"				\
	"	movl	%%sp, %%ap	\n"				\
	"	chmk	%%r0		\n"				\
	"	addl2	$20, %%sp	\n"				\
	"	movl	(%%sp)+, %%ap	\n"				\
	: "=r" (_sc_0)							\
	: "0" (_sc_0),							\
	  "m" (arg1),							\
	  "m" (arg2),							\
	  "m" (arg3),							\
	  "m" (arg4)							\
	: _syscall_clobbers);						\
									\
	_sc_ret = _sc_0;						\
	_syscall_return (type);						\
}

#ifdef _syscall5
#	undef _syscall5
#endif
#define _syscall5(type, name, type1, arg1, type2, arg2, type3, arg3,	\
		type4, arg4, type5, arg5)				\
type name (type1 arg1,							\
	   type2 arg2,							\
	   type3 arg3,							\
	   type4 arg4,							\
	   type5 arg5)							\
{									\
	register long _sc_0 __asm__("r0") = SYS_ify (name);		\
	long _sc_ret;							\
									\
	__asm__ __volatile__ (						\
	"	pushl	%%ap		\n"				\
	"	pushl	%6		\n"				\
	"	pushl	%5		\n"				\
	"	pushl	%4		\n"				\
	"	pushl	%3		\n"				\
	"	pushl	%2		\n"				\
	"	pushl	$0x5		\n"				\
	"	movl	%%sp, %%ap	\n"				\
	"	chmk	%%r0		\n"				\
	"	addl2	$24, %%sp	\n"				\
	"	movl	(%%sp)+, %%ap	\n"				\
	: "=r" (_sc_0)							\
	: "0" (_sc_0),							\
	  "m" (arg1),							\
	  "m" (arg2),							\
	  "m" (arg3),							\
	  "m" (arg4),							\
	  "m" (arg5)							\
	: _syscall_clobbers);						\
									\
	_sc_ret = _sc_0;						\
	_syscall_return (type);						\
}

#ifdef _syscall6
#	undef _syscall6
#endif
#define _syscall6(type, name, type1, arg1, type2, arg2, type3, arg3,	\
		type4, arg4, type5, arg5, type6, arg6)			\
type name (type1 arg1,							\
	   type2 arg2,							\
	   type3 arg3,							\
	   type4 arg4,							\
	   type5 arg5,							\
	   type6 arg6)							\
{									\
	register long _sc_0 __asm__("r0") = SYS_ify (name);		\
	long _sc_ret;							\
									\
	__asm__ __volatile__ (						\
	"	pushl	%%ap		\n"				\
	"	pushl	%7		\n"				\
	"	pushl	%6		\n"				\
	"	pushl	%5		\n"				\
	"	pushl	%4		\n"				\
	"	pushl	%3		\n"				\
	"	pushl	%2		\n"				\
	"	pushl	$0x6		\n"				\
	"	movl	%%sp, %%ap	\n"				\
	"	chmk	%%r0		\n"				\
	"	addl2	$28, %%sp	\n"				\
	"	movl	(%%sp)+, %%ap	\n"				\
	: "=r" (_sc_0)							\
	: "0" (_sc_0),							\
	  "m" (arg1),							\
	  "m" (arg2),							\
	  "m" (arg3),							\
	  "m" (arg4),							\
	  "m" (arg5),							\
	  "m" (arg6)							\
	: _syscall_clobbers);						\
									\
	_sc_ret = _sc_0;						\
	_syscall_return (type);						\
}

#endif /* __ASSEMBLER__ */
#endif /* _BITS_SYSCALLS_H */
