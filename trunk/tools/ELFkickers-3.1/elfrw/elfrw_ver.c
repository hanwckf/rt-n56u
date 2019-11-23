/* elfrw_ver.c: Functions for ELF version entries.
 * Copyright (C) 2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include <stdio.h>
#include <elf.h>
#include "elfrw_int.h"

/*
 * Reading and writing the version information.
 */

int elfrw_read_Verdef(FILE *fp, Elf64_Verdef *in)
{
    int r;

    r = fread(in, sizeof *in, 1, fp);
    if (r == 1) {
	if (!native_form()) {
	    revinplc2(&in->vd_version);
	    revinplc2(&in->vd_flags);
	    revinplc2(&in->vd_ndx);
	    revinplc2(&in->vd_cnt);
	    revinplc4(&in->vd_hash);
	    revinplc4(&in->vd_aux);
	    revinplc4(&in->vd_next);
	}
    }
    return r;
}

int elfrw_read_Verdaux(FILE *fp, Elf64_Verdaux *in)
{
    int r;

    r = fread(in, sizeof *in, 1, fp);
    if (r == 1) {
	if (!native_form()) {
	    revinplc4(&in->vda_name);
	    revinplc4(&in->vda_next);
	}
    }
    return r;
}

int elfrw_read_Verneed(FILE *fp, Elf64_Verneed *in)
{
    int r;

    r = fread(in, sizeof *in, 1, fp);
    if (r == 1) {
	if (!native_form()) {
	    revinplc2(&in->vn_version);
	    revinplc2(&in->vn_cnt);
	    revinplc4(&in->vn_file);
	    revinplc4(&in->vn_aux);
	    revinplc4(&in->vn_next);
	}
    }
    return r;
}

int elfrw_read_Vernaux(FILE *fp, Elf64_Vernaux *in)
{
    int r;

    r = fread(in, sizeof *in, 1, fp);
    if (r == 1) {
	if (!native_form()) {
	    revinplc4(&in->vna_hash);
	    revinplc2(&in->vna_flags);
	    revinplc2(&in->vna_other);
	    revinplc4(&in->vna_name);
	    revinplc4(&in->vna_next);
	}
    }
    return r;
}

int elfrw_write_Verdef(FILE *fp, Elf64_Verdef const *out)
{
    if (native_form()) {
	return fwrite(out, sizeof *out, 1, fp);
    } else {
	Elf64_Verdef outrev;
	outrev.vd_version = rev2(out->vd_version);
	outrev.vd_flags = rev2(out->vd_flags);
	outrev.vd_ndx = rev2(out->vd_ndx);
	outrev.vd_cnt = rev2(out->vd_cnt);
	outrev.vd_hash = rev4(out->vd_hash);
	outrev.vd_aux = rev4(out->vd_aux);
	outrev.vd_next = rev4(out->vd_next);
	return fwrite(&outrev, sizeof outrev, 1, fp);
    }
}

int elfrw_write_Verdaux(FILE *fp, Elf64_Verdaux const *out)
{
    if (native_form()) {
	return fwrite(out, sizeof *out, 1, fp);
    } else {
	Elf64_Verdaux outrev;
	outrev.vda_name = rev4(out->vda_name);
	outrev.vda_next = rev4(out->vda_next);
	return fwrite(&outrev, sizeof outrev, 1, fp);
    }
}

int elfrw_write_Verneed(FILE *fp, Elf64_Verneed const *out)
{
    if (native_form()) {
	return fwrite(out, sizeof *out, 1, fp);
    } else {
	Elf64_Verneed outrev;
	outrev.vn_version = rev2(out->vn_version);
	outrev.vn_cnt = rev2(out->vn_cnt);
	outrev.vn_file = rev4(out->vn_file);
	outrev.vn_aux = rev4(out->vn_aux);
	outrev.vn_next = rev4(out->vn_next);
	return fwrite(&outrev, sizeof outrev, 1, fp);
    }
}

int elfrw_write_Vernaux(FILE *fp, Elf64_Vernaux const *out)
{
    if (native_form()) {
	return fwrite(out, sizeof *out, 1, fp);
    } else {
	Elf64_Vernaux outrev;
	outrev.vna_hash = rev4(out->vna_hash);
	outrev.vna_flags = rev2(out->vna_flags);
	outrev.vna_other = rev2(out->vna_other);
	outrev.vna_name = rev4(out->vna_name);
	outrev.vna_next = rev4(out->vna_next);
	return fwrite(&outrev, sizeof outrev, 1, fp);
    }
}
