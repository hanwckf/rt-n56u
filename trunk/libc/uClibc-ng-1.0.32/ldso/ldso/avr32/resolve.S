/*
 * Linux dynamic resolving code for AVR32. Fixes up the GOT entry as
 * indicated in register r12 and jumps to the resolved address.
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.  See the file "COPYING.LIB" in the main directory of this
 * archive for more details.
 *
 * Copyright (C) 2004-2007 Atmel Corporation
 */

#define ip r5

	.text
	.global	_dl_linux_resolve
	.type	_dl_linux_resolve,@function
_dl_linux_resolve:
	/* The PLT code pushed r8 for us. It contains the address of this
	   function's GOT entry, that is entry 0. ip contains the address
	   of the GOT entry of the function we wanted to call. */
	stm	--sp, r9-r12, lr
	mov	r11, r8
	sub	r12, ip, r8
	rcall	_dl_linux_resolver
	mov	ip, r12
	popm	r8-r12,lr
	mov	pc, ip
	.size	_dl_linux_resolve, . - _dl_linux_resolve
