/* TI C64X DSBT ELF shared library loader suppport
 * Copyright (C) 2010 Texas Instruments Incorporated
 * Contributed by Mark Salter <msalter@redhat.com>
 *
 * Borrowed heavily from frv arch:
 * Copyright (C) 2003, 2004 Red Hat, Inc.
 * Contributed by Alexandre Oliva <aoliva@redhat.com>
 * Lots of code copied from ../i386/elfinterp.c, so:
 * Copyright (c) 1994-2000 Eric Youngdale, Peter MacDonald,
 *               David Engel, Hongjiu Lu and Mitch D'Souza
 * Copyright (C) 2001-2002, Erik Andersen
 * All rights reserved.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <features.h>

/* Program to load an ELF binary on a linux system, and run it.
   References to symbols in sharable libraries can be resolved by either
   an ELF sharable library or a linux style of shared library. */

/* Disclaimer:  I have never seen any AT&T source code for SVr4, nor have
   I ever taken any courses on internals.  This program was developed using
   information available through the book "UNIX SYSTEM V RELEASE 4,
   Programmers guide: Ansi C and Programming Support Tools", which did
   a more than adequate job of explaining everything required to get this
   working. */

extern void __c6x_cache_sync(unsigned long start, unsigned long end)
    attribute_hidden;

static void
_dl_c6x_flush_relocs(struct elf32_dsbt_loadmap *map)
{
	unsigned long s, e;
	s = map->segs[0].addr;
	e = s + map->segs[0].p_memsz;
	__c6x_cache_sync(s, e);
	s = map->segs[1].addr;
	e = s + map->segs[1].p_memsz;
	__c6x_cache_sync(s, e);
}


attribute_hidden
char *
_dl_linux_resolver (struct elf_resolve *tpnt, int reloc_entry)
{
	ELF_RELOC *this_reloc;
	char *strtab;
	ElfW(Sym) *symtab;
	int symtab_index;
	char *rel_addr;
	char *new_addr;
	char **got_addr;
	char *symname;

	rel_addr = (char *)tpnt->dynamic_info[DT_JMPREL];

	this_reloc = (ELF_RELOC *)(intptr_t)(rel_addr + reloc_entry);
	symtab_index = ELF_R_SYM(this_reloc->r_info);

	symtab = (ElfW(Sym) *) tpnt->dynamic_info[DT_SYMTAB];
	strtab = (char *) tpnt->dynamic_info[DT_STRTAB];
	symname = strtab + symtab[symtab_index].st_name;

	/* Address of GOT entry fix up */
	got_addr = (char **) DL_RELOC_ADDR(tpnt->loadaddr, this_reloc->r_offset);

	/* Get the address to be used to fill in the GOT entry.  */
	new_addr = _dl_find_hash(symname, tpnt->symbol_scope, tpnt,
				 ELF_RTYPE_CLASS_PLT, NULL);
	if (unlikely(!new_addr)) {
		_dl_dprintf(2, "%s: can't resolve symbol '%s' in lib '%s'.\n", _dl_progname, symname, tpnt->libname);
		_dl_exit(1);
	}


#if defined (__SUPPORT_LD_DEBUG__)
	if (_dl_debug_bindings) {
		_dl_dprintf(_dl_debug_file, "\nresolve function: %s", symname);
		if (_dl_debug_detail)
			_dl_dprintf(_dl_debug_file,
				    "\n\tpatched %x ==> %x @ %x\n",
				    *got_addr, new_addr, got_addr);
	}
	if (!_dl_debug_nofixups) {
		*got_addr = new_addr;
	}
#else
	*got_addr = new_addr;
#endif

	return new_addr;
}

static int
_dl_parse(struct elf_resolve *tpnt, struct dyn_elf *scope,
	  unsigned long rel_addr, unsigned long rel_size,
	  int (*reloc_fnc) (struct elf_resolve *tpnt, struct dyn_elf *scope,
			    ELF_RELOC *rpnt, ElfW(Sym) *symtab, char *strtab))
{
	unsigned int i;
	char *strtab;
	ElfW(Sym) *symtab;
	ELF_RELOC *rpnt;
	int symtab_index;

	/* Now parse the relocation information */
	rpnt = (ELF_RELOC *)rel_addr;
	rel_size = rel_size / sizeof(ELF_RELOC);

	symtab = (ElfW(Sym) *)tpnt->dynamic_info[DT_SYMTAB];
	strtab = (char *)tpnt->dynamic_info[DT_STRTAB];

	for (i = 0; i < rel_size; i++, rpnt++) {
	        int res;

		symtab_index = ELF_R_SYM(rpnt->r_info);
		debug_sym(symtab,strtab,symtab_index);
		debug_reloc(symtab,strtab,rpnt);

		res = reloc_fnc (tpnt, scope, rpnt, symtab, strtab);

		if (res==0) continue;

		_dl_dprintf(2, "\n%s: ",_dl_progname);

		if (symtab_index)
			_dl_dprintf(2, "symbol '%s': ", strtab + symtab[symtab_index].st_name);

		if (res <0) {
		        int reloc_type = ELF_R_TYPE(rpnt->r_info);
#if defined (__SUPPORT_LD_DEBUG__)
			_dl_dprintf(2, "can't handle reloc type %s\n ", _dl_reltypes(reloc_type));
#else
			_dl_dprintf(2, "can't handle reloc type %x\n", reloc_type);
#endif
			_dl_exit(-res);
		} else if (res >0) {
			_dl_dprintf(2, "can't resolve symbol\n");
			return res;
		}
	}
	_dl_c6x_flush_relocs(tpnt->loadaddr.map);
	return 0;
}

static int
_dl_do_reloc (struct elf_resolve *tpnt,struct dyn_elf *scope,
	      ELF_RELOC *rpnt, ElfW(Sym) *symtab, char *strtab)
{
	int reloc_type;
	int symtab_index;
	char *symname;
	unsigned long *reloc_addr;
	unsigned long symbol_addr, sym_val;
	long reloc_addend;
	unsigned long old_val, new_val;

	reloc_addr = (unsigned long *)(intptr_t)
		DL_RELOC_ADDR (tpnt->loadaddr, rpnt->r_offset);

	reloc_type   = ELF_R_TYPE(rpnt->r_info);
	reloc_addend = rpnt->r_addend;
	symtab_index = ELF_R_SYM(rpnt->r_info);
	symbol_addr  = 0;
	symname      = strtab + symtab[symtab_index].st_name;

	if (ELF_ST_BIND (symtab[symtab_index].st_info) == STB_LOCAL) {
		symbol_addr = (unsigned long)
			DL_RELOC_ADDR (tpnt->loadaddr, symtab[symtab_index].st_value);
	} else {
		symbol_addr = (unsigned long) _dl_find_hash(strtab + symtab[symtab_index].st_name,
							    scope, tpnt, elf_machine_type_class(reloc_type),
							    NULL);
		/*
		 * We want to allow undefined references to weak symbols - this might
		 * have been intentional.  We should not be linking local symbols
		 * here, so all bases should be covered.
		 */

		if (!symbol_addr && ELF_ST_BIND(symtab[symtab_index].st_info) != STB_WEAK) {
			_dl_dprintf (2, "%s: can't resolve symbol '%s'\n",
				     _dl_progname, strtab + symtab[symtab_index].st_name);
			_dl_exit (1);
		}
	}
	old_val = *reloc_addr;
	sym_val = symbol_addr + reloc_addend;

	switch (reloc_type) {
	case R_C6000_NONE:
		break;
	case R_C6000_ABS32:
	case R_C6000_JUMP_SLOT:
		new_val = sym_val;
		*reloc_addr = sym_val;
		break;
	case R_C6000_DSBT_INDEX:
		new_val = (old_val & ~0x007fff00) | ((tpnt->loadaddr.map->dsbt_index & 0x7fff) << 8);
		*reloc_addr = new_val;
		break;
	case R_C6000_ABS_L16:
		new_val = (old_val & ~0x007fff80) | ((sym_val & 0xffff) << 7);
		*reloc_addr = new_val;
		break;
	case R_C6000_ABS_H16:
		new_val = (old_val & ~0x007fff80) | ((sym_val >> 9) & 0x007fff80);
		*reloc_addr = new_val;
		break;
	case R_C6000_PCR_S21:
		new_val = sym_val - (((unsigned long)reloc_addr) & ~31);
		*reloc_addr = (old_val & ~0x0fffff80) | (((new_val >> 2) & 0x1fffff) << 7);
		break;
	case R_C6000_COPY:
		if (symbol_addr) {
#if defined (__SUPPORT_LD_DEBUG__)
			if (_dl_debug_move)
				_dl_dprintf(_dl_debug_file,
					    "\n%s move %d bytes from %x to %x",
					    symname, symtab[symtab_index].st_size,
					    symbol_addr, reloc_addr);
#endif
			
			_dl_memcpy((char *)reloc_addr,
				   (char *)symbol_addr,
				   symtab[symtab_index].st_size);
		}
		return 0;
	default:
		return -1; /*call _dl_exit(1) */
	}
#if defined (__SUPPORT_LD_DEBUG__)
	if (_dl_debug_reloc && _dl_debug_detail && reloc_type != R_C6000_NONE) {
		_dl_dprintf(_dl_debug_file, "\tpatched: %x ==> %x @ %x\n", old_val, new_val, reloc_addr);
	}
#endif
	return 0;
}

static int
_dl_do_lazy_reloc (struct elf_resolve *tpnt,
		   struct dyn_elf *scope attribute_unused,
		   ELF_RELOC *rpnt, ElfW(Sym) *symtab attribute_unused,
		   char *strtab attribute_unused)
{
	int reloc_type;
	unsigned long *reloc_addr;
	unsigned long old_val;

	reloc_addr = (unsigned long *) DL_RELOC_ADDR(tpnt->loadaddr, rpnt->r_offset);
	reloc_type = ELF_R_TYPE(rpnt->r_info);

	old_val = *reloc_addr;

	switch (reloc_type) {
		case R_C6000_NONE:
			break;
		case R_C6000_JUMP_SLOT:
			*reloc_addr = DL_RELOC_ADDR(tpnt->loadaddr, old_val);
			break;
		default:
			return -1;
	}

#if defined (__SUPPORT_LD_DEBUG__)
	if (_dl_debug_reloc && _dl_debug_detail)
		_dl_dprintf(_dl_debug_file, "\n\tpatched: %x ==> %x @ %x\n",
			    old_val, *reloc_addr, reloc_addr);
#endif

	return 0;
}

void
_dl_parse_lazy_relocation_information
(struct dyn_elf *rpnt, unsigned long rel_addr, unsigned long rel_size)
{
	_dl_parse(rpnt->dyn, NULL, rel_addr, rel_size, _dl_do_lazy_reloc);
}

int
_dl_parse_relocation_information
(struct dyn_elf *rpnt, unsigned long rel_addr, unsigned long rel_size)
{
	return _dl_parse(rpnt->dyn, rpnt->dyn->symbol_scope, rel_addr, rel_size, _dl_do_reloc);
}

/* We don't have copy relocs.  */
int
_dl_parse_copy_information
(struct dyn_elf *rpnt,
 unsigned long rel_addr,
 unsigned long rel_size)
{
	return 0;
}

