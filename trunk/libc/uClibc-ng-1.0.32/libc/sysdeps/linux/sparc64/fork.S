/*
 * The fork and vfork system calls are special on sparc64:
 * they return the "other process" pid in %o0 and the
 * "is child" flag in %o1
 */

#include <features.h>
#include <sys/syscall.h>

.text
.global	fork
.type 	fork, @function
.align 4

fork:
	mov	__NR_fork, %g1
	ta	0x6d
	sub	%o1, 1, %o1
	bcc,pt	%xcc, 1f
	  and	%o0, %o1, %o0
	sethi	%hi(errno), %g4
	or	%g4, %lo(errno), %g4
	st	%o0, [%g4]
	retl
	  mov	-1, %o0
1:
       	retl
	  nop

.size fork, . - fork

libc_hidden_def(fork)
