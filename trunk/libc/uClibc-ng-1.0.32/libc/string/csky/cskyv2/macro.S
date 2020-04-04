/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

.macro M_BEZ rx, label
	bez   \rx, \label
.endm

.macro M_BNEZ rx, label
	bnez  \rx, \label
.endm

.macro M_BNE rx, ry, label
	cmpne \rx, \ry
	bt    \label
.endm

