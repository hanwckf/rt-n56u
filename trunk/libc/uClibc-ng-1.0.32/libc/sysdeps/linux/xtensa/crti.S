/* glibc's sysdeps/xtensa/elf/initfini.c used for reference [PROLOG] */

	.section .init
	.align	4
	.global	_init
	.type	_init, @function
_init:
#if defined(__XTENSA_WINDOWED_ABI__)
	entry	sp, 48
#elif defined(__XTENSA_CALL0_ABI__)
	addi	sp, sp, -16
	s32i	a0, sp, 0
#else
#error Unsupported Xtensa ABI
#endif

	.section .fini
	.align	4
	.global	_fini
	.type	_fini, @function
_fini:
#if defined(__XTENSA_WINDOWED_ABI__)
	entry	sp, 48
#elif defined(__XTENSA_CALL0_ABI__)
	addi	sp, sp, -16
	s32i	a0, sp, 0
#else
#error Unsupported Xtensa ABI
#endif
