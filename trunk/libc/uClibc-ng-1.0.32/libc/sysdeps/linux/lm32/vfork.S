#include <features.h>

#define _ERRNO_H
#include <bits/errno.h>
#include <asm/unistd.h>
#define _SIGNAL_H
#include <bits/signum.h>

#define CLONE_VM        0x00000100     /* set if VM shared between processes */
#define CLONE_VFORK     0x00004000     /* set if the parent wants the child to */

.text
.global	__vfork
.hidden	__vfork
.type	__vfork,%function
.align 4

__vfork:
	mvi	r2, 0
	mvi	r3, CLONE_VFORK | CLONE_VM | SIGCHLD
	mvi	r8, __NR_clone
	scall

	mvi	r2, -4096
	bgeu	r2, r1, 1f

	addi	sp, sp, -8
	sw	(sp+4), ra
	sw	(sp+8), r1
	calli	__errno_location
	lw	r2, (sp+8)
	lw	ra, (sp+4)
	sub	r2, r0, r2
	sw	(r1+0), r2
	addi	sp, sp, 8
	mvi	r1, -1
1:
	ret

.size __vfork,.-__vfork
weak_alias(__vfork,vfork)
libc_hidden_weak(vfork)
