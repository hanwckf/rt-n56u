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

#include <setjmp.h>
#include <jmpbuf-offsets.h>

/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(jmpbuf, address) \
  ((void *) (address) < (void *) (jmpbuf)->__regs[JP_SP])


