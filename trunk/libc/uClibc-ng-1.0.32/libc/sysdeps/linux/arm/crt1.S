/* Startup code for ARM & ELF
   Copyright (C) 1995, 1996, 1997, 1998, 2001, 2002, 2005
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* This is the canonical entry point, usually the first thing in the text
   segment.

	Note that the code in the .init section has already been run.
	This includes _init and _libc_init


	At this entry point, most registers' values are unspecified, except:

   a1		Contains a function pointer to be registered with `atexit'.
		This is how the dynamic linker arranges to have DT_FINI
		functions called for shared libraries that have been loaded
		before this code runs.

   sp		The stack contains the arguments and environment:
		0(sp)			argc
		4(sp)			argv[0]
		...
		(4*argc)(sp)		NULL
		(4*(argc+1))(sp)	envp[0]
		...
					NULL
*/
/*
   For uClinux it looks like this:

        argc            argument counter (integer)
        argv            char *argv[]
        envp            char *envp[]
        argv[0]         program name (pointer)
        argv[1...N]     program args (pointers)
        argv[argc-1]    end of args (integer)
	NULL
        env[0...N]      environment variables (pointers)
        NULL

ARM register quick reference:

    Name    Number       ARM Procedure Calling Standard Role

    a1      r0           argument 1 / integer result / scratch register / argc
    a2      r1           argument 2 / scratch register / argv
    a3      r2           argument 3 / scratch register / envp
    a4      r3           argument 4 / scratch register
    v1      r4           register variable
    v2      r5           register variable
    v3      r6           register variable
    v4      r7           register variable
    v5      r8           register variable
    sb/v6   r9           static base / register variable
    sl/v7   r10          stack limit / stack chunk handle / reg. variable
    fp      r11          frame pointer
    ip      r12          scratch register / new-sb in inter-link-unit calls
    sp      r13          lower end of current stack frame
    lr      r14          link address / scratch register
    pc      r15          program counter
*/

#include <features.h>
#include <bits/arm_asm.h>

#if defined(__FDPIC__)
.text
	.globl	_start
	.type	_start,%function

	.align	2
_start:
	/* Start by self relocation.  */
#if defined(__thumb2__)
	sub r4, pc, #4
#else
	sub r4, pc, #8
#endif
	ldr r1, .L__ROFIXUP_LIST__
	add r1, r1, r4
	ldr r2, .L__ROFIXUP_END__
	add r2, r2, r4
	mov r0, r7
	push {r9, r10}
	bl __self_reloc
	pop {r9, r10}
	mov r9, r0

	/* Clear the frame pointer since this is the outermost frame.  */
	mov r3, #0
	mov fp, r3

	/* Now fill __uClibc_main arguments.  */
	sub sp, sp, #16			/* Reserve space for 3 args (keep aligment)  */
	ldr r0, .L_main_got_funcdesc	/* r0 GOT FUNCDESC of main */
	ldr r0, [r9, r0]
	ldr r1, [sp, #16]		/* argc  */
	add r2, sp, #20			/* argv  */
	ldr r3, .L_init_got_funcdesc	/* r3 GOT init  */
	ldr r3, [r9, r3]
	ldr r4, .L_fini_got_funcdesc
	ldr r4, [r9, r4]
	str r4, [sp, #0]		/* sp + 0 GOT fini  */
	movs r4, r8			/* Test if static binary (r8 is 0 as there is no interpreter).  */
	ite ne
	movne r4, r10
	moveq r4, #0
	str r4, [sp, #4]		/* sp + 4 got rtld_fini  */
	mov r4, r2			/* stack_end is unused.... */
	str r4, [sp, #8]		/* sp + 8 got stack_end  */
	bl __uClibc_main
	/* We should never get here....  */
	bl abort
.L__ROFIXUP_LIST__:
	.word __ROFIXUP_LIST__ - _start
.L__ROFIXUP_END__:
	.word __ROFIXUP_END__ - _start
.L_main_got_funcdesc:
	.word main(GOTFUNCDESC)
.L_init_got_funcdesc:
	.word _init(GOTFUNCDESC)
.L_fini_got_funcdesc:
	.word _fini(GOTFUNCDESC)

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start

#else /* defined(__FDPIC__) */

.text
	.globl	_start
	.type	_start,%function
	.type	_init,%function
	.type	_fini,%function
#ifndef __UCLIBC_CTOR_DTOR__
	.weak	_init
	.weak	_fini
#endif

#if defined(THUMB1_ONLY)
.thumb_func
_start:
	/* Clear the frame pointer since this is the outermost frame.  */
	mov r3, #0
	mov fp, r3

#ifdef __ARCH_USE_MMU__
	/* Pop argc off the stack and save a pointer to argv */
	pop {a2}
	mov a3, sp
#else
	/*
	 * uClinux/arm stacks look a little different from normal
	 * MMU-full Linux/arm stacks (for no good reason)
	 */
	/* pull argc and argv off the stack.  We are going to push 3
	 * arguments, so pop one here to maintain doubleword alignment.  */
	pop {a2}
	ldr a3, [sp]
#endif

	/* Push stack limit and rtld_fini */
	push {a1, a3}

#ifdef __PIC__
	ldr r4, .L_GOT
	adr r5, .L_GOT
	add r4, r5, r4

	ldr r5, .L_GOT+4	/* _fini */
	ldr a1, [r4, r5]
	push {a1}		/* Push _fini */

	ldr r5, .L_GOT+8	/* _init */
	ldr a4, [r4, r5]
	
	ldr r5, .L_GOT+12	/* main */
	ldr a1, [r4, r5]

#else
	/* Fetch address of fini */
	ldr r4, =_fini
	/* Push fini */
	push {r4}

	/* Set up the other arguments in registers */
	ldr a1, =main
	ldr a4, =_init
#endif
	/* __uClibc_main (main, argc, argv, init, fini, rtld_fini, stack_end) */
	/* Let the libc call main and exit with its return code.  */
	bl __uClibc_main

	/* should never get here....*/
	bl abort
.pool

#ifdef __PIC__
.L_GOT:
	.word	_GLOBAL_OFFSET_TABLE_-.L_GOT
	.word _fini(GOT)
	.word _init(GOT)
	.word main(GOT)
#endif
#else /* !THUMB1_ONLY */
_start:
	/* Clear the frame pointer and link register since this is the outermost frame.  */
	mov fp, #0
	mov lr, #0

#ifdef __ARCH_USE_MMU__
	/* Pop argc off the stack and save a pointer to argv */
	ldr a2, [sp], #4
	mov a3, sp
#else
	/*
	 * uClinux/arm stacks look a little different from normal
	 * MMU-full Linux/arm stacks (for no good reason)
	 */
	/* pull argc and argv off the stack.  We are going to push 3
	 * arguments, so pop one here to maintain doubleword alignment.  */
	ldr a2, [sp], #4
	ldr a3, [sp]
#endif

	/* Push stack limit */
	str a3, [sp, #-4]!

	/* Push rtld_fini */
	str a1, [sp, #-4]!

#ifdef __PIC__
	ldr sl, .L_GOT
	adr a4, .L_GOT
	add sl, sl, a4

	ldr ip, .L_GOT+4	/* _fini */
	ldr a1, [sl, ip]
	str a1, [sp, #-4]!	/* Push _fini */

	ldr ip, .L_GOT+8	/* _init */
	ldr a4, [sl, ip]
	
	ldr ip, .L_GOT+12	/* main */
	ldr a1, [sl, ip]

	/* __uClibc_main (main, argc, argv, init, fini, rtld_fini, stack_end) */
	/* Let the libc call main and exit with its return code.  */
	b __uClibc_main(PLT)
#else
	/* Fetch address of fini */
	ldr ip, =_fini
	/* Push fini */
	str ip, [sp, #-4]!

	/* Set up the other arguments in registers */
	ldr a1, =main
	ldr a4, =_init

	/* __uClibc_main (main, argc, argv, init, fini, rtld_fini, stack_end) */

	/* Let the libc call main and exit with its return code.  */
	b __uClibc_main
#endif

	/* should never get here....*/
	bl abort

#ifdef __PIC__
.L_GOT:
	.word _GLOBAL_OFFSET_TABLE_ - .L_GOT
	.word _fini(GOT)
	.word _init(GOT)
	.word main(GOT)
#endif
#endif

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start
#endif /* defined(__FDPIC__) */
