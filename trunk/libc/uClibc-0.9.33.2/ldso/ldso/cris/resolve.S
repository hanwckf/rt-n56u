/*
 * This function is _not_ called directly.  It is jumped to from PLT when
 * attempting to use a symbol that has not yet been resolved.  The first time a
 * jump symbol (such as a function call inside a shared library) is used
 * (before it gets resolved) it will jump here.  When we get called the stack
 * contains reloc_offset and tpnt is in MOF.
 *
 * We save all the registers, setup R10 and R11 with the right arguments then
 * call _dl_linux_resolver(tpnt, reloc_offset). _dl_linux_resolver() figures
 * out where the jump symbol is _really_ supposed to have jumped to and returns
 * that to us.  Once we have that, we overwrite tpnt with this fixed up
 * address. We then clean up after ourselves, put all the registers back how we
 * found them, then we jump to where the fixed up address, which is where the
 * jump symbol that got us here really wanted to jump to in the first place.
 */

.globl _dl_linux_resolve
.type _dl_linux_resolve,@function

#ifdef __arch_v32

_dl_linux_resolve:
	subq	4,$sp
	move.d	$r0,[$sp]
	subq	4,$sp
	move.d	$r13,[$sp]
	subq	4,$sp
	move.d	$r12,[$sp]
	subq	4,$sp
	move.d	$r11,[$sp]
	subq	4,$sp
	addoq	5*4,$sp,$acr
	move.d	$r10,[$sp]
	subq	4,$sp
	move	$mof,$r10
	move.d	$r9,[$sp]
	subq	4,$sp
	move.d	[$acr],$r11
	move	$srp,[$sp]
	lapc	_GLOBAL_OFFSET_TABLE_,$r0
	move.d	_dl_linux_resolver:PLTG,$r9
	add.d	$r0,$r9
	jsr	$r9
	nop
	move.d	$r10,$acr
	move	[$sp+],$srp
	move.d	[$sp+],$r9
	move.d	[$sp+],$r10
	move.d	[$sp+],$r11
	move.d	[$sp+],$r12
	move.d	[$sp+],$r13
	move.d	[$sp+],$r0
	jump	$acr
	addq	4,$sp

#else

_dl_linux_resolve:
	push	$r13
	push	$r12
	push	$r11
	push	$r10
	push	$r9
	push	$r0
	push	$srp
	move.d	[$sp+7*4],$r11
	move	$mof,$r10
#ifdef __PIC__
	move.d	$pc,$r0
	sub.d	.:GOTOFF,$r0
	move.d	_dl_linux_resolver:PLTG,$r9
	add.d	$r0,$r9
	jsr	$r9
#else
	jsr	_dl_linux_resolver
#endif
	move.d	$r10,[$sp+7*4]
	pop	$srp
	pop	$r0
	pop	$r9
	pop	$r10
	pop	$r11
	pop	$r12
	pop	$r13
	jump	[$sp+]

#endif /* __arch_v32 */

	.size _dl_linux_resolve, . - _dl_linux_resolve
