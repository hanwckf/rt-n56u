#include <sysdep.h>

#define __NR___libc_fork __NR_fork
SYSCALL__ (__libc_fork, 0)
	/* R1 is now 0 for the parent and 1 for the child.  Decrement it to
	   make it -1 (all bits set) for the parent, and 0 (no bits set)
	   for the child.  Then AND it with R0, so the parent gets
	   R0&-1==R0, and the child gets R0&0==0.  */
     /* i dunno what the blurb above is useful for. we just return. */
__asm__("ret\n\tnop");
weak_alias(__libc_fork, fork);

