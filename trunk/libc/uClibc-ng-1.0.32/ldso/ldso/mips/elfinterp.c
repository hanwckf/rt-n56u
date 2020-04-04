/* mips/mipsel ELF shared library loader suppport
 *
   Copyright (C) 2002, Steven J. Hill (sjhill@realitydiluted.com)
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

#include "ldso.h"

extern int _dl_runtime_resolve(void);
extern int _dl_runtime_pltresolve(void);

#define OFFSET_GP_GOT 0x7ff0

unsigned long __dl_runtime_resolve(unsigned long sym_index,
	unsigned long old_gpreg)
{
	unsigned long *got = (unsigned long *) (old_gpreg - OFFSET_GP_GOT);
	struct elf_resolve *tpnt = (struct elf_resolve *) got[1];
	ElfW(Sym) *sym;
	char *strtab;
	unsigned long local_gotno;
	unsigned long gotsym;
	unsigned long new_addr;
	unsigned long instr_addr;
	char **got_addr;
	char *symname;

	gotsym = tpnt->dynamic_info[DT_MIPS_GOTSYM_IDX];
	local_gotno = tpnt->dynamic_info[DT_MIPS_LOCAL_GOTNO_IDX];

	sym = ((ElfW(Sym) *) tpnt->dynamic_info[DT_SYMTAB]) + sym_index;
	strtab = (char *) tpnt->dynamic_info[DT_STRTAB];
	symname = strtab + sym->st_name;

	new_addr = (unsigned long) _dl_find_hash(symname,
			&_dl_loaded_modules->symbol_scope, tpnt, ELF_RTYPE_CLASS_PLT, NULL);
	if (unlikely(!new_addr)) {
		_dl_dprintf (2, "%s: can't resolve symbol '%s'\n",
				_dl_progname, symname);
		_dl_exit (1);
	}

	/* Address of jump instruction to fix up */
	instr_addr = (unsigned long) (got + local_gotno + sym_index - gotsym);
	got_addr = (char **) instr_addr;

#if defined (__SUPPORT_LD_DEBUG__)
	if (_dl_debug_bindings)
	{
		_dl_dprintf(_dl_debug_file, "\nresolve function: %s", symname);
		if (_dl_debug_detail) _dl_dprintf(_dl_debug_file,
				"\n\tpatched %x ==> %x @ %x\n", *got_addr, new_addr, got_addr);
	}
	if (!_dl_debug_nofixups) {
		*got_addr = (char*)new_addr;
	}
#else
	*got_addr = (char*)new_addr;
#endif

	return new_addr;
}

unsigned long
__dl_runtime_pltresolve(struct elf_resolve *tpnt, int reloc_entry)
{
	ELF_RELOC *this_reloc;
	char *strtab;
	ElfW(Sym) *symtab;
	int symtab_index;
	char *rel_addr;
	char *new_addr;
	char **got_addr;
	unsigned long instr_addr;
	char *symname;

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
	new_addr = _dl_find_hash(symname, &_dl_loaded_modules->symbol_scope, tpnt, ELF_RTYPE_CLASS_PLT, NULL);
	if (unlikely(!new_addr)) {
		_dl_dprintf(2, "%s: can't resolve symbol '%s' in lib '%s'.\n", _dl_progname, symname, tpnt->libname);
		_dl_exit(1);
	}

#if defined (__SUPPORT_LD_DEBUG__)
	if ((unsigned long)got_addr < 0x40000000) {
		if (_dl_debug_bindings) {
			_dl_dprintf(_dl_debug_file, "\nresolve function: %s", symname);
			if (_dl_debug_detail)
				_dl_dprintf(_dl_debug_file,
				            "\n\tpatched: %x ==> %x @ %x",
				            *got_addr, new_addr, got_addr);
		}
	}
	if (!_dl_debug_nofixups) {
		*got_addr = new_addr;
	}
#else
	*got_addr = new_addr;
#endif

	return (unsigned long)new_addr;
}

void _dl_parse_lazy_relocation_information(struct dyn_elf *rpnt,
	unsigned long rel_addr, unsigned long rel_size)
{
	/* Nothing to do */
	return;
}

int _dl_parse_relocation_information(struct dyn_elf *xpnt,
	struct r_scope_elem *scope, unsigned long rel_addr, unsigned long rel_size)
{
	ElfW(Sym) *symtab;
	ELF_RELOC *rpnt;
	char *strtab;
	unsigned long i;
	unsigned long *got;
	unsigned long *reloc_addr=NULL;
	unsigned long symbol_addr;
	int reloc_type, symtab_index;
	struct elf_resolve *tpnt = xpnt->dyn;
	char *symname = NULL;
#if defined (__SUPPORT_LD_DEBUG__)
	unsigned long old_val=0;
#endif

	struct symbol_ref sym_ref;
	/* Now parse the relocation information */
	rel_size = rel_size / sizeof(ElfW(Rel));
	rpnt = (ELF_RELOC *) rel_addr;

	symtab = (ElfW(Sym) *) tpnt->dynamic_info[DT_SYMTAB];
	strtab = (char *) tpnt->dynamic_info[DT_STRTAB];
	got = (unsigned long *) tpnt->dynamic_info[DT_PLTGOT];


	for (i = 0; i < rel_size; i++, rpnt++) {
		reloc_addr = (unsigned long *) (tpnt->loadaddr +
			(unsigned long) rpnt->r_offset);
		reloc_type = ELF_R_TYPE(rpnt->r_info);
		symtab_index = ELF_R_SYM(rpnt->r_info);
		symbol_addr = 0;

		debug_sym(symtab,strtab,symtab_index);
		debug_reloc(symtab,strtab,rpnt);
		symname = strtab + symtab[symtab_index].st_name;
#if defined (__SUPPORT_LD_DEBUG__)
		if (reloc_addr)
			old_val = *reloc_addr;
#endif

		if (reloc_type == R_MIPS_JUMP_SLOT || reloc_type == R_MIPS_COPY) {
			sym_ref.tpnt = NULL;
			sym_ref.sym = &symtab[symtab_index];
			symbol_addr = (unsigned long)_dl_find_hash(symname,
								   scope,
								   tpnt,
								   elf_machine_type_class(reloc_type), &sym_ref);
			if (unlikely(!symbol_addr && ELF_ST_BIND(symtab[symtab_index].st_info) != STB_WEAK))
				return 1;
			if (_dl_trace_prelink) {
				_dl_debug_lookup (symname, tpnt, &symtab[symtab_index],
							&sym_ref, elf_machine_type_class(reloc_type));
			}
		}
		if (!symtab_index) {
			/* Relocs against STN_UNDEF are usually treated as using a
			* symbol value of zero, and using the module containing the
			* reloc itself.
			*/
			symbol_addr = symtab[symtab_index].st_value;
		}

		switch (reloc_type) {
#if defined USE_TLS && USE_TLS
# if _MIPS_SIM == _MIPS_SIM_ABI64
		case R_MIPS_TLS_DTPMOD64:
		case R_MIPS_TLS_DTPREL64:
		case R_MIPS_TLS_TPREL64:
# else
		case R_MIPS_TLS_DTPMOD32:
		case R_MIPS_TLS_DTPREL32:
		case R_MIPS_TLS_TPREL32:
# endif
			{
				struct elf_resolve *tls_tpnt = NULL;
				sym_ref.sym =  &symtab[symtab_index];
				sym_ref.tpnt =  NULL;

				if (ELF_ST_BIND(symtab[symtab_index].st_info) != STB_LOCAL) {
					symbol_addr = (unsigned long) _dl_find_hash(symname, scope,
						tpnt, elf_machine_type_class(reloc_type), &sym_ref);
					tls_tpnt = sym_ref.tpnt;
				}
			    /* In case of a TLS reloc, tls_tpnt NULL means we have an 'anonymous'
			       symbol.  This is the case for a static tls variable, so the lookup
			       module is just that one is referencing the tls variable. */
			    if (!tls_tpnt)
			        tls_tpnt = tpnt;

				switch (reloc_type) {
					case R_MIPS_TLS_DTPMOD64:
					case R_MIPS_TLS_DTPMOD32:
						if (tls_tpnt)
							*(ElfW(Addr) *)reloc_addr = tls_tpnt->l_tls_modid;
						break;

					case R_MIPS_TLS_DTPREL64:
					case R_MIPS_TLS_DTPREL32:
						*(ElfW(Addr) *)reloc_addr +=
							TLS_DTPREL_VALUE (symbol_addr);
						break;

					case R_MIPS_TLS_TPREL32:
					case R_MIPS_TLS_TPREL64:
						CHECK_STATIC_TLS((struct link_map *)tls_tpnt);
						*(ElfW(Addr) *)reloc_addr +=
							TLS_TPREL_VALUE (tls_tpnt, symbol_addr);
						break;
				}

				break;
			}
#endif /* USE_TLS */
#if _MIPS_SIM == _MIPS_SIM_ABI64
		case (R_MIPS_64 << 8) | R_MIPS_REL32:
#else	/* O32 || N32 */
		case R_MIPS_REL32:
#endif	/* O32 || N32 */
			if (symtab_index) {
				if (symtab_index < tpnt->dynamic_info[DT_MIPS_GOTSYM_IDX])
					*reloc_addr +=
						symtab[symtab_index].st_value +
						(unsigned long) tpnt->loadaddr;
				else {
					*reloc_addr += got[symtab_index + tpnt->dynamic_info[DT_MIPS_LOCAL_GOTNO_IDX] -
						tpnt->dynamic_info[DT_MIPS_GOTSYM_IDX]];
				}
			}
			else {
				*reloc_addr += (unsigned long) tpnt->loadaddr;
			}
			break;
		case R_MIPS_JUMP_SLOT:
			*reloc_addr = symbol_addr;
			break;
		case R_MIPS_COPY:
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
			break;
		case R_MIPS_NONE:
			break;
		default:
			{
				_dl_dprintf(2, "\n%s: ",_dl_progname);

				if (symtab_index)
					_dl_dprintf(2, "symbol '%s': ", symname);

				_dl_dprintf(2, "can't handle reloc type %x in lib '%s'\n", reloc_type, tpnt->libname);
				_dl_exit(1);
			}
		}
#if defined (__SUPPORT_LD_DEBUG__)
		if (_dl_debug_reloc && _dl_debug_detail && reloc_addr)
			_dl_dprintf(_dl_debug_file, "\tpatched: %x ==> %x @ %x\n", old_val, *reloc_addr, reloc_addr);
#endif
	}

	return 0;
}

/* Relocate the global GOT entries for the object */
void _dl_perform_mips_global_got_relocations(struct elf_resolve *tpnt, int lazy)
{
	ElfW(Sym) *sym;
	char *strtab;
	unsigned long i, tmp_lazy;
	unsigned long *got_entry;

	for (; tpnt ; tpnt = tpnt->next) {

		/* We don't touch the dynamic linker */
		if (tpnt->libtype == program_interpreter)
			continue;

		/* Setup the loop variables */
		got_entry = (unsigned long *) (tpnt->dynamic_info[DT_PLTGOT])
			+ tpnt->dynamic_info[DT_MIPS_LOCAL_GOTNO_IDX];
		sym = (ElfW(Sym) *) tpnt->dynamic_info[DT_SYMTAB] + tpnt->dynamic_info[DT_MIPS_GOTSYM_IDX];
		strtab = (char *) tpnt->dynamic_info[DT_STRTAB];
		i = tpnt->dynamic_info[DT_MIPS_SYMTABNO_IDX] - tpnt->dynamic_info[DT_MIPS_GOTSYM_IDX];

#if defined (__SUPPORT_LD_DEBUG__)
		if (_dl_debug_reloc)
			_dl_dprintf(2, "_dl_perform_mips_global_got_relocations for '%s'\n", tpnt->libname);
#endif
		tmp_lazy = lazy && !tpnt->dynamic_info[DT_BIND_NOW];
		/* Relocate the global GOT entries for the object */
		while (i--) {
			if (sym->st_shndx == SHN_UNDEF) {
				if (ELF_ST_TYPE(sym->st_info) == STT_FUNC && sym->st_value && tmp_lazy) {
					*got_entry = sym->st_value + (unsigned long) tpnt->loadaddr;
				}
				else {
					*got_entry = (unsigned long) _dl_find_hash(strtab +
						sym->st_name, &_dl_loaded_modules->symbol_scope, tpnt, ELF_RTYPE_CLASS_PLT, NULL);
				}
			}
			else if (sym->st_shndx == SHN_COMMON) {
				*got_entry = (unsigned long) _dl_find_hash(strtab +
					sym->st_name, &_dl_loaded_modules->symbol_scope, tpnt, ELF_RTYPE_CLASS_PLT, NULL);
			}
			else if (ELF_ST_TYPE(sym->st_info) == STT_FUNC &&
				*got_entry != sym->st_value && tmp_lazy) {
				*got_entry += (unsigned long) tpnt->loadaddr;
			}
			else if (ELF_ST_TYPE(sym->st_info) == STT_SECTION) {
				if (sym->st_other == 0)
					*got_entry += (unsigned long) tpnt->loadaddr;
			}
			else {
				struct symbol_ref sym_ref;
				sym_ref.sym = sym;
				sym_ref.tpnt = NULL;
				*got_entry = (unsigned long) _dl_find_hash(strtab +
					sym->st_name, &_dl_loaded_modules->symbol_scope, tpnt, ELF_RTYPE_CLASS_PLT, &sym_ref);
			}

			got_entry++;
			sym++;
		}
	}
}

