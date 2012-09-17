/* Copyright (C) 2003 Red Hat, Inc.
 * Contributed by Alexandre Oliva <aoliva@redhat.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

     /* The function below is tail-called by resolver stubs when a
	lazily-bound function is called.  It must preserve all
	registers that could be used to pass arguments to the actual
	function.  Upon _dl_linux_resolve entry, GR14 holds the
	address of a lazy PLT entry, so @(GR14,-4) is the lazy
	relocation number that we have to pass to _dl_linux_resolver.
	GR15 holds the caller's GOT, from which we extract the
	elf_resolve* that _dl_linux_resolver needs as well.

	_dl_linux_resolver() figures out where the jump symbol is
	_really_ supposed to have jumped to and returns that to us.
	Once we have that, we prepare to tail-call the actual
	function, clean up after ourselves, restoring the original
	arguments, then jump to the fixed up address.  */

	.text
	.p2align 4

	.hidden	_dl_linux_resolve
	.global	_dl_linux_resolve
	.type	_dl_linux_resolve,@function

_dl_linux_resolve:
	/* Preserve arguments.  */
	addi	sp, -8*4, sp
	stdi	gr8, @(sp, 8)
	stdi	gr10, @(sp, 16)
	stdi	gr12, @(sp, 24)
	movsg	lr,gr8
	st	gr8, @(sp,gr0)

	/* Prepare to call _dl_linux_resolver.  */
	ldi	@(gr15, 8), gr8
	ldi	@(gr14, -4), gr9
	mov.p	gr5, gr15
	call	_dl_linux_resolver

	/* Move aside return value that contains the FUNCDESC_VALUE.  */
	ldd	@(gr8,gr0),gr14

	/* Restore arguments.  */
	ld	@(sp, gr0), gr8
	movgs	gr8,lr
	lddi	@(sp, 24), gr12
	lddi	@(sp, 16), gr10
	lddi	@(sp, 8), gr8
	addi	sp, 8*4, sp

	/* Now jump to the actual function.  */
	jmpl	@(gr14, gr0)
	.size	_dl_linux_resolve, . - _dl_linux_resolve
