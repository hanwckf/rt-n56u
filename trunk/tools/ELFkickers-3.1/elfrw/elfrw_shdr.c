/* elfrw_shdr.c: Functions for section header table entries.
 * Copyright (C) 2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include <stdio.h>
#include <elf.h>
#include "elfrw_int.h"

/*
 * Reading and writing a section header table entry.
 */

int elfrw_read_Shdr(FILE *fp, Elf64_Shdr *in)
{
    int r;

    if (is64bit_form()) {
	r = fread(in, sizeof *in, 1, fp);
	if (r == 1) {
	    if (!native_form()) {
		revinplc_64word(&in->sh_name);
		revinplc_64word(&in->sh_type);
		revinplc_64xword(&in->sh_flags);
		revinplc_64xword(&in->sh_addr);
		revinplc_64xword(&in->sh_offset);
		revinplc_64xword(&in->sh_size);
		revinplc_64word(&in->sh_link);
		revinplc_64word(&in->sh_info);
		revinplc_64xword(&in->sh_addralign);
		revinplc_64xword(&in->sh_entsize);
	    }
	}
    } else {
	Elf32_Shdr in32;
	r = fread(&in32, sizeof in32, 1, fp);
	if (r == 1) {
	    if (native_form()) {
		in->sh_name = in32.sh_name;
		in->sh_type = in32.sh_type;
		in->sh_flags = in32.sh_flags;
		in->sh_addr = in32.sh_addr;
		in->sh_offset = in32.sh_offset;
		in->sh_size = in32.sh_size;
		in->sh_link = in32.sh_link;
		in->sh_info = in32.sh_info;
		in->sh_addralign = in32.sh_addralign;
		in->sh_entsize = in32.sh_entsize;
	    } else {
		in->sh_name = rev_32word(in32.sh_name);
		in->sh_type = rev_32word(in32.sh_type);
		in->sh_flags = rev_32word(in32.sh_flags);
		in->sh_addr = rev_32word(in32.sh_addr);
		in->sh_offset = rev_32word(in32.sh_offset);
		in->sh_size = rev_32word(in32.sh_size);
		in->sh_link = rev_32word(in32.sh_link);
		in->sh_info = rev_32word(in32.sh_info);
		in->sh_addralign = rev_32word(in32.sh_addralign);
		in->sh_entsize = rev_32word(in32.sh_entsize);
	    }
	}
    }
    return r;
}

int elfrw_read_Shdrs(FILE *fp, Elf64_Shdr *in, int count)
{
    int i;

    for (i = 0 ; i < count ; ++i)
	if (!elfrw_read_Shdr(fp, &in[i]))
	    break;
    return i;
}

int elfrw_write_Shdr(FILE *fp, Elf64_Shdr const *out)
{
    if (is64bit_form()) {
	if (native_form()) {
	    return fwrite(out, sizeof *out, 1, fp);
	} else {
	    Elf64_Shdr outrev;
	    outrev.sh_name = rev_64word(out->sh_name);
	    outrev.sh_type = rev_64word(out->sh_type);
	    outrev.sh_flags = rev_64xword(out->sh_flags);
	    outrev.sh_addr = rev_64xword(out->sh_addr);
	    outrev.sh_offset = rev_64xword(out->sh_offset);
	    outrev.sh_size = rev_64xword(out->sh_size);
	    outrev.sh_link = rev_64word(out->sh_link);
	    outrev.sh_info = rev_64word(out->sh_info);
	    outrev.sh_addralign = rev_64xword(out->sh_addralign);
	    outrev.sh_entsize = rev_64xword(out->sh_entsize);
	    return fwrite(&outrev, sizeof outrev, 1, fp);
	}
    } else {
	Elf32_Shdr out32;
	if (native_form()) {
	    out32.sh_name = out->sh_name;
	    out32.sh_type = out->sh_type;
	    out32.sh_flags = out->sh_flags;
	    out32.sh_addr = out->sh_addr;
	    out32.sh_offset = out->sh_offset;
	    out32.sh_size = out->sh_size;
	    out32.sh_link = out->sh_link;
	    out32.sh_info = out->sh_info;
	    out32.sh_addralign = out->sh_addralign;
	    out32.sh_entsize = out->sh_entsize;
	} else {
	    out32.sh_name = rev_32word(out->sh_name);
	    out32.sh_type = rev_32word(out->sh_type);
	    out32.sh_flags = rev_32word(out->sh_flags);
	    out32.sh_addr = rev_32word(out->sh_addr);
	    out32.sh_offset = rev_32word(out->sh_offset);
	    out32.sh_size = rev_32word(out->sh_size);
	    out32.sh_link = rev_32word(out->sh_link);
	    out32.sh_info = rev_32word(out->sh_info);
	    out32.sh_addralign = rev_32word(out->sh_addralign);
	    out32.sh_entsize = rev_32word(out->sh_entsize);
	}
	return fwrite(&out32, sizeof out32, 1, fp);
    }
}

int elfrw_write_Shdrs(FILE *fp, Elf64_Shdr const *out, int count)
{
    int i;

    for (i = 0 ; i < count ; ++i)
	if (!elfrw_write_Shdr(fp, &out[i]))
	    break;
    return i;
}
