/* FR-V FDPIC ELF shared library loader suppport
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

#include <sys/cdefs.h>	    /* __attribute_used__ */
#include <features.h>

/* Program to load an ELF binary on a linux system, and run it.
   References to symbols in sharable libraries can be resolved by either
   an ELF sharable library or a linux style of shared library. */

struct funcdesc_value volatile attribute_hidden *
_dl_linux_resolver (struct elf_resolve *tpnt, int reloc_entry)
{
	ELF_RELOC *this_reloc;
	char *strtab;
	ElfW(Sym) *symtab;
	int symtab_index;
	char *rel_addr;
	char *new_addr;
	struct funcdesc_value funcval;
	struct funcdesc_value volatile *got_entry;
	char *symname;
	struct symbol_ref sym_ref;

	rel_addr = (char *)tpnt->dynamic_info[DT_JMPREL];

	this_reloc = (ELF_RELOC *)(intptr_t)(rel_addr + reloc_entry);
	symtab_index = ELF_R_SYM(this_reloc->r_info);

	symtab = (ElfW(Sym) *) tpnt->dynamic_info[DT_SYMTAB];
	strtab = (char *) tpnt->dynamic_info[DT_STRTAB];
	sym_ref.sym = &symtab[symtab_index];
	sym_ref.tpnt = NULL;
	symname= strtab + symtab[symtab_index].st_name;

	/* Address of GOT entry fix up */
	got_entry = (struct funcdesc_value *) DL_RELOC_ADDR(tpnt->loadaddr, this_reloc->r_offset);

	/* Get the address to be used to fill in the GOT entry.  */
	new_addr = _dl_find_hash(symname, &_dl_loaded_modules->symbol_scope, NULL, 0, &sym_ref);
	if (!new_addr) {
		new_addr = _dl_find_hash(symname, NULL, NULL, 0, &sym_ref);
		if (!new_addr) {
			_dl_dprintf(2, "%s: can't resolve symbol '%s'\n",
				    _dl_progname, symname);
			_dl_exit(1);
		}
	}

	funcval.entry_point = new_addr;
	funcval.got_value = sym_ref.tpnt->loadaddr.got_value;

#if defined (__SUPPORT_LD_DEBUG__)
		if (_dl_debug_bindings)
		{
			_dl_dprintf(_dl_debug_file, "\nresolve function: %s", symname);
			if (_dl_debug_detail)
				_dl_dprintf(_dl_debug_file,
					    "\n\tpatched (%x,%x) ==> (%x,%x) @ %x\n",
					    got_entry->entry_point, got_entry->got_value,
					    funcval.entry_point, funcval.got_value,
					    got_entry);
		}
	if (!_dl_debug_nofixups) {
		*got_entry = funcval;
	}
#else
	*got_entry = funcval;
#endif

	return got_entry;
}

static int
_dl_parse(struct elf_resolve *tpnt, struct r_scope_elem *scope,
	  unsigned long rel_addr, unsigned long rel_size,
	  int (*reloc_fnc) (struct elf_resolve *tpnt, struct r_scope_elem *scope,
			    ELF_RELOC *rpnt, ElfW(Sym) *symtab, char *strtab))
{
	unsigned int i;
	char *strtab;
	ElfW(Sym) *symtab;
	ELF_RELOC *rpnt;
	int symtab_index;

	/* Now parse the relocation information */
	rpnt = (ELF_RELOC *) rel_addr;
	rel_size = rel_size / sizeof(ELF_RELOC);

	symtab = (ElfW(Sym) *) tpnt->dynamic_info[DT_SYMTAB];
	strtab = (char *) tpnt->dynamic_info[DT_STRTAB];

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

		if (res <0)
		{
		        int reloc_type = ELF_R_TYPE(rpnt->r_info);
			_dl_dprintf(2, "can't handle reloc type %x\n", reloc_type);
			_dl_exit(-res);
		}
		else if (res >0)
		{
			_dl_dprintf(2, "can't resolve symbol\n");
			return res;
		}
	  }
	  return 0;
}

static int
_dl_do_reloc (struct elf_resolve *tpnt,struct r_scope_elem *scope,
	      ELF_RELOC *rpnt, ElfW(Sym) *symtab, char *strtab)
{
	int reloc_type;
	int symtab_index;
	char *symname;
	unsigned long reloc_value = 0, *reloc_addr;
	struct { unsigned long v; } __attribute__((__packed__))
					    *reloc_addr_packed;
	unsigned long symbol_addr;
	struct elf_resolve *symbol_tpnt;
	struct funcdesc_value funcval;
#if defined (__SUPPORT_LD_DEBUG__)
	unsigned long old_val;
#endif
	struct symbol_ref sym_ref;

	reloc_addr   = (unsigned long *) DL_RELOC_ADDR (tpnt->loadaddr, rpnt->r_offset);
	__asm__ ("" : "=r" (reloc_addr_packed) : "0" (reloc_addr));
	reloc_type   = ELF_R_TYPE(rpnt->r_info);
	symtab_index = ELF_R_SYM(rpnt->r_info);
	symbol_addr  = 0;
	sym_ref.sym =  &symtab[symtab_index];
	sym_ref.tpnt =  NULL;
	symname      = strtab + symtab[symtab_index].st_name;

	if (ELF_ST_BIND (symtab[symtab_index].st_info) == STB_LOCAL) {
		symbol_addr = (unsigned long) DL_RELOC_ADDR(tpnt->loadaddr, symtab[symtab_index].st_value);
		symbol_tpnt = tpnt;
	} else {

		symbol_addr = (unsigned long)
		  _dl_find_hash(symname, scope, NULL, 0, &sym_ref);

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
		symbol_tpnt = sym_ref.tpnt;
	}

#if defined (__SUPPORT_LD_DEBUG__)
	if (_dl_debug_reloc && _dl_debug_detail)
	  {
	    if ((long)reloc_addr_packed & 3)
	      old_val = reloc_addr_packed->v;
	    else
	      old_val = *reloc_addr;
	  }
	else
	  old_val = 0;
#endif
	switch (reloc_type) {
	case R_FRV_NONE:
		break;
	case R_FRV_32:
		if ((long)reloc_addr_packed & 3)
			reloc_value = reloc_addr_packed->v += symbol_addr;
		else
			reloc_value = *reloc_addr += symbol_addr;
		break;
	case R_FRV_FUNCDESC_VALUE:
		funcval.entry_point = (void*)symbol_addr;
		/* The addend of FUNCDESC_VALUE
		   relocations referencing global
		   symbols must be ignored, because it
		   may hold the address of a lazy PLT
		   entry.  */
		if (ELF_ST_BIND
		    (symtab[symtab_index].st_info)
		    == STB_LOCAL)
			funcval.entry_point += *reloc_addr;
		reloc_value = (unsigned long)funcval.entry_point;
		if (symbol_addr)
			funcval.got_value
				= symbol_tpnt->loadaddr.got_value;
		else
			funcval.got_value = 0;
		__asm__ ("std%I0\t%1, %M0"
		     : "=m" (*(struct funcdesc_value *)reloc_addr)
		     : "e" (funcval));
		break;
	case R_FRV_FUNCDESC:
		if ((long)reloc_addr_packed & 3)
			reloc_value = reloc_addr_packed->v;
		else
			reloc_value = *reloc_addr;
		if (symbol_addr)
			reloc_value = (unsigned long)_dl_funcdesc_for
				((char *)symbol_addr + reloc_value,
				 symbol_tpnt->loadaddr.got_value);
		else
			reloc_value = 0;
		if ((long)reloc_addr_packed & 3)
			reloc_addr_packed->v = reloc_value;
		else
			*reloc_addr = reloc_value;
		break;
	default:
		return -1; /*call _dl_exit(1) */
	}
#if defined (__SUPPORT_LD_DEBUG__)
	if (_dl_debug_reloc && _dl_debug_detail) {
		_dl_dprintf(_dl_debug_file, "\tpatched: %x ==> %x @ %x", old_val, reloc_value, reloc_addr);
		switch (reloc_type) {
		case R_FRV_FUNCDESC_VALUE:
			_dl_dprintf(_dl_debug_file, " got %x", ((struct funcdesc_value *)reloc_value)->got_value);
			break;
		case R_FRV_FUNCDESC:
			if (! reloc_value)
				break;
			_dl_dprintf(_dl_debug_file, " funcdesc (%x,%x)",
				    ((struct funcdesc_value *)reloc_value)->entry_point,
				    ((struct funcdesc_value *)reloc_value)->got_value);
			break;
		}
	}
#endif

	return 0;
}

static int
_dl_do_lazy_reloc (struct elf_resolve *tpnt,
		   struct r_scope_elem *scope __attribute__((unused)),
		   ELF_RELOC *rpnt, ElfW(Sym) *symtab __attribute__((unused)),
		   char *strtab __attribute__((unused)))
{
	int reloc_type;
	struct funcdesc_value volatile *reloc_addr;
	struct funcdesc_value funcval;
#if defined (__SUPPORT_LD_DEBUG__)
	unsigned long old_val;
#endif

	reloc_addr = (struct funcdesc_value *)(intptr_t)
	  DL_RELOC_ADDR (tpnt->loadaddr, rpnt->r_offset);
	reloc_type = ELF_R_TYPE(rpnt->r_info);

#if defined (__SUPPORT_LD_DEBUG__)
	old_val = (unsigned long)reloc_addr->entry_point;
#endif
		switch (reloc_type) {
			case R_FRV_NONE:
				break;
			case R_FRV_FUNCDESC_VALUE:
				funcval = *reloc_addr;
				funcval.entry_point = (void *) DL_RELOC_ADDR(tpnt->loadaddr, funcval.entry_point);
				funcval.got_value = tpnt->loadaddr.got_value;
				*reloc_addr = funcval;
				break;
			default:
				return -1; /*call _dl_exit(1) */
		}
#if defined (__SUPPORT_LD_DEBUG__)
	if (_dl_debug_reloc && _dl_debug_detail)
		_dl_dprintf(_dl_debug_file, "\tpatched: %x ==> %x @ %x\n", old_val, reloc_addr->entry_point, reloc_addr);
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
(struct dyn_elf *rpnt, struct r_scope_elem *scope, unsigned long rel_addr, unsigned long rel_size)
{
  return _dl_parse(rpnt->dyn, scope, rel_addr, rel_size, _dl_do_reloc);
}

/* We don't have copy relocs.  */

int
_dl_parse_copy_information
(struct dyn_elf *rpnt __attribute__((unused)),
 unsigned long rel_addr __attribute__((unused)),
 unsigned long rel_size __attribute__((unused)))
{
  return 0;
}

#ifndef IS_IN_libdl
# include "../../libc/sysdeps/linux/frv/crtreloc.c"
#endif

