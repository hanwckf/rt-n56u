/* elfread.c: Functions for reading specific ELF headers.
 * Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include <string.h>
#include <elf.h>
#include "gen.h"
#include "names.h"
#include "pieces.h"
#include "phdrtab.h"
#include "shdrtab.h"
#include "readelf.h"

/* Some platforms (e.g. FreeBSD) may not provide these standard
 * defines in elf.h, so default definitions are supplied as a
 * fallback.
 */
#ifndef ELFCLASSNUM
#define ELFCLASSNUM 3
#endif
#ifndef ELFDATANUM
#define ELFDATANUM 2
#endif
#ifndef EV_NUM
#define EV_NUM 2
#endif

static long filesize;			/* size of the input ELF file */
static unsigned char const *image;	/* pointer to the ELF file image */
static Elf64_Ehdr const *ehdr;		/* pointer to a scrubbed ELF header */

/* Returns true if the input file is a 64-bit ELF file.
 */
int iself64(void)
{
    return ehdr->e_ident[EI_CLASS] == ELFCLASS64;
}

/* Returns true if the input file is an ELF core file.
 */
int iscorefile(void)
{
    return ehdr->e_type == ET_CORE;
}

/* Returns a pointer to a location in the input file, given an offset.
 */
void const *getptrto(long offset, long *size)
{
    if (offset >= filesize) {
	*size = 0;
	return NULL;
    }
    if (offset + *size > filesize)
	*size = filesize - offset;
    return image + offset;
}

/* Examines the ELF file's ident header and runs some sanity checks on
 * the contents. Warnings are printed if anything in the ident looks
 * dodgy or unexpected. False is returned if there are insurmountable
 * obstacles to parsing the ELF file.
 */
static int verifyident(void)
{
    unsigned char const *ident = image;

    if (filesize < EI_NIDENT)
	return err("file is not an ELF file.");

    if (ident[EI_MAG0] != ELFMAG0 || ident[EI_MAG1] != ELFMAG1
				  || ident[EI_MAG2] != ELFMAG2
				  || ident[EI_MAG3] != ELFMAG3)
	warn("file does not contain an ELF signature.");

    if (ident[EI_CLASS] == ELFCLASSNONE || ident[EI_CLASS] >= ELFCLASSNUM)
	warn("unrecognized ELF class value: %d.", ident[EI_CLASS]);

    if (ident[EI_DATA] == ELFDATANONE || ident[EI_DATA] >= ELFDATANUM) {
	warn("unrecognized ELF data value: %d.", ident[EI_DATA]);
    } else {
	int be = 1;
	*(char*)&be = 0;
	if (ident[EI_DATA] != (be ? ELFDATA2MSB : ELFDATA2LSB))
	    return err("not a %s-endian ELF file.", (be ? "big" : "little"));
    }

    if (ident[EI_VERSION] == EV_NONE || ident[EI_VERSION] >= EV_NUM)
	warn("unrecognized ELF header version: %d.", ident[EI_VERSION]);

    return TRUE;
}

/* Makes a copy of the input file's ELF header and stores it in ehdr.
 * Warnings are printed if the examined fields don't match their
 * expected values. If the ELF header is a 32-bit structure, or if it
 * is somehow malformed, its values are extracted manually.
 */
static void copyehdr(void)
{
    Elf64_Ehdr *eh64;

    eh64 = allocate(sizeof *eh64);
    if (image[EI_CLASS] == ELFCLASS64) {
	if (filesize >= (int)(sizeof *eh64)) {
	    memcpy(eh64, image, sizeof *eh64);
	    recordpiece(0, sizeof(Elf64_Ehdr), P_EHDR, "ehdr", 0);
	} else {
	    memcpy(eh64, image, filesize);
	    memset((char*)eh64 + filesize, 0, sizeof *eh64 - filesize);
	    warn("file does not have a complete ELF header.");
	    recordpiece(0, filesize, P_SECTION, "~ehdr", 0);
	}
	if (eh64->e_version == EV_NONE || eh64->e_version >= EV_NUM)
	    warn("unrecognized ELF version: %d.", eh64->e_version);
	if (eh64->e_ehsize != sizeof(Elf64_Ehdr))
	    warn("unexpected ELF header size: %u instead of %u.",
		eh64->e_ehsize, sizeof(Elf64_Ehdr));
	if (eh64->e_phoff && eh64->e_phentsize != sizeof(Elf64_Phdr))
	    warn("unexpected program header entry size: %u instead of %u.",
		 eh64->e_phentsize, sizeof(Elf64_Phdr));
	if (eh64->e_shoff && eh64->e_shentsize != sizeof(Elf64_Shdr))
	    warn("unexpected section header entry size: %u instead of %u.",
		 eh64->e_shentsize, sizeof(Elf64_Shdr));
    } else {
	Elf32_Ehdr eh32;
	if (filesize >= (int)(sizeof eh32)) {
	    memcpy(&eh32, image, sizeof eh32);
	    recordpiece(0, sizeof(Elf32_Ehdr), P_EHDR, "ehdr", 0);
	} else {
	    memcpy(&eh32, image, filesize);
	    memset((char*)&eh32 + filesize, 0, sizeof eh32 - filesize);
	    warn("file does not have a complete ELF header.");
	    recordpiece(0, filesize, P_SECTION, "~ehdr", 0);
	}
	if (eh32.e_version == EV_NONE || eh32.e_version >= EV_NUM)
	    warn("unrecognized ELF version: %d.", eh32.e_version);
	if (eh32.e_ehsize != sizeof(Elf32_Ehdr))
	    warn("unexpected ELF header size: %u instead of %u.",
		eh32.e_ehsize, sizeof(Elf32_Ehdr));
	if (eh32.e_phoff && eh32.e_phentsize != sizeof(Elf32_Phdr))
	    warn("unexpected program header entry size: %u instead of %u.",
		 eh32.e_phentsize, sizeof(Elf32_Phdr));
	if (eh32.e_shoff && eh32.e_shentsize != sizeof(Elf32_Shdr))
	    warn("unexpected section header entry size: %u instead of %u.",
		 eh32.e_shentsize, sizeof(Elf32_Shdr));
	memcpy(eh64->e_ident, eh32.e_ident, sizeof eh64->e_ident);
	eh64->e_type = eh32.e_type;
	eh64->e_machine = eh32.e_machine;
	eh64->e_version = eh32.e_version;
	eh64->e_entry = eh32.e_entry;
	eh64->e_phoff = eh32.e_phoff;
	eh64->e_shoff = eh32.e_shoff;
	eh64->e_flags = eh32.e_flags;
	eh64->e_ehsize = eh32.e_ehsize;
	eh64->e_phentsize = eh32.e_phentsize;
	eh64->e_phnum = eh32.e_phnum;
	eh64->e_shentsize = eh32.e_shentsize;
	eh64->e_shnum = eh32.e_shnum;
	eh64->e_shstrndx = eh32.e_shstrndx;
    }

    ehdr = eh64;
}

/* Initialize the ELF file's size and location, and examine the basic
 * headers: the ELF header, the program segment header table, and the
 * section header table. False is returned if the file is not
 * analyzable.
 */
int readelf(void const *ptr, size_t size)
{
    image = ptr;
    filesize = size;
    if (!verifyident())
	return FALSE;

    recordpiece(0, filesize, P_UNCLAIMED, "~pad", 0);

    copyehdr();
    setmachinespecific(ehdr->e_machine);

    dividesegments(ehdr->e_phoff, ehdr->e_phnum, ehdr->e_phentsize);
    dividesections(ehdr->e_shoff, ehdr->e_shnum, ehdr->e_shentsize,
		   ehdr->e_shstrndx);

    return TRUE;
}
