/* vi: set sw=4 ts=4: */
/*
 * A small little readelf implementation for uClibc
 *
 * Copyright (C) 2000 by Lineo, inc and Erik Andersen
 * Copyright (C) 2000-2002 Erik Andersen <andersee@debian.org>
 *
 * Several functions in this file (specifically, elf_find_section_type(),
 * elf_find_phdr_type(), and elf_find_dynamic(), were stolen from elflib.c from
 * elfvector (http://www.BitWagon.com/elfvector.html) by John F. Reiser
 * <jreiser@BitWagon.com>, which is copyright 2000 BitWagon Software LLC
 * (GPL2).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */


#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "bswap.h"
#include "elf.h"


int byteswap;
inline uint32_t byteswap32_to_host(uint32_t value)
{
	if (byteswap==1) {
		return(bswap_32(value));
	} else {
		return(value);
	}
}

Elf32_Shdr * elf_find_section_type( int key, Elf32_Ehdr *ehdr)
{
	int j;
	Elf32_Shdr *shdr = (Elf32_Shdr *)(ehdr->e_shoff + (char *)ehdr);
	for (j = ehdr->e_shnum; --j>=0; ++shdr) {
		if (key==(int)byteswap32_to_host(shdr->sh_type)) {
			return shdr;
		}
	}
	return NULL;
}

Elf32_Phdr * elf_find_phdr_type( int type, Elf32_Ehdr *ehdr)
{
	int j;
	Elf32_Phdr *phdr = (Elf32_Phdr *)(ehdr->e_phoff + (char *)ehdr);
	for (j = ehdr->e_phnum; --j>=0; ++phdr) {
		if (type==(int)byteswap32_to_host(phdr->p_type)) {
			return phdr;
		}
	}
	return NULL;
}

/* Returns value if return_val==1, ptr otherwise */ 
void * elf_find_dynamic(int const key, Elf32_Dyn *dynp, 
	Elf32_Ehdr *ehdr, int return_val)
{
	Elf32_Phdr *pt_text = elf_find_phdr_type(PT_LOAD, ehdr);
	unsigned tx_reloc = byteswap32_to_host(pt_text->p_vaddr) - byteswap32_to_host(pt_text->p_offset);
	for (; DT_NULL!=byteswap32_to_host(dynp->d_tag); ++dynp) {
		if (key == (int)byteswap32_to_host(dynp->d_tag)) {
			if (return_val == 1)
				return (void *)(intptr_t)byteswap32_to_host(dynp->d_un.d_val);
			else
				return (void *)(byteswap32_to_host(dynp->d_un.d_val) - tx_reloc + (char *)ehdr );
		}
	}
	return NULL;
}

int check_elf_header(Elf32_Ehdr *const ehdr)
{
	if (! ehdr || strncmp((void *)ehdr, ELFMAG, SELFMAG) != 0 ||  
			ehdr->e_ident[EI_CLASS] != ELFCLASS32 ||
			ehdr->e_ident[EI_VERSION] != EV_CURRENT) 
	{
		return 1;
	}

	/* Check if the target endianness matches the host's endianness */
	byteswap = 0;
#if __BYTE_ORDER == __LITTLE_ENDIAN
	if (ehdr->e_ident[5] == ELFDATA2MSB) {
		/* Ick -- we will have to byte-swap everything */
		byteswap = 1;
	}
#elif __BYTE_ORDER == __BIG_ENDIAN
	if (ehdr->e_ident[5] == ELFDATA2LSB) {
		byteswap = 1;
	}
#else
#error Unknown host byte order!
#endif
	/* Be vary lazy, and only byteswap the stuff we use */
	if (byteswap==1) {
		ehdr->e_type=bswap_16(ehdr->e_type);
		ehdr->e_machine=bswap_16(ehdr->e_machine);
		ehdr->e_phoff=bswap_32(ehdr->e_phoff);
		ehdr->e_shoff=bswap_32(ehdr->e_shoff);
		ehdr->e_phnum=bswap_16(ehdr->e_phnum);
		ehdr->e_shnum=bswap_16(ehdr->e_shnum);
	}
	return 0;
}


#define ELFOSABI_NONE   0       /* UNIX System V ABI */
#define ELFOSABI_HPUX   1       /* HP-UX operating system */
#define ELFOSABI_NETBSD 2       /* NetBSD */
#define ELFOSABI_LINUX  3       /* GNU/Linux */
#define ELFOSABI_HURD   4       /* GNU/Hurd */
#define ELFOSABI_SOLARIS 6      /* Solaris */
#define ELFOSABI_AIX    7       /* AIX */
#define ELFOSABI_IRIX   8       /* IRIX */
#define ELFOSABI_FREEBSD 9      /* FreeBSD */
#define ELFOSABI_TRU64  10      /* TRU64 UNIX */
#define ELFOSABI_MODESTO 11     /* Novell Modesto */
#define ELFOSABI_OPENBSD 12     /* OpenBSD */
#define ELFOSABI_STANDALONE 255 /* Standalone (embedded) application */
#define ELFOSABI_ARM   97       /* ARM */
static void describe_elf_hdr(Elf32_Ehdr* ehdr)
{
	char *tmp, *tmp1;

	switch (ehdr->e_type) {
		case ET_NONE:	tmp = "None"; tmp1 = "NONE"; break;
		case ET_REL:	tmp = "Relocatable File"; tmp1 = "REL"; break;
		case ET_EXEC:	tmp = "Executable file"; tmp1 = "EXEC"; break;
		case ET_DYN:	tmp = "Shared object file"; tmp1 = "DYN"; break;
		case ET_CORE:	tmp = "Core file"; tmp1 = "CORE"; break;
		default:
						tmp = tmp1 = "Unknown";
	}
	printf( "Type:\t\t%s (%s)\n", tmp1, tmp);

	switch (ehdr->e_machine) {
		case EM_NONE:		tmp="No machine"; break;
		case EM_M32:		tmp="AT&T WE 32100"; break;
		case EM_SPARC:		tmp="SUN SPARC"; break;
		case EM_386:		tmp="Intel 80386"; break;
		case EM_68K:		tmp="Motorola m68k family"; break;
		case EM_88K:		tmp="Motorola m88k family"; break;
		case EM_860:		tmp="Intel 80860"; break;
		case EM_MIPS:		tmp="MIPS R3000 big-endian"; break;
		case EM_S370:		tmp="IBM System/370"; break;
		case EM_MIPS_RS3_LE:	tmp="MIPS R3000 little-endian"; break;
		case EM_PARISC:		tmp="HPPA"; break;
		case EM_VPP500:		tmp="Fujitsu VPP500"; break;
		case EM_SPARC32PLUS:	tmp="Sun's v8plus"; break;
		case EM_960:		tmp="Intel 80960"; break;
		case EM_PPC:		tmp="PowerPC"; break;
		case EM_PPC64:		tmp="PowerPC 64-bit"; break;
		case EM_S390:		tmp="IBM S390"; break;
		case EM_V800:		tmp="NEC V800 series"; break;
		case EM_FR20:		tmp="Fujitsu FR20"; break;
		case EM_RH32:		tmp="TRW RH-32"; break;
		case EM_RCE:		tmp="Motorola RCE"; break;
		case EM_ARM:		tmp="ARM"; break;
		case EM_FAKE_ALPHA:	tmp="Digital Alpha"; break;
		case EM_SH:			tmp="Renesas SH"; break;
		case EM_SPARCV9:	tmp="SPARC v9 64-bit"; break;
		case EM_TRICORE:	tmp="Siemens Tricore"; break;
		case EM_ARC:		tmp="Argonaut RISC Core"; break;
		case EM_H8_300:		tmp="Renesas H8/300"; break;
		case EM_H8_300H:	tmp="Renesas H8/300H"; break;
		case EM_H8S:		tmp="Renesas H8S"; break;
		case EM_H8_500:		tmp="Renesas H8/500"; break;
		case EM_IA_64:		tmp="Intel Merced"; break;
		case EM_MIPS_X:		tmp="Stanford MIPS-X"; break;
		case EM_COLDFIRE:	tmp="Motorola Coldfire"; break;
		case EM_68HC12:		tmp="Motorola M68HC12"; break;
		case EM_MMA:		tmp="Fujitsu MMA Multimedia Accelerator"; break;
		case EM_PCP:		tmp="Siemens PCP"; break;
		case EM_NCPU:		tmp="Sony nCPU embeeded RISC"; break;
		case EM_NDR1:		tmp="Denso NDR1 microprocessor"; break;
		case EM_STARCORE:	tmp="Motorola Start*Core processor"; break;
		case EM_ME16:		tmp="Toyota ME16 processor"; break;
		case EM_ST100:		tmp="STMicroelectronic ST100 processor"; break;
		case EM_TINYJ:		tmp="Advanced Logic Corp. Tinyj emb.fam"; break;
		case EM_X86_64:		tmp="AMD x86-64 architecture"; break;
		case EM_PDSP:		tmp="Sony DSP Processor"; break;
		case EM_FX66:		tmp="Siemens FX66 microcontroller"; break;
		case EM_ST9PLUS:	tmp="STMicroelectronics ST9+ 8/16 mc"; break;
		case EM_ST7:		tmp="STmicroelectronics ST7 8 bit mc"; break;
		case EM_68HC16:		tmp="Motorola MC68HC16 microcontroller"; break;
		case EM_68HC11:		tmp="Motorola MC68HC11 microcontroller"; break;
		case EM_68HC08:		tmp="Motorola MC68HC08 microcontroller"; break;
		case EM_68HC05:		tmp="Motorola MC68HC05 microcontroller"; break;
		case EM_SVX:		tmp="Silicon Graphics SVx"; break;
		case EM_ST19:		tmp="STMicroelectronics ST19 8 bit mc"; break;
		case EM_VAX:		tmp="Digital VAX"; break;
		case EM_CRIS:		tmp="Axis Communications 32-bit embedded processor"; break;
		case EM_JAVELIN:	tmp="Infineon Technologies 32-bit embedded processor"; break;
		case EM_FIREPATH:	tmp="Element 14 64-bit DSP Processor"; break;
		case EM_ZSP:		tmp="LSI Logic 16-bit DSP Processor"; break;
		case EM_MMIX:		tmp="Donald Knuth's educational 64-bit processor"; break;
		case EM_HUANY:		tmp="Harvard University machine-independent object files"; break;
		case EM_PRISM:		tmp="SiTera Prism"; break;
		case EM_AVR:		tmp="Atmel AVR 8-bit microcontroller"; break;
		case EM_FR30:		tmp="Fujitsu FR30"; break;
		case EM_D10V:		tmp="Mitsubishi D10V"; break;
		case EM_D30V:		tmp="Mitsubishi D30V"; break;
		case EM_V850:		tmp="NEC v850"; break;
		case EM_M32R:		tmp="Renesas M32R"; break;
		case EM_MN10300:	tmp="Matsushita MN10300"; break;
		case EM_MN10200:	tmp="Matsushita MN10200"; break;
		case EM_PJ:			tmp="picoJava"; break;
		default:			tmp="unknown";
	}
	printf( "Machine:\t%s\n", tmp);	

	switch (ehdr->e_ident[EI_CLASS]) {
		case ELFCLASSNONE: tmp = "Invalid class";  break;
		case ELFCLASS32:   tmp = "ELF32"; break;
		case ELFCLASS64:   tmp = "ELF64"; break;
		default:           tmp = "Unknown";
	}
	printf( "Class:\t\t%s\n", tmp);

	switch (ehdr->e_ident[EI_DATA]) {
		case ELFDATANONE:  tmp = "Invalid data encoding"; break;
		case ELFDATA2LSB:  tmp = "2's complement, little endian"; break;
		case ELFDATA2MSB:  tmp = "2's complement, big endian"; break;
		default:           tmp = "Unknown";
	}
	printf( "Data:\t\t%s\n", tmp);

	printf( "Version:\t%d %s\n", ehdr->e_ident[EI_VERSION],
			(ehdr->e_ident[EI_VERSION]==EV_CURRENT)? 
			"(current)" : "(unknown: %lx)");

	switch (ehdr->e_ident[EI_OSABI]) {
		case ELFOSABI_SYSV:       tmp ="UNIX - System V"; break;
		case ELFOSABI_HPUX:       tmp ="UNIX - HP-UX"; break;
		case ELFOSABI_NETBSD:     tmp ="UNIX - NetBSD"; break;
		case ELFOSABI_LINUX:      tmp ="UNIX - Linux"; break;
		case ELFOSABI_HURD:       tmp ="GNU/Hurd"; break;
		case ELFOSABI_SOLARIS:    tmp ="UNIX - Solaris"; break;
		case ELFOSABI_AIX:        tmp ="UNIX - AIX"; break;
		case ELFOSABI_IRIX:       tmp ="UNIX - IRIX"; break;
		case ELFOSABI_FREEBSD:    tmp ="UNIX - FreeBSD"; break;
		case ELFOSABI_TRU64:      tmp ="UNIX - TRU64"; break;
		case ELFOSABI_MODESTO:    tmp ="Novell - Modesto"; break;
		case ELFOSABI_OPENBSD:    tmp ="UNIX - OpenBSD"; break;
		case ELFOSABI_STANDALONE: tmp ="Standalone App"; break;
		case ELFOSABI_ARM:        tmp ="ARM"; break;
		default:                  tmp = "Unknown";
	}
	printf( "OS/ABI:\t\t%s\n", tmp);

	printf( "ABI Version:\t%d\n", ehdr->e_ident[EI_ABIVERSION]);
}

static void list_needed_libraries(Elf32_Dyn* dynamic, char *strtab)
{
	Elf32_Dyn  *dyns;

	printf("Dependancies:\n");
	for (dyns=dynamic; byteswap32_to_host(dyns->d_tag)!=DT_NULL; ++dyns) {
		if (dyns->d_tag == DT_NEEDED) {
			printf("\t%s\n", (char*)strtab + byteswap32_to_host(dyns->d_un.d_val));
		}
	}
}
    
static void describe_elf_interpreter(Elf32_Ehdr* ehdr)
{
	Elf32_Phdr *phdr;
	phdr = elf_find_phdr_type(PT_INTERP, ehdr);
	if (phdr) {
		printf("Interpreter:\t%s\n", (char*)ehdr + byteswap32_to_host(phdr->p_offset));
	}
}

int main( int argc, char** argv)
{
	/* map the .so, and locate interesting pieces */
	char *dynstr;
	char *thefilename = argv[1];
	FILE *thefile;
	struct stat statbuf;
	Elf32_Ehdr *ehdr = 0;
	Elf32_Shdr *dynsec;
	Elf32_Dyn *dynamic;

	if (argc < 2 || !thefilename) {
		fprintf(stderr, "No filename specified.\n");
		exit(EXIT_FAILURE);
	}
	if (!(thefile = fopen(thefilename, "r"))) {
		perror(thefilename);
		exit(EXIT_FAILURE);
	}
	if (fstat(fileno(thefile), &statbuf) < 0) {
		perror(thefilename);
		exit(EXIT_FAILURE);
	}

	if ((size_t)statbuf.st_size < sizeof(Elf32_Ehdr))
		goto foo;

	/* mmap the file to make reading stuff from it effortless */
	ehdr = (Elf32_Ehdr *)mmap(0, statbuf.st_size, 
			PROT_READ|PROT_WRITE, MAP_PRIVATE, fileno(thefile), 0);

foo:
	/* Check if this looks legit */
	if (check_elf_header(ehdr)) {
		fprintf(stderr, "This does not appear to be an ELF file.\n");
		exit(EXIT_FAILURE);
	}
	describe_elf_hdr(ehdr);
	describe_elf_interpreter(ehdr);

	dynsec = elf_find_section_type(SHT_DYNAMIC, ehdr);
	if (dynsec) {
		dynamic = (Elf32_Dyn*)(byteswap32_to_host(dynsec->sh_offset) + (intptr_t)ehdr);
		dynstr = (char *)elf_find_dynamic(DT_STRTAB, dynamic, ehdr, 0);
		list_needed_libraries(dynamic, dynstr);
	}

	return 0;
}

