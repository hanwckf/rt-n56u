#include <sys/syscall.h>

#ifndef __NR_vfork

/* No vfork so use fork instead */
.weak vfork ; vfork = __libc_fork

#else

.text
.global __vfork
.type   __vfork,@function
.type   __syscall_error,@function

__vfork:
	li	0, __NR_vfork
	sc
	bnslr+
	b	__syscall_error
.size __vfork,.-__vfork

.weak vfork ; vfork = __vfork
#endif
