/*-
 * Copyright (c) 2017
 *	mirabilos <m@mirbsd.org>
 *
 * Provided that these terms and disclaimer and all copyright notices
 * are retained or reproduced in an accompanying document, permission
 * is granted to deal in this work without restriction, including un-
 * limited rights to use, publicly perform, distribute, sell, modify,
 * merge, give away, or sublicence.
 *
 * This work is provided "AS IS" and WITHOUT WARRANTY of any kind, to
 * the utmost extent permitted by applicable law, neither express nor
 * implied; without malicious intent or gross negligence. In no event
 * may a licensor, author or contributor be held liable for indirect,
 * direct, other damage, loss, or other issues arising in any way out
 * of dealing in the work, even if advised of the possibility of such
 * damage or existence of a defect, except proven that it results out
 * of said person's immediate fault when using the work as intended.
 */

	.intel_syntax noprefix
	.text

/*-
 * long __libc_i386_syscall6(unsigned long nr, ...) with six varargs
 *
 * C definition:
 * extern long __libc_i386_syscall6(unsigned long, ...)
 *    __attribute__((__cdecl__));
 *
 * Stack layout upon entry:		Offsets after saving:
 * [esp + 28]	arg6	-> ebp !	[esp + 44]
 * [esp + 24]	arg5	-> edi !	[esp + 40]
 * [esp + 20]	arg4	-> esi !	[esp + 36]
 * [esp + 16]	arg3	-> edx		[esp + 32]
 * [esp + 12]	arg2	-> ecx		[esp + 28]
 * [esp + 8]	arg1	-> ebx !	[esp + 24]
 * [esp + 4]	nr	-> eax		[esp + 20]
 * [esp]	return address		[esp + 16]
 *					[esp + 12]	saved ebp
 *					[esp + 8]	saved ebx
 *					[esp + 4]	saved esi
 *					[esp]		saved edi
 */

	.p2align 4,0x90
	.globl	__libc_i386_syscall6
	.type	__libc_i386_syscall6,@function
__libc_i386_syscall6:
	push	ebp
	push	ebx
	push	esi
	push	edi
	mov	eax,[esp + 20]	/* nr */
	mov	ebx,[esp + 24]	/* arg1 */
	mov	ecx,[esp + 28]	/* arg2 */
	mov	edx,[esp + 32]	/* arg3 */
	mov	esi,[esp + 36]	/* arg4 */
	mov	edi,[esp + 40]	/* arg5 */
	mov	ebp,[esp + 44]	/* arg6 */
	int	0x80
	pop	edi
	pop	esi
	pop	ebx
	pop	ebp
	ret
	.size	__libc_i386_syscall6,.-__libc_i386_syscall6
