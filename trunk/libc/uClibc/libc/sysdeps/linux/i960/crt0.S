#
# crt0.S, part of the i960 support for the uClibc library.
#
# Copyright (C) 2002 by Okiok Data Ltd.  http://www.okiok.com/
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU Library General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option) any
# later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Library General Public License for more
# details.
#
# You should have received a copy of the GNU Library General Public License
# along with this program; if not, write to the Free Software Foundation, Inc.,
# at 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

/*
 *
 * The behavior in this file is tightly coupled with how the linux kernel sets things up
 * on the stack before calling us.
 *
 * Currently fs/binfmt_flat.c (for STACK_GROWS_UP) and arch/i960/kernel/process.c
 * build things so that a pointer to argc is left in g13 by start_thread().
 *
 * ^
 * |                        <- sp somewhere around here, after being aligned.
 * |
 * |envp    -> envp[0]
 * |argv    -> argv[0]
 * |argc                    <- g13
 *
 * A complete picture of how things are set up can be seen in the comments of
 * create_flat_tables_stack_grows_up in fs/binfmt_flat.c
 *
 * I believe having to use this register could probably be avoided.
 *
 */

        .globl  start
start:
        mov     g13, r3
        ldt     (r3), g0
        callx   ___uClibc_main

/* We might want to add some instruction so that it crashes if main returns */

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start

