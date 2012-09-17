/* Startup code compliant to the ELF CRIS ABI */

#include <features.h>

	.syntax no_register_prefix

/* The first piece of initialized data.  */
	.data
	.global	__data_start
	.align	2
	.type	__data_start, @object
	.size	__data_start, 4
__data_start:
	.dword	0

	.text
	.align	1
	.global	_start
	.type	_start, %function
#if defined(__UCLIBC_CTOR_DTOR__)
	.type	_init, %function
	.type	_fini, %function
#else
	.weak	_init
	.weak	_fini
#endif
	.type	main, %function
	.type	__uClibc_main, %function

/*
 * On the stack we have argc. We can calculate argv/envp
 * from that and the succeeding stack location, but fix so
 * we get the right calling convention (regs in r10/r11).
 *
 * Please view linux/fs/binfmt_elf.c for a complete
 * understanding of this.
 */

/*
 * Need to call __uClibc_main(main, argc, argv, _init, _fini)
 */

_start:
	move.d	main, r10
	move.d	[sp+], r11
	move.d	sp, r12
	subq	4, sp
	move	srp, [sp]
	subq	4, sp
	move.d	_fini, r13
	move.d	r13, [sp]
	move.d	_init, r13
	/* Leave control to the libc */
	jsr	__uClibc_main
	nop
	.size	_start, .-_start
