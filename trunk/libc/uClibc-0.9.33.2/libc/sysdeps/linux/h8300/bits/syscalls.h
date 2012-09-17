/* Unlike the asm/unistd.h kernel header file (which this is partly based on),
 * this file must be able to cope with PIC and non-PIC code.  For some arches
 * there is no difference.  For x86 (which has far too few registers) there is
 * a difference.   Regardless, including asm/unistd.h is hereby officially
 * forbidden.  Don't do it.  It is bad for you.
 */
#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

#define __syscall_return(type, res) \
do { \
	if ((unsigned long)(res) >= (unsigned long)(-125)) { \
	/* avoid using res which is declared to be in register d0; \
	   errno might expand to a function call and clobber it.  */ \
		int __err = -(res); \
		errno = __err; \
		res = -1; \
	} \
	return (type) (res); \
} while (0)

#define _syscall0(type, name)							\
type name(void)									\
{										\
  register long __res __asm__("er0");						\
  __asm__ __volatile__ ("mov.l	%1,er0\n\t"					\
  			"trapa	#0\n\t"						\
			: "=r" (__res)						\
			: "ir" (__NR_##name)					\
			: "cc");						\
  if ((unsigned long)(__res) >= (unsigned long)(-125)) {			\
    errno = -__res;								\
    __res = -1;									\
  }										\
  return (type)__res;								\
}

#define _syscall1(type, name, atype, a)						\
type name(atype a)								\
{										\
  register long __res __asm__("er0");						\
  __asm__ __volatile__ ("mov.l	%2, er1\n\t"					\
  			"mov.l	%1, er0\n\t"					\
  			"trapa	#0\n\t"						\
			: "=r" (__res)						\
			: "ir" (__NR_##name),					\
			  "g" ((long)a)						\
			: "cc", "er1");					\
  if ((unsigned long)(__res) >= (unsigned long)(-125)) {			\
    errno = -__res;								\
    __res = -1;									\
  }										\
  return (type)__res;								\
}

#define _syscall2(type, name, atype, a, btype, b)				\
type name(atype a, btype b)							\
{										\
  register long __res __asm__("er0");						\
  __asm__ __volatile__ ("mov.l	%3, er2\n\t"					\
  			"mov.l	%2, er1\n\t"					\
			"mov.l	%1, er0\n\t"					\
  			"trapa	#0\n\t"						\
			: "=r" (__res)						\
			: "ir" (__NR_##name),					\
			  "g" ((long)a),					\
			  "g" ((long)b)						\
			: "cc", "er1", "er2"); 				\
  if ((unsigned long)(__res) >= (unsigned long)(-125)) {			\
    errno = -__res;								\
    __res = -1;									\
  }										\
  return (type)__res;								\
}

#define _syscall3(type, name, atype, a, btype, b, ctype, c)			\
type name(atype a, btype b, ctype c)						\
{										\
  register long __res __asm__("er0");						\
  __asm__ __volatile__ ("mov.l	%4, er3\n\t"					\
			"mov.l	%3, er2\n\t"					\
  			"mov.l	%2, er1\n\t"					\
			"mov.l	%1, er0\n\t"					\
  			"trapa	#0\n\t"						\
			: "=r" (__res)						\
			: "ir" (__NR_##name),					\
			  "g" ((long)a),					\
			  "g" ((long)b),					\
			  "g" ((long)c)						\
			: "cc", "er1", "er2", "er3");  			\
  if ((unsigned long)(__res) >= (unsigned long)(-125)) {			\
    errno = -__res;								\
    __res = -1;									\
  }										\
  return (type)__res;								\
}

#define _syscall4(type, name, atype, a, btype, b, ctype, c, dtype, d)		\
type name(atype a, btype b, ctype c, dtype d)					\
{										\
  register long __res __asm__("er0");						\
  __asm__ __volatile__ ("mov.l	%5, er4\n\t"					\
			"mov.l	%4, er3\n\t"					\
			"mov.l	%3, er2\n\t"					\
  			"mov.l	%2, er1\n\t"					\
			"mov.l	%1, er0\n\t"					\
  			"trapa	#0\n\t"						\
			: "=r" (__res)						\
			: "ir" (__NR_##name),					\
			  "g" ((long)a),					\
			  "g" ((long)b),					\
			  "g" ((long)c),					\
			  "g" ((long)d)						\
			: "cc", "er1", "er2", "er3", "er4");			\
  if ((unsigned long)(__res) >= (unsigned long)(-125)) {			\
    errno = -__res;								\
    __res = -1;									\
  }										\
  return (type)__res;								\
}

#define _syscall5(type, name, atype, a, btype, b, ctype, c, dtype, d, etype, e)	\
type name(atype a, btype b, ctype c, dtype d, etype e)				\
{										\
  register long __res __asm__("er0");						\
  __asm__ __volatile__ ( \
                        "mov.l  er5,@-sp\n\t"                                   \
			"mov.l	%5, er4\n\t"					\
			"mov.l	%4, er3\n\t"					\
			"mov.l	%3, er2\n\t"					\
  			"mov.l	%2, er1\n\t"					\
			"mov.l	%1, er0\n\t"					\
			"mov.l	%6, er5\n\t"					\
  			"trapa	#0\n\t"						\
                       "mov.l  @sp+,er5\n\t"                                   \
			: "=r" (__res)						\
			: "ir" (__NR_##name),					\
			  "g" ((long)a),					\
			  "g" ((long)b),					\
			  "g" ((long)c),					\
			  "g" ((long)d),					\
			  "m" ((long)e)						\
			: "cc", "er1", "er2", "er3", "er4");		        \
  if ((unsigned long)(__res) >= (unsigned long)(-125)) {		       	\
    errno = -__res;								\
    __res = -1;									\
  }										\
  return (type)__res;								\
}

