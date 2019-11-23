/* outelf32.c: Output functions for 32-bit ELF structures.
 * Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include <stddef.h>
#include <elf.h>
#include "gen.h"
#include "outbase.h"
#include "names.h"
#include "outitems.h"
#include "address.h"
#include "pieces.h"
#include "shdrtab.h"
#include "dynamic.h"
#include "outelf32.h"

/* For platforms that fail to supply a definition of NT_AUXV, the
 * usual definition is given here.
 */
#ifndef NT_AUXV
#define NT_AUXV 6
#endif

/* The output function for the 32-bit ELF header. This is the only
 * output function that outputs a single struct instead of an array.
 */
void outehdr32(void const *ptr, long size, int ndx)
{
    Elf32_Ehdr const *ehdr = ptr;
    int phndx, shndx, i;

    (void)size;
    (void)ndx;
    beginblock(TRUE);
    beginblock(FALSE);
    outchar(ehdr->e_ident[EI_MAG0]);
    outchar(ehdr->e_ident[EI_MAG1]);
    outchar(ehdr->e_ident[EI_MAG2]);
    outchar(ehdr->e_ident[EI_MAG3]);
    outdefbyte(ehdr->e_ident[EI_CLASS], "ELFCLASS");
    outdefbyte(ehdr->e_ident[EI_DATA], "ELFDATA");
    outdefbyte(ehdr->e_ident[EI_VERSION], "EV_");
    outdefbyte(ehdr->e_ident[EI_OSABI], "ELFOSABI_");
    outdec(ehdr->e_ident[EI_ABIVERSION]);
    for (i = EI_ABIVERSION + 1 ; i < EI_NIDENT ; ++i)
	outhex(ehdr->e_ident[i]);
    endblock();
    outdefint(ehdr->e_type, "ET_");
    outdefint(ehdr->e_machine, "EM_");
    outdefint(ehdr->e_version, "EV_");
    outaddress(ehdr->e_entry);
    phndx = getindexfromoffset(ehdr->e_phoff);
    shndx = getindexfromoffset(ehdr->e_shoff);
    out(getoffsetstr(ehdr->e_phoff, phndx));
    out(getoffsetstr(ehdr->e_shoff, shndx));
    outdefflags(ehdr->e_flags, "EH_");
    if (ehdr->e_ehsize == sizeof(Elf32_Ehdr))
	out("sizeof(Elf32_Ehdr)");
    else
	outdec(ehdr->e_ehsize);
    if (ehdr->e_phentsize == sizeof(Elf32_Phdr))
	out("sizeof(Elf32_Phdr)");
    else
	out(getsizestr(ehdr->e_phentsize, phndx));
    out(getcountstr(ehdr->e_phnum, phndx));
    if (ehdr->e_shentsize == sizeof(Elf32_Shdr))
	out("sizeof(Elf32_Shdr)");
    else
	out(getsizestr(ehdr->e_shentsize, shndx));
    out(getcountstr(ehdr->e_shnum, shndx));
    out(getsectionid(ehdr->e_shstrndx));
    endblock();
}

/* The output function for the 32-bit program segment header table.
 */
void outphdr32(void const *ptr, long size, int ndx)
{
    Elf32_Phdr const *phdrs = ptr;
    long count = size / sizeof *phdrs;
    char const *filesz;
    long i;

    beginblock(TRUE);
    for (i = 0 ; i < count ; ++i) {
	beginblock(FALSE);
	outdefint(phdrs[i].p_type, "PT_");
	ndx = getindexfromoffset(phdrs[i].p_offset);
	out(getoffsetstr(phdrs[i].p_offset, ndx));
	outaddress(phdrs[i].p_vaddr);
	outaddress(phdrs[i].p_paddr);
	if (phdrs[i].p_filesz == 0) {
	    out("0");
	    outhex(phdrs[i].p_memsz);
	} else {
	    filesz = getsizestr(phdrs[i].p_filesz, ndx);
	    out(filesz);
	    if (phdrs[i].p_memsz == phdrs[i].p_filesz)
		out(filesz);
	    else if (phdrs[i].p_memsz < phdrs[i].p_filesz)
		outhex(phdrs[i].p_memsz);
	    else
		outf("%s + %ld", filesz, phdrs[i].p_memsz - phdrs[i].p_filesz);
	}
	outdefflags(phdrs[i].p_flags, "PF_");
	out(getentsizestr(phdrs[i].p_align, ndx));
	endblock();
    }
    endblock();
}

/* The output function for the 32-bit section header table.
 */
void outshdr32(void const *ptr, long size, int ndx)
{
    Elf32_Shdr const *shdrs = ptr;
    long count = size / sizeof *shdrs;
    char const *strtab;
    long i;

    strtab = getpiecestrtable(ndx, &size);
    beginblock(TRUE);
    for (i = 0 ; i < count ; ++i) {
	if (strtab && strtab[shdrs[i].sh_name]) {
	    linebreak();
	    outcomment(strtab + shdrs[i].sh_name);
	}
	beginblock(FALSE);
	outdec(shdrs[i].sh_name);
	outdefint(shdrs[i].sh_type, "SHT_");
	outdefflags(shdrs[i].sh_flags, "SHF_");
	outaddress(shdrs[i].sh_addr);
	ndx = getindexfromoffset(shdrs[i].sh_offset);
	out(getoffsetstr(shdrs[i].sh_offset, ndx));
	out(getsizestr(shdrs[i].sh_size, ndx));
	out(getsectionid(shdrs[i].sh_link));
	if (shdrs[i].sh_type == SHT_REL || shdrs[i].sh_type == SHT_RELA
					|| (shdrs[i].sh_flags & SHF_INFO_LINK))
	    out(getsectionid(shdrs[i].sh_info));
	else
	    outdec(shdrs[i].sh_info);
	out(getentsizestr(shdrs[i].sh_addralign, ndx));
	out(getentsizestr(shdrs[i].sh_entsize, ndx));
	endblock();
    }
    endblock();
}

/* The output function for a 32-bit symbol table.
 */
void outsym32(void const *ptr, long size, int ndx)
{
    Elf32_Sym const *syms = ptr;
    long count = size / sizeof *syms;
    char const *strtab;
    char *bind;
    long i;

    strtab = getpiecestrtable(ndx, &size);
    beginblock(TRUE);
    for (i = 0 ; i < count ; ++i) {
	if (syms[i].st_name < (Elf32_Word)size && strtab[syms[i].st_name]) {
	    linebreak();
	    outcomment(strtab + syms[i].st_name);
	}
	beginblock(FALSE);
	outdec(syms[i].st_name);
	outaddress(syms[i].st_value);
	out(getsizestr(syms[i].st_size,
		       getindexfromsection(syms[i].st_shndx)));
	if (i == 0 && syms[i].st_info == 0 && syms[i].st_other == 0) {
	    out("0");
	    out("0");
	} else {
	    bind = strallocate(strdefint(ELF32_ST_BIND(syms[i].st_info),
					 "STB_"));
	    outf("ELF32_ST_INFO(%s, %s)",
		 bind, strdefint(ELF32_ST_TYPE(syms[i].st_info), "STT_"));
	    deallocate(bind);
	    outdefint(syms[i].st_other, "STV_");
	}
	out(getsectionid(syms[i].st_shndx));
	endblock();
    }
    endblock();
}

/* The output function for a 32-bit syminfo table.
 */
void outsyminfo32(void const *ptr, long size, int ndx)
{
    Elf32_Syminfo const *syminfos = ptr;
    long count = size / sizeof *syminfos;
    long i;

    (void)ndx;
    beginblock(TRUE);
    for (i = 0 ; i < count ; ++i) {
	beginblock(FALSE);
	outdefint(syminfos[i].si_boundto, "SYMINFO_BT_");
	outdefflags(syminfos[i].si_flags, "SYMINFO_FLG_");
	endblock();
    }
    endblock();
}

/* The output function for a 32-bit relocation table.
 */
void outrel32(void const *ptr, long size, int ndx)
{
    Elf32_Rel const *rels = ptr;
    long count = size / sizeof *rels;
    Elf32_Sym const *symtab;
    char const *strtab;
    long i;
    int n;

    symtab = getpiecemisctable(ndx, &size);
    strtab = getpiecestrtable(ndx, &size);
    beginblock(TRUE);
    for (i = 0 ; i < count ; ++i) {
	if (symtab && strtab) {
	    n = ELF32_R_SYM(rels[i].r_info);
	    if (n && symtab[n].st_name < (Elf32_Word)size
		  && strtab[symtab[n].st_name]) {
		linebreak();
		outcomment(strtab + symtab[n].st_name);
	    }
	}
	beginblock(FALSE);
	outaddress(rels[i].r_offset);
	outf("ELF32_R_INFO(%u, %s)",
	     ELF32_R_SYM(rels[i].r_info),
	     strdefint(ELF32_R_TYPE(rels[i].r_info), "R_"));
	endblock();
    }
    endblock();
}

/* The output function for a 32-bit relocation table.
 */
void outrela32(void const *ptr, long size, int ndx)
{
    Elf32_Rela const *relas = ptr;
    long count = size / sizeof *relas;
    Elf32_Sym const *symtab;
    char const *strtab;
    long i;
    int n;

    symtab = getpiecemisctable(ndx, &size);
    strtab = getpiecestrtable(ndx, &size);
    beginblock(TRUE);
    for (i = 0 ; i < count ; ++i) {
	if (symtab && strtab) {
	    n = ELF32_R_SYM(relas[i].r_info);
	    if (n && symtab[n].st_name < (Elf32_Word)size &&
		     strtab[symtab[n].st_name]) {
		linebreak();
		outcomment(strtab + symtab[n].st_name);
	    }
	}
	beginblock(FALSE);
	outaddress(relas[i].r_offset);	
	outf("ELF32_R_INFO(%u, %s)",
	     ELF32_R_SYM(relas[i].r_info),
	     strdefint(ELF32_R_TYPE(relas[i].r_info), "R_"));
	if (relas[i].r_addend >= 0 || relas[i].r_addend < -0x01000000)
	    outhex((unsigned long)relas[i].r_addend);
	else
	    outf("-%s", strhex(-relas[i].r_addend));
	endblock();
    }
    endblock();
}

/* The output function for the 32-bit move table.
 */
void outmove32(void const *ptr, long size, int ndx)
{
    Elf32_Move const *moves = ptr;
    long count = size / sizeof *moves;
    long i;

    (void)ndx;
    beginblock(TRUE);
    for (i = 0 ; i < count ; ++i) {
	beginblock(FALSE);
	outdec(moves[i].m_value);
	outf("ELF32_M_INFO(%lu, %u)",
	     ELF32_M_SYM(moves[i].m_info), ELF32_M_SIZE(moves[i].m_info));
	outhex(moves[i].m_poffset);
	outdec(moves[i].m_repeat);
	outdec(moves[i].m_stride);
	endblock();
    }
    endblock();
}

/* The output function for the 32-bit dynamic table. The
 * strdynamictable() function provides intelligent representation for
 * several entry values. A handful of others can be displayed well
 * without referring to any other entries. The remainder are
 * represented either as plain integers or as address values.
 */
void outdyn32(void const *ptr, long size, int ndx)
{
    Elf32_Dyn const *dyns = ptr;
    long count = size / sizeof *dyns;
    char const *strtab;
    char const *str = NULL;
    long value;
    long i;

    strtab = getpiecestrtable(ndx, &size);
    beginblock(TRUE);
    for (i = 0 ; i < count ; ++i) {
	beginblock(FALSE);
	outdefint(dyns[i].d_tag, "DT_");
	value = dyns[i].d_un.d_val;
	str = strdynamicvalue(dyns[i].d_tag, value);
	if (str) {
	    outf("{ %s }", str);
	    endblock();
	    continue;
	}
	switch (dyns[i].d_tag) {
	  case DT_NEEDED:
	  case DT_RPATH:
	  case DT_RUNPATH:
	  case DT_SONAME:
	    outf("{ %ld }", value);
	    if (strtab && value < size && strtab[value])
		str = strtab + value;
	    break;
	  case DT_FLAGS:
	    outf("{ %s }", strdefflags(value, "DF_"));
	    break;
	  case DT_FLAGS_1:
	    outf("{ %s }", strdefflags(value, "DF_1_"));
	    break;
	  case DT_PLTREL:
	    outf("{ %s }", strdefint(value, "DT_"));
	    break;
	  case DT_NULL:		case DT_STRSZ:	  
	  case DT_SYMENT:	case DT_SYMINSZ:	  
	  case DT_RELENT:	case DT_RELSZ:	  
	  case DT_RELAENT:	case DT_RELASZ:	  
	  case DT_SYMBOLIC:	case DT_PLTRELSZ:	  
	  case DT_BIND_NOW:	case DT_INIT_ARRAYSZ:	  
	  case DT_TEXTREL:	case DT_FINI_ARRAYSZ:	  
	  case DT_RELCOUNT:	case DT_PREINIT_ARRAYSZ:
	  case DT_RELACOUNT:
	    outf("{ %ld }", value);
	    break;
	  default:
	    outf("{ %s }", straddress(value));
	    break;
	}
	endblock();
	if (str)
	    outcomment(str);
    }
    endblock();
}

/* The output function for note sections in core files. The structure
 * of the data in a note section depends on the type of the note, so
 * most of the time only part of the arrangement can be displayed.
 * Line breaks are used to indicate where a new note header begins.
 */
void outnote32(void const *ptr, long size, int ndx)
{
    Elf64_Word const *words = ptr;
    long count = size / sizeof *words;
    long i;
    int namesize, descsize, tag;

    (void)ndx;
    beginblock(TRUE);
    i = 0;
    while (i + 3 < count) {
	namesize = (words[i] + 3) / 4;
	descsize = (words[i + 1] + 3) / 4;
	if (i + namesize > count)
	    namesize = count - i;
	if (i + namesize + descsize > count)
	    descsize = count - i - namesize;
	linebreak();
	outdec(words[i++]);
	outdec(words[i++]);
	tag = words[i++];
	outdefint(tag, "NT_");
	while (namesize--)
	    outhex(words[i++]);
	if (!descsize)
	    continue;
	if (tag == NT_AUXV) {
	    for ( ; descsize > 2 ; descsize -= 2) {
		outdefint(words[i++], "AT_");
		outhex(words[i++]);
	    }
	}
	while (descsize--)
	    outhex(words[i++]);
    }
    while (i < count)
	outhex(words[i++]);
    endblock();
}

/* The output function for a table of addresses.
 */
void outaddr32(void const *ptr, long size, int ndx)
{
    Elf32_Addr const *addrs = ptr;
    long count = size / sizeof *addrs;
    long i;

    (void)ndx;
    beginblock(TRUE);
    for (i = 0 ; i < count ; ++i)
	outaddress(addrs[i]);
    endblock();
}
