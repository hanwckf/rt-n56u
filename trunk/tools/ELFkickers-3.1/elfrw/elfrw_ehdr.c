/* elfrw_ehdr.c: Functions for the ELF header.
 * Copyright (C) 2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include <stdio.h>
#include <string.h>
#include <elf.h>
#include "elfrw_int.h"

/*
 * Reading and writing the ELF header. elfrw_read_Ehdr() is unique in
 * that it also automatically initializes the elfrw settings.
 */

int elfrw_read_Ehdr(FILE *fp, Elf64_Ehdr *in)
{
    int r;

    r = fread(in->e_ident, EI_NIDENT, 1, fp);
    if (r != 1)
	return r;
    r = elfrw_initialize_ident(in->e_ident);
    if (r < 0)
	return r;
    if (is64bit_form()) {
	r = fread((char*)in + EI_NIDENT, sizeof *in - EI_NIDENT, 1, fp);
	if (r == 1) {
	    if (!native_form()) {
		revinplc_64half(&in->e_type);
		revinplc_64half(&in->e_machine);
		revinplc_64word(&in->e_version);
		revinplc_64xword(&in->e_entry);
		revinplc_64xword(&in->e_phoff);
		revinplc_64xword(&in->e_shoff);
		revinplc_64word(&in->e_flags);
		revinplc_64half(&in->e_ehsize);
		revinplc_64half(&in->e_phentsize);
		revinplc_64half(&in->e_phnum);
		revinplc_64half(&in->e_shentsize);
		revinplc_64half(&in->e_shnum);
		revinplc_64half(&in->e_shstrndx);
	    }
	}
    } else {
	Elf32_Ehdr in32;
	r = fread((char*)&in32 + EI_NIDENT, sizeof in32 - EI_NIDENT, 1, fp);
	if (r == 1) {
	    if (native_form()) {
		in->e_type = in32.e_type;
		in->e_machine = in32.e_machine;
		in->e_version = in32.e_version;
		in->e_entry = in32.e_entry;
		in->e_phoff = in32.e_phoff;
		in->e_shoff = in32.e_shoff;
		in->e_flags = in32.e_flags;
		in->e_ehsize = in32.e_ehsize;
		in->e_phentsize = in32.e_phentsize;
		in->e_phnum = in32.e_phnum;
		in->e_shentsize = in32.e_shentsize;
		in->e_shnum = in32.e_shnum;
		in->e_shstrndx = in32.e_shstrndx;
	    } else {
		in->e_type = rev_32half(in32.e_type);
		in->e_machine = rev_32half(in32.e_machine);
		in->e_version = rev_32word(in32.e_version);
		in->e_entry = rev_32word(in32.e_entry);
		in->e_phoff = rev_32word(in32.e_phoff);
		in->e_shoff = rev_32word(in32.e_shoff);
		in->e_flags = rev_32word(in32.e_flags);
		in->e_ehsize = rev_32half(in32.e_ehsize);
		in->e_phentsize = rev_32half(in32.e_phentsize);
		in->e_phnum = rev_32half(in32.e_phnum);
		in->e_shentsize = rev_32half(in32.e_shentsize);
		in->e_shnum = rev_32half(in32.e_shnum);
		in->e_shstrndx = rev_32half(in32.e_shstrndx);
	    }
	}
    }
    return r;
}

int elfrw_write_Ehdr(FILE *fp, Elf64_Ehdr const *out)
{
    if (elfrw_initialize_ident(out->e_ident))
	return 0;
    if (is64bit_form()) {
	if (native_form()) {
	    return fwrite(out, sizeof *out, 1, fp);
	} else {
	    Elf64_Ehdr outrev;
	    memcpy(outrev.e_ident, out->e_ident, EI_NIDENT);
	    outrev.e_type = rev_64half(out->e_type);
	    outrev.e_machine = rev_64half(out->e_machine);
	    outrev.e_version = rev_64word(out->e_version);
	    outrev.e_entry = rev_64xword(out->e_entry);
	    outrev.e_phoff = rev_64xword(out->e_phoff);
	    outrev.e_shoff = rev_64xword(out->e_shoff);
	    outrev.e_flags = rev_64word(out->e_flags);
	    outrev.e_ehsize = rev_64half(out->e_ehsize);
	    outrev.e_phentsize = rev_64half(out->e_phentsize);
	    outrev.e_phnum = rev_64half(out->e_phnum);
	    outrev.e_shentsize = rev_64half(out->e_shentsize);
	    outrev.e_shnum = rev_64half(out->e_shnum);
	    outrev.e_shstrndx = rev_64half(out->e_shstrndx);
	    return fwrite(&outrev, sizeof outrev, 1, fp);
	}
    } else {
	Elf32_Ehdr out32;
	if (native_form()) {
	    memcpy(out32.e_ident, out->e_ident, EI_NIDENT);
	    out32.e_type = out->e_type;
	    out32.e_machine = out->e_machine;
	    out32.e_version = out->e_version;
	    out32.e_entry = out->e_entry;
	    out32.e_phoff = out->e_phoff;
	    out32.e_shoff = out->e_shoff;
	    out32.e_flags = out->e_flags;
	    out32.e_ehsize = out->e_ehsize;
	    out32.e_phentsize = out->e_phentsize;
	    out32.e_phnum = out->e_phnum;
	    out32.e_shentsize = out->e_shentsize;
	    out32.e_shnum = out->e_shnum;
	    out32.e_shstrndx = out->e_shstrndx;
	} else {
	    memcpy(out32.e_ident, out->e_ident, EI_NIDENT);
	    out32.e_type = rev_32half(out->e_type);
	    out32.e_machine = rev_32half(out->e_machine);
	    out32.e_version = rev_32word(out->e_version);
	    out32.e_entry = rev_32word(out->e_entry);
	    out32.e_phoff = rev_32word(out->e_phoff);
	    out32.e_shoff = rev_32word(out->e_shoff);
	    out32.e_flags = rev_32word(out->e_flags);
	    out32.e_ehsize = rev_32half(out->e_ehsize);
	    out32.e_phentsize = rev_32half(out->e_phentsize);
	    out32.e_phnum = rev_32half(out->e_phnum);
	    out32.e_shentsize = rev_32half(out->e_shentsize);
	    out32.e_shnum = rev_32half(out->e_shnum);
	    out32.e_shstrndx = rev_32half(out->e_shstrndx);
	}
	return fwrite(&out32, sizeof out32, 1, fp);
    }
}
