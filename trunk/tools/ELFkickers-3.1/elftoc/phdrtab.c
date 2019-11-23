/* phdrtab.c: Reading the program segment header table.
 * Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include <string.h>
#include <elf.h>
#include "gen.h"
#include "readelf.h"
#include "pieces.h"
#include "address.h"
#include "dynamic.h"
#include "phdrtab.h"

/* Examines an entry in the program segment header table and uses this
 * information to identify one piece of the ELF image. If the entry
 * also indicates mapping of offsets to addresses, this is also
 * recorded. Finally, if the entry points to a dynamic table, the
 * table's contents are examined as well.
 */
static void dividesegment(Elf64_Phdr const *phdr)
{
    char const *str;
    int type, n;

    if (phdr->p_type == PT_NULL || phdr->p_filesz == 0)
        return;
    switch (phdr->p_type) {
      case PT_PHDR:	    type = P_PHDRTAB;  str = "~phdrs";	    break;
      case PT_DYNAMIC:	    type = P_DYNAMIC;  str = "~dynamic";    break;
      case PT_INTERP:	    type = P_STRINGS;  str = "~interp";	    break;
      case PT_NOTE:	    type = P_NOTE;     str = "~note";	    break;
#ifdef PT_GNU_EH_FRAME
      case PT_GNU_EH_FRAME: type = P_WORDS;    str = "~eh_frame";   break;
#endif
      case PT_LOAD:
        type = P_SECTION;
        if (phdr->p_type == PT_LOAD) {
	    if (phdr->p_flags & PF_X)
		str = "~text";
	    else if (phdr->p_flags & PF_W)
		str = "~data";
	    else if (phdr->p_flags & PF_R)
		str = "~rodata";
	    else
		str = "";
        }
        break;
      default:
        type = P_SECTION;
        str = "";
        break;
    }
    n = recordpiece(phdr->p_offset, phdr->p_filesz, type, str, phdr->p_align);
    if (phdr->p_memsz && str)
        recordaddress(phdr->p_vaddr, phdr->p_offset, phdr->p_memsz, str);
    if (type == P_DYNAMIC)
	hashdynamicsection(phdr->p_offset, phdr->p_filesz, n);
}

/* Reads the given section of the ELF image as a program segment
 * header table, and records the information contained therein.
 * Warnings are displayed if the table itself appears to be malformed
 * or incorrectly sized. If this is a 32-bit table, each entry is
 * translated into a 64-bit entry so that the dividesegment() function
 * doesn't have to understand both types.
 */
void dividesegments(long offset, int count, int entsize)
{
    Elf64_Phdr ph64;
    Elf32_Phdr ph32;
    void const *ptr;
    long size;
    int i;

    if (!offset || !count)
	return;

    if (!entsize) {
	warn("ignoring zero value for phentsize.");
	entsize = sizeof_elf(Phdr);
    } else if (entsize != sizeof_elf(Phdr)) {
	warn("program header table entry size: %d instead of %d.",
	    entsize, sizeof_elf(Phdr));
    }

    size = count * entsize;
    ptr = getptrto(offset, &size);
    if (size != count * entsize) {
	if (size == 0) {
	    warn("invalid program segment header table offset: %ld.", offset);
	    return;
	}
	warn("program header table extends %ld bytes past EOF.",
	     count * entsize - size);
    }

    recordpiece(offset, size, P_PHDRTAB, "phdrs", sizeof_elf(Addr));

    count = size / entsize;
    if (!count)
	return;

    for (i = 0 ; i < count ; ++i) {
	if (iself64()) {
	    if (entsize >= (int)sizeof(Elf64_Phdr)) {
		dividesegment((Elf64_Phdr const*)ptr + i);
	    } else {
		memcpy(&ph64, (Elf64_Phdr const*)ptr + i, entsize);
		memset((char*)&ph64 + entsize, 0, sizeof ph64 - entsize);
		dividesegment(&ph64);
	    }
	} else {
	    if (entsize >= (int)sizeof(Elf32_Phdr)) {
		memcpy(&ph32, (char const*)ptr + i * entsize, sizeof ph32);
	    } else {
		memcpy(&ph32, (char const*)ptr + i * entsize, entsize);
		memset((char*)&ph32 + entsize, 0, sizeof ph32 - entsize);
	    }
	    ph64.p_type = ph32.p_type;
	    ph64.p_flags = ph32.p_flags;
	    ph64.p_offset = ph32.p_offset;
	    ph64.p_vaddr = ph32.p_vaddr;
	    ph64.p_paddr = ph32.p_paddr;
	    ph64.p_filesz = ph32.p_filesz;
	    ph64.p_memsz = ph32.p_memsz;
	    ph64.p_align = ph32.p_align;
	    dividesegment(&ph64);
	}
    }

    dividedynsegments();
}
