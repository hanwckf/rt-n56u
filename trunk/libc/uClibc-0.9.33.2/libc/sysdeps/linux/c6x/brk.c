/*
 * Port of uClibc for TMS320C6000 DSP architecture
 * Copyright (C) 2004 Texas Instruments Incorporated
 * Author of TMS320C6000 port: Aurelien Jacquiot
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>

libc_hidden_proto(brk)

/* This must be initialized data because commons can't have aliases.  */
void * __curbrk attribute_hidden = 0;

int brk (void *addr)
{
	void *newbrk;

	__asm__ __volatile__ ( \
		"mv .d1	%2, A4\n\t" \
		"mvk .s2	%1, B0\n\t" \
		"swe\n\t" \
		"nop\n\t" \
		"mv .d2	B0, %0" \
		: "=b" (newbrk) \
		: "i" (__NR_brk), \
		  "a" (addr) \
		: "memory", "cc", "B0", "A4"); \

	__curbrk = newbrk;

	if (newbrk < addr) {
		__set_errno (ENOMEM);
		return -1;
	}
	return 0;
}
libc_hidden_def(brk)
