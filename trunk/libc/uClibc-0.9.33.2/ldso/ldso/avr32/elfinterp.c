/*
 * AVR32 ELF shared library loader suppport
 *
 * Copyright (C) 2004-2006 Atmel Corporation
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

unsigned long _dl_linux_resolver(unsigned long got_offset, unsigned long *got)
{
	/*
	 * AVR32 currently does not do lazy relocation.
	 */
#if 0
	struct elf_resolve *tpnt = (struct elf_resolve *)got[1];
	ElfW(Sym) *sym;
	unsigned long local_gotno;
	unsigned long gotsym;
	unsigned long new_addr;
	char *strtab, *symname;
	unsigned long *entry;
	unsigned long sym_index = got_offset / 4;

	local_gotno = tpnt->dynamic_info[DT_AVR32_LOCAL_GOTNO];
	gotsym = tpnt->dynamic_info[DT_AVR32_GOTSYM];

	sym = ((ElfW(Sym) *)(tpnt->dynamic_info[DT_SYMTAB] + tpnt->loadaddr))
		+ sym_index;
	strtab = (char *)(tpnt->dynamic_info[DT_STRTAB] + tpnt->loadaddr);
	symname = strtab + sym->st_name;

	new_addr = (unsigned long) _dl_find_hash(symname,
						 &_dl_loaded_modules->symbol_scope, tpnt,
						 ELF_RTYPE_CLASS_PLT, NULL);

	entry = (unsigned long *)(got + local_gotno + sym_index - gotsym);
	*entry = new_addr;

	return new_addr;
#endif
	return 0;
}

static int
_dl_parse(struct elf_resolve *tpnt, struct r_scope_elem *scope,
	  unsigned long rel_addr, unsigned long rel_size,
	  int (*reloc_func)(struct elf_resolve *tpnt, struct r_scope_elem *scope,
			    ElfW(Rela) *rpnt, ElfW(Sym) *symtab, char *strtab))
{
	ElfW(Sym) *symtab;
	ElfW(Rela) *rpnt;
	char *strtab;
	int i;

	rpnt = (ElfW(Rela) *)rel_addr;
	rel_size /= sizeof(ElfW(Rela));
	symtab = (ElfW(Sym) *)tpnt->dynamic_info[DT_SYMTAB];
	strtab = (char *)tpnt->dynamic_info[DT_STRTAB];

	for (i = 0; i < rel_size; i++, rpnt++) {
		int symtab_index, res;

		symtab_index = ELF_R_SYM(rpnt->r_info);

		debug_sym(symtab, strtab, symtab_index);
		debug_reloc(symtab, strtab, rpnt);

		res = reloc_func(tpnt, scope, rpnt, symtab, strtab);

		if (res == 0)
			continue;

		_dl_dprintf(2, "\n%s: ", _dl_progname);

		if (symtab_index)
			_dl_dprintf(2, "symbol '%s': ",
				    strtab + symtab[symtab_index].st_name);

		if (res < 0) {
			int reloc_type = ELF_R_TYPE(rpnt->r_info);
#if defined(__SUPPORT_LD_DEBUG__)
			_dl_dprintf(2, "can't handle reloc type %s\n",
				    _dl_reltypes(reloc_type));
#else
			_dl_dprintf(2, "can't handle reloc type %x\n",
				    reloc_type);
#endif
			_dl_exit(-res);
		} else {
			_dl_dprintf(2, "can't resolve symbol\n");
			return res;
		}
	}

	return 0;
}

static int _dl_do_reloc(struct elf_resolve *tpnt, struct r_scope_elem *scope,
			ElfW(Rela) *rpnt, ElfW(Sym) *symtab, char *strtab)
{
	int reloc_type;
	int symtab_index;
	char *symname;
	unsigned long *reloc_addr;
	unsigned long symbol_addr;
#if defined(__SUPPORT_LD_DEBUG__)
	unsigned long old_val;
#endif
	struct symbol_ref sym_ref;

	reloc_addr = (unsigned long *)(tpnt->loadaddr + rpnt->r_offset);
	reloc_type = ELF_R_TYPE(rpnt->r_info);
	symtab_index = ELF_R_SYM(rpnt->r_info);
	symbol_addr = 0;
	sym_ref.sym = &symtab[symtab_index];
	sym_ref.tpnt = NULL;
	symname = strtab + symtab[symtab_index].st_name;

	if (symtab_index) {
		symbol_addr = (unsigned long)
			_dl_find_hash(symname, scope, tpnt,
				      elf_machine_type_class(reloc_type), &sym_ref);

		/* Allow undefined references to weak symbols */
		if (!symbol_addr &&
		    ELF_ST_BIND(symtab[symtab_index].st_info) != STB_WEAK) {
			_dl_dprintf(2, "%s: can't resolve symbol '%s'\n",
				    _dl_progname, symname);
			return 0;
		}
		if (_dl_trace_prelink) {
			_dl_debug_lookup (symname, tpnt, &symtab[symtab_index],
				&sym_ref, elf_machine_type_class(reloc_type));
		}
	}

#if defined(__SUPPORT_LD_DEBUG__)
	old_val = *reloc_addr;
#endif
	switch (reloc_type) {
	case R_AVR32_NONE:
		break;
	case R_AVR32_GLOB_DAT:
	case R_AVR32_JMP_SLOT:
		*reloc_addr = symbol_addr + rpnt->r_addend;
		break;
	case R_AVR32_RELATIVE:
		*reloc_addr = (unsigned long)tpnt->loadaddr
			+ rpnt->r_addend;
		break;
	default:
		return -1;
	}

#if defined(__SUPPORT_LD_DEBUG__)
	if (_dl_debug_reloc && _dl_debug_detail)
		_dl_dprintf(_dl_debug_file, "\tpatched: %x ==> %x @ %x\n",
			    old_val, *reloc_addr);
#endif

	return 0;
}

void _dl_parse_lazy_relocation_information(struct dyn_elf *rpnt,
					   unsigned long rel_addr,
					   unsigned long rel_size)
{
	/* TODO: Might want to support this in order to get faster
	 * startup times... */
}

int _dl_parse_relocation_information(struct dyn_elf *rpnt,
				     struct r_scope_elem *scope,
				     unsigned long rel_addr,
				     unsigned long rel_size)
{
	return _dl_parse(rpnt->dyn, scope, rel_addr, rel_size,
			 _dl_do_reloc);
}
