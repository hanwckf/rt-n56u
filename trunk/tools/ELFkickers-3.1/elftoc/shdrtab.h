/* shdrtab.h: Reading the section header table.
 * Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#ifndef _shdrtab_h_
#define _shdrtab_h_

/* Uses the section header table to break the ELF file up into pieces,
 * with the section header string table supplying the piece names.
 */
extern void dividesections(long offset, int count, int entsize, int shstrndx);

/* Returns the identifier name for a given section index. If no such
 * name exists, a decimal literal is returned instead.
 */
extern char const *getsectionid(int ndx);

/* Outputs the list of section names as an enum definition.
 */
extern void outputenum(void);

#endif
