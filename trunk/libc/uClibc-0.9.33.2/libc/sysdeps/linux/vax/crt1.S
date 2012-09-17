/*
 * crt0 for VAX
 */

/*
 * Program stack looks like:
 * sp->	argc            argument counter (integer)
 *	argv[0]         program name (pointer)
 *	argv[1...N]     program args (pointers)
 *	argv[argc-1]    end of args (integer)
 *	NULL
 *	env[0...N]      environment variables (pointers)
 *	NULL
 */

#include <features.h>

.text
.align 4

.global __start
__start:
.global _start
_start:
	/* Kernel uses a_interp + 2, so __start isn't exactly CALLSed,	*/
	/* but we need to have two bytes here, so we use NOPs. This	*/
	/* won't hurt, though R0 would be invalid to push, but at	*/
	/* lease this looks like a real function.			*/
	.word	0x0101

	movl	$0, %fp			/* FP = 0, since this is the	*/
					/* top-most stack frame		*/
	movl	%sp, %r0		/* R0 = %sp			*/
	movl	(%sp)+, %r4		/* R4 = argc			*/
	movl	%sp, %r3		/* R3 = argv = &argv[0]		*/

#if (defined L_crt1 || defined L_gcrt1) && defined __UCLIBC_CTOR_DTOR__
	pushl	%r0	/* stack_end					*/
	pushl	$0	/* rtld_fini. This is probably needed for the	*/
			/* case where a dynamic linker is involved. So	*/
			/* this is an open FIXME that needs to be	*/
			/* addressed at some time...			*/
	pushl	$_fini
	pushl	$_init
	pushl	%r3	/* Argument pointer				*/
	pushl	%r4	/* And the argument count			*/
	pushl	$main	/* main()					*/

	/* We need to call __uClibc_main which should not return.
	 * __uClibc_main (int (*main) (int, char **, char **),
	 *                int argc,
	 *                char **argv,
	 *                void (*init) (void),
	 *                void (*fini) (void),
	 *                void (*rtld_fini) (void),
	 *                void *stack_end);
	 */
	calls	$7, __uClibc_main
#else /* FIXME: THIS IS BROKEN!!! */
	/* start to load the arguments from the stack */
	/* arguments are on ap stack */
	pushl	%r2
	pushl	%r3
	pushl	%r4

	calls	$3, __uClibc_main
#endif

	/* The above __uClibc_start_main() shouldn't ever return. If it	*/
	/* does, we just crash.						*/
	halt
.align 2

