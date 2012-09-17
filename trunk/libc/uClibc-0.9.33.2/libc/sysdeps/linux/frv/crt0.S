/* Copyright (C) 1991, 1992, 2003, 2004 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation; either version 2.1 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

In addition to the permissions in the GNU Lesser General Public
License, the Free Software Foundation gives you unlimited
permission to link the compiled version of this file with other
programs, and to distribute those programs without any restriction
coming from the use of this file.  (The GNU Lesser General Public
License restrictions do apply in other respects; for example, they
cover modification of the file, and distribution when not linked
into another program.)

You should have received a copy of the GNU Lesser General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */


/*  Based on ../i386/crt0.S and newlib's libgloss/frv/crt0.S  */

/*
    When we enter this piece of code, the program stack looks like this:
        argc            argument counter (integer)
        argv[0]         program name (pointer)
        argv[1...N]     program args (pointers)
        argv[argc-1]    end of args (integer)
	NULL
        env[0...N]      environment variables (pointers)
        NULL

    Also, GR16 holds a pointer to a memory map.  */

#include <features.h>

	.text
	.global _start
	.type	_start,%function
#if defined L_crt0 || defined L_Scrt0 || ! defined __UCLIBC_CTOR_DTOR__
	.type __uClibc_main,%function
#else
	.weak	_init
	.weak	_fini
	.type	__uClibc_start_main,%function
#endif
_start:
	/* Make sure the stack pointer is properly aligned.  Save the
	   original value in gr21 such that we can get to arguments and
	   such from there.  */
	mov.p	sp, gr21
	andi	sp, #-8, sp
	/* At program start-up, gr16 contains a pointer to a memory
	   map, that we use to relocate addresses.  */
	call	.Lcall
.Lcall:
	movsg	lr, gr4
	sethi.p	#gprelhi(.Lcall), gr5
	setlo	#gprello(.Lcall), gr5
	sub.p	gr4, gr5, gr4
	/* gr4 now holds the _gp address.  */

	mov	gr16, gr8
	sethi.p #gprelhi(__ROFIXUP_LIST__), gr9
	sethi	#gprelhi(__ROFIXUP_END__), gr10
	setlo.p #gprello(__ROFIXUP_LIST__), gr9
	setlo	#gprello(__ROFIXUP_END__), gr10
	add.p	gr9, gr4, gr9
	add	gr10, gr4, gr10
	call	__self_reloc
	mov.p	gr8, gr17
	mov	gr8, gr15
	/* gr17 now holds the self-relocated _GLOBAL_OFFSET_TABLE_
	address, because the linker added its unrelocated address as
	the last entry in the ROFIXUP list, and __self_reloc returns
	the last entry, relocated.  */

	/* Prepare arguments for uClibc main.  */
	ld	@(gr21, gr0), gr8
	slli	gr8, #2, gr10
	add	gr21, gr10, gr10
	addi.p	gr21, #4, gr9
	addi	gr10, #8, gr10

	/* Set up an invalid (NULL return address, NULL frame pointer)
	   callers stack frame so anybody unrolling the stack knows where
	   to stop */
	mov	gr0, fp
	movgs	gr0, lr

#if (defined L_crt1 || defined L_Scrt1) && defined __UCLIBC_CTOR_DTOR__
	/* Pass .init and .fini arguments to __uClibc_start_main().  */
	sethi.p	#gotfuncdeschi(_init), gr11
	sethi	#gotfuncdeschi(_fini), gr12
	setlo.p	#gotfuncdesclo(_init), gr11
	setlo	#gotfuncdesclo(_fini), gr12
	ld.p	@(gr11, gr17), gr11
	mov	gr17, gr15
	ld.p	@(gr12, gr17), gr12
	call	__uClibc_start_main
#else
	mov.p	gr17, gr15
	call	__uClibc_main
#endif

	/* Crash if somehow `exit' returns anyways.  */
	jmpl	@(gr0,gr0)
.size _start,.-_start

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start

