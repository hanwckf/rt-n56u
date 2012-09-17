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
#ifndef SYS_ify
# define SYS_ify(syscall_name)  (__NR_##syscall_name)
#endif

#ifndef __ASSEMBLER__

/* user-visible error numbers are in the range -1 - -125: see <asm-sh64/errno.h> */
#define __syscall_return(type, res) \
do { \
	/* Note: when returning from kernel the return value is in r9	    \
	**       This prevents conflicts between return value and arg1      \
	**       when dispatching signal handler, in other words makes	    \
	**       life easier in the system call epilogue (see entry.S)      \
	*/								    \
        register unsigned long __sr2 __asm__ ("r2") = res;		    \
	if ((unsigned long)(res) >= (unsigned long)(-125)) { \
		errno = -(res);						    \
		__sr2 = -1; 						    \
	} \
	return (type) (__sr2); 						    \
} while (0)

/* XXX - _foo needs to be __foo, while __NR_bar could be _NR_bar. */

#define _syscall0(type,name) \
type name(void) \
{ \
register unsigned long __sc0 __asm__ ("r9") = ((0x10 << 16) | __NR_##name); \
__asm__ __volatile__ ("trapa	%1" \
	: "=r" (__sc0) 							    \
	: "r" (__sc0) ); 						    \
__syscall_return(type,__sc0); 						    \
}

#define _syscall1(type,name,type1,arg1) \
type name(type1 arg1) \
{ \
register unsigned long __sc0 __asm__ ("r9") = ((0x11 << 16) | __NR_##name); \
register unsigned long __sc2 __asm__ ("r2") = (unsigned long) arg1;	    \
__asm__ __volatile__ ("trapa	%1" \
	: "=r" (__sc0) 							    \
	: "r" (__sc0), "r" (__sc2));					    \
__syscall_return(type,__sc0); 						    \
}

#define _syscall2(type,name,type1,arg1,type2,arg2) \
type name(type1 arg1,type2 arg2) \
{ \
register unsigned long __sc0 __asm__ ("r9") = ((0x12 << 16) | __NR_##name); \
register unsigned long __sc2 __asm__ ("r2") = (unsigned long) arg1;	    \
register unsigned long __sc3 __asm__ ("r3") = (unsigned long) arg2;	    \
__asm__ __volatile__ ("trapa	%1" \
	: "=r" (__sc0) 							    \
	: "r" (__sc0), "r" (__sc2), "r" (__sc3) );			    \
__syscall_return(type,__sc0); 						    \
}

#define _syscall3(type,name,type1,arg1,type2,arg2,type3,arg3) \
type name(type1 arg1,type2 arg2,type3 arg3) \
{ \
register unsigned long __sc0 __asm__ ("r9") = ((0x13 << 16) | __NR_##name); \
register unsigned long __sc2 __asm__ ("r2") = (unsigned long) arg1;	    \
register unsigned long __sc3 __asm__ ("r3") = (unsigned long) arg2;	    \
register unsigned long __sc4 __asm__ ("r4") = (unsigned long) arg3;	    \
__asm__ __volatile__ ("trapa	%1" \
	: "=r" (__sc0) 							    \
	: "r" (__sc0), "r" (__sc2), "r" (__sc3), "r" (__sc4) );		    \
__syscall_return(type,__sc0); 						    \
}

#define _syscall4(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4) \
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4) \
{ \
register unsigned long __sc0 __asm__ ("r9") = ((0x14 << 16) | __NR_##name); \
register unsigned long __sc2 __asm__ ("r2") = (unsigned long) arg1;	    \
register unsigned long __sc3 __asm__ ("r3") = (unsigned long) arg2;	    \
register unsigned long __sc4 __asm__ ("r4") = (unsigned long) arg3;	    \
register unsigned long __sc5 __asm__ ("r5") = (unsigned long) arg4;	    \
__asm__ __volatile__ ("trapa	%1" \
	: "=r" (__sc0) 							    \
	: "r" (__sc0), "r" (__sc2), "r" (__sc3), "r" (__sc4), "r" (__sc5) );\
__syscall_return(type,__sc0); 						    \
}

#define _syscall5(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4,type5,arg5) \
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5) \
{ \
register unsigned long __sc0 __asm__ ("r9") = ((0x15 << 16) | __NR_##name); \
register unsigned long __sc2 __asm__ ("r2") = (unsigned long) arg1;	    \
register unsigned long __sc3 __asm__ ("r3") = (unsigned long) arg2;	    \
register unsigned long __sc4 __asm__ ("r4") = (unsigned long) arg3;	    \
register unsigned long __sc5 __asm__ ("r5") = (unsigned long) arg4;	    \
register unsigned long __sc6 __asm__ ("r6") = (unsigned long) arg5;	    \
__asm__ __volatile__ ("trapa	%1" \
	: "=r" (__sc0) 							    \
	: "r" (__sc0), "r" (__sc2), "r" (__sc3), "r" (__sc4), "r" (__sc5), \
	  "r" (__sc6));							    \
__syscall_return(type,__sc0); 						    \
}

#define _syscall6(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4,type5,arg5, type6, arg6) \
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6) \
{ \
register unsigned long __sc0 __asm__ ("r9") = ((0x16 << 16) | __NR_##name); \
register unsigned long __sc2 __asm__ ("r2") = (unsigned long) arg1;	    \
register unsigned long __sc3 __asm__ ("r3") = (unsigned long) arg2;	    \
register unsigned long __sc4 __asm__ ("r4") = (unsigned long) arg3;	    \
register unsigned long __sc5 __asm__ ("r5") = (unsigned long) arg4;	    \
register unsigned long __sc6 __asm__ ("r6") = (unsigned long) arg5;	    \
register unsigned long __sc7 __asm__ ("r7") = (unsigned long) arg6;	    \
__asm__ __volatile__ ("trapa	%1" \
	: "=r" (__sc0) 							    \
	: "r" (__sc0), "r" (__sc2), "r" (__sc3), "r" (__sc4), "r" (__sc5), \
	  "r" (__sc6), "r" (__sc7));					    \
__syscall_return(type,__sc0); 						    \
}

#endif /* __ASSEMBLER__ */
#endif /* _BITS_SYSCALLS_H */

