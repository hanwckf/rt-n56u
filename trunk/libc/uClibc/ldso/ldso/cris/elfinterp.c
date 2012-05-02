/*
 * CRIS ELF shared library loader support.
 *
 * Program to load an elf binary on a linux system, and run it.
 * References to symbols in sharable libraries can be resolved
 * by either an ELF sharable library or a linux style of shared
 * library.
 *
 * Copyright (C) 2002-2004, Axis Communications AB
 * All rights reserved
 *
 * Author: Tobias Anderberg, <tobiasa@axis.com>
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

#include "ldso.h"

/* Defined in resolve.S. */
extern int _dl_linux_resolve(void);

unsigned long
_dl_linux_resolver(struct elf_resolve *tpnt, int reloc_entry)
{
	int reloc_type;
	int symtab_index;
	char *strtab;
	char *symname;
	char *new_addr;
	char *rel_addr;
	char **got_addr;
	Elf32_Sym *symtab;
	ELF_RELOC *this_reloc;
	unsigned long instr_addr;

	rel_addr = (char *)tpnt->dynamic_info[DT_JMPREL];

	this_reloc = (ELF_RELOC *)(intptr_t)(rel_addr + reloc_entry);
	reloc_type = ELF32_R_TYPE(this_reloc->r_info);
	symtab_index = ELF32_R_SYM(this_reloc->r_info);

	symtab = (Elf32_Sym *)(intptr_t)tpnt->dynamic_info[DT_SYMTAB];
	strtab = (char *)tpnt->dynamic_info[DT_STRTAB];
	symname = strtab + symtab[symtab_index].st_name;

	if (unlikely(reloc_type != R_CRIS_JUMP_SLOT)) {
		_dl_dprintf(2, "%s: Incorrect relocation type in jump relocations\n",
			    _dl_progname);
		_dl_exit(1);
	}

	/* Address of the jump instruction to fix up. */
	instr_addr = ((unsigned long)this_reloc->r_offset +
		      (unsigned long)tpnt->loadaddr);
	got_addr = (char **)instr_addr;

	/* Get the address of the GOT entry. */
	new_addr = _dl_find_hash(symname, tpnt->symbol_scope, tpnt, ELF_RTYPE_CLASS_PLT);
	if (unlikely(!new_addr)) {
		_dl_dprintf(2, "%s: Can't resolve symbol '%s'\n", _dl_progname, symname);
		_dl_exit(1);
	}

#if defined (__SUPPORT_LD_DEBUG__)
	if (_dl_debug_bindings) {
		_dl_dprintf(_dl_debug_file, "\nresolve function: %s", symname);
		if (_dl_debug_detail)
			_dl_dprintf(_dl_debug_file,
				    "\n\tpatched: %x ==> %x @ %x",
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
_dl_parse(struct elf_resolve *tpnt, struct dyn_elf *scope,
	  unsigned long rel_addr, unsigned long rel_size,
	  int (*reloc_fnc)(struct elf_resolve *tpnt, struct dyn_elf *scope,
			   ELF_RELOC *rpnt, Elf32_Sym *symtab, char *strtab))
{
	int symtab_index;
	unsigned int i;
	char *strtab;
	Elf32_Sym *symtab;
	ELF_RELOC *rpnt;

	/* Parse the relocation information. */
	rpnt = (ELF_RELOC *)(intptr_t)rel_addr;
	rel_size /= sizeof(ELF_RELOC);

	symtab = (Elf32_Sym *)(intptr_t)tpnt->dynamic_info[DT_SYMTAB];
	strtab = (char *)tpnt->dynamic_info[DT_STRTAB];

	for (i = 0; i < rel_size; i++, rpnt++) {
		int res;

		symtab_index = ELF32_R_SYM(rpnt->r_info);

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
			int reloc_type = ELF32_R_TYPE(rpnt->r_info);

#if defined (__SUPPORT_LD_DEBUG__)
			_dl_dprintf(2, "can't handle reloc type %s\n",
				    _dl_reltypes(reloc_type));
#else
			_dl_dprintf(2, "can't handle reloc type %x\n",
				    reloc_type);
#endif
			_dl_exit(-res);
		} else if (unlikely(res > 0)) {
			_dl_dprintf(2, "can't resolve symbol\n");
			return res;
		}
	}

	return 0;
}

static int
_dl_do_reloc(struct elf_resolve *tpnt, struct dyn_elf *scope,
	     ELF_RELOC *rpnt, Elf32_Sym *symtab, char *strtab)
{
	int reloc_type;
	int symtab_index;
	char *symname;
	unsigned long *reloc_addr;
	unsigned long symbol_addr;
#if defined (__SUPPORT_LD_DEBUG__)
	unsigned long old_val;
#endif

	reloc_addr = (unsigned long *)(intptr_t)(tpnt->loadaddr + (unsigned long)rpnt->r_offset);
	reloc_type = ELF32_R_TYPE(rpnt->r_info);
	symtab_index = ELF32_R_SYM(rpnt->r_info);
	symbol_addr = 0;
	symname = strtab + symtab[symtab_index].st_name;

	if (symtab_index) {
		if (symtab[symtab_index].st_shndx != SHN_UNDEF &&
		    ELF32_ST_BIND(symtab[symtab_index].st_info) == STB_LOCAL) {
			symbol_addr = (unsigned long)tpnt->loadaddr;
		} else {
		  symbol_addr = (unsigned long)_dl_find_hash(symname, scope, tpnt,
								   elf_machine_type_class(reloc_type));
		}

		if (unlikely(!symbol_addr && ELF32_ST_BIND(symtab[symtab_index].st_info) != STB_WEAK)) {
			_dl_dprintf(2, "%s: can't resolve symbol '%s'\n", _dl_progname, symname);
			_dl_exit(1);
		};

		symbol_addr += rpnt->r_addend;
	}

#if defined (__SUPPORT_LD_DEBUG__)
	old_val = *reloc_addr;
#endif

	switch (reloc_type) {
		case R_CRIS_NONE:
			break;
		case R_CRIS_GLOB_DAT:
		case R_CRIS_JUMP_SLOT:
		case R_CRIS_32:
			*reloc_addr = symbol_addr;
			break;
		case R_CRIS_COPY:
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
			break;
		case R_CRIS_RELATIVE:
			*reloc_addr = (unsigned long)tpnt->loadaddr + rpnt->r_addend;
			break;
		default:
			return -1;	/* Calls _dl_exit(1). */
	}

#if defined (__SUPPORT_LD_DEBUG__)
	if (_dl_debug_reloc && _dl_debug_detail)
		_dl_dprintf(_dl_debug_file, "\n\tpatched: %x ==> %x @ %x",
			    old_val, *reloc_addr, reloc_addr);
#endif

	return 0;
}

static int
_dl_do_lazy_reloc(struct elf_resolve *tpnt, struct dyn_elf *scope,
		  ELF_RELOC *rpnt, Elf32_Sym *symtab, char *strtab)
{
	int reloc_type;
	unsigned long *reloc_addr;
#if defined (__SUPPORT_LD_DEBUG__)
	unsigned long old_val;
#endif

	/* Don't care about these, just keep the compiler happy. */
	(void)scope;
	(void)symtab;
	(void)strtab;

	reloc_addr = (unsigned long *)(intptr_t)(tpnt->loadaddr + (unsigned long)rpnt->r_offset);
	reloc_type = ELF32_R_TYPE(rpnt->r_info);

#if defined (__SUPPORT_LD_DEBUG__)
	old_val = *reloc_addr;
#endif

	switch (reloc_type) {
		case R_CRIS_NONE:
			break;
		case R_CRIS_JUMP_SLOT:
			*reloc_addr += (unsigned long)tpnt->loadaddr;
			break;
		default:
			return -1;	/* Calls _dl_exit(1). */
	}

#if defined (__SUPPORT_LD_DEBUG__)
	if (_dl_debug_reloc && _dl_debug_detail)
		_dl_dprintf(_dl_debug_file, "\n\tpatched: %x ==> %x @ %x",
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
	(void)_dl_parse(rpnt->dyn, NULL, rel_addr, rel_size, _dl_do_lazy_reloc);
}

int
_dl_parse_relocation_information(struct dyn_elf *rpnt,
				 unsigned long rel_addr,
				 unsigned long rel_size)
{
	return _dl_parse(rpnt->dyn, rpnt->dyn->symbol_scope, rel_addr, rel_size, _dl_do_reloc);
}
