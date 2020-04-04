/*
 * Copyright (C) 2004-2007 Atmel Corporation
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.  See the file "COPYING.LIB" in the main directory of this
 * archive for more details.
 *
 * When we enter _start, the stack looks like this:
 *	argc		argument counter
 *	argv[0]		pointer to program name
 *	argv[1..argc-1]	pointers to program args
 *	NULL
 *	env[0..N]	pointers to environment variables
 *	NULL
 *
 * r12 contains a function pointer to be registered with `atexit'.
 * This is how the dynamic linker arranges to have DT_FINI functions
 * called for shared libraries that have been loaded before this
 * code runs.
 *
 * We're going to call the following function:
 * __uClibc_main(int (*main)(int, char **, char **), int argc,
 *		 char **argv, void (*app_init)(void), void (*app_fini)(void),
 *		 void (*rtld_fini)(void), void *stack_end)
 *
 * So we need to set up things as follows:
 *	r12 = address of main
 *	r11 = argc
 *	r10 = &argv[0]
 *	r9  = address of _init
 *	r8  = address of _fini
 *	sp[0] = whatever we got passed in r12
 */

#include <features.h>

	.text
	.global _start
	.type	_start, @function
_start:
	/* Clear the frame pointer and link register since this is the outermost frame.  */
	mov	r7, 0
	mov	lr, 0

	ld.w	r11, sp++		/* argc		*/
	mov	r10, sp			/* &argv[0]	*/

	st.w	--sp, r10		/* stack_end */
	st.w	--sp, r12		/* rtld_fini */

#ifdef __PIC__
	lddpc	r6, .L_GOT
.L_RGOT:
	rsub	r6, pc
	lda.w	r9, _init
	lda.w	r8, _fini
	lda.w	r12, main

	/* Ok, now run uClibc's main() -- should not return */
	call	__uClibc_main

	.align	2
.L_GOT:
	.long	.L_RGOT - _GLOBAL_OFFSET_TABLE_
#else
	lddpc	r9, __init_addr		/* app_init */
	lddpc	r8, __fini_addr		/* app_fini */
	lddpc	r12, __main_addr	/* main */

	/* Ok, now run uClibc's main() -- should not return */
	lddpc	pc, ___uClibc_main_addr

	.align	2
__init_addr:
	.long	_init
__fini_addr:
	.long	_fini
__main_addr:
	.long	main
___uClibc_main_addr:
	.long	__uClibc_main
#endif
	.size	_start, . - _start

	/*
	 * The LSB says we need this.
	 */
	.section ".note.ABI-tag", "a"
	.align	4
	.long	2f - 1f		/* namesz */
	.long	4f - 3f		/* descsz */
	.long	1		/* type   */
1:	.asciz	"GNU"		/* name */
2:	.align	4
3:	.long	0		/* Linux executable */
	.long	2,6,0		/* Earliest compatible kernel */
4:	.align	4
