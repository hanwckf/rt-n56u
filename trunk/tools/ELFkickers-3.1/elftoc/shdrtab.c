/* shdrtab.c: Reading the section header table.
 * Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <elf.h>
#include "gen.h"
#include "readelf.h"
#include "names.h"
#include "pieces.h"
#include "outbase.h"
#include "outitems.h"
#include "shdrtab.h"

/* An extra value is added to the section name enum to represent the
 * total number of sections.
 */
#define LAST_ENUM "COUNT"

/* Each entry in the enum is identified by a name and an index value.
 */
struct enumid {
    char *name;
    int ndx;
};

/* The list of enum identifiers.
 */
static struct enumid *sectionenums = NULL;
static int sectionenumcount = 0;

/* Creates the enum identifier for a section based on its name.
 * (Sections entries without names do not get included in the enum.)
 */
static void addenumname(int ndx, char const *name)
{
    char const *src;
    char *dest;
    int i;

    dest = allocate(16 + strlen(name));
    for (src = name ; *src ; ++src)
	if (isalnum(*src))
	    break;
    if (*src) {
	memcpy(dest, "SHN_", 4);
	for (i = 0 ; src[i] ; ++i)
	    dest[4 + i] = isalnum(src[i]) ? toupper(src[i]) : '_';
	dest[4 + i] = '\0';
    } else {
	sprintf(dest, "SHN_%d", ndx);
    }
    sectionenums = reallocate(sectionenums,
			      (sectionenumcount + 1) * sizeof *sectionenums);
    sectionenums[sectionenumcount].name = dest;
    sectionenums[sectionenumcount].ndx = ndx;
    ++sectionenumcount;
}

/* Ensures that there are no duplicated enum identifiers, and that no
 * enum name matches one of the predefined SHN_* identifiers.
 */
static void enumfinalize(void)
{
    int i, j;

    for (i = 0 ; i < sectionenumcount ; ++i) {
	if (lookupname(sectionenums[i].name, NULL))
	    goto conflict;
	if (!strcmp(sectionenums[i].name, "SHN_" LAST_ENUM))
	    goto conflict;
	for (j = 0 ; j < i ; ++j) {
	    if (!strcmp(sectionenums[i].name, sectionenums[j].name))
		goto conflict;
	}
	continue;
      conflict:
	sprintf(sectionenums[i].name + strlen(sectionenums[i].name),
		"_%d", sectionenums[i].ndx);
    }
}

/* Extract the section header table as an array of Elf64_Shdr. If the
 * section header table is not already in this form, then a translated
 * copy is made.
 */
static Elf64_Shdr const *copytable(char const *ptr, int count, int entsize)
{
    Elf64_Shdr *shdrs;
    Elf32_Shdr sh32;
    int i;

    if (iself64() && entsize == sizeof *shdrs)
	return (Elf64_Shdr const*)ptr;

    shdrs = allocate(count * sizeof *shdrs);
    for (i = 0 ; i < count ; ++i) {
	if (iself64()) {
	    if (entsize > (int)(sizeof *shdrs)) {
		memcpy(shdrs + i, ptr + i * entsize, sizeof *shdrs);
	    } else {
		memcpy(shdrs + i, ptr + i * entsize, entsize);
		memset((char*)(shdrs + i) + entsize, 0,
		       sizeof *shdrs - entsize);
	    }
	} else {
	    if (entsize >= (int)(sizeof *shdrs)) {
		memcpy(&sh32, ptr + i * entsize, sizeof sh32);
	    } else {
		memcpy(&sh32, ptr + i * entsize, entsize);
		memset((char*)&sh32 + entsize, 0, sizeof sh32 - entsize);
	    }
	    shdrs[i].sh_name = sh32.sh_name;
	    shdrs[i].sh_type = sh32.sh_type;
	    shdrs[i].sh_flags = sh32.sh_flags;
	    shdrs[i].sh_addr = sh32.sh_addr;
	    shdrs[i].sh_offset = sh32.sh_offset;
	    shdrs[i].sh_size = sh32.sh_size;
	    shdrs[i].sh_link = sh32.sh_link;
	    shdrs[i].sh_info = sh32.sh_info;
	    shdrs[i].sh_addralign = sh32.sh_addralign;
	    shdrs[i].sh_entsize = sh32.sh_entsize;
	}
    }
    return shdrs;
}

/* Uses a section's name and other information in the section header
 * table to guess the type of its contents.
 */
static int selectshtype(Elf64_Shdr const *shdr, char const *name)
{
    if (!strcmp(name, ".got"))
	return P_ADDRS;
    if (!strncmp(name, ".got.", 5))
	return P_ADDRS;
    if (!strncmp(name, ".ctors", 6))
	return P_ADDRS;
    if (!strncmp(name, ".dtors", 6))
	return P_ADDRS;

    if (shdr->sh_flags & SHF_STRINGS)
	return P_STRINGS;
    if (shdr->sh_flags & SHF_EXECINSTR)
	return P_BYTES;

    switch (shdr->sh_entsize) {
      case 8:	return P_XWORDS;
      case 4:	return P_WORDS;
      case 2:	return P_HALVES;
      case 1:	return P_BYTES;
    }

    return P_SECTION;
}

/* Consider an entry in the section header table and use its
 * information to identify one piece of the ELF iamge. A section's
 * size and type is typically the best indicator this program has as
 * to its contents. The name in the section header string table is
 * used to assign an identifier for the piece's identifier, as well as
 * the section's enum.
 */
static void dividesection(Elf64_Shdr const *shdrs, int shndx,
			  char const *strtab)
{
    char name[64];
    char const *str;
    int type, n;

    if (shdrs[shndx].sh_type == SHT_NULL)
	return;

    switch (shdrs[shndx].sh_type) {
      case SHT_NOBITS:		type = 0;		break;
      case SHT_PROGBITS:	type = P_SECTION;	break;
      case SHT_STRTAB:		type = P_STRINGS;	break;
      case SHT_SYMTAB:		type = P_SYMTAB;	break;
      case SHT_DYNSYM:		type = P_SYMTAB;	break;
      case SHT_HASH:		type = P_HASH;		break;
      case SHT_DYNAMIC:		type = P_DYNAMIC;	break;
      case SHT_REL:		type = P_REL;		break;
      case SHT_RELA:		type = P_RELA;		break;
      case SHT_NOTE:		type = P_NOTE;		break;
      case SHT_INIT_ARRAY:	type = P_ADDRS;		break;
      case SHT_FINI_ARRAY:	type = P_ADDRS;		break;
      case SHT_PREINIT_ARRAY:	type = P_ADDRS;		break;
#ifdef SHT_GNU_HASH
      case SHT_GNU_HASH:	type = P_GNUHASH;	break;
#endif
#ifdef SHT_GNU_verdef
      case SHT_GNU_verdef:	type = P_WORDS;		break;
      case SHT_GNU_verneed:	type = P_WORDS;		break;
      case SHT_GNU_versym:	type = P_HALVES;	break;
#endif
      case SHT_SUNW_move:	type = P_MOVE;		break;
      case SHT_SUNW_syminfo:	type = P_SYMINFO;	break;
      default:			type = P_SECTION;	break;
    }

    *name = '\0';
    if (strtab && strtab[shdrs[shndx].sh_name]) {
        str = strtab + shdrs[shndx].sh_name;
        addenumname(shndx, str);
	if (type == P_SECTION)
	    type = selectshtype(shdrs + shndx, str);
        while (*str && !isalpha(*str))
	    ++str;
        sprintf(name, "%-.*s", (int)sizeof name - 1, str);
        for (n = 0 ; name[n] ; ++n)
	    if (!isalnum(name[n]))
		name[n] = '_';
    }

    if (type) {
        n = recordpiece(shdrs[shndx].sh_offset, shdrs[shndx].sh_size,
			type, name, shdrs[shndx].sh_addralign);
	if (n < 0)
	    return;
	setpieceshndx(n, shndx);
	if (shdrs[shndx].sh_link) {
	    shndx = shdrs[shndx].sh_link;
	    switch (type) {
	      case P_SYMTAB:
		setpiecestrtable(n, shdrs[shndx].sh_offset,
				    shdrs[shndx].sh_size);
		break;
	      case P_REL:
	      case P_RELA:
		setpiecemisctable(n, shdrs[shndx].sh_offset,
				     shdrs[shndx].sh_size);
		shndx = shdrs[shndx].sh_link;
		setpiecestrtable(n, shdrs[shndx].sh_offset,
				    shdrs[shndx].sh_size);
		break;
	    }
	}
    }
}

/* Reads the given section of the ELF image as a section header table,
 * and uses it break the file into separate pieces. Warnings are
 * displayed if the table appears to be malformed or invalid. The
 * section header string table, if present, is loaded as well.
 */
void dividesections(long offset, int count, int entsize, int shstrndx)
{
    void const *ptr;
    Elf64_Shdr const *shdrs;
    char const *strtab = NULL;
    long size;
    int i, n;

    if (!offset || !count)
	return;

    if (!entsize) {
	warn("ignoring zero value for shentsize.");
	entsize = sizeof_elf(Shdr);
    } else if (entsize != sizeof_elf(Shdr)) {
	warn("section table entry size: %d instead of %d.",
	     entsize, sizeof_elf(Shdr));
    }

    size = count * entsize;
    ptr = getptrto(offset, &size);
    if (size != count * entsize) {
	if (size == 0) {
	    warn("invalid section header table offset: %ld.", offset);
	    return;
	}
	warn("section table extends %ld bytes past EOF.",
	     count * entsize - size);
    }

    count = size / entsize;
    if (!count)
	return;

    shdrs = copytable(ptr, count, entsize);

    n = recordpiece(offset, size, P_SHDRTAB, "shdrs", sizeof_elf(Addr));
    if (shstrndx) {
	if (shstrndx >= count) {
	    warn("invalid section header string table index: %d.", shstrndx);
	} else {
	    size = shdrs[shstrndx].sh_size;
	    strtab = getptrto(shdrs[shstrndx].sh_offset, &size);
	    if (size <= 0) {
		strtab = NULL;
		warn("invalid section header string table offset: %ld.",
		     shdrs[shstrndx].sh_offset);
	    } else {
		setpiecestrtable(n, shdrs[shstrndx].sh_offset, size);
	    }
	}
    }

    for (i = 0 ; i < count ; ++i)
	dividesection(shdrs, i, strtab);

    if (sectionenumcount) {
	enumfinalize();
	addenumname(count, LAST_ENUM);
    }
}

/* Returns the identifier name for a given section index. If no such
 * name exists, a decimal literal is returned instead.
 */
char const *getsectionid(int ndx)
{
    static char buf[16];
    char const *name;
    int i;

    if (sectionenums)
	for (i = 0 ; i < sectionenumcount ; ++i)
	    if (sectionenums[i].ndx == ndx)
		return sectionenums[i].name;
    name = findname(ndx, "SHN_");
    if (name)
	return name;
    sprintf(buf, "%d", ndx);
    return buf;
}

/* Outputs the section index identifiers as an enum definition.
 */
void outputenum(void)
{
    int prev = -1;
    int i;

    if (sectionenumcount == 0)
	return;
    out("enum sections");
    beginblock(TRUE);
    for (i = 0 ; i < sectionenumcount ; ++i) {
	if (sectionenums[i].ndx == prev + 1)
	    out(sectionenums[i].name);
	else
	    outf("%s = %d", sectionenums[i].name, sectionenums[i].ndx);
	prev = sectionenums[i].ndx;
    }
    endblock();
    out("\n");
}
