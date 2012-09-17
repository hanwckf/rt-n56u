#include <features.h>

/*
 * longjmp.S	atp sept 2001
 *		Jan-Benedict Glaw <jbglaw@lug-owl.de> 2006
 *
 * Restore regs and info and jmp back to a previous setjmp
 */

.globl	__longjmp
.align	4
__longjmp:
	.word	0x0040		/* This matches setjmp and PLT	*/
	movl	0x4(%ap), %r0	/* Our scratch reg		*/
/*	movl	$0, %r0						*/
/*	movl	(%r0), %r0					*/
	/* We are going to modify our stack frame		*/
	/* to the same as that of the setjmp we called earlier	*/
	movl	(%r0), (%fp)		/* cond handler		*/
	movl	0x4(%r0), 0x4(%fp)	/* psw			*/
	movl	0x8(%r0), 0x8(%fp)	/* ap			*/
	movl	0xc(%r0), 0xc(%fp)	/* fp			*/
	movl	0x10(%r0), 0x10(%fp)	/* pc			*/

	/* Restore the regs					*/
	movl	0x14(%r0), %r1
	movl	0x18(%r0), %r2
	movl	0x1c(%r0), %r3
	movl	0x20(%r0), %r4
	movl	0x24(%r0), %r5
	movl	0x28(%r0), %r6
	movl	0x2c(%r0), %r7
	movl	0x30(%r0), %r8
	movl	0x34(%r0), %r9
	movl	0x38(%r0), %r10
	movl	0x3c(%r0), %r11

	/* Check val and set to 1 if set to zero		*/
	movl	0x8(%ap), %r0
	tstl	%r0
	bneq	exit_ok
	movl	$0x1, %r0
exit_ok:
	ret
.size __longjmp,.-__longjmp
libc_hidden_def(__longjmp)

