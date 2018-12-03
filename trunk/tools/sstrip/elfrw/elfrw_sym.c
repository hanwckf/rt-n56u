/* elfrw_sym.c: Functions for symbol table entries.
 * Copyright (C) 2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include <stdio.h>
#include <elf.h>
#include "elfrw_int.h"

/*
 * Reading and writing symbol table entries.
 */

int elfrw_read_Sym(FILE *fp, Elf64_Sym *in)
{
    int r;

    if (is64bit_form()) {
	r = fread(in, sizeof *in, 1, fp);
	if (r == 1) {
	    if (!native_form()) {
		revinplc_64word(&in->st_name);
		revinplc_64word(&in->st_shndx);
		revinplc_64xword(&in->st_value);
		revinplc_64xword(&in->st_size);
	    }
	}
    } else {
	Elf32_Sym in32;
	r = fread(&in32, sizeof in32, 1, fp);
	if (r == 1) {
	    if (native_form()) {
		in->st_name = in32.st_name;
		in->st_info = ELF64_ST_INFO(ELF32_ST_BIND(in32.st_info),
					    ELF32_ST_TYPE(in32.st_info));
		in->st_other = in32.st_other;
		in->st_shndx = in32.st_shndx;
		in->st_value = in32.st_value;
		in->st_size = in32.st_size;
	    } else {
		in->st_name = rev_32word(in32.st_name);
		in->st_info = ELF64_ST_INFO(ELF32_ST_BIND(in32.st_info),
					    ELF32_ST_TYPE(in32.st_info));
		in->st_other = in32.st_other;
		in->st_value = rev_32word(in32.st_value);
		in->st_size = rev_32word(in32.st_size);
		in->st_shndx = rev_32word(in32.st_shndx);
	    }
	}
    }
    return r;
}

int elfrw_read_Syms(FILE *fp, Elf64_Sym *in, int count)
{
    int i;

    for (i = 0 ; i < count ; ++i)
	if (!elfrw_read_Sym(fp, &in[i]))
	    break;
    return i;
}

int elfrw_read_Syminfo(FILE *fp, Elf64_Syminfo *in)
{
    int r;

    r = fread(in, sizeof *in, 1, fp);
    if (r == 1) {
	if (!native_form()) {
	    revinplc2(&in->si_boundto);
	    revinplc2(&in->si_flags);
	}
    }
    return r;
}

int elfrw_read_Syminfos(FILE *fp, Elf64_Syminfo *in, int count)
{
    int i;

    for (i = 0 ; i < count ; ++i)
	if (!elfrw_read_Syminfo(fp, &in[i]))
	    break;
    return i;
}

int elfrw_write_Sym(FILE *fp, Elf64_Sym const *out)
{
    if (is64bit_form()) {
	if (native_form()) {
	    return fwrite(out, sizeof *out, 1, fp);
	} else {
	    Elf64_Sym outrev;
	    outrev.st_name = rev_64word(out->st_name);
	    outrev.st_info = out->st_info;
	    outrev.st_other = out->st_other;
	    outrev.st_shndx = rev_64word(out->st_shndx);
	    outrev.st_value = rev_64xword(out->st_value);
	    outrev.st_size = rev_64xword(out->st_size);
	    return fwrite(&outrev, sizeof outrev, 1, fp);
	}
    } else {
	Elf32_Sym out32;
	if (native_form()) {
	    out32.st_name = out->st_name;
	    out32.st_value = out->st_value;
	    out32.st_size = out->st_size;
	    out32.st_info = ELF32_ST_INFO(ELF64_ST_BIND(out->st_info),
					  ELF64_ST_TYPE(out->st_info));
	    out32.st_other = out->st_other;
	    out32.st_shndx = out->st_shndx;
	} else {
	    out32.st_name = rev_32word(out->st_name);
	    out32.st_value = rev_32word(out->st_value);
	    out32.st_size = rev_32word(out->st_size);
	    out32.st_info = ELF32_ST_INFO(ELF64_ST_BIND(out->st_info),
					  ELF64_ST_TYPE(out->st_info));
	    out32.st_other = out->st_other;
	    out32.st_shndx = rev_32word(out->st_shndx);
	}
	return fwrite(&out32, sizeof out32, 1, fp);
    }
}

int elfrw_write_Syms(FILE *fp, Elf64_Sym const *out, int count)
{
    int i;

    for (i = 0 ; i < count ; ++i)
	if (!elfrw_write_Sym(fp, &out[i]))
	    break;
    return i;
}

int elfrw_write_Syminfo(FILE *fp, Elf64_Syminfo const *out)
{
    if (native_form()) {
	return fwrite(out, sizeof *out, 1, fp);
    } else {
	Elf64_Syminfo outrev;
	outrev.si_boundto = rev2(out->si_boundto);
	outrev.si_flags = rev2(out->si_flags);
	return fwrite(&outrev, sizeof outrev, 1, fp);
    }
}

int elfrw_write_Syminfos(FILE *fp, Elf64_Syminfo const *out, int count)
{
    int i;

    for (i = 0 ; i < count ; ++i)
	if (!elfrw_write_Syminfo(fp, &out[i]))
	    break;
    return i;
}

int elfrw_count_Syms(int size)
{
    return size / (is64bit_form() ? sizeof(Elf64_Sym) : sizeof(Elf32_Sym));
}

int elfrw_count_Syminfos(int size)
{
    return size / (is64bit_form() ? sizeof(Elf64_Syminfo)
				  : sizeof(Elf32_Syminfo));
}
