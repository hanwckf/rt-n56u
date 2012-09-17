
#include <asm/unistd.h>

#ifndef __NR_vfork
#define __NR_vfork __NR_fork /* uClinux-2.0 only has fork which is vfork */
#endif

#ifdef __H8300S__
	.h8300s
#else
	.h8300h
#endif
	.text
	.align 2
	.globl	_errno
	.globl	___vfork
	.hidden	___vfork
	.type	___vfork,@function
___vfork:
	mov.l	@sp+, er1
	sub.l	er0,er0
	mov.b	#__NR_vfork,r0l
	trapa	#0
	mov.l	#-4096, er2
	cmp.l 	er0,er2
	bcs     fix_errno
	jmp	@er1                     /* don't return,  just jmp directly */
fix_errno:
	neg.l	er0
#if !defined(__PIC__)
	mov.l	er0,@_errno
#else
	mov.l	@(_errno@GOTOFF,er5),er2
	mov.l	er0,@er2
#endif
	sub.l	er0,er0
	dec.l	#1,er0
	jmp	@er1                     /* don't return,  just jmp directly */

weak_alias(__vfork,vfork)
libc_hidden_weak(vfork)
