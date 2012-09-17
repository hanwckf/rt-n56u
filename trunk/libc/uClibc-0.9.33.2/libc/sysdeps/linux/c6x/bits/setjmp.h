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

#ifndef _SETJMP_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

typedef struct {
	unsigned long __regs[12]; /* save A10,B10... A15,B15*/
	unsigned long __pc;       /* the return address */
} __jmp_buf[1];

/* the stack pointer (B15) */
#define JP_SP 11 

/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(jmpbuf, address) \
  ((void *) (address) < (void *) (jmpbuf)->__regs[JP_SP])


