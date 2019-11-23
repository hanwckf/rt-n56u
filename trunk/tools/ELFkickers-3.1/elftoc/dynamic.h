/* dynamic.h: Reading the dynamic information table.
 * Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#ifndef _dynamic_h_
#define _dynamic_h_

/* Record the entries in the dynamic table for easy random-access
 * lookup.
 */
extern void hashdynamicsection(long offset, long size, int ndx);

/* Uses the dynamic table entries previously recorded to identify
 * subparts of the ELF file image.
 */
extern void dividedynsegments(void);

/* Takes the tag and value pair from an entry in the dynamic table,
 * and returns a string representation of the value appropriate to its
 * content. NULL is returned if the function cannot supply a better
 * representation for the value than the default.
 */
extern char const *strdynamicvalue(long tag, long value);

#endif
