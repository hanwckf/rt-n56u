/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Lots of code copied from ../i386/elfinterp.c, so:
 * Copyright (c) 1994-2000 Eric Youngdale, Peter MacDonald,
 *               David Engel, Hongjiu Lu and Mitch D'Souza
 * Copyright (C) 2001-2002, Erik Andersen
 * All rights reserved.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */
#include "ldso.h"

#ifdef __A7__
#define ARC_PLT_SIZE	12
#else
#define ARC_PLT_SIZE	16
#endif

unsigned long
_dl_linux_resolver(struct elf_resolve *tpnt, unsigned int plt_pc)
{
	ELF_RELOC *this_reloc, *rel_base;
	char *strtab, *symname, *new_addr;
	ElfW(Sym) *symtab;
	int symtab_index;
	unsigned int *got_addr;
	unsigned long plt_base;
	int plt_idx;

	/* start of .rela.plt */
	rel_base = (ELF_RELOC *)(tpnt->dynamic_info[DT_JMPREL]);

	/* starts of .plt (addr of PLT0) */
	plt_base = tpnt->dynamic_info[DT_PLTGOT];

	/*
	 * compute the idx of the yet-unresolved PLT entry in .plt
	 * Same idx will be used to find the relo entry in .rela.plt
	 */
	plt_idx = (plt_pc - plt_base)/ARC_PLT_SIZE  - 2; /* ignoring 2 dummy PLTs */

	this_reloc = rel_base + plt_idx;

	symtab_index = ELF_R_SYM(this_reloc->r_info);
	symtab = (ElfW(Sym) *)(intptr_t) (tpnt->dynamic_info[DT_SYMTAB]);
	strtab = (char *) (tpnt->dynamic_info[DT_STRTAB]);
	symname= strtab + symtab[symtab_index].st_name;

	/* relo-offset to fixup, shd be a .got entry */
	got_addr = (unsigned int *)(this_reloc->r_offset + tpnt->loadaddr);

	/* Get the address of the GOT entry */
	new_addr = _dl_find_hash(symname, &_dl_loaded_modules->symbol_scope, tpnt,
				 ELF_RTYPE_CLASS_PLT, NULL);

	if (unlikely(!new_addr)) {
		_dl_dprintf(2, "%s: can't resolve symbol '%s'\n", _dl_progname, symname);
		_dl_exit(1);
	}


#if defined __SUPPORT_LD_DEBUG__
	if (_dl_debug_bindings) {
		_dl_dprintf(_dl_debug_file, "\nresolve function: %s", symname);
		if (_dl_debug_detail)
			_dl_dprintf(_dl_debug_file, "\n\tpatched %x ==> %pc @ %p\n",
					*got_addr, new_addr, got_addr);
	}

	if (!_dl_debug_nofixups)
		*got_addr = (unsigned int)new_addr;
#else
	/* Update the .got entry with the runtime address of symbol */
	*got_addr = (unsigned int)new_addr;
#endif

	/*
	 * Return the new addres, where the asm trampoline will jump to
	 *  after re-setting up the orig args
	 */
	return (unsigned long) new_addr;
}


static int
_dl_do_reloc(struct elf_resolve *tpnt, struct r_scope_elem *scope,
	     ELF_RELOC *rpnt, ElfW(Sym) *symtab, char *strtab)
{
	int reloc_type;
	int symtab_index;
	char *symname;
	unsigned long *reloc_addr;
	unsigned long symbol_addr;
#if defined __SUPPORT_LD_DEBUG__
	unsigned long old_val = 0;
#endif
	struct symbol_ref sym_ref;
	struct elf_resolve *tls_tpnt = NULL;

	reloc_addr   = (unsigned long *)(tpnt->loadaddr + rpnt->r_offset);
	reloc_type   = ELF_R_TYPE(rpnt->r_info);
	symtab_index = ELF_R_SYM(rpnt->r_info);
	symbol_addr  = 0;

	sym_ref.sym = &symtab[symtab_index];
	sym_ref.tpnt = NULL;

#if defined __SUPPORT_LD_DEBUG__
	if (reloc_addr)
		old_val = *reloc_addr;
#endif

	if (symtab_index) {
		symname = strtab + symtab[symtab_index].st_name;
		symbol_addr = (unsigned long) _dl_find_hash(symname, scope, tpnt,
				elf_machine_type_class(reloc_type), &sym_ref);

		/*
		 * We want to allow undefined references to weak symbols,
		 * this might have been intentional. We should not be linking
		 * local symbols here, so all bases should be covered.
		 */

		if (unlikely(!symbol_addr
		    && ELF_ST_BIND(symtab[symtab_index].st_info) != STB_WEAK
		    && ELF_ST_TYPE(symtab[symtab_index].st_info) != STT_TLS)) {
			/* Non-fatal if called from dlopen, hence different ret code */
			return 1;
		}

		tls_tpnt = sym_ref.tpnt;
	} else if (reloc_type == R_ARC_RELATIVE ) {
		*reloc_addr += tpnt->loadaddr;
		goto log_entry;
	}

#if defined USE_TLS && USE_TLS
	/* In case of a TLS reloc, tls_tpnt NULL means we have an 'anonymous'
	   symbol.  This is the case for a static tls variable, so the lookup
	   module is just that one is referencing the tls variable. */
	if (!tls_tpnt)
		tls_tpnt = tpnt;
#endif

	switch (reloc_type) {
	case R_ARC_NONE:
		break;
	case R_ARC_32:
		*reloc_addr += symbol_addr + rpnt->r_addend;
		break;
	case R_ARC_PC32:
		*reloc_addr += symbol_addr + rpnt->r_addend - (unsigned long) reloc_addr;
		break;
	case R_ARC_GLOB_DAT:
	case R_ARC_JMP_SLOT:
		*reloc_addr = symbol_addr;
		break;
	case R_ARC_COPY:
		_dl_memcpy((void *) reloc_addr,(void *) symbol_addr,
				symtab[symtab_index].st_size);
		break;
#if defined USE_TLS && USE_TLS
	case R_ARC_TLS_DTPMOD:
		*reloc_addr = tls_tpnt->l_tls_modid;
		break;
	case R_ARC_TLS_DTPOFF:
		*reloc_addr += symbol_addr;
		break;
	case R_ARC_TLS_TPOFF:
		CHECK_STATIC_TLS ((struct link_map *) tls_tpnt);
		*reloc_addr = tls_tpnt->l_tls_offset + symbol_addr + rpnt->r_addend;
		break;
#endif
	default:
		return -1;
	}

log_entry:
#if defined __SUPPORT_LD_DEBUG__
	if (_dl_debug_detail && (reloc_type != R_ARC_NONE))
		_dl_dprintf(_dl_debug_file,"\tpatched: %x ==> %x @ %x",
				old_val, *reloc_addr, reloc_addr);
#endif

	return 0;
}

static int
_dl_do_lazy_reloc(struct elf_resolve *tpnt, struct r_scope_elem *scope,
		  ELF_RELOC *rpnt)
{
	int reloc_type;
	unsigned long *reloc_addr;
#if defined __SUPPORT_LD_DEBUG__
	unsigned long old_val;
#endif

	reloc_addr = (unsigned long *)(tpnt->loadaddr + rpnt->r_offset);
	reloc_type = ELF_R_TYPE(rpnt->r_info);

#if defined __SUPPORT_LD_DEBUG__
	old_val = *reloc_addr;
#endif

	switch (reloc_type) {
	case R_ARC_NONE:
		break;
	case R_ARC_JMP_SLOT:
		*reloc_addr += tpnt->loadaddr;
		break;
	default:
		return -1;
	}

#if defined __SUPPORT_LD_DEBUG__
	if (_dl_debug_reloc && _dl_debug_detail && (reloc_type != R_ARC_NONE))
		_dl_dprintf(_dl_debug_file, "\tpatched: %x ==> %x @ %x\n",
				old_val, *reloc_addr, reloc_addr);
#endif

	return 0;
}

#define ___DO_LAZY  1
#define ___DO_NOW   2

static int _dl_parse(struct elf_resolve *tpnt, struct r_scope_elem *scope,
		     unsigned long rel_addr, unsigned long rel_size, int type)
{
	unsigned int i;
	char *strtab;
	ElfW(Sym) *symtab;
	ELF_RELOC *rpnt;
	int symtab_index;
	int res = 0;

	/* Now parse the relocation information */
	rpnt = (ELF_RELOC *)(intptr_t) (rel_addr);
	rel_size = rel_size / sizeof(ELF_RELOC);

	symtab = (ElfW(Sym) *)(intptr_t) (tpnt->dynamic_info[DT_SYMTAB]);
	strtab = (char *) (tpnt->dynamic_info[DT_STRTAB]);

	for (i = 0; i < rel_size; i++, rpnt++) {

		symtab_index = ELF_R_SYM(rpnt->r_info);

		debug_sym(symtab,strtab,symtab_index);
		debug_reloc(symtab,strtab,rpnt);

		/* constant propagation subsumes the 'if' */
		if (type == ___DO_LAZY)
			res = _dl_do_lazy_reloc(tpnt, scope, rpnt);
		else
			res = _dl_do_reloc(tpnt, scope, rpnt, symtab, strtab);

		if (res != 0)
			break;
	}

	if (unlikely(res != 0)) {
		if (res < 0) {
			int reloc_type = ELF_R_TYPE(rpnt->r_info);
			_dl_dprintf(2, "can't handle reloc type %x\n",
				    reloc_type);
			_dl_exit(-res);
		} else {
			_dl_dprintf(2, "can't resolve symbol\n");
			/* Fall thru to return res */
		}
	}

	return res;
}

void
_dl_parse_lazy_relocation_information(struct dyn_elf *rpnt,
				      unsigned long rel_addr,
				      unsigned long rel_size)
{
	/* This func is called for processing .rela.plt of loaded module(s)
	 * The relo entries handled are JMP_SLOT type for fixing up .got slots
	 * for external function calls.
	 * This function doesn't resolve the slots: that is done lazily at
	 * runtime. The build linker (at least thats what happens for ARC) had
	 * pre-init the .got slots to point to PLT0. All that is done here is
	 * to fix them up to point to load value of PLT0 (as opposed to the
	 * build value).
	 * On ARC, the loadaddr of dyn exec is zero, thus elfaddr == loadaddr
	 * Thus there is no point in adding "0" to values and un-necessarily
	 * stir up the caches and TLB.
	 * For ldso processing busybox binary, this skips over 380 relo entries
	 */
	if (rpnt->dyn->loadaddr != 0)
		_dl_parse(rpnt->dyn, NULL, rel_addr, rel_size, ___DO_LAZY);
}

int
_dl_parse_relocation_information(struct dyn_elf  *rpnt,
				 struct r_scope_elem *scope,
				 unsigned long rel_addr,
				 unsigned long rel_size)
{
	return _dl_parse(rpnt->dyn, scope, rel_addr, rel_size, ___DO_NOW);
}
