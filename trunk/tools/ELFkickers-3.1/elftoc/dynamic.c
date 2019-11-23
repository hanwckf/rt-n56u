/* dynamic.c: Reading the dynamic information table.
 * Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include <stdio.h>
#include <search.h>
#include <elf.h>
#include "gen.h"
#include "pieces.h"
#include "address.h"
#include "readelf.h"
#include "outitems.h"
#include "dynamic.h"

/* A stringification macro to accompany the sizeof_elf macro.
 */
#define strsizeof_elf(type) \
    (iself64() ? "sizeof(Elf64_" #type ")" : "sizeof(Elf32_" #type ")")

/* A binary tree of entries in the dynamic table. Only unique entries
 * are remembered; tags that can have multiple entries will only have
 * the last value stored.
 */
static void *dynvalues = NULL;

/* Number of entries in the dynamic symbol table.
 */
static int symcount = 0;

/* An ordering function, arranging entries by their tag value.
 */
static int dyncmp(void const *e1, void const *e2)
{
    return ((Elf64_Dyn const*)e1)->d_tag - ((Elf64_Dyn const*)e2)->d_tag;
}

/* Finds an tag in the dynamic table and return its associated value.
 * Zero is returned if the tag is not present.
 */
static unsigned long dynget(long tag)
{
    Elf64_Dyn key;
    Elf64_Dyn const **dyn;

    key.d_tag = tag;
    dyn = tfind(&key, &dynvalues, dyncmp);
    return dyn ? (*dyn)->d_un.d_val : 0;
}

/* Stores the entries in the dynamic section in a binary tree indexed
 * by tag, and notes separately the location of the dynamic string
 * table.
 */
void hashdynamicsection(long offset, long size, int ndx)
{
    long i;

    if (!offset || !size)
	return;

    if (iself64()) {
	Elf64_Dyn const *dyns = getptrto(offset, &size);
	for (i = 0 ; (dyns + i) - dyns < size ; ++i) {
	    if (dyns[i].d_tag == DT_NULL)
		break;
	    tsearch(dyns + i, &dynvalues, dyncmp);
	}
    } else {
	Elf32_Dyn const *dyn32s = getptrto(offset, &size);
	Elf64_Dyn *dyns;
	dyns = allocate(size * sizeof(Elf64_Dyn) / sizeof(Elf32_Dyn));
	for (i = 0 ; (dyn32s + i) - dyn32s < size ; ++i) {
	    if (dyn32s[i].d_tag == DT_NULL)
		break;
	    dyns[i].d_tag = dyn32s[i].d_tag;
	    dyns[i].d_un.d_val = dyn32s[i].d_un.d_val;
	    tsearch(dyns + i, &dynvalues, dyncmp);
	}
    }

    offset = dynget(DT_STRTAB);
    size = dynget(DT_STRSZ);
    if (offset && size)
	setpiecestrtable(ndx, getaddressoffset(offset), size);
}

/* Displays a stock warning message.
 */
static void warnpartial(char const *present, char const *absent)
{
    warn("Dynamic section contains %s but no %s.", present, absent);
}

/* Weeds through the entries in the dynamic table for values that
 * identify pieces of the ELF image. Each subpart is slightly
 * different as to how its identifying data are stored in the dynamic
 * table.
 */
void dividedynsegments(void)
{
    long strtab = 0, strsize = 0;
    long symtab = 0, symsize = 0;
    long offset, size, value;
    int entsize, n;

    value = dynget(DT_STRTAB);
    if (value) {
	strsize = dynget(DT_STRSZ);
	if (strsize) {
	    strtab = getaddressoffset(value);
	    recordpiece(strtab, strsize, P_STRINGS, "~dynstr", 1);
	} else {
	    warnpartial("DT_STRTAB", "DT_STRSZ");
	}
    }

    value = dynget(DT_HASH);
    if (value) {
	Elf64_Word const *table;
	offset = getaddressoffset(value);
	size = 2 * sizeof(Elf64_Word);
	table = getptrto(offset, &size);
	if (size < 2 * (int)sizeof(Elf64_Word)) {
	    warn("invalid hash table offset: %ld.", value);
	} else {
	    symcount = table[1];
	    recordpiece(offset, (2 + table[0] + symcount) * sizeof *table,
			P_HASH, "~hash", 4);
	    value = dynget(DT_SYMTAB);
	    if (value) {
		size = dynget(DT_SYMENT);
		if (size != sizeof_elf(Sym))
		    warn("unexepected value for DT_SYMENT: %ld instead of %d.",
			 size, sizeof_elf(Sym));
		symtab = getaddressoffset(value);
		symsize = symcount * size;
		n = recordpiece(symtab, symsize, P_SYMTAB, "~dynsym",
				sizeof_elf(Addr));
		if (strsize)
		    setpiecestrtable(n, strtab, strsize);
	    }
	    value = dynget(DT_VERSYM);
	    if (value) {
		offset = getaddressoffset(value);
		recordpiece(offset, symcount * 2, P_HALVES, "~versym", 2);
	    }
	}
    }

    value = dynget(DT_REL);
    if (value) {
	size = dynget(DT_RELSZ);
	if (size) {
	    if (size % sizeof_elf(Rel) != 0)
		warn("unexpected value for DT_RELSZ: %ld instead of %d.",
		     size, sizeof_elf(Rel));
	    offset = getaddressoffset(value);
	    n = recordpiece(offset, size, P_REL, "~rel_got", sizeof_elf(Addr));
	    if (symtab) {
		setpiecemisctable(n, symtab, symsize);
		setpiecestrtable(n, strtab, strsize);
	    }
	} else {
	    warnpartial("DT_REL", "DT_RELSZ");
	}
    }
    value = dynget(DT_RELA);
    if (value) {
	size = dynget(DT_RELASZ);
	if (size) {
	    if (size % sizeof_elf(Rela) != 0)
		warn("unexpected value for DT_RELASZ: %ld instead of %d.",
		     size, sizeof_elf(Rela));
	    offset = getaddressoffset(value);
	    n = recordpiece(offset, size, P_RELA, "~rela_got",
			    sizeof_elf(Addr));
	    if (symtab) {
		setpiecemisctable(n, symtab, symsize);
		setpiecestrtable(n, strtab, strsize);
	    }
	} else {
	    warnpartial("DT_RELA", "DT_RELASZ");
	}
    }
    value = dynget(DT_SYMINFO);
    if (value) {
	size = dynget(DT_SYMINSZ);
	if (size) {
	    entsize = dynget(DT_SYMINENT);
	    if (entsize != sizeof_elf(Syminfo))
		warn("unexpected value for DT_SYMINENT: %ld instead of %d.",
		     entsize, sizeof_elf(Syminfo));
	    if (size % entsize != 0)
		warn("unexpected value for DT_SYMINSZ:"
		     " %ld is not an even multiple of %ld.", size, entsize);
	    offset = getaddressoffset(value);
	    n = recordpiece(offset, size, P_SYMINFO, "~syminfo",
			    sizeof_elf(Addr));
	    if (strsize)
		setpiecestrtable(n, strtab, strsize);
	} else {
	    warnpartial("DT_SYMINFO", "DT_SYMINSZ");
	}
    }
#ifdef DT_MOVETAB
    value = dynget(DT_MOVETAB);
    if (value) {
	size = dynget(DT_MOVESZ);
	if (size) {
	    entsize = dynget(DT_MOVEENT);
	    if (entsize != sizeof_elf(Move))
		warn("unexpected value for DT_MOVEENT: %ld instead of %d.",
		     entsize, sizeof_elf(Move));
	    if (size % entsize != 0)
		warn("unexpected value for DT_MOVESZ:"
		     " %ld is not an even multiple of %ld.", size, entsize);
	    offset = getaddressoffset(value);
	    recordpiece(offset, size, P_MOVE, "~move", sizeof_elf(Addr));
	} else {
	    warnpartial("DT_MOVETAB", "DT_MOVESZ");
	}
    }
#endif
    value = dynget(DT_INIT_ARRAY);
    if (value) {
	size = dynget(DT_INIT_ARRAYSZ);
	if (size) {
	    offset = getaddressoffset(value);
	    recordpiece(offset, size, P_ADDRS, "~init_array",
			sizeof_elf(Addr));
	} else {
	    warnpartial("DT_INIT_ARRAY","DT_INIT_ARRAYSZ");
	}
    }
    value = dynget(DT_FINI_ARRAY);
    if (value) {
	size = dynget(DT_FINI_ARRAYSZ);
	if (size) {
	    offset = getaddressoffset(value);
	    recordpiece(offset, size, P_ADDRS, "~fini_array",
			sizeof_elf(Addr));
	} else {
	    warnpartial("DT_FINI_ARRAY", "DT_FINI_ARRAYSZ");
	}
    }
    value = dynget(DT_PREINIT_ARRAY);
    if (value) {
	size = dynget(DT_PREINIT_ARRAYSZ);
	if (size) {
	    offset = getaddressoffset(value);
	    recordpiece(offset, size, P_ADDRS, "~preinit_array",
			sizeof_elf(Addr));
	} else {
	    warnpartial("DT_PREINIT_ARRAY", "DT_PREINIT_ARRAYSZ");
	}
    }

    value = dynget(DT_JMPREL);
    if (value) {
	size = dynget(DT_PLTRELSZ);
	if (size) {
	    offset = getaddressoffset(value);
	    value = dynget(DT_PLTREL);
	    if (value == DT_REL)
		n = recordpiece(offset, size, P_REL, "~plt_rel",
				sizeof_elf(Addr));
	    else if (value == DT_RELA)
		n = recordpiece(offset, size, P_RELA, "~plt_rela",
				sizeof_elf(Addr));
	    else
		n = warn("unrecognized value for DT_PLTREL: %ld.", value);
	    if (n > 0 && symtab) {
		setpiecemisctable(n, symtab, symsize);
		setpiecestrtable(n, strtab, strsize);
	    }
	} else {
	    warnpartial("DT_JMPREL", "DT_PLTRELSZ");
	}
    }
}

/* Produces a string representation of the value stored in one entry
 * of the dyanmic table by associating it with its related entries. A
 * NULL return value indicates that the caller should use a default
 * string representation.
 */
char const *strdynamicvalue(long tag, long value)
{
    long offset;
    int ndx;

    switch (tag) {
      case DT_STRSZ:
	if ((offset = dynget(DT_STRTAB)) != 0)
	    if ((ndx = getindexfromoffset(getaddressoffset(offset))) >= 0)
		return getsizestr(value, ndx);
	break;
      case DT_RELSZ:
	if ((offset = dynget(DT_REL)) != 0)
	    if ((ndx = getindexfromoffset(getaddressoffset(offset))) >= 0)
		return getsizestr(value, ndx);
	break;
      case DT_RELASZ:
	if ((offset = dynget(DT_RELA)) != 0)
	    if ((ndx = getindexfromoffset(getaddressoffset(offset))) >= 0)
		return getsizestr(value, ndx);
	break;
      case DT_PLTRELSZ:
	if ((offset = dynget(DT_JMPREL)) != 0)
	    if ((ndx = getindexfromoffset(getaddressoffset(offset))) >= 0)
		return getsizestr(value, ndx);
	break;
      case DT_SYMINSZ:
	if ((offset = dynget(DT_SYMINFO)) != 0)
	    if ((ndx = getindexfromoffset(getaddressoffset(offset))) >= 0)
		return getsizestr(value, ndx);
	break;
      case DT_INIT_ARRAYSZ:
	if ((offset = dynget(DT_INIT_ARRAY)) != 0)
	    if ((ndx = getindexfromoffset(getaddressoffset(offset))) >= 0)
		return getsizestr(value, ndx);
	break;
      case DT_FINI_ARRAYSZ:
	if ((offset = dynget(DT_FINI_ARRAY)) != 0)
	    if ((ndx = getindexfromoffset(getaddressoffset(offset))) >= 0)
		return getsizestr(value, ndx);
	break;
      case DT_PREINIT_ARRAYSZ:
	if ((offset = dynget(DT_PREINIT_ARRAY)) != 0)
	    if ((ndx = getindexfromoffset(getaddressoffset(offset))) >= 0)
		return getsizestr(value, ndx);
	break;
      case DT_RELENT:
	if (value == sizeof_elf(Rel))
	    return strsizeof_elf(Rel);
	if ((offset = dynget(DT_REL)) != 0)
	    if ((ndx = getindexfromoffset(getaddressoffset(offset))) >= 0)
		return getsizestr(value, ndx);
	break;
      case DT_RELAENT:
	if (value == sizeof_elf(Rela))
	    return strsizeof_elf(Rela);
	if ((offset = dynget(DT_RELA)) != 0)
	    if ((ndx = getindexfromoffset(getaddressoffset(offset))) >= 0)
		return getsizestr(value, ndx);
	break;
      case DT_SYMENT:
	if (value == sizeof_elf(Sym))
	    return strsizeof_elf(Sym);
	if ((offset = dynget(DT_SYMTAB)) != 0)
	    if ((ndx = getindexfromoffset(getaddressoffset(offset))) >= 0)
		return getsizestr(value, ndx);
	break;
      case DT_SYMINENT:
	if (value == sizeof_elf(Syminfo))
	    return strsizeof_elf(Syminfo);
	if ((offset = dynget(DT_SYMINFO)) != 0)
	    if ((ndx = getindexfromoffset(getaddressoffset(offset))) >= 0)
		return getsizestr(value, ndx);
	break;
#ifdef DT_MOVETAB
      case DT_MOVESZ:
	if ((offset = dynget(DT_MOVETAB)) != 0)
	    if ((ndx = getindexfromoffset(getaddressoffset(offset))) >= 0)
		return getsizestr(value, ndx);
	break;
      case DT_MOVEENT:
	if (value == sizeof_elf(Move))
	    return strsizeof_elf(Move);
	if ((offset = dynget(DT_MOVETAB)) != 0)
	    if ((ndx = getindexfromoffset(getaddressoffset(offset))) >= 0)
		return getsizestr(value, ndx);
	break;
#endif
    }
    return NULL;
}
