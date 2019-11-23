/* phdrtab.h: Reading the program segment header table.
 * Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#ifndef _phdrtab_h_
#define _phdrtab_h_

/* Uses the program segment header table to identify pieces of the ELF
 * file image. If this function finds the dynamic table, it also has
 * it analyzed before returning.
 */
extern void dividesegments(long offset, int count, int entsize);

#endif
