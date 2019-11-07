/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

#include <sysdep.h>

 .text
.global	_start
.type   _start,%function
.global __exit
.global atexit

.global _init
.global _fini

.global main
.global __uClibc_main

/*
 * argc, argv and envp are on the stack
 *
 * Call:
 * void __uClibc_main(
 *         int (*main)(int, char **, char **),
 *         int argc,
 *         char **argv,
 *         void (*app_init)(void),
 *         void (*app_fini)(void),
 *         void (*rtld_fini)(void),
 *         void *stack_end attribute_unused);
 */

_start:
#ifdef __PIC__
	__GET_GB
	lrw	a0, main@GOT
	addu	a0, gb
	ldw	a0, (a0)

	ldw	a1, (sp)
	mov	a2, sp
	addi	a2, 4

	mov	a3, sp /* push stack_end */
	subi	sp, 8
	stw	a3, (sp)

	lrw	a3, _init@GOT
	addu	a3, gb
	ldw	a3, (a3)

#ifdef __CSKYABIV2__
	subi	sp, 8
	lrw	l4, _fini@GOT
	addu	l4, gb
	ldw	l4, (l4)
	stw	l4, (sp)

	stw	r7, (sp, 4) /* push rtld_fini */
#else
	lrw	a4, _fini@GOT
	addu	a4, gb
	ldw	a4, (a4)
#endif

	lrw	l4,  __uClibc_main@PLT
	addu	l4, gb
	ldw	l4, (l4)
	jsr	l4

#else /* __PIC__ */
	lrw	a0, main

	ldw	a1, (sp)
	mov	a2, sp
	addi	a2, 4

	mov	a3, sp     /* push stack_end */
	subi	sp, 8
	stw	a3, (sp)

	lrw	a3, _init
#ifdef __CSKYABIV2__
	subi	sp, 8
	lrw	l4, _fini
	stw	l4, (sp)
	stw	r7, (sp, 4)
#else
	lrw	a4, _fini
#endif

	lrw	l4, __uClibc_main
	jsr	l4
#endif /* __PIC__ */
	bsr	__exit
__exit:
	DO_CALL(exit, 0)
	br .

