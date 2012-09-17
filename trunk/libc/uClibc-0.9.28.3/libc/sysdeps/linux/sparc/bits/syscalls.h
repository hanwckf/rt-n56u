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

/*
   macros taken from glibc-2.3/sysdeps/unix/sysv/linux/sparc/sysdep.h
   Copyright (C) 2000, 2002 Free Software Foundation, Inc.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2000.
*/

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

#undef _syscall6
#define _syscall6(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4, \
	  type5,arg5,type6,arg6) \
type name (type1 arg1,type2 arg2,type3 arg3,type4 arg4,type5 arg5, type6 arg6) \
{ \
return (type) (INLINE_SYSCALL(name, 6, arg1, arg2, arg3, arg4, arg5, arg6)); \
}

#undef _syscall7
#define _syscall7(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4, \
	  type5,arg5,type6,arg6,type7,arg7) \
type name (type1 arg1,type2 arg2,type3 arg3,type4 arg4,type5 arg5, type6 arg6,type7 arg7) \
{ \
return (type) (INLINE_SYSCALL(name, 7, arg1, arg2, arg3, arg4, arg5, arg6, arg7)); \
}

#define __SYSCALL_STRING						\
	"ta	0x10;"							\
	"bcs	2f;"							\
	" nop;"								\
	"1:"								\
	".subsection 2;"						\
	"2:"								\
	"save	%%sp, -192, %%sp;"					\
	"call	__errno_location;"					\
	" nop;"								\
	"st	%%i0,[%%o0];"						\
	"ba	1b;"							\
	" restore %%g0, -1, %%o0;"					\
	".previous;"

#define __SYSCALL_CLOBBERS "g2", "g3", "g4", "g5", "g7",		\
	"f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7",			\
	"f8", "f9", "f10", "f11", "f12", "f13", "f14", "f15",		\
	"f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23",		\
	"f24", "f25", "f26", "f27", "f28", "f29", "f30", "f31",		\
	"cc", "memory"

#undef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...) _INLINE_SYSCALL##nr(name, args)

#define _INLINE_SYSCALL0(name,dummy...)					\
({									\
	register long __o0 __asm__ ("o0");				\
	register long __g1 __asm__ ("g1") = __NR_##name;		\
	__asm __volatile (__SYSCALL_STRING : "=r" (__g1), "=r" (__o0) :	\
			  "0" (__g1) :					\
			  __SYSCALL_CLOBBERS);				\
	__o0;								\
})

#define _INLINE_SYSCALL1(name,arg1)						\
({									\
	register long __o0 __asm__ ("o0") = (long)(arg1);		\
	register long __g1 __asm__ ("g1") = __NR_##name;		\
	__asm __volatile (__SYSCALL_STRING : "=r" (__g1), "=r" (__o0) :	\
			  "0" (__g1), "1" (__o0) :			\
			  __SYSCALL_CLOBBERS);				\
	__o0;								\
})

#define _INLINE_SYSCALL2(name,arg1,arg2)					\
({									\
	register long __o0 __asm__ ("o0") = (long)(arg1);		\
	register long __o1 __asm__ ("o1") = (long)(arg2);		\
	register long __g1 __asm__ ("g1") = __NR_##name;		\
	__asm __volatile (__SYSCALL_STRING : "=r" (__g1), "=r" (__o0) :	\
			  "0" (__g1), "1" (__o0), "r" (__o1) :		\
			  __SYSCALL_CLOBBERS);				\
	__o0;								\
})

#define _INLINE_SYSCALL3(name,arg1,arg2,arg3)					\
({									\
	register long __o0 __asm__ ("o0") = (long)(arg1);		\
	register long __o1 __asm__ ("o1") = (long)(arg2);		\
	register long __o2 __asm__ ("o2") = (long)(arg3);		\
	register long __g1 __asm__ ("g1") = __NR_##name;		\
	__asm __volatile (__SYSCALL_STRING : "=r" (__g1), "=r" (__o0) :	\
			  "0" (__g1), "1" (__o0), "r" (__o1),		\
			  "r" (__o2) :					\
			  __SYSCALL_CLOBBERS);				\
	__o0;								\
})

#define _INLINE_SYSCALL4(name,arg1,arg2,arg3,arg4)				\
({									\
	register long __o0 __asm__ ("o0") = (long)(arg1);		\
	register long __o1 __asm__ ("o1") = (long)(arg2);		\
	register long __o2 __asm__ ("o2") = (long)(arg3);		\
	register long __o3 __asm__ ("o3") = (long)(arg4);		\
	register long __g1 __asm__ ("g1") = __NR_##name;		\
	__asm __volatile (__SYSCALL_STRING : "=r" (__g1), "=r" (__o0) :	\
			  "0" (__g1), "1" (__o0), "r" (__o1),		\
			  "r" (__o2), "r" (__o3) :			\
			  __SYSCALL_CLOBBERS);				\
	__o0;								\
})

#define _INLINE_SYSCALL5(name,arg1,arg2,arg3,arg4,arg5)			\
({									\
	register long __o0 __asm__ ("o0") = (long)(arg1);		\
	register long __o1 __asm__ ("o1") = (long)(arg2);		\
	register long __o2 __asm__ ("o2") = (long)(arg3);		\
	register long __o3 __asm__ ("o3") = (long)(arg4);		\
	register long __o4 __asm__ ("o4") = (long)(arg5);		\
	register long __g1 __asm__ ("g1") = __NR_##name;		\
	__asm __volatile (__SYSCALL_STRING : "=r" (__g1), "=r" (__o0) :	\
			  "0" (__g1), "1" (__o0), "r" (__o1),		\
			  "r" (__o2), "r" (__o3), "r" (__o4) :		\
			  __SYSCALL_CLOBBERS);				\
	__o0;								\
})

#define _INLINE_SYSCALL6(name,arg1,arg2,arg3,arg4,arg5,arg6)			\
({									\
	register long __o0 __asm__ ("o0") = (long)(arg1);		\
	register long __o1 __asm__ ("o1") = (long)(arg2);		\
	register long __o2 __asm__ ("o2") = (long)(arg3);		\
	register long __o3 __asm__ ("o3") = (long)(arg4);		\
	register long __o4 __asm__ ("o4") = (long)(arg5);		\
	register long __o5 __asm__ ("o5") = (long)(arg6);		\
	register long __g1 __asm__ ("g1") = __NR_##name;		\
	__asm __volatile (__SYSCALL_STRING : "=r" (__g1), "=r" (__o0) :	\
			  "0" (__g1), "1" (__o0), "r" (__o1),		\
			  "r" (__o2), "r" (__o3), "r" (__o4),		\
			  "r" (__o5) :					\
			  __SYSCALL_CLOBBERS);				\
	__o0;								\
})

#endif /* __ASSEMBLER__ */
#endif /* _BITS_SYSCALLS_H */

