#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

/* This includes the `__NR_<name>' syscall numbers taken from the Linux kernel
 * header files.  It also defines the traditional `SYS_<name>' macros for older
 * programs.  */
#include <bits/sysnum.h>

#ifndef SYS_ify
# define SYS_ify(syscall_name)  (__NR_##syscall_name)
#endif

#ifndef __ASSEMBLER__

/* user-visible error numbers are in the range -1 - -4095: see <asm-frv/errno.h> */
#if defined _LIBC && !defined __set_errno
# define __syscall_return(type, res) \
do { \
        unsigned long __sr2 = (res);		    			    \
	if (__builtin_expect ((unsigned long)(__sr2)			    \
			      >= (unsigned long)(-4095), 0)) {		    \
		extern int __syscall_error (int);			    \
		return (type) __syscall_error (__sr2);		    	    \
	}								    \
	return (type) (__sr2); 						    \
} while (0)
#else
# define __syscall_return(type, res) \
do { \
        unsigned long __sr2 = (res);		    			    \
	if (__builtin_expect ((unsigned long)(__sr2)			    \
			      >= (unsigned long)(-4095), 0)) {		    \
		__set_errno (-__sr2);				    	    \
		__sr2 = -1; 						    \
	}								    \
	return (type) (__sr2); 						    \
} while (0)
#endif

#ifndef __set_errno
# define __set_errno(val) ((*__errno_location ()) = (val))
#endif

/* XXX - _foo needs to be __foo, while __NR_bar could be _NR_bar. */

#define _syscall0(type,name) \
type name(void) \
{ \
register unsigned long __scnum __asm__ ("gr7") = (__NR_##name); 	    \
register unsigned long __sc0 __asm__ ("gr8");				    \
__asm__ __volatile__ ("tra	gr0,gr0"				    \
	: "=r" (__sc0) 							    \
	: "r" (__scnum)); 						    \
__syscall_return(type,__sc0); 						    \
}

#define _syscall1(type,name,type1,arg1) \
type name(type1 arg1) \
{ \
register unsigned long __scnum __asm__ ("gr7") = (__NR_##name);		    \
register unsigned long __sc0 __asm__ ("gr8") = (unsigned long) arg1;	    \
__asm__ __volatile__ ("tra	gr0,gr0"				    \
	: "+r" (__sc0) 							    \
	: "r" (__scnum));						    \
__syscall_return(type,__sc0);	 					    \
}

#define _syscall2(type,name,type1,arg1,type2,arg2) \
type name(type1 arg1,type2 arg2) \
{ \
register unsigned long __scnum __asm__ ("gr7") = (__NR_##name);		    \
register unsigned long __sc0 __asm__ ("gr8") = (unsigned long) arg1;	    \
register unsigned long __sc1 __asm__ ("gr9") = (unsigned long) arg2;	    \
__asm__ __volatile__ ("tra	gr0,gr0"				    \
	: "+r" (__sc0)	 						    \
	: "r" (__scnum), "r" (__sc1));					    \
__syscall_return(type,__sc0);	 					    \
}

#define _syscall3(type,name,type1,arg1,type2,arg2,type3,arg3) \
type name(type1 arg1,type2 arg2,type3 arg3) \
{ \
register unsigned long __scnum __asm__ ("gr7") = (__NR_##name);		    \
register unsigned long __sc0 __asm__ ("gr8") = (unsigned long) arg1;	    \
register unsigned long __sc1 __asm__ ("gr9") = (unsigned long) arg2;	    \
register unsigned long __sc2 __asm__ ("gr10") = (unsigned long) arg3;	    \
__asm__ __volatile__ ("tra	gr0,gr0"				    \
	: "+r" (__sc0)	 						    \
	: "r" (__scnum), "r" (__sc1), "r" (__sc2));			    \
__syscall_return(type,__sc0);	 					    \
}

#define _syscall4(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4) \
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4) \
{ \
register unsigned long __scnum __asm__ ("gr7") = (__NR_##name);		    \
register unsigned long __sc0 __asm__ ("gr8") = (unsigned long) arg1;	    \
register unsigned long __sc1 __asm__ ("gr9") = (unsigned long) arg2;	    \
register unsigned long __sc2 __asm__ ("gr10") = (unsigned long) arg3;	    \
register unsigned long __sc3 __asm__ ("gr11") = (unsigned long) arg4;	    \
__asm__ __volatile__ ("tra	gr0,gr0"				    \
	: "+r" (__sc0)	 						    \
	: "r" (__scnum), "r" (__sc1), "r" (__sc2), "r" (__sc3));	    \
__syscall_return(type,__sc0);	 					    \
}

#define _syscall5(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4,type5,arg5) \
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5) \
{ \
register unsigned long __scnum __asm__ ("gr7") = (__NR_##name);		    \
register unsigned long __sc0 __asm__ ("gr8") = (unsigned long) arg1;	    \
register unsigned long __sc1 __asm__ ("gr9") = (unsigned long) arg2;	    \
register unsigned long __sc2 __asm__ ("gr10") = (unsigned long) arg3;	    \
register unsigned long __sc3 __asm__ ("gr11") = (unsigned long) arg4;	    \
register unsigned long __sc4 __asm__ ("gr12") = (unsigned long) arg5;	    \
__asm__ __volatile__ ("tra	gr0,gr0"				    \
	: "+r" (__sc0)	 						    \
	: "r" (__scnum), "r" (__sc1), "r" (__sc2),		 	    \
	  "r" (__sc3), "r" (__sc4));					    \
__syscall_return(type,__sc0);	 					    \
}

#define _syscall6(type,name,type1,arg1,type2,arg2,type3,arg3,type4,arg4,type5,arg5, type6, arg6) \
type name (type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6) \
{ \
register unsigned long __scnum __asm__ ("gr7") = (__NR_##name);		    \
register unsigned long __sc0 __asm__ ("gr8") = (unsigned long) arg1;	    \
register unsigned long __sc1 __asm__ ("gr9") = (unsigned long) arg2;	    \
register unsigned long __sc2 __asm__ ("gr10") = (unsigned long) arg3;	    \
register unsigned long __sc3 __asm__ ("gr11") = (unsigned long) arg4;	    \
register unsigned long __sc4 __asm__ ("gr12") = (unsigned long) arg5;	    \
register unsigned long __sc5 __asm__ ("gr13") = (unsigned long) arg6;	    \
__asm__ __volatile__ ("tra	gr0,gr0"				    \
	: "+r" (__sc0)	 						    \
	: "r" (__scnum), "r" (__sc1), "r" (__sc2),			    \
	  "r" (__sc3), "r" (__sc4), "r" (__sc5));			    \
__syscall_return(type,__sc0);	 					    \
}

#endif /* __ASSEMBLER__ */
#endif /* _BITS_SYSCALLS_H */
