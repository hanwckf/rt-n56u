/* elfrw.c: Initialization and single-value functions.
 * Copyright (C) 2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include <stdio.h>
#include <elf.h>
#include "elfrw_int.h"

/* The library's current settings.
 */
unsigned char _elfrw_native_data;
unsigned char _elfrw_current_class;
unsigned char _elfrw_current_data;
unsigned char _elfrw_current_version;

/*
 * Initialization functions.
 */

int elfrw_initialize_direct(unsigned char class, unsigned char data,
			    unsigned char version)
{
    if (!_elfrw_native_data) {
	int msb = 1;
	*(char*)&msb = 0;
	_elfrw_native_data = msb ? ELFDATA2MSB : ELFDATA2LSB;
    }

    switch (class) {
      case ELFCLASS32:	_elfrw_current_class = ELFCLASS32;	break;
      case ELFCLASS64:	_elfrw_current_class = ELFCLASS64;	break;
      default:		return -EI_CLASS;
    }

    switch (data) {
      case ELFDATA2LSB:	_elfrw_current_data = ELFDATA2LSB;	break;
      case ELFDATA2MSB:	_elfrw_current_data = ELFDATA2MSB;	break;
      default:		return -EI_DATA;
    }

    _elfrw_current_version = version;
    if (_elfrw_current_version != EV_CURRENT)
	return -EI_VERSION;

    return 0;
}

int elfrw_initialize_ident(unsigned char const *ident)
{
    if (ident[EI_MAG0] != ELFMAG0 || ident[EI_MAG1] != ELFMAG1
				  || ident[EI_MAG2] != ELFMAG2
				  || ident[EI_MAG3] != ELFMAG3)
	return -1;
    return elfrw_initialize_direct(ident[EI_CLASS], ident[EI_DATA],
				   ident[EI_VERSION]);
}

void elfrw_getsettings(unsigned char *class, unsigned char *data,
		       unsigned char *version)
{
    if (class)
	*class = _elfrw_current_class;
    if (data)
	*data = _elfrw_current_data;
    if (version)
	*version = _elfrw_current_version;
}

/*
 * The basic read functions.
 */

int elfrw_read_Half(FILE *fp, Elf64_Half *in)
{
    int r;

    r = fread(in, sizeof *in, 1, fp);
    if (!native_form())
	if (r == 1)
	    *in = rev2(*in);
    return r;
}

int elfrw_read_Word(FILE *fp, Elf64_Word *in)
{
    int r;

    r = fread(in, sizeof *in, 1, fp);
    if (!native_form())
	if (r == 1)
	    *in = rev4(*in);
    return r;
}

int elfrw_read_Xword(FILE *fp, Elf64_Xword *in)
{
    int r;

    r = fread(in, sizeof *in, 1, fp);
    if (!native_form())
	if (r == 1)
	    *in = rev8(*in);
    return r;
}

int elfrw_read_Addr(FILE *fp, Elf64_Addr *in)
{
    Elf32_Word word;
    int r;

    if (is64bit_form())
	return elfrw_read_Xword(fp, (Elf64_Xword*)in);
    r = elfrw_read_Word(fp, &word);
    if (r == 1)
	*in = (Elf64_Addr)word;
    return r;
}

int elfrw_read_Sword(FILE *fp, Elf64_Sword *in)
{
    return elfrw_read_Word(fp, (Elf64_Word*)in);
}

int elfrw_read_Sxword(FILE *fp, Elf64_Sxword *in)
{
    return elfrw_read_Xword(fp, (Elf64_Xword*)in);
}

int elfrw_read_Off(FILE *fp, Elf64_Off *in)
{
    return elfrw_read_Addr(fp, (Elf64_Addr*)in);
}

int elfrw_read_Versym(FILE *fp, Elf64_Versym *in)
{
    return elfrw_read_Half(fp, (Elf64_Half*)in);
}

/*
 * The basic write functions.
 */

int elfrw_write_Half(FILE *fp, Elf64_Half const *out)
{
    Elf64_Half outrev;

    if (native_form())
	return fwrite(out, sizeof *out, 1, fp);
    outrev = rev2(*out);
    return fwrite(&outrev, sizeof outrev, 1, fp);
}

int elfrw_write_Word(FILE *fp, Elf64_Word const *out)
{
    Elf64_Word outrev;

    if (native_form())
	return fwrite(out, sizeof *out, 1, fp);
    outrev = rev4(*out);
    return fwrite(&outrev, sizeof outrev, 1, fp);
}

int elfrw_write_Xword(FILE *fp, Elf64_Xword const *out)
{
    Elf64_Xword outrev;

    if (native_form())
	return fwrite(out, sizeof *out, 1, fp);
    outrev = rev8(*out);
    return fwrite(&outrev, sizeof outrev, 1, fp);
}

int elfrw_write_Addr(FILE *fp, Elf64_Addr const *out)
{
    Elf32_Word word;

    if (is64bit_form())
	return elfrw_write_Xword(fp, (Elf64_Xword const*)out);
    word = *out;
    return elfrw_write_Word(fp, &word);
}

int elfrw_write_Sword(FILE *fp, Elf64_Sword const *out)
{
    return elfrw_write_Word(fp, (Elf64_Word const*)out);
}

int elfrw_write_Sxword(FILE *fp, Elf64_Sxword const *out)
{
    return elfrw_write_Xword(fp, (Elf64_Xword const*)out);
}

int elfrw_write_Off(FILE *fp, Elf64_Off const *out)
{
    return elfrw_write_Addr(fp, (Elf64_Addr const*)out);
}

int elfrw_write_Versym(FILE *fp, Elf64_Versym const *out)
{
    return elfrw_write_Half(fp, (Elf64_Half const*)out);
}
