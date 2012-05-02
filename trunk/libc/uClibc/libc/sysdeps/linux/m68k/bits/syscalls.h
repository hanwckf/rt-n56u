#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

/* This includes the `__NR_<name>' syscall numbers taken from the Linux kernel
 * header files.  It also defines the traditional `SYS_<name>' macros for older
 * programs.  */
#include <bits/sysnum.h>

#ifndef __set_errno
# define __set_errno(val) (*__errno_location ()) = (val)
#endif

#ifndef __ASSEMBLER__

/* Linux takes system call arguments in registers:

	syscall number	%d0	     call-clobbered
	arg 1		%d1	     call-clobbered
	arg 2		%d2	     call-saved
	arg 3		%d3	     call-saved
	arg 4		%d4	     call-saved
	arg 5		%d5	     call-saved

   The stack layout upon entering the function is:

	20(%sp)		Arg# 5
	16(%sp)		Arg# 4
	12(%sp)		Arg# 3
	 8(%sp)		Arg# 2
	 4(%sp)		Arg# 1
	  (%sp)		Return address

   (Of course a function with say 3 arguments does not have entries for
   arguments 4 and 5.)

   Separate move's are faster than movem, but need more space.  Since
   speed is more important, we don't use movem.  Since %a0 and %a1 are
   scratch registers, we can use them for saving as well.  */

#define __syscall_return(type, res) \
do { \
	if ((unsigned long)(res) >= (unsigned long)(-125)) { \
		/* avoid using res which is declared to be in register d0; \
		   errno might expand to a function call and clobber it.  */ \
		int __err = -(res); \
		__set_errno(__err); \
		res = -1; \
	} \
	return (type) (res); \
} while (0)

#define _syscall0(type, name) \
type name(void) \
{ \
	long __res; \
	__asm__ __volatile__ ( \
		"movel	%1, %%d0\n\t" \
		"trap	#0\n\t" \
		"movel	%%d0, %0" \
		: "=g" (__res) \
		: "i" (__NR_##name) \
		: "cc", "%d0"); \
	__syscall_return(type, __res); \
}

#define _syscall1(type, name, atype, a) \
type name(atype a) \
{ \
	long __res; \
	__asm__ __volatile__ ( \
		"movel	%2, %%d1\n\t" \
		"movel	%1, %%d0\n\t" \
		"trap	#0\n\t" \
		"movel	%%d0, %0" \
		: "=g" (__res) \
		: "i" (__NR_##name), \
		  "g" ((long)a) \
		: "cc", "%d0", "%d1"); \
	__syscall_return(type, __res); \
}

#define _syscall2(type, name, atype, a, btype, b) \
type name(atype a, btype b) \
{ \
	long __res; \
	__asm__ __volatile__ ( \
		"movel	%3, %%d2\n\t" \
		"movel	%2, %%d1\n\t" \
		"movel	%1, %%d0\n\t" \
		"trap	#0\n\t" \
		"movel	%%d0, %0" \
		: "=g" (__res) \
		: "i" (__NR_##name), \
		  "a" ((long)a), \
		  "g" ((long)b) \
		: "cc", "%d0", "%d1", "%d2"); \
	__syscall_return(type, __res); \
}

#define _syscall3(type, name, atype, a, btype, b, ctype, c) \
type name(atype a, btype b, ctype c) \
{ \
	long __res; \
	__asm__ __volatile__ ( \
		"movel	%4, %%d3\n\t" \
		"movel	%3, %%d2\n\t" \
		"movel	%2, %%d1\n\t" \
		"movel	%1, %%d0\n\t" \
		"trap	#0\n\t" \
		"movel	%%d0, %0" \
		: "=g" (__res) \
		: "i" (__NR_##name), \
		  "a" ((long)a), \
		  "a" ((long)b), \
		  "g" ((long)c) \
		: "cc", "%d0", "%d1", "%d2", "%d3"); \
	__syscall_return(type, __res); \
}

#define _syscall4(type, name, atype, a, btype, b, ctype, c, dtype, d) \
type name(atype a, btype b, ctype c, dtype d) \
{ \
	long __res; \
	__asm__ __volatile__ ( \
		"movel	%5, %%d4\n\t" \
		"movel	%4, %%d3\n\t" \
		"movel	%3, %%d2\n\t" \
		"movel	%2, %%d1\n\t" \
		"movel	%1, %%d0\n\t" \
		"trap	#0\n\t" \
		"movel	%%d0, %0" \
		: "=g" (__res) \
		: "i" (__NR_##name), \
		  "a" ((long)a), \
		  "a" ((long)b), \
		  "a" ((long)c), \
		  "g" ((long)d) \
		: "cc", "%d0", "%d1", "%d2", "%d3", \
		  "%d4"); \
	__syscall_return(type, __res); \
}

#define _syscall5(type, name, atype, a, btype, b, ctype, c, dtype, d, etype, e) \
type name(atype a, btype b, ctype c, dtype d, etype e) \
{ \
	long __res; \
	__asm__ __volatile__ ( \
		"movel	%6, %%d5\n\t" \
		"movel	%5, %%d4\n\t" \
		"movel	%4, %%d3\n\t" \
		"movel	%3, %%d2\n\t" \
		"movel	%2, %%d1\n\t" \
		"movel	%1, %%d0\n\t" \
		"trap	#0\n\t" \
		"movel	%%d0, %0" \
		: "=g" (__res) \
		: "i" (__NR_##name), \
		  "a" ((long)a), \
		  "a" ((long)b), \
		  "a" ((long)c), \
		  "a" ((long)d), \
		  "g" ((long)e) \
		: "cc", "%d0", "%d1", "%d2", "%d3", \
		  "%d4", "%d5"); \
	__syscall_return(type, __res); \
}

#define _syscall6(type, name, atype, a, btype, b, ctype, c, dtype, d, etype, e, ftype, f) \
type name(atype a, btype b, ctype c, dtype d, etype e, ftype f) \
{ \
	long __res; \
	__asm__ __volatile__ ( \
		"movel	%7, %%a0\n\t" \
		"movel	%6, %%d5\n\t" \
		"movel	%5, %%d4\n\t" \
		"movel	%4, %%d3\n\t" \
		"movel	%3, %%d2\n\t" \
		"movel	%2, %%d1\n\t" \
		"movel	%1, %%d0\n\t" \
		"trap	#0\n\t" \
		"movel	%%d0, %0" \
		: "=g" (__res) \
		: "i" (__NR_##name), \
		  "a" ((long)a), \
		  "a" ((long)b), \
		  "a" ((long)c), \
		  "a" ((long)d), \
		  "g" ((long)e), \
		  "g" ((long)f) \
		: "cc", "%d0", "%d1", "%d2", "%d3", \
		  "%d4", "%d5", "%a0"); \
	__syscall_return(type, __res); \
}

#endif /* __ASSEMBLER__ */
#endif /* _BITS_SYSCALLS_H */
