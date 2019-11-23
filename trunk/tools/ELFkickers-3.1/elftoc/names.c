/* names.c: Functions for retrieving macro names.
 * Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include <stdio.h>
#include <string.h>
#include <elf.h>
#include "gen.h"
#include "names.h"

/* Description of a defined macro.
 */
struct name {
    char const     *str;	/* the macro name as a string */
    unsigned long   value;	/* the numerical value */
    unsigned long   rangesize;	/* size of the macro's range, or zero */
};

/* A collection of macros defined with a particular prefix and
 * machine name.
 */
struct nameset {
    struct name	   *names;	/* the array of macro names */
    int		    count;	/* size of the array */
};

/* The namesets array is defined in a compile-time-generated file, along
 * with the list of prefixes and machine IDs.
 */
#include "elfnames.c"

/* The index of the machine ID of the current input file. No other
 * machine-specific macros will be considered for output except for
 * the designated machine.
 */
static int machine = 0;

/* Set the machine ID for the current ELF file.
 */
void setmachinespecific(int id)
{
    int m;

    for (m = 0 ; m < machineidcount ; ++m) {
	if (machineids[m] == id) {
	    machine = m;
	    return;
	}
    }
    machine = 0;
}

/* Find a macro with a specific value in the given nameset. NULL is
 * returned if no macro in the set matches the value. If the value
 * falls inside a range, the return value will represent an
 * expression.
 */
static char const *findnameinset(unsigned long value,
				 struct nameset const *set)
{
    static char buf[256];
    int i;

    for (i = 0 ; i < set->count ; ++i)
	if (set->names[i].value == value)
	    return set->names[i].str;
    for (i = 0 ; i < set->count ; ++i)
	if (set->names[i].rangesize > 0 && set->names[i].value < value &&
			value < set->names[i].value + set->names[i].rangesize)
	    break;
    if (i < set->count) {
	sprintf(buf, "%s + %lu", set->names[i].str,
				 value - set->names[i].value);
	return buf;
    }
    return NULL;
}

/* Find a macro whose 1-bits are a subset of the 1-bits in the value
 * pointed to by flags. If one is found, the macro value's 1-bits are
 * turned off in the passed-in value and the macro's name is returned.
 */
static char const *findflaginset(unsigned long *flags,
				 struct nameset const *set)
{
    int i;

    for (i = 0 ; i < set->count ; ++i) {
	if (set->names[i].value == 0)
	    continue;
	if ((*flags & set->names[i].value) == set->names[i].value) {
	    *flags ^= set->names[i].value;
	    return set->names[i].str;
	}
    }
    return NULL;
}

/* Find a macro-based expression that corresponds to the given value.
 */
char const *findname(unsigned long value, char const *prefixstr)
{
    char const *name;
    int prefix;

    for (prefix = 0 ; prefix < prefixcount ; ++prefix) {
	if (strcmp(prefixstr, prefixes[prefix]))
	    continue;
	name = findnameinset(value, &namesets[prefix][machine]);
	if (name)
	    return name;
	name = findnameinset(value, &namesets[prefix][0]);
	if (name)
	    return name;
    }
    return NULL;
}

/* Find a macro-based expression that has been ORed into the given
 * value.
 */
char const *findflag(unsigned long *flags, char const *prefixstr)
{
    char const *name;
    int prefix;

    for (prefix = 0 ; prefix < prefixcount ; ++prefix) {
	if (strcmp(prefixstr, prefixes[prefix]))
	    continue;
	name = findflaginset(flags, &namesets[prefix][machine]);
	if (name)
	    return name;
	name = findflaginset(flags, &namesets[prefix][0]);
	if (name)
	    return name;
    }
    return NULL;
}

#if 0
/* Find a set of macros that equal the given value when ORed together.
 * Any bits that fall outside of the available flags will be
 * represented as a hex literal.
 * !!!
 */
char const *findflagset(unsigned long flags, char const *prefixstr)
{
    static char *buf = NULL;
    static int bufsize = 0;
    char const *name;
    int prefix;
    int size, n;

    if (flags == 0)
	return "0";
    for (prefix = 0 ; prefix < prefixcount ; ++prefix) {
	if (strcmp(prefixstr, prefixes[prefix]))
	    continue;
	while (flags) {
	    name = findflaginset(&flags, &namesets[prefix][machine]);
	    if (!name)
		name = findflaginset(&flags, &namesets[prefix][0]);
	    if (!name)
		break;
	    n = 3 + strlen(name);
	    if (size + n > bufsize) {
		bufsize += n;
		buf = reallocate(buf, bufsize + 32);
	    }
	    sprintf(buf + size, " | %s", name);
	    size += n;
	}
	if (!flags)
	    break;
    }
    if (!bufsize) {
	bufsize = 32;
	buf = allocate(bufsize);
    }
    if (flags)
	sprintf(buf + size, " | 0x%04lX", flags);
    return buf + 3;
}
#endif

/* Look up a value by its macro name.
 */
int lookupname(char const *name, unsigned long *pval)
{
    int prefix;
    int i;

    for (prefix = 0 ; prefix < prefixcount ; ++prefix) {
	if (memcmp(name, prefixes[prefix], strlen(prefixes[prefix])))
	    continue;
	for (i = 0 ; i < namesets[prefix][machine].count ; ++i) {
	    if (!strcmp(name, namesets[prefix][machine].names[i].str)) {
		if (pval)
		    *pval = namesets[prefix][machine].names[i].value;
		return TRUE;
	    }
	}
	for (i = 0 ; i < namesets[prefix][0].count ; ++i) {
	    if (!strcmp(name, namesets[prefix][0].names[i].str)) {
		if (pval)
		    *pval = namesets[prefix][0].names[i].value;
		return TRUE;
	    }
	}
    }
    return FALSE;
}
