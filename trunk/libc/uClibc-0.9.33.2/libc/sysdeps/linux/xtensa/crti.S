/* glibc's sysdeps/xtensa/elf/initfini.c used for reference [PROLOG] */

	.section .init
	.align	4
	.global	_init
	.type	_init, @function
_init:
	entry	sp, 48


	.section .fini
	.align	4
	.global	_fini
	.type	_fini, @function
_fini:
	entry	sp, 48
