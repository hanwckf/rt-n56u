/* m68k ELF shared library loader suppport
 *
 * Copyright (c) 1994-2000 Eric Youngdale, Peter MacDonald,
 *                         David Engel, Hongjiu Lu and Mitch D'Souza
 * Adapted to ELF/68k by Andreas Schwab.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. The name of the above contributors may not be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* Program to load an ELF binary on a linux system, and run it.
   References to symbols in sharable libraries can be resolved by either
   an ELF sharable library or a linux style of shared library. */

#include "ldso.h"

extern int _dl_linux_resolve(void);

unsigned long
_dl_linux_resolver(struct elf_resolve *tpnt, int reloc_entry)
{
	ELF_RELOC *this_reloc;
	char *strtab;
	ElfW(Sym) *symtab;
	int symtab_index;
	char *rel_addr;
	char *new_addr;
	char **got_addr;
	ElfW(Addr) instr_addr;
	char *symname;

	rel_addr = (char *)tpnt->dynamic_info[DT_JMPREL];
	this_reloc = (ELF_RELOC *)(rel_addr + reloc_entry);
	symtab_index = ELF_R_SYM(this_reloc->r_info);

	symtab = (ElfW(Sym) *)tpnt->dynamic_info[DT_SYMTAB];
	strtab = (char *)tpnt->dynamic_info[DT_STRTAB];
	symname = strtab + symtab[symtab_index].st_name;

	/* Address of the jump instruction to fix up. */
	instr_addr = (this_reloc->r_offset + tpnt->loadaddr);
	got_addr = (char **)instr_addr;

	/* Get the address of the GOT entry. */
	new_addr = _dl_find_hash(symname, &_dl_loaded_modules->symbol_scope, tpnt, ELF_RTYPE_CLASS_PLT, NULL);
	if (unlikely(!new_addr)) {
		_dl_dprintf(2, "%s: Can't resolve symbol '%s'\n", _dl_progname, symname);
		_dl_exit(1);
	}

#if defined (__SUPPORT_LD_DEBUG__)
	if ((unsigned long)got_addr < 0x40000000) {
		if (_dl_debug_bindings) {
			_dl_dprintf(_dl_debug_file, "\nresolve function: %s", symname);
			if (_dl_debug_detail)
				_dl_dprintf(_dl_debug_file,
				            "\tpatched: %x ==> %x @ %x\n",
				            *got_addr, new_addr, got_addr);
		}
	}
	if (!_dl_debug_nofixups)
#endif
		*got_addr = new_addr;

	return (unsigned int)new_addr;
}

static int
_dl_parse(struct elf_resolve *tpnt, struct r_scope_elem *scope,
	  unsigned long rel_addr, unsigned long rel_size,
	  int (*reloc_fnc)(struct elf_resolve *tpnt, struct r_scope_elem *scope,
			   ELF_RELOC *rpnt, ElfW(Sym) *symtab, char *strtab))
{
	unsigned int i;
	char *strtab;
	ElfW(Sym) *symtab;
	ELF_RELOC *rpnt;
	int symtab_index;

	/* Parse the relocation information. */
	rpnt = (ELF_RELOC *)rel_addr;
	rel_size /= sizeof(ELF_RELOC);

	symtab = (ElfW(Sym) *)tpnt->dynamic_info[DT_SYMTAB];
	strtab = (char *)tpnt->dynamic_info[DT_STRTAB];

	for (i = 0; i < rel_size; i++, rpnt++) {
		int res;

		symtab_index = ELF_R_SYM(rpnt->r_info);

		debug_sym(symtab, strtab, symtab_index);
		debug_reloc(symtab, strtab, rpnt);

		res = reloc_fnc(tpnt, scope, rpnt, symtab, strtab);

		if (res == 0)
			continue;

		_dl_dprintf(2, "\n%s: ", _dl_progname);

		if (symtab_index)
			_dl_dprintf(2, "symbol '%s': ",
				    strtab + symtab[symtab_index].st_name);

		if (unlikely(res < 0)) {
			int reloc_type = ELF_R_TYPE(rpnt->r_info);
			_dl_dprintf(2, "can't handle reloc type "
				    "%x\n", reloc_type);
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
	char *symname;
	struct symbol_ref sym_ref;
	ElfW(Addr) *reloc_addr;
	ElfW(Addr) symbol_addr;
#if defined (__SUPPORT_LD_DEBUG__)
	ElfW(Addr) old_val;
#endif
#if defined USE_TLS && USE_TLS
	struct elf_resolve *tls_tpnt = NULL;
#endif

	reloc_addr = (ElfW(Addr)*)(tpnt->loadaddr + (unsigned long)rpnt->r_offset);
	reloc_type = ELF_R_TYPE(rpnt->r_info);
	symtab_index = ELF_R_SYM(rpnt->r_info);
	sym_ref.sym = &symtab[symtab_index];
	sym_ref.tpnt = NULL;
	symbol_addr = 0;
	symname = strtab + sym_ref.sym->st_name;

	if (symtab_index) {
		symbol_addr = (ElfW(Addr))_dl_find_hash(symname, scope, tpnt,
							    elf_machine_type_class(reloc_type), &sym_ref);
		/*
		 * We want to allow undefined references to weak symbols - this
		 * might have been intentional.  We should not be linking local
		 * symbols here, so all bases should be covered.
		 */
		if (unlikely (!symbol_addr &&
					  ELF_ST_TYPE (sym_ref.sym->st_info) != STT_TLS &&
					  ELF_ST_BIND (sym_ref.sym->st_info) != STB_WEAK)) {
			return 1;
		}
		if (_dl_trace_prelink) {
			_dl_debug_lookup (symname, tpnt, &symtab[symtab_index],
						&sym_ref, elf_machine_type_class(reloc_type));
		}
#if defined USE_TLS && USE_TLS
		tls_tpnt = sym_ref.tpnt;
#endif
	}

#if defined USE_TLS && USE_TLS
	/* In case of a TLS reloc, tls_tpnt NULL means we have an 'anonymous'
	   symbol.  This is the case for a static tls variable, so the lookup
	   module is just that one is referencing the tls variable. */
	if (!tls_tpnt)
		tls_tpnt = tpnt;
#endif

#if defined (__SUPPORT_LD_DEBUG__)
	old_val = *reloc_addr;
#endif

	switch (reloc_type) {
		case R_68K_NONE:
			break;
		case R_68K_8:
			*(char *) reloc_addr = symbol_addr + rpnt->r_addend;
			break;
		case R_68K_16:
			*(short *) reloc_addr = symbol_addr + rpnt->r_addend;
			break;
		case R_68K_32:
			*reloc_addr = symbol_addr + rpnt->r_addend;
			break;
		case R_68K_PC8:
			*(char *) reloc_addr = (symbol_addr + rpnt->r_addend
			                       - (unsigned int) reloc_addr);
			break;
		case R_68K_PC16:
			*(short *) reloc_addr = (symbol_addr + rpnt->r_addend
			                        - (unsigned int) reloc_addr);
			break;
		case R_68K_PC32:
			*reloc_addr = (symbol_addr + rpnt->r_addend
			              - (unsigned int) reloc_addr);
			break;
		case R_68K_GLOB_DAT:
		case R_68K_JMP_SLOT:
			*reloc_addr = symbol_addr + rpnt->r_addend;
			break;
		case R_68K_COPY:
			if (symbol_addr) {
#if defined (__SUPPORT_LD_DEBUG__)
				if (_dl_debug_move)
					_dl_dprintf(_dl_debug_file,
						    "\t%s move %d bytes from %x to %x\n",
						    symname, sym_ref.sym->st_size,
						    symbol_addr, reloc_addr);
#endif
				_dl_memcpy ((void *) reloc_addr,
				            (void *) symbol_addr,
				            sym_ref.sym->st_size);
			}
#if defined (__SUPPORT_LD_DEBUG__)
			else
				_dl_dprintf(_dl_debug_file, "no symbol_addr to copy !?\n");
#endif
			break;
#if defined USE_TLS && USE_TLS
		case R_68K_TLS_DTPMOD32:
			*reloc_addr = tls_tpnt->l_tls_modid;
			break;
		case R_68K_TLS_DTPREL32:
			if (sym_ref.sym != NULL)
			  *reloc_addr = TLS_DTPREL_VALUE (sym_ref.sym, rpnt);
			break;
		case R_68K_TLS_TPREL32:
			if (sym_ref.sym != NULL)
			{
			  CHECK_STATIC_TLS ((struct link_map *) tls_tpnt);
			  *reloc_addr = TLS_TPREL_VALUE ((struct link_map *) tls_tpnt, sym_ref.sym, rpnt);
			}
			break;
#endif
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

#undef LAZY_RELOC_WORKS
#ifdef LAZY_RELOC_WORKS
static int
_dl_do_lazy_reloc(struct elf_resolve *tpnt, struct r_scope_elem *scope,
		  ELF_RELOC *rpnt, ElfW(Sym) *symtab, char *strtab)
{
	int reloc_type;
	int symtab_index;
	ElfW(Addr) *reloc_addr;
#if defined (__SUPPORT_LD_DEBUG__)
	ElfW(Addr) old_val;
#endif

	(void)scope;
	symtab_index = ELF_R_SYM(rpnt->r_info);
	(void)strtab;

	reloc_addr = (ElfW(Addr)*)(tpnt->loadaddr + rpnt->r_offset);
	reloc_type = ELF_R_TYPE(rpnt->r_info);

#if defined (__SUPPORT_LD_DEBUG__)
	old_val = *reloc_addr;
#endif

	switch (reloc_type) {
		case R_68K_NONE:
			break;
		case R_68K_JMP_SLOT:
			*reloc_addr += (unsigned int) tpnt->loadaddr;
			break;
		default:
			_dl_exit(1);
	}

#if defined (__SUPPORT_LD_DEBUG__)
	if (_dl_debug_reloc && _dl_debug_detail)
		_dl_dprintf(_dl_debug_file, "\tpatched_lazy: %x ==> %x @ %x\n",
			    old_val, *reloc_addr, reloc_addr);
#endif

	return 0;
}
#endif

void
_dl_parse_lazy_relocation_information(struct dyn_elf *rpnt,
				      unsigned long rel_addr,
				      unsigned long rel_size)
{
#ifdef LAZY_RELOC_WORKS
	(void)_dl_parse(rpnt->dyn, NULL, rel_addr, rel_size, _dl_do_lazy_reloc);
#else
	_dl_parse_relocation_information(rpnt, &_dl_loaded_modules->symbol_scope, rel_addr, rel_size);
#endif
}

int
_dl_parse_relocation_information(struct dyn_elf *rpnt,
				 struct r_scope_elem *scope,
				 unsigned long rel_addr,
				 unsigned long rel_size)
{
	return _dl_parse(rpnt->dyn, scope, rel_addr, rel_size, _dl_do_reloc);
}
