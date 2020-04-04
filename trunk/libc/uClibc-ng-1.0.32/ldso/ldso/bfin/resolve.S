     /* Copyright (C) 2003 Red Hat, Inc.
	Contributed by Alexandre Oliva <aoliva@redhat.com>

This file is part of uClibc.

uClibc is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation; either version 2.1 of the
License, or (at your option) any later version.

uClibc is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with uClibc; see the file COPYING.LIB.  If not, see
<http://www.gnu.org/licenses/>.  */

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

	.hidden	__dl_linux_resolve
	.global	__dl_linux_resolve
	.type	__dl_linux_resolve,@function

__dl_linux_resolve:
	/* Preserve arguments.  */
	[--SP] = RETS;
	[--SP] = P0;
	[--SP] = R0;
	[--SP] = R1;
	[--SP] = R2;
	sp += -12;

	/* Prepare to call _dl_linux_resolver.  */
	R0 = [P3 + 8];
	/* Not aligned for space reasons.  */
	R1 = W[P1 + -4] (Z);
	P1 += -2;
	R1.H = W[P1];

	P3 = R3;
	CALL	__dl_linux_resolver;

	/* Move aside return value that contains the FUNCDESC_VALUE.  */
	P3 = R0;
	P1 = [P3];
	P3 = [P3 + 4];

	/* Restore arguments.  */
	sp += 12;
	R2 = [SP++];
	R1 = [SP++];
	R0 = [SP++];
	P0 = [SP++];
	RETS = [SP++];

	/* Now jump to the actual function.  */
	JUMP	(P1);
	.size	__dl_linux_resolve, . - __dl_linux_resolve
