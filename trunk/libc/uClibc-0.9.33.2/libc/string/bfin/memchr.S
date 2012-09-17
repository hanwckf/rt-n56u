/* memchr.S
 * Copyright (C) 2003-2007 Analog Devices Inc., All Rights Reserved.
 *
 * This file is subject to the terms and conditions of the GNU Library General
 * Public License. See the file "COPYING.LIB" in the main directory of this
 * archive for more details.
 *
 * Non-LGPL License also available as part of VisualDSP++
 * http://www.analog.com/processors/resources/crosscore/visualDspDevSoftware.html
 */

#include <sysdep.h>

/* void *memchr(const void *s, int c, size_t n);
 * R0 = address (s)
 * R1 = sought byte (c)
 * R2 = count (n)
 *
 * Returns pointer to located character.
 */

.text

.align 2

.weak _memchr
ENTRY(_memchr)
	P0 = R0;             /* P0 = address */
	P2 = R2;             /* P2 = count */
	R1 = R1.B(Z);
	CC = R2 == 0;
	IF CC JUMP .Lfailed;

.Lbytes:
	LSETUP (.Lbyte_loop_s, .Lbyte_loop_e) LC0=P2;

.Lbyte_loop_s:
	R3 = B[P0++](Z);
	CC = R3 == R1;
	IF CC JUMP .Lfound;
.Lbyte_loop_e:
	NOP;

.Lfailed:
	R0=0;
	RTS;

.Lfound:
	R0 = P0;
	R0 += -1;
	RTS;

.size _memchr,.-_memchr

libc_hidden_def (memchr)
