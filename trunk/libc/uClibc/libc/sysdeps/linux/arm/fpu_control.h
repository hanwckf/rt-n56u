/* FPU control word definitions.  ARM version.
   Copyright (C) 1996, 1997, 1998, 2000 Free Software Foundation, Inc.
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

#ifndef _FPU_CONTROL_H
#define _FPU_CONTROL_H

/* We have a slight terminology confusion here.  On the ARM, the register
 * we're interested in is actually the FPU status word - the FPU control
 * word is something different (which is implementation-defined and only
 * accessible from supervisor mode.)
 *
 * The FPSR looks like this:
 *
 *     31-24        23-16          15-8              7-0
 * | system ID | trap enable | system control | exception flags |
 *
 * We ignore the system ID bits; for interest's sake they are:
 *
 *  0000	"old" FPE
 *  1000	FPPC hardware
 *  0001	FPE 400
 *  1001	FPA hardware
 *
 * The trap enable and exception flags are both structured like this:
 *
 *     7 - 5     4     3     2     1     0
 * | reserved | INX | UFL | OFL | DVZ | IVO |
 *
 * where a `1' bit in the enable byte means that the trap can occur, and
 * a `1' bit in the flags byte means the exception has occurred.
 *
 * The exceptions are:
 *
 *  IVO - invalid operation
 *  DVZ - divide by zero
 *  OFL - overflow
 *  UFL - underflow
 *  INX - inexact (do not use; implementations differ)
 *
 * The system control byte looks like this:
 *
 *     7-5      4    3    2    1    0
 * | reserved | AC | EP | SO | NE | ND |
 *
 * where the bits mean
 *
 *  ND - no denormalised numbers (force them all to zero)
 *  NE - enable NaN exceptions
 *  SO - synchronous operation
 *  EP - use expanded packed-decimal format
 *  AC - use alternate definition for C flag on compare operations
 */

/* masking of interrupts */
#define _FPU_MASK_IM	0x00010000	/* invalid operation */
#define _FPU_MASK_ZM	0x00020000	/* divide by zero */
#define _FPU_MASK_OM	0x00040000	/* overflow */
#define _FPU_MASK_UM	0x00080000	/* underflow */
#define _FPU_MASK_PM	0x00100000	/* inexact */
#define _FPU_MASK_DM	0x00000000	/* denormalized operation */

/* The system id bytes cannot be changed.
   Only the bottom 5 bits in the trap enable byte can be changed.
   Only the bottom 5 bits in the system control byte can be changed.
   Only the bottom 5 bits in the exception flags are used.
   The exception flags are set by the fpu, but can be zeroed by the user. */
#define _FPU_RESERVED	0xffe0e0e0	/* These bits are reserved.  */

/* The fdlibm code requires strict IEEE double precision arithmetic,
   no interrupts for exceptions, rounding to nearest.  Changing the
   rounding mode will break long double I/O.  Turn on the AC bit,
   the compiler generates code that assumes it is on.  */
#define _FPU_DEFAULT	0x00001000	/* Default value.  */
#define _FPU_IEEE	0x001f1000	/* Default + exceptions enabled. */

/* Type of the control word.  */
typedef unsigned int fpu_control_t;

/* Macros for accessing the hardware control word.  */
#define _FPU_GETCW(cw) __asm__ ("rfs %0" : "=r" (cw))
#define _FPU_SETCW(cw) __asm__ ("wfs %0" : : "r" (cw))

/* Default control word set at startup.  */
extern fpu_control_t __fpu_control;

#endif /* _FPU_CONTROL_H */
