/* names.h: Looking up macro names and their values.
 * Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#ifndef	_names_h_
#define	_names_h_

/* Set the machine architecture for the ELF input file. id is a value
 * from the e_machine field of the ELF header.
 */
extern void setmachinespecific(int id);

/* Find a macro whose name begins with the given prefix and is equal
 * to the supplied value. The return value is the macro's name, or
 * NULL if no such macro could be found.
 */
extern char const *findname(unsigned long value, char const *prefix);

/* Find a macro-defined bitflag that is present in the given flag
 * value. If successful, the macro name is returned and that flag's
 * bit value is removed from the flag value. If no suitable flag value
 * is found, NULL is returned.
 * !!!
 */
extern char const *findflag(unsigned long *flags, char const *prefix);

/* Search for a macro name. The return value is false if the macro
 * does not exist. Otherwise true is returned, and if pval is not NULL
 * it will be used to store the macro's value.
 */
extern int lookupname(char const *name, unsigned long *pval);

#endif
