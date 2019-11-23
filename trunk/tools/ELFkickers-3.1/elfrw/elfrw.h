/* elfrw.h: The elfrw library's exported functions.
 * Copyright (C) 2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#ifndef _elfrw_h_
#define _elfrw_h_

#include <stdio.h>
#include <elf.h>

/*
 * The initialization functions. Call one of these to set the flavor
 * of ELF structures to translate to and from. The library can be
 * re-initialized at any time to change the current flavor.
 *
 * NOTE: It is not necessary to explicitly call an initialization
 * function if you first call elfrw_read_Ehdr() or elfrw_write_Ehdr().
 * These functions will automatically initialize the current settings
 * from the header's e_ident field.
 */

/* Selects the current flavor of ELF files by class (i.e. bitness --
 * either ELFCLASS32 or ELFCLASS64), data (i.e. endianness -- either
 * ELFDATA2LSB or ELFDATA2MSB), and version (currently only EV_CURRENT
 * is valid). Returns 0 on success, or a negative value if an invalid
 * setting was specified.
 */
extern int elfrw_initialize_direct(unsigned char class, unsigned char data,
				   unsigned char version);

/* Selects the current flavor of ELF files using the e_ident header of
 * an ELF file. Returns 0 on success, or a negative value if an
 * invalid setting was specified.
 */
extern int elfrw_initialize_ident(unsigned char const *e_ident);

/* Queries the library for the current settings. Any of the parameters
 * can be NULL to not return that value.
 */
extern void elfrw_getsettings(unsigned char *class, unsigned char *data,
			      unsigned char *version);

/*
 * The file reading functions. After initialization, the elfrw library
 * functions can be used to read parts of ELF files of that flavor.
 * The library will translate files of the selected flavor into 64-bit
 * ELF structures with the native bitness and endianness.
 *
 * The return values of the read functions are the same as that of
 * calling fread(in, sizeof *in, 1, fp) -- namely, 1 on success and 0
 * on error or end-of-file. (Note that if a short read occurs, the
 * caller's input buffer may or may not contain any of the bytes of
 * the partial read.) The only exception is elfrw_read_Ehdr, which can
 * also return a negative value if the header read was not a valid ELF
 * header.
 *
 * The plural forms of the functions read an array of ELF structures
 * from the file. The return value indicates the number of structures
 * successfully read.
 */

extern int elfrw_read_Half(FILE *fp, Elf64_Half *in);
extern int elfrw_read_Word(FILE *fp, Elf64_Word *in);
extern int elfrw_read_Sword(FILE *fp, Elf64_Sword *in);
extern int elfrw_read_Xword(FILE *fp, Elf64_Xword *in);
extern int elfrw_read_Sxword(FILE *fp, Elf64_Sxword *in);
extern int elfrw_read_Addr(FILE *fp, Elf64_Addr *in);
extern int elfrw_read_Off(FILE *fp, Elf64_Off *in);
extern int elfrw_read_Versym(FILE *fp, Elf64_Versym *in);

extern int elfrw_read_Ehdr(FILE *fp, Elf64_Ehdr *in);
extern int elfrw_read_Shdr(FILE *fp, Elf64_Shdr *in);
extern int elfrw_read_Sym(FILE *fp, Elf64_Sym *in);
extern int elfrw_read_Syminfo(FILE *fp, Elf64_Syminfo *in);
extern int elfrw_read_Rel(FILE *fp, Elf64_Rel *in);
extern int elfrw_read_Rela(FILE *fp, Elf64_Rela *in);
extern int elfrw_read_Phdr(FILE *fp, Elf64_Phdr *in);
extern int elfrw_read_Dyn(FILE *fp, Elf64_Dyn *in);
extern int elfrw_read_Verdef(FILE *fp, Elf64_Verdef *in);
extern int elfrw_read_Verdaux(FILE *fp, Elf64_Verdaux *in);
extern int elfrw_read_Verneed(FILE *fp, Elf64_Verneed *in);
extern int elfrw_read_Vernaux(FILE *fp, Elf64_Vernaux *in);

extern int elfrw_read_Shdrs(FILE *fp, Elf64_Shdr *in, int count);
extern int elfrw_read_Syms(FILE *fp, Elf64_Sym *in, int count);
extern int elfrw_read_Syminfos(FILE *fp, Elf64_Syminfo *in, int count);
extern int elfrw_read_Rels(FILE *fp, Elf64_Rel *in, int count);
extern int elfrw_read_Relas(FILE *fp, Elf64_Rela *in, int count);
extern int elfrw_read_Phdrs(FILE *fp, Elf64_Phdr *in, int count);
extern int elfrw_read_Dyns(FILE *fp, Elf64_Dyn *in, int count);

/*
 * The count functions. These simply take a file size (in bytes) of a
 * specific ELF section, and return the number of ELF structures that
 * it can contain.
 *
 * The ELF standard omits storing the number of entries in a couple of
 * sections, allowing it to be implicitly specified by the section's
 * file size. This is inconvenient for code using this library, since
 * ideally it shouldn't need to know the size of the ELF structs in
 * the file. These function help cover that omission.
 */

extern int elfrw_count_Syms(int size);
extern int elfrw_count_Syminfos(int size);
extern int elfrw_count_Dyns(int size);

/*
 * The file writing functions. After initialization, the elfrw library
 * functions can be used to write parts of ELF files of that flavor.
 * The library will accept 64-bit ELF structures with the native
 * bitness and endianness and translate them to the selected flavor.
 * Each function returns 1 on success and 0 on error.
 *
 * The plural forms of the functions write an array of ELF structures
 * to the file. The return value indicates the number of structures
 * successfully written.
 */

extern int elfrw_write_Half(FILE *fp, Elf64_Half const *out);
extern int elfrw_write_Word(FILE *fp, Elf64_Word const *out);
extern int elfrw_write_Sword(FILE *fp, Elf64_Sword const *out);
extern int elfrw_write_Xword(FILE *fp, Elf64_Xword const *out);
extern int elfrw_write_Sxword(FILE *fp, Elf64_Sxword const *out);
extern int elfrw_write_Addr(FILE *fp, Elf64_Addr const *out);
extern int elfrw_write_Off(FILE *fp, Elf64_Off const *out);
extern int elfrw_write_Versym(FILE *fp, Elf64_Versym const *out);

extern int elfrw_write_Ehdr(FILE *fp, Elf64_Ehdr const *out);
extern int elfrw_write_Shdr(FILE *fp, Elf64_Shdr const *out);
extern int elfrw_write_Sym(FILE *fp, Elf64_Sym const *out);
extern int elfrw_write_Syminfo(FILE *fp, Elf64_Syminfo const *out);
extern int elfrw_write_Rel(FILE *fp, Elf64_Rel const *out);
extern int elfrw_write_Rela(FILE *fp, Elf64_Rela const *out);
extern int elfrw_write_Phdr(FILE *fp, Elf64_Phdr const *out);
extern int elfrw_write_Dyn(FILE *fp, Elf64_Dyn const *out);
extern int elfrw_write_Verdef(FILE *fp, Elf64_Verdef const *out);
extern int elfrw_write_Verdaux(FILE *fp, Elf64_Verdaux const *out);
extern int elfrw_write_Verneed(FILE *fp, Elf64_Verneed const *out);
extern int elfrw_write_Vernaux(FILE *fp, Elf64_Vernaux const *out);

extern int elfrw_write_Shdrs(FILE *fp, Elf64_Shdr const *out, int count);
extern int elfrw_write_Syms(FILE *fp, Elf64_Sym const *out, int count);
extern int elfrw_write_Syminfos(FILE *fp, Elf64_Syminfo const *out, int count);
extern int elfrw_write_Rels(FILE *fp, Elf64_Rel const *out, int count);
extern int elfrw_write_Relas(FILE *fp, Elf64_Rela const *out, int count);
extern int elfrw_write_Phdrs(FILE *fp, Elf64_Phdr const *out, int count);
extern int elfrw_write_Dyns(FILE *fp, Elf64_Dyn const *out, int count);

#endif
