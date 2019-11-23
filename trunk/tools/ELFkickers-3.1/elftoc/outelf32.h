/* outelf32.h: Output functions for 32-bit ELF structures.
 * Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#ifndef	_outelf32_h_
#define	_outelf32_h_

/*
 * This module defines one output function for each type of supported
 * Elf32 struct. Every output function accepts a pointer to memory
 * containing the array of structs, a size (measured in bytes), and
 * the piece's index, and outputs the contents of that memory.
 * outehdr32 is the only output function that accepts a fixed-size
 * piece; all other functions need to use the size parameter to
 * determine the number of entries to output.
 */

extern void outehdr32(void const *ehdr, long size, int ndx);
extern void outphdr32(void const *phdrs, long size, int ndx);
extern void outshdr32(void const *shdrs, long size, int ndx);
extern void outsym32(void const *syms, long size, int ndx);
extern void outsyminfo32(void const *syminfos, long size, int ndx);
extern void outrel32(void const *rels, long size, int ndx);
extern void outrela32(void const *relas, long size, int ndx);
extern void outdyn32(void const *dyns, long size, int ndx);
extern void outmove32(void const *moves, long size, int ndx);
extern void outnote32(void const *ptr, long size, int ndx);
extern void outaddr32(void const *addrs, long size, int ndx);

#endif
