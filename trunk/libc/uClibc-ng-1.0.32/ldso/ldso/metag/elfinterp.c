/*
 * Meta ELF shared library loader support.
 *
 * Program to load an elf binary on a linux system, and run it.
 * References to symbols in sharable libraries can be resolved
 * by either an ELF sharable library or a linux style of shared
 * library.
 *
 * Copyright (C) 2013, Imagination Technologies Ltd.
 *
 * Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include "ldso.h"

/* Defined in resolve.S. */
extern int _dl_linux_resolve(void);

static inline unsigned long __get_unaligned_reloc(unsigned long *addr)
{
	char *rel_addr = (char *)addr;
	unsigned long val;

	val = *rel_addr++ & 0xff;
	val |= (*rel_addr++ << 8) & 0x0000ff00;
	val |= (*rel_addr++ << 16) & 0x00ff0000;
	val |= (*rel_addr++ << 24) & 0xff000000;

	return val;
}

static inline void __put_unaligned_reloc(unsigned long *addr,
					 unsigned long val)
{
	char *rel_addr = (char *)addr;

	*rel_addr++ = (val & 0x000000ff);
	*rel_addr++ = ((val & 0x0000ff00) >> 8);
	*rel_addr++ = ((val & 0x00ff0000) >> 16);
	*rel_addr++ = ((val & 0xff000000) >> 24);
}

unsigned long
_dl_linux_resolver(struct elf_resolve *tpnt, int reloc_entry)
{
	int symtab_index;
	char *strtab;
	char *symname;
	char *new_addr;
	char *rel_addr;
	char **got_addr;
	ElfW(Sym) *symtab;
	ELF_RELOC *this_reloc;
	unsigned long instr_addr;

	rel_addr = (char *)tpnt->dynamic_info[DT_JMPREL];

	this_reloc = (ELF_RELOC *)(intptr_t)(rel_addr + reloc_entry);
	symtab_index = ELF_R_SYM(this_reloc->r_info);

	symtab = (ElfW(Sym) *)(intptr_t)tpnt->dynamic_info[DT_SYMTAB];
	strtab = (char *)tpnt->dynamic_info[DT_STRTAB];
	symname = strtab + symtab[symtab_index].st_name;

	/* Address of the jump instruction to fix up. */
	instr_addr = ((unsigned long)this_reloc->r_offset +
		      (unsigned long)tpnt->loadaddr);
	got_addr = (char **)instr_addr;

	/* Get the address of the GOT entry. */
	new_addr = _dl_find_hash(symname, &_dl_loaded_modules->symbol_scope, tpnt,
			ELF_RTYPE_CLASS_PLT, NULL);
	if (unlikely(!new_addr)) {
		_dl_dprintf(2, "%s: Can't resolve symbol '%s'\n",
			    _dl_progname, symname);
		_dl_exit(1);
	}

#if defined (__SUPPORT_LD_DEBUG__)
	if (_dl_debug_bindings) {
		_dl_dprintf(_dl_debug_file, "\nresolve function: %s", symname);
		if (_dl_debug_detail)
			_dl_dprintf(_dl_debug_file,
				    "\n\tpatched: %x ==> %x @ %x\n",
				    *got_addr, new_addr, got_addr);
	}
	if (!_dl_debug_nofixups) {
		*got_addr = new_addr;
	}
#else
	*got_addr = new_addr;
#endif

	return (unsigned long)new_addr;
}

static int
_dl_parse(struct elf_resolve *tpnt, struct r_scope_elem *scope,
	  unsigned long rel_addr, unsigned long rel_size,
	  int (*reloc_fnc)(struct elf_resolve *tpnt, struct r_scope_elem *scope,
			   ELF_RELOC *rpnt, ElfW(Sym) *symtab, char *strtab))
{
	int symtab_index;
	unsigned int i;
	char *strtab;
	ElfW(Sym) *symtab;
	ELF_RELOC *rpnt;

	/* Parse the relocation information. */
	rpnt = (ELF_RELOC *)(intptr_t)rel_addr;
	rel_size /= sizeof(ELF_RELOC);

	symtab = (ElfW(Sym) *)(intptr_t)tpnt->dynamic_info[DT_SYMTAB];
	strtab = (char *)tpnt->dynamic_info[DT_STRTAB];

	for (i = 0; i < rel_size; i++, rpnt++) {
		int res;

		symtab_index = ELF_R_SYM(rpnt->r_info);

		debug_sym(symtab, strtab, symtab_index);
		debug_reloc(symtab, strtab, rpnt);

		/* Pass over to actual relocation function. */
		res = reloc_fnc(tpnt, scope, rpnt, symtab, strtab);

		if (res == 0)
			continue;

		_dl_dprintf(2, "\n%s: ", _dl_progname);

		if (symtab_index)
			_dl_dprintf(2, "symbol '%s': ",
				    strtab + symtab[symtab_index].st_name);

		if (unlikely(res < 0)) {
			int reloc_type = ELF_R_TYPE(rpnt->r_info);
			_dl_dprintf(2, "can't handle reloc type %x\n",
				    reloc_type);
			_dl_exit(-res);
		} else if (unlikely(res > 0)) {
			_dl_dprintf(2, "can't resolve symbol\n");
			return res;
		}
	}

	return 0;
}

static int
_dl_do_reloc(struct elf_resolve *tpnt, struct r_scope_elem *scope,
	     ELF_RELOC *rpnt, ElfW(Sym) *symtab, char *strtab)
{
	int reloc_type;
	int symtab_index;
	char *symname = NULL;
	unsigned long *reloc_addr;
	unsigned long symbol_addr;
#if defined (__SUPPORT_LD_DEBUG__)
	unsigned long old_val = 0;
#endif
	struct elf_resolve *tls_tpnt = NULL;
	struct symbol_ref sym_ref;

	reloc_addr = (unsigned long *)(tpnt->loadaddr + rpnt->r_offset);
	reloc_type = ELF_R_TYPE(rpnt->r_info);
	symtab_index = ELF_R_SYM(rpnt->r_info);
	symbol_addr = 0;
	sym_ref.sym = &symtab[symtab_index];
	sym_ref.tpnt = NULL;

	if (symtab_index) {
		symname = strtab + symtab[symtab_index].st_name;
		symbol_addr = (unsigned long)_dl_find_hash(symname, scope, tpnt,
							   elf_machine_type_class(reloc_type), &sym_ref);

		if (!symbol_addr
		    && ELF_ST_TYPE(symtab[symtab_index].st_info) != STT_TLS
		    && ELF_ST_BIND(symtab[symtab_index].st_info) != STB_WEAK) {
			_dl_dprintf(2, "%s: can't resolve symbol '%s'\n",
				    _dl_progname, symname);
			return 1;
		};
		if (_dl_trace_prelink) {
			_dl_debug_lookup(symname, tpnt, &symtab[symtab_index],
				&sym_ref, elf_machine_type_class(reloc_type));
		}
		tls_tpnt = sym_ref.tpnt;
	}

#if defined (__SUPPORT_LD_DEBUG__)
	if (reloc_type != R_METAG_NONE)
		old_val = __get_unaligned_reloc(reloc_addr);
#endif

#if defined USE_TLS && USE_TLS
	/* In case of a TLS reloc, tls_tpnt NULL means we have an 'anonymous'
	   symbol.  This is the case for a static tls variable, so the lookup
	   module is just that one is referencing the tls variable. */
	if (!tls_tpnt)
		tls_tpnt = tpnt;
#endif
	switch (reloc_type) {
		case R_METAG_NONE:
			break;
		case R_METAG_GLOB_DAT:
		case R_METAG_JMP_SLOT:
		case R_METAG_ADDR32:
			__put_unaligned_reloc(reloc_addr,
					      symbol_addr + rpnt->r_addend);
			break;
		case R_METAG_COPY:
#if defined (__SUPPORT_LD_DEBUG__)
			if (_dl_debug_move)
				_dl_dprintf(_dl_debug_file,
					    "\t%s move %d bytes from %x to %x\n",
					    symname, symtab[symtab_index].st_size,
					    symbol_addr + rpnt->r_addend,
					    reloc_addr);
#endif

			_dl_memcpy((char *)reloc_addr,
				   (char *)symbol_addr + rpnt->r_addend,
				   symtab[symtab_index].st_size);
			break;
		case R_METAG_RELATIVE:
			__put_unaligned_reloc(reloc_addr,
					      (unsigned long)tpnt->loadaddr +
					      rpnt->r_addend);
			break;
#if defined USE_TLS && USE_TLS
		case R_METAG_TLS_DTPMOD:
			*reloc_addr = tls_tpnt->l_tls_modid;
			break;
		case R_METAG_TLS_DTPOFF:
			*reloc_addr = symbol_addr;
			break;
		case R_METAG_TLS_TPOFF:
			CHECK_STATIC_TLS ((struct link_map *) tls_tpnt);
			*reloc_addr = tls_tpnt->l_tls_offset + symbol_addr + rpnt->r_addend;
			break;
#endif
		default:
			return -1;	/* Calls _dl_exit(1). */
	}

#if defined (__SUPPORT_LD_DEBUG__)
	if (_dl_debug_reloc && _dl_debug_detail && reloc_type != R_METAG_NONE) {
		unsigned long new_val = __get_unaligned_reloc(reloc_addr);
		_dl_dprintf(_dl_debug_file, "\tpatched: %x ==> %x @ %x\n",
			    old_val, new_val, reloc_addr);
	}
#endif

	return 0;
}

static int
_dl_do_lazy_reloc(struct elf_resolve *tpnt, struct r_scope_elem *scope,
		  ELF_RELOC *rpnt, ElfW(Sym) *symtab, char *strtab)
{
	int reloc_type;
	unsigned long *reloc_addr;
#if defined (__SUPPORT_LD_DEBUG__)
	unsigned long old_val;
#endif

	reloc_addr = (unsigned long *)(tpnt->loadaddr + rpnt->r_offset);
	reloc_type = ELF_R_TYPE(rpnt->r_info);

#if defined (__SUPPORT_LD_DEBUG__)
	old_val = *reloc_addr;
#endif

	switch (reloc_type) {
		case R_METAG_NONE:
			break;
		case R_METAG_JMP_SLOT:
			*reloc_addr += (unsigned long)tpnt->loadaddr;
			break;
		default:
			return -1;	/* Calls _dl_exit(1). */
	}

#if defined (__SUPPORT_LD_DEBUG__)
	if (_dl_debug_reloc && _dl_debug_detail)
		_dl_dprintf(_dl_debug_file, "\tpatched: %x ==> %x @ %x\n",
			    old_val, *reloc_addr, reloc_addr);
#endif

	return 0;
}

/* External interface to the generic part of the dynamic linker. */

void
_dl_parse_lazy_relocation_information(struct dyn_elf *rpnt,
				      unsigned long rel_addr,
				      unsigned long rel_size)
{
	_dl_parse(rpnt->dyn, NULL, rel_addr, rel_size, _dl_do_lazy_reloc);
}

int
_dl_parse_relocation_information(struct dyn_elf *rpnt,
				 struct r_scope_elem *scope,
				 unsigned long rel_addr,
				 unsigned long rel_size)
{
	return _dl_parse(rpnt->dyn, scope, rel_addr,
			 rel_size, _dl_do_reloc);
}
