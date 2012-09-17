/*
 * Port of uClibc for TMS320C6000 DSP architecture
 * Copyright (C) 2004, 2011 Texas Instruments Incorporated
 * Author of TMS320C6000 port: Aurelien Jacquiot
 *
 *   Use clone syscall: Mark Salter <msalter@redhat.com>
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
 */
#define __ASSEMBLY__

#include <asm/errno.h>
#include <sys/syscall.h>
#include <linux/sched.h>
#include <asm/signal.h>

#define CLONE_FLAGS (CLONE_VFORK | CLONE_VM | SIGCHLD)
	
	.global __vfork
__vfork:
	MVK	.S2	SYS_clone,B0
 ||	MVKL	.S1	CLONE_FLAGS,A4
	MVKH	.S1	CLONE_FLAGS,A4
 ||	MVK	.L2	0,B4
#ifndef	_TMS320C6400_PLUS
	MVC	.S2	CSR,B2
	CLR	.S2	B2,0,0,B1
	MVC	.S2	B1,CSR
	MVC	.S2	IFR,B1
	SET	.S2	B1,6,6,B1
	MVC	.S2	B1,ISR
	MVC	.S2	B2,CSR
	NOP
#else
	SWE
#endif

	MVK	.S2	-4096,B4
	CMPGTU	.L2X	B4,A4,B2	; check error
  [B2]	BNOP	.S2	B3,5

	NEG	.S1	A4,A4
	STW	.D2T1	A4,*B15--[2]
	STW	.D2T2	B3,*+B15[1]
	CALLP	.S2	__errno_location,B3
	LDW	.D2T2	*+B15[1],B3
	LDW	.D2T1	*++B15[2],A5
	NOP	3
	BNOP	.S2	B3,3
	STW	.D1T1	A5,*A4
	MVK	.L1	-1,A4

weak_alias(__vfork,vfork)
libc_hidden_weak(vfork)
