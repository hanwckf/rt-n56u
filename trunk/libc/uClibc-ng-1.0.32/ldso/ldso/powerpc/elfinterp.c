/* powerpc shared library loader suppport
 *
 * Copyright (C) 2001-2002 David A. Schleef
 * Copyright (C) 2003-2004 Erik Andersen
 * Copyright (C) 2004 Joakim Tjernlund
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
#define TLS_DTV_OFFSET 0x8000
#define TLS_TP_OFFSET 0x7000

extern int _dl_linux_resolve(void);

void _dl_init_got(unsigned long *plt,struct elf_resolve *tpnt)
{
	Elf32_Word *tramp;
	Elf32_Word num_plt_entries;
	Elf32_Word data_words;
	Elf32_Word rel_offset_words;
	Elf32_Word dlrr = (Elf32_Word) _dl_linux_resolve;

	if (tpnt->dynamic_info[DT_JMPREL] == 0)
		return;
	if (tpnt->dynamic_info[DT_PPC_GOT_IDX] != 0) {
		tpnt->dynamic_info[DT_PPC_GOT_IDX] += tpnt->loadaddr;
		return;
	}
	num_plt_entries = tpnt->dynamic_info[DT_PLTRELSZ] / sizeof(ELF_RELOC);
	rel_offset_words = PLT_DATA_START_WORDS(num_plt_entries);
	data_words = (Elf32_Word) (plt + rel_offset_words);
	tpnt->data_words = data_words;

	plt[PLT_LONGBRANCH_ENTRY_WORDS] = OPCODE_ADDIS_HI(11, 11, data_words);
	plt[PLT_LONGBRANCH_ENTRY_WORDS+1] = OPCODE_LWZ(11,data_words,11);

	plt[PLT_LONGBRANCH_ENTRY_WORDS+2] = OPCODE_MTCTR(11);
	plt[PLT_LONGBRANCH_ENTRY_WORDS+3] = OPCODE_BCTR();

	/* [4] */
	/* [5] */
	tramp = (Elf32_Word *) (plt + PLT_TRAMPOLINE_ENTRY_WORDS);

	/* For the long entries, subtract off data_words.  */
	tramp[0] = OPCODE_ADDIS_HI(11,11,-data_words);
	tramp[1] = OPCODE_ADDI(11,11,-data_words);

	/* Multiply index of entry by 3 (in r11).  */
	tramp[2] = OPCODE_SLWI(12,11,1);
	tramp[3] = OPCODE_ADD(11,12,11);
	if (dlrr <= 0x01fffffc || dlrr >= 0xfe000000) {
		/* Load address of link map in r12.  */
		tramp[4] = OPCODE_LI (12, (Elf32_Word) tpnt);
		tramp[5] = OPCODE_ADDIS_HI (12, 12, (Elf32_Word) tpnt);

		/* Call _dl_linux_resolve .  */
		tramp[6] = OPCODE_BA (dlrr);
	} else {
		/* Get address of _dl_linux_resolve in CTR.  */
		tramp[4] = OPCODE_LI(12,dlrr);
		tramp[5] = OPCODE_ADDIS_HI(12,12,dlrr);
		tramp[6] = OPCODE_MTCTR(12);

		/* Load address of link map in r12.  */
		tramp[7] = OPCODE_LI(12,(Elf32_Word) tpnt);
		tramp[8] = OPCODE_ADDIS_HI(12,12,(Elf32_Word) tpnt);

		/* Call _dl_linux_resolve.  */
		tramp[9] = OPCODE_BCTR();
	}
	/* [16] unused */
	/* [17] unused */

	PPC_DCBST(plt);
	PPC_DCBST(plt+4);
	PPC_DCBST(plt+8);
	PPC_DCBST(plt+12);
	PPC_DCBST(plt+16-1);
	PPC_SYNC;
	PPC_ICBI(plt);
	PPC_ICBI(plt+16-1);
	PPC_ISYNC;
}

unsigned long _dl_linux_resolver(struct elf_resolve *tpnt, int reloc_entry)
{
	ELF_RELOC *this_reloc;
	char *strtab;
	ElfW(Sym) *symtab;
	ELF_RELOC *rel_addr;
	int symtab_index;
	char *symname;
	ElfW(Addr) *reloc_addr;
	ElfW(Addr)  finaladdr;
	Elf32_Sword delta;

	rel_addr = (ELF_RELOC *)tpnt->dynamic_info[DT_JMPREL];

	this_reloc = (void *)rel_addr + reloc_entry;
	symtab_index = ELF_R_SYM(this_reloc->r_info);

	symtab = (ElfW(Sym) *)tpnt->dynamic_info[DT_SYMTAB];
	strtab = (char *)tpnt->dynamic_info[DT_STRTAB];
	symname = strtab + symtab[symtab_index].st_name;

	debug_sym(symtab,strtab,symtab_index);
	debug_reloc(symtab,strtab,this_reloc);

	/* Address of dump instruction to fix up */
	reloc_addr = (ElfW(Addr) *) (tpnt->loadaddr + this_reloc->r_offset);

#if defined (__SUPPORT_LD_DEBUG__)
	if (_dl_debug_reloc && _dl_debug_detail)
		_dl_dprintf(_dl_debug_file, "\n\tResolving symbol %s %x --> ", symname, (ElfW(Addr))reloc_addr);
#endif

	/* Get the address of the GOT entry */
	finaladdr = (ElfW(Addr)) _dl_find_hash(symname,
			&_dl_loaded_modules->symbol_scope, tpnt, ELF_RTYPE_CLASS_PLT, NULL);
	if (unlikely(!finaladdr)) {
		_dl_dprintf(2, "%s: can't resolve symbol '%s' in lib '%s'.\n", _dl_progname, symname, tpnt->libname);
		_dl_exit(1);
	}
	finaladdr += this_reloc->r_addend;
#if defined (__SUPPORT_LD_DEBUG__)
	if (_dl_debug_reloc && _dl_debug_detail)
		_dl_dprintf(_dl_debug_file, "%x\n", finaladdr);
#endif
	if (tpnt->dynamic_info[DT_PPC_GOT_IDX] != 0) {
		*reloc_addr = finaladdr;
	} else {
		delta = finaladdr - (Elf32_Word)reloc_addr;
		if (delta<<6>>6 == delta) {
			*reloc_addr = OPCODE_B(delta);
		} else if (finaladdr <= 0x01fffffc) {
			*reloc_addr = OPCODE_BA (finaladdr);
		} else {
			/* Warning: we don't handle double-sized PLT entries */
			Elf32_Word *plt, *data_words, idx, offset;

			plt = (Elf32_Word *)tpnt->dynamic_info[DT_PLTGOT];
			offset = reloc_addr - plt;
			idx = (offset - PLT_INITIAL_ENTRY_WORDS)/2;
			data_words = (Elf32_Word *)tpnt->data_words;
			reloc_addr += 1;

			data_words[idx] = finaladdr;
			PPC_SYNC;
			*reloc_addr =  OPCODE_B ((PLT_LONGBRANCH_ENTRY_WORDS - (offset+1)) * 4);
		}

		/* instructions were modified */
		PPC_DCBST(reloc_addr);
		PPC_SYNC;
		PPC_ICBI(reloc_addr);
		PPC_ISYNC;
	}
	return finaladdr;
}

static __inline__ int
_dl_do_reloc (struct elf_resolve *tpnt,struct r_scope_elem *scope,
	      ELF_RELOC *rpnt, ElfW(Sym) *symtab, char *strtab)
{
	int reloc_type;
	int symtab_index;
	struct symbol_ref sym_ref;
	ElfW(Addr) *reloc_addr;
	ElfW(Addr) finaladdr;
	struct elf_resolve *tls_tpnt = NULL;
	unsigned long symbol_addr;
	char *symname;
#if defined (__SUPPORT_LD_DEBUG__)
	unsigned long old_val;
#endif

	symbol_addr  = tpnt->loadaddr; /* For R_PPC_RELATIVE */
	reloc_addr   = (ElfW(Addr) *)(intptr_t) (symbol_addr + (unsigned long) rpnt->r_offset);
	reloc_type   = ELF_R_TYPE(rpnt->r_info);
	symtab_index = ELF_R_SYM(rpnt->r_info);
	sym_ref.sym  = &symtab[symtab_index];
	sym_ref.tpnt = NULL;
	symname      = strtab + sym_ref.sym->st_name;
	if (symtab_index) {
		symbol_addr = (unsigned long) _dl_find_hash(symname, scope, tpnt,
							    elf_machine_type_class(reloc_type),  &sym_ref);
		/* We want to allow undefined references to weak symbols - this might
		 * have been intentional.  We should not be linking local symbols
		 * here, so all bases should be covered.
		 */
		if (unlikely(!symbol_addr
			&& (ELF_ST_TYPE(sym_ref.sym->st_info) != STT_TLS
				&& ELF_ST_BIND(sym_ref.sym->st_info) != STB_WEAK)))
			return 1;
		if (_dl_trace_prelink) {
			_dl_debug_lookup (symname, tpnt, &symtab[symtab_index],
						&sym_ref, elf_machine_type_class(reloc_type));
		}
		tls_tpnt = sym_ref.tpnt;
	} else {
		symbol_addr = sym_ref.sym->st_value;
		tls_tpnt = tpnt;
	}
#if defined (__SUPPORT_LD_DEBUG__)
	old_val = *reloc_addr;
#endif
	finaladdr = (ElfW(Addr)) (symbol_addr + rpnt->r_addend);

	switch (reloc_type) {
	case R_PPC_RELATIVE:
	case R_PPC_ADDR32:
	case R_PPC_GLOB_DAT:
		*reloc_addr = finaladdr;
		goto out_nocode; /* No code modified */
	case R_PPC_JMP_SLOT:
	{
		if (tpnt->dynamic_info[DT_PPC_GOT_IDX] != 0) {
			*reloc_addr = finaladdr;
			goto out_nocode; /* No code modified */
		} else {
			Elf32_Sword delta = finaladdr - (Elf32_Word)reloc_addr;
			if (delta<<6>>6 == delta) {
				*reloc_addr = OPCODE_B(delta);
			} else if (finaladdr <= 0x01fffffc) {
				*reloc_addr = OPCODE_BA (finaladdr);
			} else {
				/* Warning: we don't handle double-sized PLT entries */
				Elf32_Word *plt, *data_words, idx, offset;

				plt = (Elf32_Word *)tpnt->dynamic_info[DT_PLTGOT];
				offset = reloc_addr - plt;
				idx = (offset - PLT_INITIAL_ENTRY_WORDS)/2;
				data_words = (Elf32_Word *)tpnt->data_words;

				data_words[idx] = finaladdr;
				reloc_addr[0] = OPCODE_LI(11,idx*4);
				reloc_addr[1] = OPCODE_B((PLT_LONGBRANCH_ENTRY_WORDS - (offset+1)) * 4);

				/* instructions were modified */
				PPC_DCBST(reloc_addr+1);
				PPC_SYNC;
				PPC_ICBI(reloc_addr+1);
			}
		}
		break;
	}
	case R_PPC_COPY:
#if defined (__SUPPORT_LD_DEBUG__)
		if (_dl_debug_move)
			_dl_dprintf(_dl_debug_file,"\n%s move %x bytes from %x to %x",
				    symname, sym_ref.sym->st_size,
				    symbol_addr, reloc_addr);
#endif
		_dl_memcpy((char *) reloc_addr, (char *) finaladdr, sym_ref.sym->st_size);
		goto out_nocode; /* No code modified */
	case R_PPC_ADDR16_HA:
		finaladdr += 0x8000; /* fall through. */
	case R_PPC_ADDR16_HI:
		finaladdr >>= 16; /* fall through. */
	case R_PPC_ADDR16_LO:
		*(short *)reloc_addr = finaladdr;
		break;
#if USE_TLS
	case R_PPC_DTPMOD32:
		*reloc_addr = tls_tpnt->l_tls_modid;
		break;
	case R_PPC_DTPREL32:
		/* During relocation all TLS symbols are defined and used.
		   Therefore the offset is already correct.  */
		*reloc_addr = finaladdr - TLS_DTV_OFFSET;
		break;
	case R_PPC_TPREL32:
		*reloc_addr = tls_tpnt->l_tls_offset + finaladdr - TLS_TP_OFFSET;
		break;
#endif
	case R_PPC_REL24:
#if 0
		{
			Elf32_Sword delta = finaladdr - (Elf32_Word)reloc_addr;
			if (unlikely(delta<<6>>6 != delta)) {
				_dl_dprintf(2, "%s: symbol '%s' R_PPC_REL24 is out of range.\n\t"
						"Compile shared libraries with -fPIC!\n",
						_dl_progname, symname);
				_dl_exit(1);
			}
			*reloc_addr = (*reloc_addr & 0xfc000003) | (delta & 0x3fffffc);
			break;
		}
#else
		_dl_dprintf(2,"R_PPC_REL24: Compile shared libraries with -fPIC!\n");
		return -1;
#endif
	case R_PPC_NONE:
		goto out_nocode; /* No code modified */
	default:
		_dl_dprintf(2, "%s: can't handle reloc type ", _dl_progname);
		if (symtab_index)
			_dl_dprintf(2, "'%s'\n", symname);
		return -1;
	}

	/* instructions were modified */
	PPC_DCBST(reloc_addr);
	PPC_SYNC;
	PPC_ICBI(reloc_addr);
	PPC_ISYNC;
 out_nocode:
#if defined (__SUPPORT_LD_DEBUG__)
	if (_dl_debug_reloc && _dl_debug_detail)
		_dl_dprintf(_dl_debug_file, "\tpatched: %x ==> %x @ %x\n", old_val, *reloc_addr, reloc_addr);
#endif
	return 0;
}

void _dl_parse_lazy_relocation_information(struct dyn_elf *rpnt,
	unsigned long rel_addr, unsigned long rel_size)
{
	struct elf_resolve *tpnt = rpnt->dyn;
	Elf32_Word *plt, offset, i,  num_plt_entries, rel_offset_words;

	num_plt_entries = rel_size / sizeof(ELF_RELOC);
	plt = (Elf32_Word *)tpnt->dynamic_info[DT_PLTGOT];
	if (tpnt->dynamic_info[DT_PPC_GOT_IDX] != 0) {
		/* Secure PLT */
		ElfW(Addr) *got = (ElfW(Addr) *)tpnt->dynamic_info[DT_PPC_GOT_IDX];
		Elf32_Word dlrr = (Elf32_Word) _dl_linux_resolve;

		got[1] = (ElfW(Addr)) dlrr;
		got[2] = (ElfW(Addr)) tpnt;

		/* Relocate everything in .plt by the load address offset.  */
		while (num_plt_entries-- != 0)
			*plt++ += tpnt->loadaddr;
		return;
	}

	rel_offset_words = PLT_DATA_START_WORDS(num_plt_entries);

	/* Set up the lazy PLT entries.  */
	offset = PLT_INITIAL_ENTRY_WORDS;
	i = 0;
	/* Warning: we don't handle double-sized PLT entries */
	while (i < num_plt_entries) {
		plt[offset  ] = OPCODE_LI(11, i * 4);
		plt[offset+1] = OPCODE_B((PLT_TRAMPOLINE_ENTRY_WORDS + 2 - (offset+1)) * 4);
		i++;
		offset += 2;
	}
	/* Now, we've modified code.  We need to write the changes from
	   the data cache to a second-level unified cache, then make
	   sure that stale data in the instruction cache is removed.
	   (In a multiprocessor system, the effect is more complex.)
	   Most of the PLT shouldn't be in the instruction cache, but
	   there may be a little overlap at the start and the end.

	   Assumes that dcbst and icbi apply to lines of 16 bytes or
	   more.  Current known line sizes are 16, 32, and 128 bytes.  */
	for (i = 0; i < rel_offset_words; i += 4)
		PPC_DCBST (plt + i);
	PPC_DCBST (plt + rel_offset_words - 1);
	PPC_SYNC;
	PPC_ICBI (plt);
	PPC_ICBI (plt + rel_offset_words - 1);
	PPC_ISYNC;
}

static __inline__ int
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
	rpnt = (ELF_RELOC *)(intptr_t)rel_addr;
	rel_size = rel_size / sizeof(ELF_RELOC);

	symtab = (ElfW(Sym) *)(intptr_t)tpnt->dynamic_info[DT_SYMTAB];
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

		if (unlikely(res <0))
		{
		        int reloc_type = ELF_R_TYPE(rpnt->r_info);
			_dl_dprintf(2, "can't handle reloc type %x in lib '%s'\n", reloc_type, tpnt->libname);
			return res;
		}
		if (unlikely(res >0))
		{
			_dl_dprintf(2, "can't resolve symbol in lib '%s'.\n", tpnt->libname);
			return res;
		}
	  }
	  return 0;
}

int _dl_parse_relocation_information(struct dyn_elf *rpnt,
	struct r_scope_elem *scope, unsigned long rel_addr, unsigned long rel_size)
{
	return _dl_parse(rpnt->dyn, scope, rel_addr, rel_size, _dl_do_reloc);
}
