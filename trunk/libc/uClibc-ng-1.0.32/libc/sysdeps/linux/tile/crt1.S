/* Copyright (C) 2011-2018 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   In addition to the permissions in the GNU Lesser General Public
   License, the Free Software Foundation gives you unlimited
   permission to link the compiled version of this file with other
   programs, and to distribute those programs without any restriction
   coming from the use of this file. (The GNU Lesser General Public
   License restrictions do apply in other respects; for example, they
   cover modification of the file, and distribution when not linked
   into another program.)

   Note that people who make modified versions of this file are not
   obligated to grant this special exception for their modified
   versions; it is their choice whether to do so. The GNU Lesser
   General Public License gives permission to release a modified
   version without this exception; this exception also makes it
   possible to release a modified version which carries forward this
   exception.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

/* This is the canonical entry point, usually the first thing in the text
   segment.  The ELF standard tells us that the stack is set up like this on
   entry (the left side is the offset from "sp"), in units of
   __SIZEOF_POINTER__ entries:

		+0		argc
		+1		argv[0]
		...
		+(argc+1)	NULL
		+(argc+2)	envp[0]
		...
				NULL
		...		ElfInfo

   The ElfInfo is pairs of key/value long words following the envp
   pointers and terminated by a zero-valued key.

   Although not mandated by the standard, it happens to be the case
   that we store the actual argv and envp strings immediately after
   the ElfInfo data on the stack.

   On entry r0 points to the shared library termination function, or 0
   if there isn't one.
*/

#include <features.h>
#include <sysdep.h>
#include <arch/abi.h>

/* Just create no-ops if we don't support PC-relative PLT relocations. */
#ifdef NO_PLT_PCREL
# define hw2_last_plt(x)	0
# define hw1_plt(x)		0
# define hw0_plt(x)		0
#endif

	.text
	.global _start
	.type   _start,@function
	.align 8
_start:
	/* Linux starts us with sp pointing at the conventional Elf layout,
	   but we need to allow two "caller" words for our ABI convention.  */
	{
	 /* Load argc (stored as a "long", equivalent to a pointer type). */
	 LD_PTR r1, sp

	 /* Save incoming 'sp', which points to the Elf argument block. */
	 move r52, sp
	}

	{
	 /* Allocate stack frame callee space for __libc_start_main. */
	 ADDI_PTR r12, sp, -(2 * REGSIZE)
	}

	{
	 /* Get our PC. */
	 lnk r13

	 /* sp is not necessarily properly aligned on startup because
	    of the way ld.so pops off leading argv elements. So align it. */
	 andi sp, r12, -8
	}
.Lmy_pc:

	{
	 /* Pass the address of the shared library termination function. */
	 move r5, r0

	 /* Compute location where __libc_start_main's caller is supposed to
	    store its frame pointer. */
	 ADDI_PTR r12, sp, REGSIZE

	 /* Zero out callee space for return address. Unnecessary but free.
	    This is just paranoia to help backtracing not go awry. */
	 st sp, zero
	}
	{
	 /* Zero out our frame pointer for __libc_start_main. */
	 st r12, zero

	 /* Zero out lr to make __libc_start_main the end of backtrace.  */
	 move lr, zero

	 /* Compute a pointer to argv. envp will be determined
	    later in __libc_start_main.  We set up the first argument
	    (the address of main) below. */
	 ADDI_PTR r2, r52, __SIZEOF_POINTER__
	}
	{
	 /* Pass the highest stack address to user code. */
	 ADDI_PTR r6, sp, (2 * REGSIZE)

	 /* Pass address of main() in r0, and of our own entry
	    points to .fini and .init in r3 and r4.  */
	 moveli r0, hw2_last(main - .Lmy_pc)
	}
	{
	 shl16insli r0, r0, hw1(main - .Lmy_pc)
	 moveli r3, hw2_last(_init - .Lmy_pc)
	}
	{
	 shl16insli r0, r0, hw0(main - .Lmy_pc)
	 shl16insli r3, r3, hw1(_init - .Lmy_pc)
	}
	{
	 ADD_PTR r0, r0, r13
	 shl16insli r3, r3, hw0(_init - .Lmy_pc)
	}
	{
	 moveli r12, hw2_last_plt(__uClibc_main - .Lmy_pc)
	 ADD_PTR r3, r3, r13
	}
	{
	 shl16insli r12, r12, hw1_plt(__uClibc_main - .Lmy_pc)
	 moveli r4, hw2_last(_fini - .Lmy_pc)
	}
	{
	 shl16insli r12, r12, hw0_plt(__uClibc_main - .Lmy_pc)
	 shl16insli r4, r4, hw1(_fini - .Lmy_pc)
	}
	{
	 ADD_PTR r12, r12, r13
	 shl16insli r4, r4, hw0(_fini - .Lmy_pc)
	}
	{
	 ADD_PTR r4, r4, r13
#ifdef NO_PLT_PCREL
	 j plt(__uClibc_main)
#else
	 jr r12
#endif
	}

	{
	 /* Tell backtracer to give up (_start has no caller). */
	 info INFO_OP_CANNOT_BACKTRACE
	}
.size _start, .-_start

/* Define a symbol for the first piece of initialized data.  */
	.data
	.global __data_start
	.align 8
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start
