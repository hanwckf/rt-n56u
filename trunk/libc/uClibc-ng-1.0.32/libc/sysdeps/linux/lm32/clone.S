#include <features.h>
#include <sys/syscall.h>
#define _ERRNO_H    1
#include <bits/errno.h>

#ifdef __NR_clone

.text
.global clone
.type clone,@function
.align 4

clone:
	be	r1, r0, 2f
	be	r2, r0, 2f

	mvi	r8, __NR_clone
	scall

	mvi	r3, -4095
	bgeu	r1, r3, 1f

	be      r1, r0, __thread_start

	ret

__thread_start:
	mvi	fp, 0
	call	r2

	calli HIDDEN_JUMPTARGET(_exit)

    /* Stop the unstoppable.  */
9:
	bi      9b

2:
	mvi     r1, -EINVAL
1:
	addi	sp, sp, -8
	sw	(sp+4), ra
	sw	(sp+8), r1
	calli   __errno_location
 	lw	r2, (sp+8)
	lw	ra, (sp+4)
	sub	r2, r0, r2
	sw      (r1+0), r2
	mvi     r1, -1
	addi	sp, sp, 8
	ret

#endif /* __NR_clone */
