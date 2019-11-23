/* elfrw_phdr.c: Functions for program segment header table entries.
 * Copyright (C) 2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include <stdio.h>
#include <elf.h>
#include "elfrw_int.h"

/*
 * Reading and writing program header table entries.
 */

int elfrw_read_Phdr(FILE *fp, Elf64_Phdr *in)
{
    int r;

    if (is64bit_form()) {
	r = fread(in, sizeof *in, 1, fp);
	if (r == 1) {
	    if (!native_form()) {
		revinplc_64word(&in->p_type);
		revinplc_64word(&in->p_flags);
		revinplc_64xword(&in->p_offset);
		revinplc_64xword(&in->p_vaddr);
		revinplc_64xword(&in->p_paddr);
		revinplc_64xword(&in->p_filesz);
		revinplc_64xword(&in->p_memsz);
		revinplc_64xword(&in->p_align);
	    }
	}
    } else {
	Elf32_Phdr in32;
	r = fread(&in32, sizeof in32, 1, fp);
	if (r == 1) {
	    if (native_form()) {
		in->p_type = in32.p_type;
		in->p_flags = in32.p_flags;
		in->p_offset = in32.p_offset;
		in->p_vaddr = in32.p_vaddr;
		in->p_paddr = in32.p_paddr;
		in->p_filesz = in32.p_filesz;
		in->p_memsz = in32.p_memsz;
		in->p_align = in32.p_align;
	    } else {
		in->p_type = rev_32word(in32.p_type);
		in->p_offset = rev_32word(in32.p_offset);
		in->p_vaddr = rev_32word(in32.p_vaddr);
		in->p_paddr = rev_32word(in32.p_paddr);
		in->p_filesz = rev_32word(in32.p_filesz);
		in->p_memsz = rev_32word(in32.p_memsz);
		in->p_flags = rev_32word(in32.p_flags);
		in->p_align = rev_32word(in32.p_align);
	    }
	}
    }
    return r;
}

int elfrw_read_Phdrs(FILE *fp, Elf64_Phdr *in, int count)
{
    int i;

    for (i = 0 ; i < count ; ++i)
	if (!elfrw_read_Phdr(fp, &in[i]))
	    break;
    return i;
}

int elfrw_write_Phdr(FILE *fp, Elf64_Phdr const *out)
{
    if (is64bit_form()) {
	if (native_form()) {
	    return fwrite(out, sizeof *out, 1, fp);
	} else {
	    Elf64_Phdr outrev;
	    outrev.p_type = rev_64word(out->p_type);
	    outrev.p_flags = rev_64word(out->p_flags);
	    outrev.p_offset = rev_64xword(out->p_offset);
	    outrev.p_vaddr = rev_64xword(out->p_vaddr);
	    outrev.p_paddr = rev_64xword(out->p_paddr);
	    outrev.p_filesz = rev_64xword(out->p_filesz);
	    outrev.p_memsz = rev_64xword(out->p_memsz);
	    outrev.p_align = rev_64xword(out->p_align);
	    return fwrite(&outrev, sizeof outrev, 1, fp);
	}
    } else {
	Elf32_Phdr out32;
	if (native_form()) {
	    out32.p_type = out->p_type;
	    out32.p_offset = out->p_offset;
	    out32.p_vaddr = out->p_vaddr;
	    out32.p_paddr = out->p_paddr;
	    out32.p_filesz = out->p_filesz;
	    out32.p_memsz = out->p_memsz;
	    out32.p_flags = out->p_flags;
	    out32.p_align = out->p_align;
	} else {
	    out32.p_type = rev_32word(out->p_type);
	    out32.p_offset = rev_32word(out->p_offset);
	    out32.p_vaddr = rev_32word(out->p_vaddr);
	    out32.p_paddr = rev_32word(out->p_paddr);
	    out32.p_filesz = rev_32word(out->p_filesz);
	    out32.p_memsz = rev_32word(out->p_memsz);
	    out32.p_flags = rev_32word(out->p_flags);
	    out32.p_align = rev_32word(out->p_align);
	}
	return fwrite(&out32, sizeof out32, 1, fp);
    }
}

int elfrw_write_Phdrs(FILE *fp, Elf64_Phdr const *out, int count)
{
    int i;

    for (i = 0 ; i < count ; ++i)
	if (!elfrw_write_Phdr(fp, &out[i]))
	    break;
    return i;
}
