/*
 * June 3, 2003             Erik Andersen
 *
 */

#include <sys/syscall.h>

#ifndef __NR_vfork

/* No vfork so use fork instead */
.weak vfork ; vfork = __libc_fork

#else

.text
.global __vfork
.type   __vfork,%function
.align 1<<4

__vfork:
	popl %ecx
	movl $__NR_vfork,%eax
	int $0x80
	pushl %ecx
	cmpl $-4095,%eax
	jae __syscall_error
	ret

.size __vfork,.-__vfork


.weak vfork ; vfork = __vfork
#endif
