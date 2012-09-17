.globl _setjmp
.align 4
_setjmp:
	.word	0x0040

	/* push an empty word onto the stack */
	pushl	$0

	/* now copy handler, psw, ap, fp and pc on the stack up one word */
	movl	4(%sp), (%sp)		/* copy handler */
	movl	8(%sp), 4(%sp)		/* psw */
	movl	12(%sp), 8(%sp)		/* ap */
	movl	16(%sp), 12(%sp)	/* fp */
	movl	20(%sp), 16(%sp)	/* pc */
	movl	24(%sp), 20(%sp)	/* r6 from register mask */

	movl	$2, 24(%sp)		/* set the number of arguments to 2 */
	movl	32(%sp), 28(%sp)	/* copy the jmp_buf */
	movl	$1, 32(%sp)		/* put the 1 on the stack */

	addl3	$24, %sp, %ap
	movl	%sp, %fp

	moval	__sigsetjmp, %r0
	addl2	$2, %r0
	pushl	%r0
	rsb

.globl setjmp
.align 4
setjmp:
	.word	0x0040
	pushl	$0

	/* now copy handler, psw, ap, fp and pc on the stack up one word */
	movl	4(%sp), (%sp)
	movl	8(%sp), 4(%sp)
	movl	12(%sp), 8(%sp)
	movl	16(%sp), 12(%sp)
	movl	20(%sp), 16(%sp)
	movl	24(%sp), 20(%sp)	/* r6 from register mask */

	movl	$2, 24(%sp)		/* set the number of arguments to 2 */
	movl	32(%sp), 28(%sp)	/* copy the jmp_buf */
	movl	$0, 32(%sp)		/* put the 0 on the stack */

	addl3	$24, %sp, %ap
	movl	%sp, %fp

	moval	__sigsetjmp, %r0
	addl2	$2, %r0
	pushl	%r0
	rsb
