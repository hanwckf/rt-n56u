
#include <asm/unistd.h>

#ifndef __NR_vfork
#define __NR_vfork __NR_fork /* uClinux-2.0 only has fork which is vfork */
#endif

#define IMM #

	.text
	.align 2
	.globl errno
	.globl vfork
#if defined __HAVE_ELF__
	.type	 vfork,@function
#endif
vfork:
	movl	%sp@+, %a1               /* save the return address for later */
	movl	IMM __NR_vfork,%d0
	trap	#0
	movl	IMM -4097, %d1
	cmpl 	%d0, %d1
	bcs     fix_errno
	jmp		%a1@                     /* don't return,  just jmp directly */
fix_errno:
	negl	%d0
#ifndef __PIC__
	movl	errno, %a0
#else
	movl	errno@GOT(%a5), %a0
#endif
	movl	%d0, %a0@
	movl	IMM -1, %d0
	jmp		%a1@                     /* don't return,  just jmp directly */

