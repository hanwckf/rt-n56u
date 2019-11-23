/* elfrw_dyn.c: Functions for dynamic table entries.
 * Copyright (C) 2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include <elf.h>
#include "elfrw_int.h"

/*
 * Reading and writing the dynamic table entries.
 */

int elfrw_read_Dyn(FILE *fp, Elf64_Dyn *in)
{
    int r;

    if (is64bit_form()) {
	r = fread(in, sizeof *in, 1, fp);
	if (r == 1) {
	    if (!native_form()) {
		revinplc_64xword(&in->d_tag);
		revinplc_64xword(&in->d_un);
	    }
	}
    } else {
	Elf32_Dyn in32;
	r = fread(&in32, sizeof in32, 1, fp);
	if (r == 1) {
	    if (native_form()) {
		in->d_tag = in32.d_tag;
		in->d_un.d_val = in32.d_un.d_val;
	    } else {
		in->d_tag = rev_32word(in32.d_tag);
		in->d_un.d_val = rev_32word(in32.d_un.d_val);
	    }
	}
    }
    return r;
}

int elfrw_read_Dyns(FILE *fp, Elf64_Dyn *in, int count)
{
    int i;

    for (i = 0 ; i < count ; ++i)
	if (!elfrw_read_Dyn(fp, &in[i]))
	    break;
    return i;
}

int elfrw_write_Dyn(FILE *fp, Elf64_Dyn const *out)
{
    if (is64bit_form()) {
	if (native_form()) {
	    return fwrite(out, sizeof *out, 1, fp);
	} else {
	    Elf32_Dyn outrev;
	    outrev.d_tag = rev_64xword(out->d_tag);
	    outrev.d_un.d_val = rev_64xword(out->d_un.d_val);
	    return fwrite(&outrev, sizeof outrev, 1, fp);
	}
    } else {
	Elf32_Dyn out32;
	if (native_form()) {
	    out32.d_tag = out->d_tag;
	    out32.d_un.d_val = out->d_un.d_val;
	} else {
	    out32.d_tag = rev_32word(out->d_tag);
	    out32.d_un.d_val = rev_32word(out->d_un.d_val);
	}
	return fwrite(&out32, sizeof out32, 1, fp);
    }
}

int elfrw_write_Dyns(FILE *fp, Elf64_Dyn const *out, int count)
{
    int i;

    for (i = 0 ; i < count ; ++i)
	if (!elfrw_write_Dyn(fp, &out[i]))
	    break;
    return i;
}

int elfrw_count_Dyns(int size)
{
    return size / (is64bit_form() ? sizeof(Elf64_Dyn) : sizeof(Elf32_Dyn));
}
