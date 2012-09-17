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

#define __SYSCALL_CLOBBERS "$1", "$3", "$8", "$9", "$10", "$11", "$12", "$13", \
	"$14", "$15", "$24", "$25", "memory"

#define _syscall0(type,name) \
type name(void) \
{ 									\
	long err;							\
	long sys_result;						\
	{								\
	register unsigned long __v0 asm("$2"); 				\
	register unsigned long __a3 asm("$7"); 				\
	__asm__ volatile ( 						\
	".set	noreorder\n\t" 						\
	"li	$2, %2	# " #name "\n\t"				\
	"syscall\n\t" 							\
	".set reorder" 							\
	: "=r" (__v0), "=r" (__a3) 					\
	: "i" (SYS_ify(name))						\
	: __SYSCALL_CLOBBERS); \
	err = __a3;							\
	sys_result = __v0;						\
	}								\
	if (err == 0)							\
		return (type) sys_result;				\
	__set_errno(sys_result);					\
	return (type)-1;						\
}

#define _syscall1(type,name,atype,a) \
type name(atype a) \
{ 									\
	long err;							\
	long sys_result;						\
	{								\
	register unsigned long __v0 asm("$2"); 				\
	register unsigned long __a0 asm("$4") = (unsigned long) a; 	\
	register unsigned long __a3 asm("$7"); 				\
	__asm__ volatile ( 						\
	".set	noreorder\n\t" 						\
	"li	$2, %3\t\t\t# " #name "\n\t"				\
	"syscall\n\t" 							\
	".set reorder" 							\
	: "=r" (__v0), "=r" (__a3) 					\
	: "r" (__a0), "i" (SYS_ify(name)) 				\
	: __SYSCALL_CLOBBERS); \
	err = __a3;							\
	sys_result = __v0;						\
	}								\
	if (err == 0)							\
		return (type) sys_result;				\
	__set_errno(sys_result);					\
	return (type)-1;						\
}

#define _syscall2(type,name,atype,a,btype,b) \
type name(atype a,btype b) \
{ 									\
	long err;							\
	long sys_result;						\
	{								\
	register unsigned long __v0 asm("$2"); 				\
	register unsigned long __a0 asm("$4") = (unsigned long) a; 	\
	register unsigned long __a1 asm("$5") = (unsigned long) b; 	\
	register unsigned long __a3 asm("$7"); 				\
	__asm__ volatile ( 						\
	".set	noreorder\n\t" 						\
	"li	$2, %4\t\t\t# " #name "\n\t" 				\
	"syscall\n\t" 							\
	".set	reorder" 						\
	: "=r" (__v0), "=r" (__a3) 					\
	: "r" (__a0), "r" (__a1), "i" (SYS_ify(name))			\
	: __SYSCALL_CLOBBERS); \
	err = __a3;							\
	sys_result = __v0;						\
	}								\
	if (err == 0)							\
		return (type) sys_result;				\
	__set_errno(sys_result);					\
	return (type)-1;						\
}

#define _syscall3(type,name,atype,a,btype,b,ctype,c) \
type name (atype a, btype b, ctype c) \
{ 									\
	long err;							\
	long sys_result;						\
	{								\
	register unsigned long __v0 asm("$2"); 				\
	register unsigned long __a0 asm("$4") = (unsigned long) a; 	\
	register unsigned long __a1 asm("$5") = (unsigned long) b; 	\
	register unsigned long __a2 asm("$6") = (unsigned long) c; 	\
	register unsigned long __a3 asm("$7"); 				\
	__asm__ volatile ( 						\
	".set	noreorder\n\t" 						\
	"li	$2, %5\t\t\t# " #name "\n\t" 				\
	"syscall\n\t" 							\
	".set	reorder" 						\
	: "=r" (__v0), "=r" (__a3) 					\
	: "r" (__a0), "r" (__a1), "r" (__a2), "i" (SYS_ify(name)) 	\
	: __SYSCALL_CLOBBERS); \
	err = __a3;							\
	sys_result = __v0;						\
	}								\
	if (err == 0)							\
		return (type) sys_result;				\
	__set_errno(sys_result);					\
	return (type)-1;						\
}

#define _syscall4(type,name,atype,a,btype,b,ctype,c,dtype,d) \
type name (atype a, btype b, ctype c, dtype d) \
{ 									\
	long err;							\
	long sys_result;						\
	{								\
	register unsigned long __v0 asm("$2"); 				\
	register unsigned long __a0 asm("$4") = (unsigned long) a; 	\
	register unsigned long __a1 asm("$5") = (unsigned long) b; 	\
	register unsigned long __a2 asm("$6") = (unsigned long) c; 	\
	register unsigned long __a3 asm("$7") = (unsigned long) d; 	\
	__asm__ volatile ( 						\
	".set	noreorder\n\t" 						\
	"li	$2, %5\t\t\t# " #name "\n\t" 				\
	"syscall\n\t" 							\
	".set	reorder" 						\
	: "=r" (__v0), "+r" (__a3) 					\
	: "r" (__a0), "r" (__a1), "r" (__a2), "i" (SYS_ify(name)) 	\
	: __SYSCALL_CLOBBERS); \
	err = __a3;							\
	sys_result = __v0;						\
	}								\
	if (err == 0)							\
		return (type) sys_result;				\
	__set_errno(sys_result);					\
	return (type)-1;						\
}

#define _syscall5(type,name,atype,a,btype,b,ctype,c,dtype,d,etype,e) \
type name (atype a,btype b,ctype c,dtype d,etype e) \
{ 									\
	long err;							\
	long sys_result;						\
	const unsigned long *constE = (void*)(unsigned long) e;		\
	{								\
	register unsigned long __v0 asm("$2"); 				\
	register unsigned long __a0 asm("$4") = (unsigned long) a; 	\
	register unsigned long __a1 asm("$5") = (unsigned long) b; 	\
	register unsigned long __a2 asm("$6") = (unsigned long) c; 	\
	register unsigned long __a3 asm("$7") = (unsigned long) d; 	\
	__asm__ volatile ( 						\
	".set	noreorder\n\t" 						\
	"lw	$2, %6\n\t" 						\
	"subu	$29, 32\n\t" 						\
	"sw	$2, 16($29)\n\t" 					\
	"li	$2, %5\t\t\t# " #name "\n\t" 				\
	"syscall\n\t" 							\
	"addiu	$29, 32\n\t" 						\
	".set	reorder" 						\
	: "=r" (__v0), "+r" (__a3) 					\
	: "r" (__a0), "r" (__a1), "r" (__a2), "i" (SYS_ify(name)), 	\
	  "m" (constE)							\
	: __SYSCALL_CLOBBERS); \
	err = __a3;							\
	sys_result = __v0;						\
	}								\
	if (err == 0)							\
		return (type) sys_result;				\
	__set_errno(sys_result);					\
	return (type)-1;						\
}

#define _syscall6(type,name,atype,a,btype,b,ctype,c,dtype,d,etype,e,ftype,f) \
type name (atype a,btype b,ctype c,dtype d,etype e,ftype f) \
{ 									\
	long err;							\
	long sys_result;						\
	const unsigned long *constE = (void*)(unsigned long) e;		\
	const unsigned long *constF = (void*)(unsigned long) f;		\
	{								\
	register unsigned long __v0 asm("$2"); 				\
	register unsigned long __a0 asm("$4") = (unsigned long) a; 	\
	register unsigned long __a1 asm("$5") = (unsigned long) b; 	\
	register unsigned long __a2 asm("$6") = (unsigned long) c; 	\
	register unsigned long __a3 asm("$7") = (unsigned long) d;	\
	__asm__ volatile ( 						\
	".set	noreorder\n\t" 						\
	"lw	$2, %6\n\t" 						\
	"lw	$8, %7\n\t" 						\
	"subu	$29, 32\n\t" 						\
	"sw	$2, 16($29)\n\t" 					\
	"sw	$8, 20($29)\n\t" 					\
	"li	$2, %5\t\t\t# " #name "\n\t" 				\
	"syscall\n\t" 							\
	"addiu	$29, 32\n\t" 						\
	".set	reorder" 						\
	: "=r" (__v0), "+r" (__a3) 					\
	: "r" (__a0), "r" (__a1), "r" (__a2), "i" (SYS_ify(name)), 	\
	  "m" (constE), "m" (constF)					\
	: __SYSCALL_CLOBBERS); \
	err = __a3;							\
	sys_result = __v0;						\
	}								\
	if (err == 0)							\
		return (type) sys_result;				\
	__set_errno(sys_result);					\
	return (type)-1;						\
}

#define _syscall7(type,name,atype,a,btype,b,ctype,c,dtype,d,etype,e,ftype,f,gtype,g) \
type name (atype a,btype b,ctype c,dtype d,etype e,ftype f,gtype g) \
{ 									\
	long err;							\
	long sys_result;						\
	const unsigned long *constE = (void*)(unsigned long) e;		\
	const unsigned long *constF = (void*)(unsigned long) f;		\
	const unsigned long *constG = (void*)(unsigned long) g;		\
	{								\
	register unsigned long __v0 asm("$2"); 				\
	register unsigned long __a0 asm("$4") = (unsigned long) a; 	\
	register unsigned long __a1 asm("$5") = (unsigned long) b; 	\
	register unsigned long __a2 asm("$6") = (unsigned long) c; 	\
	register unsigned long __a3 asm("$7") = (unsigned long) d;	\
	__asm__ volatile ( 						\
	".set	noreorder\n\t" 						\
	"lw	$2, %6\n\t" 						\
	"lw	$8, %7\n\t" 						\
	"lw	$9, %8\n\t" 						\
	"subu	$29, 32\n\t" 						\
	"sw	$2, 16($29)\n\t" 					\
	"sw	$8, 20($29)\n\t" 					\
	"sw	$9, 24($29)\n\t" 					\
	"li	$2, %5\t\t\t# " #name "\n\t" 				\
	"syscall\n\t" 							\
	"addiu	$29, 32\n\t" 						\
	".set	reorder" 						\
	: "=r" (__v0), "+r" (__a3) 					\
	: "r" (__a0), "r" (__a1), "r" (__a2), "i" (SYS_ify(name)), 	\
	  "m" (constE), "m" (constF), "m" (constG)			\
	: __SYSCALL_CLOBBERS); \
	err = __a3;							\
	sys_result = __v0;						\
	}								\
	if (err == 0)							\
		return (type) sys_result;				\
	__set_errno(sys_result);					\
	return (type)-1;						\
}

#endif /* __ASSEMBLER__ */
#endif /* _BITS_SYSCALLS_H */
