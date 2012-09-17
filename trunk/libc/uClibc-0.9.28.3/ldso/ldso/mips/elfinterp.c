/* vi: set sw=4 ts=4: */
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

#define OFFSET_GP_GOT 0x7ff0

unsigned long __dl_runtime_resolve(unsigned long sym_index,
	unsigned long old_gpreg)
{
	unsigned long *got = (unsigned long *) (old_gpreg - OFFSET_GP_GOT);
	struct elf_resolve *tpnt = (struct elf_resolve *) got[1];
	Elf32_Sym *sym;
	char *strtab;
	unsigned long local_gotno;
	unsigned long gotsym;
	unsigned long new_addr;
	unsigned long instr_addr;
	char **got_addr;
	char *symname;

	gotsym = tpnt->dynamic_info[DT_MIPS_GOTSYM_IDX];
	local_gotno = tpnt->dynamic_info[DT_MIPS_LOCAL_GOTNO_IDX];

	sym = ((Elf32_Sym *) tpnt->dynamic_info[DT_SYMTAB]) + sym_index;
	strtab = (char *) tpnt->dynamic_info[DT_STRTAB];
	symname = strtab + sym->st_name;

	new_addr = (unsigned long) _dl_find_hash(symname,
			tpnt->symbol_scope, tpnt, ELF_RTYPE_CLASS_PLT);
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
		if(_dl_debug_detail) _dl_dprintf(_dl_debug_file,
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

void _dl_parse_lazy_relocation_information(struct dyn_elf *rpnt,
	unsigned long rel_addr, unsigned long rel_size)
{
	/* Nothing to do */
	return;
}

int _dl_parse_relocation_information(struct dyn_elf *xpnt,
	unsigned long rel_addr, unsigned long rel_size)
{
	Elf32_Sym *symtab;
	Elf32_Rel *rpnt;
	char *strtab;
	unsigned long i;
	unsigned long *got;
	unsigned long *reloc_addr=NULL;
	int reloc_type, symtab_index;
	struct elf_resolve *tpnt = xpnt->dyn;
#if defined (__SUPPORT_LD_DEBUG__)
	unsigned long old_val=0;
#endif

	/* Now parse the relocation information */
	rel_size = rel_size / sizeof(Elf32_Rel);
	rpnt = (Elf32_Rel *) rel_addr;

	symtab = (Elf32_Sym *) tpnt->dynamic_info[DT_SYMTAB];
	strtab = (char *) tpnt->dynamic_info[DT_STRTAB];
	got = (unsigned long *) tpnt->dynamic_info[DT_PLTGOT];

	for (i = 0; i < rel_size; i++, rpnt++) {
		reloc_addr = (unsigned long *) (tpnt->loadaddr +
			(unsigned long) rpnt->r_offset);
		reloc_type = ELF_R_TYPE(rpnt->r_info);
		symtab_index = ELF_R_SYM(rpnt->r_info);

		debug_sym(symtab,strtab,symtab_index);
		debug_reloc(symtab,strtab,rpnt);
#if defined (__SUPPORT_LD_DEBUG__)
		if (reloc_addr)
			old_val = *reloc_addr;
#endif

		switch (reloc_type) {
		case R_MIPS_REL32:
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
		case R_MIPS_NONE:
			break;
		default:
			{
				_dl_dprintf(2, "\n%s: ",_dl_progname);

				if (symtab_index)
					_dl_dprintf(2, "symbol '%s': ", strtab + symtab[symtab_index].st_name);

#if defined (__SUPPORT_LD_DEBUG__)
				_dl_dprintf(2, "can't handle reloc type %s\n ", _dl_reltypes(reloc_type));
#else
				_dl_dprintf(2, "can't handle reloc type %x\n", reloc_type);
#endif
				_dl_exit(1);
			}
		};

	};
#if defined (__SUPPORT_LD_DEBUG__)
	if(_dl_debug_reloc && _dl_debug_detail && reloc_addr)
		_dl_dprintf(_dl_debug_file, "\tpatched: %x ==> %x @ %x\n", old_val, *reloc_addr, reloc_addr);
#endif

	return 0;
}

/* Relocate the global GOT entries for the object */
void _dl_perform_mips_global_got_relocations(struct elf_resolve *tpnt, int lazy)
{
	Elf32_Sym *sym;
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
		sym = (Elf32_Sym *) tpnt->dynamic_info[DT_SYMTAB] + tpnt->dynamic_info[DT_MIPS_GOTSYM_IDX];
		strtab = (char *) tpnt->dynamic_info[DT_STRTAB];
		i = tpnt->dynamic_info[DT_MIPS_SYMTABNO_IDX] - tpnt->dynamic_info[DT_MIPS_GOTSYM_IDX];

#if defined (__SUPPORT_LD_DEBUG__)
		if(_dl_debug_reloc)
			_dl_dprintf(2, "_dl_perform_mips_global_got_relocations for '%s'\n", tpnt->libname);
#endif
		tmp_lazy = lazy && !tpnt->dynamic_info[DT_BIND_NOW];
		/* Relocate the global GOT entries for the object */
		while(i--) {
			if (sym->st_shndx == SHN_UNDEF) {
				if (ELF32_ST_TYPE(sym->st_info) == STT_FUNC && sym->st_value && tmp_lazy) {
					*got_entry = sym->st_value + (unsigned long) tpnt->loadaddr;
				}
				else {
					*got_entry = (unsigned long) _dl_find_hash(strtab +
						sym->st_name, tpnt->symbol_scope, tpnt, ELF_RTYPE_CLASS_PLT);
				}
			}
			else if (sym->st_shndx == SHN_COMMON) {
				*got_entry = (unsigned long) _dl_find_hash(strtab +
					sym->st_name, tpnt->symbol_scope, tpnt, ELF_RTYPE_CLASS_PLT);
			}
			else if (ELF32_ST_TYPE(sym->st_info) == STT_FUNC &&
				*got_entry != sym->st_value && tmp_lazy) {
				*got_entry += (unsigned long) tpnt->loadaddr;
			}
			else if (ELF32_ST_TYPE(sym->st_info) == STT_SECTION) {
				if (sym->st_other == 0)
					*got_entry += (unsigned long) tpnt->loadaddr;
			}
			else {
				*got_entry = (unsigned long) _dl_find_hash(strtab +
					sym->st_name, tpnt->symbol_scope, tpnt, ELF_RTYPE_CLASS_PLT);
			}

			got_entry++;
			sym++;
		}
	}
}

