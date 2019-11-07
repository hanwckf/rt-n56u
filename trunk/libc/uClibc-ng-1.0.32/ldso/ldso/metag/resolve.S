/*
 * Meta dynamic resolver
 *
 * Copyright (C) 2013 Imagination Technologies Ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 *
 * This function is _not_ called directly.  It is jumped to (so no return
 * address is on the stack) when attempting to use a symbol that has not yet
 * been resolved.  The first time a jump symbol (such as a function call inside
 * a shared library) is used (before it gets resolved) it will jump here to
 * _dl_linux_resolve.  When we get called the stack looks like this:
 *	reloc_entry
 *	tpnt
 *
 * This function saves all the registers then makes the function call
 * _dl_linux_resolver(tpnt, reloc_entry).  _dl_linux_resolver() figures out
 * where the jump symbol is _really_ supposed to have jumped to and returns
 * that to us.  Once we have that, we overwrite tpnt with this fixed up
 * address. We then clean up after ourselves, put all the registers back how we
 * found them, then we jump to the fixed up address, which is where the jump
 * symbol that got us here really wanted to jump to in the first place.
 *  -Erik Andersen
 */

	.text
	.global __dl_linux_resolve
	.type   __dl_linux_resolve,@function

__dl_linux_resolve:
	!! Save registers on the stack. Do we need to save any more here?
	MSETL [A0StP++],D0Ar6,D0Ar4,D0Ar2,D0FrT
	SETL  [A0StP++],A0FrP,A1LbP
	!! Get the args for _dl_linux_resolver off the stack
	GETL  D0Re0,D1Re0,[A0StP+#-(6*8)]
	GETD  D1Ar1,[D0Re0]
	MOV   D0Ar2,D1Re0
	!! Multiply plt_index by sizeof(Elf32_Rela)
	MULW  D0Ar2,D0Ar2,#12
	!! Call the resolver
	CALLR D1RtP,__dl_linux_resolver
	!! Restore the registers from the stack
	SUB   A0.2,A0StP,#(1*8)
	GETL  A0FrP,A1LbP,[A0.2]
	SUB   A0.2,A0.2,#(4*8)
	MGETL D0Ar6,D0Ar4,D0Ar2,D0FrT,[A0.2]
	!! Also take into account args pushed by PLT
	SUB   A0StP,A0StP,#(6*8)
	!! Jump to the resolved address
	MOV   PC,D0Re0
	.size __dl_linux_resolve, .-__dl_linux_resolve
