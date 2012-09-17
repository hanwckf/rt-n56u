/* glibc's sysdeps/cris/elf/initfini.c used for reference [PROLOG] */

	.section	.init
	.align	1
	.global	_init
	.type	_init, @function
_init:
	subq	4,$sp
	move.d	$r1,[$sp]
	move	$srp,$r1
	subq	4,$sp
	move.d	$r0,[$sp]
#ifdef __arch_v32
	lapc	_GLOBAL_OFFSET_TABLE_,$r0
#else
	move.d	$pc,$r0
	sub.d	.:GOTOFF,$r0
#endif
	.align	1

	.section	.fini
	.align	1
	.global	_fini
	.type	_fini, @function
_fini:
	subq	4,$sp
	move.d	$r1,[$sp]
	move	$srp,$r1
	subq	4,$sp
	move.d	$r0,[$sp]
#ifdef __arch_v32
	lapc	_GLOBAL_OFFSET_TABLE_,$r0
#else
	move.d	$pc,$r0
	sub.d	.:GOTOFF,$r0
#endif
	.align	1
