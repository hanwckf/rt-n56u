/* readelf.h: Reading the headers of the ELF file.
 * Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#ifndef	_readelf_h_
#define	_readelf_h_

/* Read all of the basic headers from the file image (namely the ELF
 * header, the program segment header table, and the section header
 * table). False is returned if the file could not be parsed as an ELF
 * file. This function should be called before any other function that
 * examines the ELF file image.
 */
extern int readelf(void const *image, size_t size);

/* Returns true if the input file is a 64-bit ELF file.
 */
extern int iself64(void);

/* A macro for getting a class-appropriate sizeof.
 */
#define sizeof_elf(type) \
    ((int)(iself64() ? sizeof(Elf64_##type) : sizeof(Elf32_##type)))

/* Returns true if the input file is an ELF core file.
 */
extern int iscorefile(void);

/* Given a file offset, returns a pointer to that position in the
 * input file. size points to the desired amount of data to read. If
 * the requested amount would extend past the end of the file, the
 * value is adjusted upon return to the actual amount of data
 * available, or zero if offset itself extends beyond the file.
 */
extern void const *getptrto(long offset, long *size);

#endif
