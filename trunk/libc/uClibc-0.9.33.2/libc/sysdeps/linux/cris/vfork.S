/*-
 * Copyright (c) 2011
 *	Thorsten Glaser <tg@freewrt.org>
 *
 * This file is available either under the terms and conditions of
 * the MirOS Licence, or the same terms as klibc or uClibc.
 */

#include "sysdep.h"

	.syntax no_register_prefix

/*
 * vfork is special, but PSEUDO() would probably work were it not broken;
 * there must be nothing at all on the stack above the stack frame of the
 * enclosing function
 */

ENTRY(__vfork)
	movu.w	__NR_vfork,$r9
	break	13
	cmps.w	-4096,$r10
	bhs	0f
	 nop
	Ret
	 nop
PSEUDO_END(__vfork)

weak_alias(__vfork,vfork)
libc_hidden_weak(vfork)
