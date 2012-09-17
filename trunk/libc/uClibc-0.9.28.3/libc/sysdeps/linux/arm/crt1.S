/* Startup code for ARM & ELF
   Copyright (C) 1995, 1996, 1997, 1998, 2001, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

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

	.text
	.globl _start
	.type _start,#function
_start:
	/* Clear the frame pointer and link register since this is the outermost frame.  */
	mov fp, #0
	mov lr, #0

	/* Pop argc off the stack and save a pointer to argv */
	ldr a2, [sp], #4
	mov a3, sp

	/* Push stack limit */
	str a3, [sp, #-4]!

	/* Push rtld_fini */
	str a1, [sp, #-4]!

#ifdef __PIC__
	ldr sl, .L_GOT
.L_GOT_OFF:
	add sl, pc, sl

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

#ifdef __PIC__
.L_GOT:
	.word	_GLOBAL_OFFSET_TABLE_-(.L_GOT_OFF+8)
	.word _fini(GOT)
	.word _init(GOT)
	.word main(GOT)
#endif

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start
