
	.section .init
	.align	2
	.global	_init
	.type	_init, @function
_init:
	stm	--sp, r6, lr
	lddpc	r6, 2f
1:	rsub	r6, pc
	rjmp	3f
	.align	2
2:	.long	1b - _GLOBAL_OFFSET_TABLE_
3:

	.section .fini
	.align	2
	.global	_fini
	.type	_fini, @function
_fini:
	stm	--sp, r6, lr
	lddpc	r6, 2f
1:	rsub	r6, pc
	rjmp	3f
	.align	2
2:	.long	1b - _GLOBAL_OFFSET_TABLE_
3:
