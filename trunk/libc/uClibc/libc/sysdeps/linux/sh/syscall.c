

#include <features.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/syscall.h>

long syscall(long sysnum,
			 long arg1, long arg2, long arg3,
			 long arg4, long arg5, long arg6)
{
register long __sc3 __asm__ ("r3") = sysnum;
register long __sc4 __asm__ ("r4") = (long) arg1;
register long __sc5 __asm__ ("r5") = (long) arg2;
register long __sc6 __asm__ ("r6") = (long) arg3;
register long __sc7 __asm__ ("r7") = (long) arg4;
register long __sc0 __asm__ ("r0") = (long) arg5;
register long __sc1 __asm__ ("r1") = (long) arg6;
__asm__ __volatile__ ("trapa	#0x16" \
	: "=z" (__sc0) \
	: "0" (__sc0), "r" (__sc4), "r" (__sc5), "r" (__sc6), "r" (__sc7), \
	  "r" (__sc3), "r" (__sc1) \
	: "memory" );
__syscall_return(long,__sc0);
}
