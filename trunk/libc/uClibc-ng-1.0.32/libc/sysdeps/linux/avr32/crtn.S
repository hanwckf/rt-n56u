
	.section .init
	.align	2
	.global	_init
	.type	_init, @function
	ldm	sp++, r6, pc

	.section .fini
	.align	2
	.global _fini
	.type	_fini, @function
	ldm	sp++, r6, pc
