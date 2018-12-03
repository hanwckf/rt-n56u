/* elfrw_rel.c: Functions for ELF relocation entries.
 * Copyright (C) 2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include <stdio.h>
#include <elf.h>
#include "elfrw_int.h"

int elfrw_read_Rel(FILE *fp, Elf64_Rel *in)
{
    int r;

    if (is64bit_form()) {
	r = fread(in, sizeof *in, 1, fp);
	if (r == 1) {
	    if (!native_form()) {
		revinplc_64xword(&in->r_offset);
		revinplc_64xword(&in->r_info);
	    }
	}
    } else {
	Elf32_Rel in32;
	r = fread(&in32, sizeof in32, 1, fp);
	if (r == 1) {
	    if (native_form()) {
		in->r_offset = in32.r_offset;
		in->r_info = ELF64_R_INFO(ELF32_R_SYM(in32.r_info),
					  ELF32_R_TYPE(in32.r_info));
	    } else {
		in->r_offset = rev_32word(in32.r_offset);
		revinplc_32word(&in32.r_info);
		in->r_info = ELF64_R_INFO(ELF32_R_SYM(in32.r_info),
					  ELF32_R_TYPE(in32.r_info));
	    }
	}
    }
    return r;
}

int elfrw_read_Rels(FILE *fp, Elf64_Rel *in, int count)
{
    int i;

    for (i = 0 ; i < count ; ++i)
	if (!elfrw_read_Rel(fp, &in[i]))
	    break;
    return i;
}

int elfrw_read_Rela(FILE *fp, Elf64_Rela *in)
{
    int r;

    if (is64bit_form()) {
	r = fread(in, sizeof *in, 1, fp);
	if (r == 1) {
	    if (!native_form()) {
		revinplc_64xword(&in->r_offset);
		revinplc_64xword(&in->r_info);
		revinplc_64xword(&in->r_addend);
	    }
	}
    } else {
	Elf32_Rela in32;
	r = fread(&in32, sizeof in32, 1, fp);
	if (r == 1) {
	    if (native_form()) {
		in->r_offset = in32.r_offset;
		in->r_info = ELF64_R_INFO(ELF32_R_SYM(in32.r_info),
					  ELF32_R_TYPE(in32.r_info));
		in->r_addend = in32.r_addend;
	    } else {
		in->r_offset = rev_32word(in32.r_offset);
		revinplc_32word(&in32.r_info);
		in->r_info = ELF64_R_INFO(ELF32_R_SYM(in32.r_info),
					  ELF32_R_TYPE(in32.r_info));
		in->r_addend = rev_32word(in32.r_addend);
	    }
	}
    }
    return r;
}

int elfrw_read_Relas(FILE *fp, Elf64_Rela *in, int count)
{
    int i;

    for (i = 0 ; i < count ; ++i)
	if (!elfrw_read_Rela(fp, &in[i]))
	    break;
    return i;
}

int elfrw_write_Rel(FILE *fp, Elf64_Rel const *out)
{
    if (is64bit_form()) {
	if (native_form()) {
	    return fwrite(out, sizeof *out, 1, fp);
	} else {
	    Elf64_Rel outrev;
	    outrev.r_offset = rev_64xword(out->r_offset);
	    outrev.r_info = rev_64xword(out->r_info);
	    return fwrite(&outrev, sizeof outrev, 1, fp);
	}
    } else {
	Elf32_Rel out32;
	if (native_form()) {
	    out32.r_offset = out->r_offset;
	    out32.r_info = ELF32_R_INFO(ELF64_R_SYM(out->r_info),
					ELF64_R_TYPE(out->r_info));
	} else {
	    out32.r_offset = rev_32word(out->r_offset);
	    out32.r_info = rev_32word(ELF32_R_INFO(ELF64_R_SYM(out->r_info),
						   ELF64_R_TYPE(out->r_info)));
	}
	return fwrite(&out32, sizeof out32, 1, fp);
    }
}

int elfrw_write_Rels(FILE *fp, Elf64_Rel const *out, int count)
{
    int i;

    for (i = 0 ; i < count ; ++i)
	if (!elfrw_write_Rel(fp, &out[i]))
	    break;
    return i;
}

int elfrw_write_Rela(FILE *fp, Elf64_Rela const *out)
{
    if (is64bit_form()) {
	if (native_form()) {
	    return fwrite(out, sizeof *out, 1, fp);
	} else {
	    Elf64_Rela outrev;
	    outrev.r_offset = rev_64xword(out->r_offset);
	    outrev.r_info = rev_64xword(out->r_info);
	    outrev.r_addend = rev_64xword(out->r_addend);
	    return fwrite(&outrev, sizeof outrev, 1, fp);
	}
    } else {
	Elf32_Rela out32;
	if (native_form()) {
	    out32.r_offset = out->r_offset;
	    out32.r_info = ELF32_R_INFO(ELF64_R_SYM(out->r_info),
					ELF64_R_TYPE(out->r_info));
	    out32.r_addend = out->r_addend;
	} else {
	    out32.r_offset = rev_32word(out->r_offset);
	    out32.r_info = rev_32word(ELF32_R_INFO(ELF64_R_SYM(out->r_info),
						   ELF64_R_TYPE(out->r_info)));
	    out32.r_addend = rev_32word(out->r_addend);
	}
	return fwrite(&out32, sizeof out32, 1, fp);
    }
}

int elfrw_write_Relas(FILE *fp, Elf64_Rela const *out, int count)
{
    int i;

    for (i = 0 ; i < count ; ++i)
	if (!elfrw_write_Rela(fp, &out[i]))
	    break;
    return i;
}
