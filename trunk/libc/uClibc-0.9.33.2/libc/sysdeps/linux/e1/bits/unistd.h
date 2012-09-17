 /* This file is lisenced under LGPL.
  * Copyright (C) 2002-2003,    George Thanos <george.thanos@gdt.gr>
  *                             Yannis Mitsos <yannis.mitsos@gdt.gr>
  */
#ifndef _BITS_UNISTD_H_
#define _BITS_UNISTD_H_

#include <bits/proto.h>
#include <errno.h>

#define __E1_COFF_GCC__

/* The following macros have been provided by C.Baumhof
 * They can be inlined in contrast to the previous ones*/
#define _syscall0(type, name)  \
type name(void)              \
{                                               \
	register int par1 __asm__("L15");   \
	register int par2 __asm__("L14");   \
    par1 = -1;              \
	par2 = __NR_##name;         \
	__asm__ __volatile__(                   \
		"trap    47"                        \
		:"=l"(par1)                             \
		:"0"(par1), "l"(par2)  \
		:"memory","L14","L15");   \
																	\
	if( par1 < 0 ) {													\
			__set_errno( -par1 );										\
			return -1;											\
	} else															\
		return (type)(par1);                    					\
}

#define _syscall1(type, name,atype, a)  \
type name(atype a)              \
{                                               \
	register int par1 __asm__("L15");   \
	register int par2 __asm__("L14");   \
	register int par3 __asm__("L13");   \
    par1 = -1;              \
	par2 = __NR_##name;         \
	par3 = (int)a;              \
	__asm__ __volatile__(                   \
		"trap    47"                        \
		:"=l"(par1)                             \
		:"0"(par1), "l"(par2), "l"(par3)  \
		:"memory","L13","L14","L15");   \
																	\
	if( par1 < 0 ) {													\
			__set_errno( -par1 );										\
			return -1;											\
	} else															\
		return (type)(par1);                    					\
}

#define _syscall2(type, name,atype, a, btype, b)  \
type name(atype a, btype b)              \
{                                               \
	register int par1 __asm__("L15");   \
	register int par2 __asm__("L14");   \
	register int par3 __asm__("L13");   \
	register int par4 __asm__("L12");   \
    par1 = -1;              \
	par2 = __NR_##name;         \
	par3 = (int)a;              \
	par4 = (int)b;              \
	__asm__ __volatile__(                   \
		"trap    47"                        \
		:"=l"(par1)                             \
		:"0"(par1), "l"(par2), "l"(par3), "l"(par4) \
		:"memory","L12","L13","L14","L15");   \
																	\
	if( par1 < 0 ) {													\
			__set_errno( -par1 );										\
			return -1;											\
	} else															\
		return (type)(par1);                    					\
}

#define _syscall3(type, name,atype, a, btype, b, ctype, c)  \
type name(atype a, btype b, ctype c)                      \
{                                               \
	register int par1 __asm__("L15");   \
	register int par2 __asm__("L14");   \
	register int par3 __asm__("L13");   \
	register int par4 __asm__("L12");   \
	register int par5 __asm__("L11");   \
    par1 = -1;              \
	par2 = __NR_##name;         \
	par3 = (int)a;              \
	par4 = (int)b;              \
	par5 = (int)c;              \
	__asm__ __volatile__(                   \
		"trap    47"                        \
		:"=l"(par1)                             \
		:"0"(par1), "l"(par2), "l"(par3), "l"(par4), "l"(par5) \
		:"memory","L11","L12","L13","L14","L15");    \
																	\
	if( par1 < 0 ) {													\
			__set_errno( -par1 );										\
			return -1;											\
	} else															\
		return (type)(par1);                    					\
}

#define _syscall4(type, name,atype, a, btype, b, ctype, c, dtype, d)  \
type name(atype a, btype b, ctype c,dtype d)                      \
{                                               \
	register int par1 __asm__("L15");   \
	register int par2 __asm__("L14");   \
	register int par3 __asm__("L13");   \
	register int par4 __asm__("L12");   \
	register int par5 __asm__("L11");   \
	register int par6 __asm__("L10");   \
    par1 = -1;              \
	par2 = __NR_##name;         \
	par3 = (int)a;              \
	par4 = (int)b;              \
	par5 = (int)c;              \
	par6 = (int)d;              \
	__asm__ __volatile__(                   \
		"trap    47"                        \
		:"=l"(par1)                             \
		:"0"(par1),"l"(par2),"l"(par3),"l"(par4),"l"(par5),"l"(par6) \
		:"memory","L10","L11","L12","L13","L14","L15");    \
																	\
	if( par1 < 0 ) {													\
			__set_errno( -par1 );										\
			return -1;											\
	} else															\
		return (type)(par1);                    					\
}

#define _syscall5(type, name,atype, a, btype, b, ctype, c, dtype, d, etype, e)  \
type name(atype a, btype b, ctype c,dtype d, etype e)                      \
{                                                       \
	register int par1 __asm__("L15");   \
	register int par2 __asm__("L14");   \
	register int par3 __asm__("L13");   \
	register int par4 __asm__("L12");   \
	register int par5 __asm__("L11");   \
	register int par6 __asm__("L10");   \
	register int par7 __asm__("L9");   \
    par1 = -1;              	\
	par2 = __NR_##name;         \
	par3 = (int)a;              \
	par4 = (int)b;              \
	par5 = (int)c;              \
	par6 = (int)d;              \
	par7 = (int)e;              \
	__asm__ __volatile__(                   \
		"trap    47"                        \
		:"=l"(par1)                             \
		:"0"(par1),"l"(par2),"l"(par3),"l"(par4),"l"(par5),"l"(par6),"l"(par7) \
		:"memory","L9","L10","L11","L12","L13","L14","L15");    \
																	\
	if( par1 < 0 ) {													\
			__set_errno( -par1 );										\
			return -1;											\
	} else															\
		return (type)(par1);                    					\
	return (type)(par1);                    \
}

#define _syscall6(type, name,atype, a, btype, b, ctype, c, dtype, d, etype, e, ftype, f)  \
type name(atype a, btype b, ctype c,dtype d, etype e, ftype f)                      \
{                                                               \
	register int par1 __asm__("L15");   \
	register int par2 __asm__("L14");   \
	register int par3 __asm__("L13");   \
	register int par4 __asm__("L12");   \
	register int par5 __asm__("L11");   \
	register int par6 __asm__("L10");   \
	register int par7 __asm__("L9");   \
	register int par8 __asm__("L8");   \
	int sys_retval;				\
    par1 = -1;              	\
	par2 = __NR_##name;         \
	par3 = (int)a;              \
	par4 = (int)b;              \
	par5 = (int)c;              \
	par6 = (int)d;              \
	par7 = (int)e;              \
	par7 = (int)f;              \
	__asm__ __volatile__(                   \
		"trap    47"                        \
		:"=l"(par1)                             \
		:"0"(par1),"l"(par2),"l"(par3),"l"(par4),"l"(par5),"l"(par6),"l"(par7),"l"(par8) \
		:"memory","L8","L9","L10","L11","L12","L13","L14","L15");	\
																	\
	if( par1 < 0 ) {													\
			__set_errno( -par1 );										\
			return -1;											\
	} else															\
		return (type)(par1);                    					\
}

#define __syscall0(type, name)  \
type name(...)                      \
{                                               \
	register int par1 __asm__("L15");   \
	register int par2 __asm__("L14");   \
	par1 = -1;              \
	par2 = __NR_##name;         \
	__asm__ __volatile__(                   \
		"trap    47"                        \
		:"=l"(par1)                             \
		:"0"(par1), "l"(par2)\
		:"memory","L14","L15");    \
																	\
	if( par1 < 0 ) {													\
			__set_errno( -par1 );										\
			return -1;											\
	} else															\
		return (type)(par1);                    					\
}

#define __syscall1(type, name, atype, a)  \
type name(atype a, ...)                      \
{                                               \
	register int par1 __asm__("L15");   \
	register int par2 __asm__("L14");   \
	register int par3 __asm__("L13");   \
	par1 = -1;              \
	par2 = __NR_##name;         \
	par3 = (int)a;              \
	__asm__ __volatile__(                   \
		"trap    47"                        \
		:"=l"(par1)                             \
		:"0"(par1), "l"(par2), "l"(par3)\
		:"memory","L13","L14","L15");    \
																	\
	if( par1 < 0 ) {													\
			__set_errno( -par1 );										\
			return -1;											\
	} else															\
		return (type)(par1);                    					\
}

#define __syscall2(type, name,atype, a, btype, b)  \
type name(atype a, btype b, ...)                      \
{                                               \
	register int par1 __asm__("L15");   \
	register int par2 __asm__("L14");   \
	register int par3 __asm__("L13");   \
	register int par4 __asm__("L12");   \
	par1 = -1;              \
	par2 = __NR_##name;         \
	par3 = (int)a;              \
	par4 = (int)b;              \
	__asm__ __volatile__(                   \
		"trap    47"                        \
		:"=l"(par1)                             \
		:"0"(par1), "l"(par2), "l"(par3), "l"(par4)\
		:"memory","L12","L13","L14","L15");    \
																	\
	if( par1 < 0 ) {													\
			__set_errno( -par1 );										\
			return -1;											\
	} else															\
		return (type)(par1);                    					\
}

#define __syscall3(type, name,atype, a, btype, b, ctype, c)  \
type name(atype a, btype b, ctype c, ...)                      \
{                                               \
	register int par1 __asm__("L15");   \
	register int par2 __asm__("L14");   \
	register int par3 __asm__("L13");   \
	register int par4 __asm__("L12");   \
	register int par5 __asm__("L11");   \
	par1 = -1;              \
	par2 = __NR_##name;         \
	par3 = (int)a;              \
	par4 = (int)b;              \
	par5 = (int)c;              \
	__asm__ __volatile__(                   \
		"trap    47"                        \
		:"=l"(par1)                             \
		:"0"(par1), "l"(par2), "l"(par3), "l"(par4), "l"(par5) \
		:"memory","L11","L12","L13","L14","L15");    \
																	\
	if( par1 < 0 ) {													\
			__set_errno( -par1 );										\
			return -1;											\
	} else															\
		return (type)(par1);                    					\
}

#define __syscall4(type, name,atype, a, btype, b, ctype, c, dtype, d)  \
type name(atype a, btype b, ctype c,dtype d, ...)                      \
{                                               \
	register int par1 __asm__("L15");   \
	register int par2 __asm__("L14");   \
	register int par3 __asm__("L13");   \
	register int par4 __asm__("L12");   \
	register int par5 __asm__("L11");   \
	register int par6 __asm__("L10");   \
	par1 = -1;              \
	par2 = __NR_##name;         \
	par3 = (int)a;              \
	par4 = (int)b;              \
	par5 = (int)c;              \
	par6 = (int)d;              \
	__asm__ __volatile__(                   \
		"trap    47"                        \
		:"=l"(par1)                             \
		:"0"(par1),"l"(par2),"l"(par3),"l"(par4),"l"(par5),"l"(par6) \
		:"memory","L10","L11","L12","L13","L14","L15");    \
																	\
	if( par1 < 0 ) {													\
			__set_errno( -par1 );										\
			return -1;											\
	} else															\
		return (type)(par1);                    					\
}

#define __syscall5(type, name,atype, a, btype, b, ctype, c, dtype, d, etype, e)  \
type name(atype a, btype b, ctype c,dtype d, etype e, ...)                      \
{                                                       \
	register int par1 __asm__("L15");   \
	register int par2 __asm__("L14");   \
	register int par3 __asm__("L13");   \
	register int par4 __asm__("L12");   \
	register int par5 __asm__("L11");   \
	register int par6 __asm__("L10");   \
	register int par7 __asm__("L9");   \
	par1 = -1;                  \
	par2 = __NR_##name;         \
	par3 = (int)a;              \
	par4 = (int)b;              \
	par5 = (int)c;              \
	par6 = (int)d;              \
	par7 = (int)e;              \
	__asm__ __volatile__(                   \
		"trap    47"                        \
		:"=l"(par1)                             \
		:"0"(par1),"l"(par2),"l"(par3),"l"(par4),"l"(par5),"l"(par6),"l"(par7) \
		:"memory","L9","L10","L11","L12","L13","L14","L15");    \
																	\
	if( par1 < 0 ) {													\
			__set_errno( -par1 );										\
			return -1;											\
	} else															\
		return (type)(par1);                    					\
}

#define __syscall6(type, name,atype, a, btype, b, ctype, c, dtype, d, etype, e, ftype, f)  \
type name(atype a, btype b, ctype c,dtype d, etype e, ftype f, ...)                      \
{                                                               \
	register int par1 __asm__("L15");   \
	register int par2 __asm__("L14");   \
	register int par3 __asm__("L13");   \
	register int par4 __asm__("L12");   \
	register int par5 __asm__("L11");   \
	register int par6 __asm__("L10");   \
	register int par7 __asm__("L9");   \
	register int par8 __asm__("L8");   \
	par1 = -1;                  \
	par2 = __NR_##name;         \
	par3 = (int)a;              \
	par4 = (int)b;              \
	par5 = (int)c;              \
	par6 = (int)d;              \
	par7 = (int)e;              \
	par7 = (int)f;              \
	__asm__ __volatile__(                   \
		"trap    47"                        \
		:"=l"(par1)                             \
		:"0"(par1),"l"(par2),"l"(par3),"l"(par4),"l"(par5),"l"(par6),"l"(par7),"l"(par8) \
		:"memory","L8","L9","L10","L11","L12","L13","L14","L15");    \
																	\
	if( par1 < 0 ) {													\
			__set_errno( -par1 );										\
			return -1;											\
	} else															\
		return (type)(par1);                    					\
}

#include <sys/types.h>
/* Taken from <bits/errno.h> */
#ifndef _LIBC
/* We don't support pthreads for the moment*/
#define __set_errno(val) ((errno) = (val))
#endif

#if 0
#define _syscall3(type, name,atype, a , btype, b, ctype, c)  \
type name(atype a, btype b, ctype c,)                      \
{                                               \
		__asm__ __volatile__(					\
			"movi  L9, -1\n\t"                  \
			"movi  L8, %0\n\t"					\
			"ldw.d G3, L7, 0\n\t"				\
			"ldw.d G3, L6, 4\n\t"				\
			"ldw.d G3, L5, 8\n\t"				\
		:/* no output */						\
		:"i"(__NR_##name)						\
		:"cc","memory","%L5","L6","L7","L8","L9");\
        __asm__ __volatile__(                   \
			"trap    47\n\t"                    \
			"mov L2, L9\n\t");                  \
}

#define _syscall4(type, name,atype, a, btype, b, ctype, c, dtype, d)  \
type name(atype a, btype b, ctype c,dtype d)                      \
{                                               \
		__asm__ __volatile__(					\
			"movi  L11, -1\n\t"                  \
			"movi  L10, %0\n\t"					\
			"ldw.d G3, L9, 0\n\t"				\
			"ldw.d G3, L8, 4\n\t"				\
			"ldw.d G3, L7, 8\n\t"				\
			"ldw.d G3, L6, 12\n\t"				\
		:/* no output */						\
		:"i"(__NR_##name)						\
		:"cc","memory","L6","L7","L8","L9","L10","L11");\
        __asm__ __volatile__(                   \
			"trap    47\n\t"                    \
			"mov L2, L11\n\t");                  \
}

#define _syscall5(type, name,atype, a, btype, b, ctype, c, dtype, d, etype, e)  \
type name(atype a, btype b, ctype c,dtype d, etype e)                      \
{                                                       \
		__asm__ __volatile__(					\
			"movi  L13, -1\n\t"                  \
			"movi  L12, %0\n\t"					\
			"ldw.d G3, L11, 0\n\t"				\
			"ldw.d G3, L10, 4\n\t"				\
			"ldw.d G3, L9, 8\n\t"				\
			"ldw.d G3, L8, 12\n\t"				\
			"ldw.d G3, L7, 16\n\t"				\
		:/* no output */						\
		:"i"(__NR_##name)						\
		:"cc","memory","L7","L8","L9","L10","L11","L12","L13");\
        __asm__ __volatile__(                   \
			"trap    47\n\t"                    \
			"mov L2, L13\n\t");                  \
}

#define _syscall6(type, name,atype, a, btype, b, ctype, c, dtype, d, etype, e, ftype, f)  \
type name(atype a, btype b, ctype c,dtype d, etype e, ftype f)                      \
{                                                               \
		__asm__ __volatile__(					\
			"movi  L15, -1\n\t"                  \
			"movi  L14, %0\n\t"					\
			"ldw.d G3, L13, 0\n\t"				\
			"ldw.d G3, L12, 4\n\t"				\
			"ldw.d G3, L11, 8\n\t"				\
			"ldw.d G3, L10, 12\n\t"				\
			"ldw.d G3, L9, 16\n\t"				\
			"ldw.d G3, L8, 20\n\t"				\
		:/* no output */						\
		:"i"(__NR_##name)						\
		:"cc","memory","L8","L9","L10","L11","L12","L13","L14","L15");\
        __asm__ __volatile__(                   \
			"trap    47\n\t"                    \
			"mov L2, L15\n\t");                  \
}
#endif

#endif /* !_HYPERSTONE_NOMMU_UNISTD_H_ */
