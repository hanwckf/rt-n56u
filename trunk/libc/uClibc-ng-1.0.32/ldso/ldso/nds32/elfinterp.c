/*
 * Copyright (C) 2016 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* NDS32 ELF shared library loader suppport
 *
 * Copyright (C) 2001-2004 Erik Andersen
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

#if defined(USE_TLS) && USE_TLS
#include "dl-tls.h"
#include "tlsdeschtab.h"
#endif

extern int _dl_linux_resolve(void);

unsigned long _dl_linux_resolver(struct elf_resolve *tpnt, int reloc_entry)
{
	int reloc_type;
	ELF_RELOC *this_reloc;
	char *strtab;
	char *symname;
	Elf32_Sym *symtab;
	ELF_RELOC *rel_addr;
	int symtab_index;
	char *new_addr;
	char **got_addr;
	unsigned long instr_addr;

	rel_addr = (ELF_RELOC *) tpnt->dynamic_info[DT_JMPREL];

	this_reloc = rel_addr + reloc_entry/sizeof(ELF_RELOC);
	reloc_type = ELF32_R_TYPE(this_reloc->r_info);
	symtab_index = ELF32_R_SYM(this_reloc->r_info);

	symtab = (Elf32_Sym *) tpnt->dynamic_info[DT_SYMTAB];
	strtab = (char *) tpnt->dynamic_info[DT_STRTAB];
	symname = strtab + symtab[symtab_index].st_name;

	if (unlikely(reloc_type != R_NDS32_JMP_SLOT)) {
		_dl_dprintf(2, "%s: Incorrect relocation type in jump relocations\n",
			_dl_progname);
		_dl_exit(1);
	}

	/* Address of jump instruction to fix up */
	instr_addr = ((unsigned long) this_reloc->r_offset +
		(unsigned long) tpnt->loadaddr);
	got_addr = (char **) instr_addr;

	/* Get the address of the GOT entry */
	new_addr = _dl_find_hash(symname, &_dl_loaded_modules->symbol_scope, tpnt,
			ELF_RTYPE_CLASS_PLT, NULL);
	if (unlikely(!new_addr)) {
		_dl_dprintf(2, "%s: can't resolve symbol '%s'\n",
			_dl_progname, symname);
		_dl_exit(1);
	}
#if defined (__SUPPORT_LD_DEBUG__)
	if ((unsigned long) got_addr < 0x40000000)
	{
		if (_dl_debug_bindings)
		{
			_dl_dprintf(_dl_debug_file, "\nresolve function: %s", symname);
			if (_dl_debug_detail) _dl_dprintf(_dl_debug_file,
					"\tpatch %x ==> %x @ %x", (unsigned int)*got_addr, (unsigned int)new_addr, (unsigned int)got_addr);
		}
	}
	if (!_dl_debug_nofixups) {
		*got_addr = new_addr;
	}
#else
	*got_addr = new_addr;
#endif

	return (unsigned long) new_addr;
}

static int
_dl_parse(struct elf_resolve *tpnt, struct r_scope_elem *scope,
	  unsigned long rel_addr, unsigned long rel_size,
	  int (*reloc_fnc) (struct elf_resolve *tpnt, struct r_scope_elem *scope,
			    ELF_RELOC *rpnt, ElfW(Sym) *symtab, char *strtab))
{
	int symtab_index;
	int i;
	char *strtab;
	int goof = 0;
	ElfW(Sym) *symtab;
	ELF_RELOC *rpnt;

	/* Now parse the relocation information */
	rpnt = (ELF_RELOC *) rel_addr;
	rel_size = rel_size / sizeof(ELF_RELOC);

	symtab = (Elf32_Sym *) tpnt->dynamic_info[DT_SYMTAB];
	strtab = (char *) tpnt->dynamic_info[DT_STRTAB];

	  for (i = 0; i < rel_size; i++, rpnt++) {
	        int res;

		symtab_index = ELF32_R_SYM(rpnt->r_info);

		debug_sym(symtab,strtab,symtab_index);
		debug_reloc(symtab,strtab,rpnt);

		res = reloc_fnc (tpnt, scope, rpnt, symtab, strtab);

		if (res==0) continue;

		_dl_dprintf(2, "\n%s: ",_dl_progname);

		if (symtab_index)
		  _dl_dprintf(2, "symbol '%s': ", strtab + symtab[symtab_index].st_name);

		if (unlikely(res <0))
		{
		        int reloc_type = ELF32_R_TYPE(rpnt->r_info);
			_dl_dprintf(2, "can't handle reloc type %x\n", reloc_type);
			_dl_exit(-res);
		}
		if (unlikely(res >0))
		{
			_dl_dprintf(2, "can't resolve symbol\n");
			goof += res;
		}
	  }
	  return goof;
}


static int
_dl_do_reloc (struct elf_resolve *tpnt, struct r_scope_elem *scope,
	      ELF_RELOC *rpnt, ElfW(Sym) *symtab, char *strtab)
{
	int reloc_type;
	int symtab_index;
	char *symname = NULL;
#if defined USE_TLS && USE_TLS
	struct elf_resolve *tls_tpnt = NULL;
#endif
	unsigned long *reloc_addr;
	unsigned long symbol_addr;
	int goof = 0;
	struct symbol_ref sym_ref;

	reloc_addr = (unsigned long *) (tpnt->loadaddr + (unsigned long) rpnt->r_offset);
	reloc_type = ELF32_R_TYPE(rpnt->r_info);
	symtab_index = ELF32_R_SYM(rpnt->r_info);
	symbol_addr = 0;
	sym_ref.sym = &symtab[symtab_index];
	sym_ref.tpnt = NULL;

	if (symtab_index) {
		symname = strtab + symtab[symtab_index].st_name;
		symbol_addr = (unsigned long)_dl_find_hash(symname, scope, tpnt,
							   elf_machine_type_class(reloc_type), &sym_ref);

		/*
		 * We want to allow undefined references to weak symbols - this might
		 * have been intentional.  We should not be linking local symbols
		 * here, so all bases should be covered.
		 */
		if (!symbol_addr
		    && (ELF32_ST_TYPE(symtab[symtab_index].st_info) != STT_TLS)
		    && (ELF32_ST_BIND(symtab[symtab_index].st_info) != STB_WEAK)) {
			_dl_dprintf (2, "%s: can't resolve symbol '%s'\n",
				     _dl_progname, symname);
			_dl_exit (1);
		}
		if (_dl_trace_prelink) {
			_dl_debug_lookup(symname, tpnt, &symtab[symtab_index],
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

#define COPY_UNALIGNED_WORD(swp, twp) \
{ \
	__typeof (swp) __tmp = __builtin_nds32_unaligned_load_w ((unsigned int*)&swp); \
	__builtin_nds32_unaligned_store_w ((unsigned int *)twp, __tmp); \
}
#if defined (__SUPPORT_LD_DEBUG__)
	{
		unsigned long old_val = 0;
		if(reloc_type != R_NDS32_NONE)
			old_val = *reloc_addr;
#endif
		symbol_addr += rpnt->r_addend ;
		switch (reloc_type) {
			case R_NDS32_NONE:
				break;
			case R_NDS32_32:
			case R_NDS32_GLOB_DAT:
			case R_NDS32_JMP_SLOT:
				*reloc_addr = symbol_addr;
				break;
			case R_NDS32_32_RELA:
				COPY_UNALIGNED_WORD (symbol_addr, reloc_addr);
				break;
#undef COPY_UNALIGNED_WORD
			case R_NDS32_RELATIVE:
				*reloc_addr = (unsigned long) tpnt->loadaddr + rpnt->r_addend;
				break;
			case R_NDS32_COPY:
				_dl_memcpy((void *) reloc_addr,
					   (void *) symbol_addr, symtab[symtab_index].st_size);
				break;
#if defined USE_TLS && USE_TLS
			case R_NDS32_TLS_TPOFF:
				CHECK_STATIC_TLS ((struct link_map *) tls_tpnt);
				*reloc_addr = (symbol_addr + tls_tpnt->l_tls_offset);
				break;
			case R_NDS32_TLS_DESC:
				{
					struct tlsdesc volatile *td = 
							(struct tlsdesc volatile *)reloc_addr;
#ifndef SHARED
					CHECK_STATIC_TLS((struct link_map *) tls_tpnt);
#else
					if (!TRY_STATIC_TLS ((struct link_map *) tls_tpnt))
					{
					        td->argument.pointer = _dl_make_tlsdesc_dynamic((struct link_map *) tls_tpnt, symbol_addr);
					        td->entry = _dl_tlsdesc_dynamic;
					}
					else
#endif
					{
					        td->argument.value = symbol_addr + tls_tpnt->l_tls_offset;
					        td->entry = _dl_tlsdesc_return;
					}
				}
				break;
#endif
			default:
				return -1; /*call _dl_exit(1) */
		}
#if defined (__SUPPORT_LD_DEBUG__)
		if (_dl_debug_reloc && _dl_debug_detail)
			_dl_dprintf(_dl_debug_file, "\tpatch: %x ==> %x @ %x", (unsigned int)old_val, (unsigned int)*reloc_addr, (unsigned int)reloc_addr);
	}

#endif

	return goof;
}

static int
_dl_do_lazy_reloc (struct elf_resolve *tpnt, struct r_scope_elem *scope,
		   ELF_RELOC *rpnt, ElfW(Sym) *symtab, char *strtab)
{
	int reloc_type;
	unsigned long *reloc_addr;

	reloc_addr = (unsigned long *) (tpnt->loadaddr + (unsigned long) rpnt->r_offset);
	reloc_type = ELF32_R_TYPE(rpnt->r_info);

#if defined (__SUPPORT_LD_DEBUG__)
	{
		unsigned long old_val = *reloc_addr;
#endif
		switch (reloc_type) {
			case R_NDS32_NONE:
				break;
			case R_NDS32_JMP_SLOT:
				*reloc_addr += (unsigned long) tpnt->loadaddr;
				break;
			default:
				return -1; /*call _dl_exit(1) */
		}
#if defined (__SUPPORT_LD_DEBUG__)
		if (_dl_debug_reloc && _dl_debug_detail)
			_dl_dprintf(_dl_debug_file, "\tpatch: %x ==> %x @ %x", (unsigned int)old_val, (unsigned int)*reloc_addr, (unsigned int)reloc_addr);
	}

#endif
	return 0;

}

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

