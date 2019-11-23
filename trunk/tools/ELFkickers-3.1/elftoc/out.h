/* out.h: The top-level output functions.
 * Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#ifndef	_out_h_
#define	_out_h_

/* Outputs one piece of the ELF file, located at the given offset and
 * size, and represented as the indicated type. ndx is the piece's
 * index value, which can be used to access data associated with the
 * piece.
 */
extern void outtypedblock(int type, long offset, long size, int ndx);

/* Runs the entire process of outputting the ELF file image as C
 * source code. Call this function at the end of the program.
 */
extern void output(void);

#endif
