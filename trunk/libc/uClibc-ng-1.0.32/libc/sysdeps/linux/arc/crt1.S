/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <features.h>

.text

#ifndef __UCLIBC_CTOR_DTOR__
	.weak	_init
	.weak	_fini
#endif

/* Stick in a dummy reference to main(), so that if an application
 * is linking when the main() function is in a static library (.a)
 * we can be sure that main() actually gets linked in
 */
	.type	main,@function
	.type	_main,@function


/* When we enter this piece of code, the program stack looks like this:
        argc            argument counter (integer)
        argv[0]         program name (pointer)
        argv[1...N]     program args (pointers)
        argv[argc-1]    end of args (integer)
	NULL
        env[0...N]      environment variables (pointers)
        NULL
*/
	.text
	.align 4
	.global __start
	.hidden __start
	.type __start,@function
__start:
	mov	fp, 0
	ld_s	r1, [sp]	; argc

	mov_s	r5, r0		; rltd_fini
	add_s	r2, sp, 4	; argv
#ifdef L_Scrt1
	ld	r0, [pcl, @main@gotpc]
	ld	r3, [pcl, @_init@gotpc]
	ld	r4, [pcl, @_fini@gotpc]
#else
	mov_s	r0, main
	mov_s	r3, _init
	mov	r4, _fini
#endif
	and	sp, sp, -8
	mov	r6, sp

	/* __uClibc_main (main, argc, argv, init, fini, rtld_fini, stack_end) */
	bl	__uClibc_main

	/* Should never get here....  */
	flag    1
.size __start,.-__start

/* Implement a .note.ABI-tag section that is mandatory for Linux executables
   according to LSB. See:
   https://refspecs.linuxfoundation.org/LSB_1.2.0/gLSB/noteabitag.html.
   Also: libc/sysdeps/linux/avr32/crt1.S. */
.section ".note.ABI-tag", "a"
	.align 4
	.long 1f - 0f		/* Name length */
	.long 3f - 2f		/* Data length */
	.long 1			/* Note type */
0:	.asciz "GNU"		/* Vendor name */
1:	.align 4
2:	.long 0			/* Note data: Linux executable */
#if (__GNUC__ > 4)
	.long 4,8,0		/* Earliest compatible kernel for ABI v4 */
#else
	.long 3,9,0		/* Earliest compatible kernel for ABI v3 */
#endif
3:	.align 4		/* Pad out section */
