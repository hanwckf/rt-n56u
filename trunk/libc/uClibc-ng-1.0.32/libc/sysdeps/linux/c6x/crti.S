/*
 * This file just supplies function prologues for the .init and .fini
 * sections.  It is linked in before crtbegin.o.
 */

	.section .init
	.globl  _init
	.type   _init,@function
_init:
	add	.l2	-8, B15, B15
	stw	.d2t2	B3,*+B15(4)
	.section .fini
	.globl  _fini
	.type   _fini,@function
_fini:
	add	.l2	-8, B15, B15
	stw	.d2t2	B3,*+B15(4)
