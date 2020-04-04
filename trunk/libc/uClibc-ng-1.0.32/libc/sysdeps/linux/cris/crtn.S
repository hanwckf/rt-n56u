/* glibc's sysdeps/cris/elf/initfini.c used for reference [EPILOG] */

	.section	.init
	.align	1
	.global	_init
	.type	_init, @function
	move.d	[$sp+],$r0
	move	$r1,$srp
	move.d	[$sp+],$r1
	Ret
	nop

	.section	.fini
	.align	1
	.global	_fini
	.type	_fini, @function
	move.d	[$sp+],$r0
	move	$r1,$srp
	move.d	[$sp+],$r1
	Ret
	nop
