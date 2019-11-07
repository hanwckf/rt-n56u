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
 * along with this program; see the file COPYING.LIB.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef _BITS_SETJMP_H
#define _BITS_SETJMP_H 1

#if !defined _SETJMP_H && !defined _PTHREAD_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

typedef struct {
	unsigned long __regs[12]; /* save A10,B10... A15,B15*/
	unsigned long __pc;       /* the return address */
} __jmp_buf[1];

#endif
