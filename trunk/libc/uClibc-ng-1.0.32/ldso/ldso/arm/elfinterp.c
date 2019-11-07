/* ARM ELF shared library loader suppport
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

extern int _dl_linux_resolve(void);

#if __FDPIC__
unsigned long _dl_linux_resolver (struct elf_resolve *tpnt, int reloc_offet)
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

	this_reloc = (ELF_RELOC *)(intptr_t)(rel_addr + reloc_offet);
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
			_dl_dprintf(2, "%s: can't resolve symbol '%s'\n", _dl_progname, symname);
			_dl_exit(1);
		}
	}

	funcval.entry_point = new_addr;
	funcval.got_value = sym_ref.tpnt->loadaddr.got_value;

#if defined (__SUPPORT_LD_DEBUG__)
	if (_dl_debug_bindings) {
		_dl_dprintf(_dl_debug_file, "\nresolve function: %s", symname);
		if (_dl_debug_detail)
			_dl_dprintf(_dl_debug_file,
				    "\n\tpatched (%x,%x) ==> (%x,%x) @ %x\n",
				    got_entry->entry_point, got_entry->got_value,
				    funcval.entry_point, funcval.got_value,
				    got_entry);
	}
	if (1 || !_dl_debug_nofixups) {
		*got_entry = funcval;
	}
#else
	*got_entry = funcval;
#endif

	return got_entry;
}
#else
unsigned long _dl_linux_resolver(struct elf_resolve *tpnt, int reloc_entry)
{
	ELF_RELOC *this_reloc;
	char *strtab;
	char *symname;
	ElfW(Sym) *symtab;
	ELF_RELOC *rel_addr;
	int symtab_index;
	unsigned long new_addr;
	char **got_addr;
	unsigned long instr_addr;

	rel_addr = (ELF_RELOC *) tpnt->dynamic_info[DT_JMPREL];

	this_reloc = rel_addr + reloc_entry;
	symtab_index = ELF_R_SYM(this_reloc->r_info);

	symtab = (ElfW(Sym) *) tpnt->dynamic_info[DT_SYMTAB];
	strtab = (char *) tpnt->dynamic_info[DT_STRTAB];
	symname = strtab + symtab[symtab_index].st_name;

	/* Address of jump instruction to fix up */
	instr_addr = ((unsigned long) this_reloc->r_offset +
		(unsigned long) tpnt->loadaddr);
	got_addr = (char **) instr_addr;

	/* Get the address of the GOT entry */
	new_addr = (unsigned long)_dl_find_hash(symname, &_dl_loaded_modules->symbol_scope,
				 tpnt, ELF_RTYPE_CLASS_PLT, NULL);
	if (unlikely(!new_addr)) {
		_dl_dprintf(2, "%s: can't resolve symbol '%s'\n",
			_dl_progname, symname);
		_dl_exit(1);
	}
#if defined (__SUPPORT_LD_DEBUG__)
# if !defined __SUPPORT_LD_DEBUG_EARLY__
	if ((unsigned long) got_addr < 0x40000000)
# endif
	{
		if (_dl_debug_bindings)
		{
			_dl_dprintf(_dl_debug_file, "\nresolve function: %s", symname);
			if (_dl_debug_detail) _dl_dprintf(_dl_debug_file,
					"\tpatch %x ==> %x @ %x", *got_addr, new_addr, got_addr);
		}
	}
	if (!_dl_debug_nofixups) {
		*got_addr = (char *)new_addr;
	}
#else
	*got_addr = (char *)new_addr;
#endif

	return new_addr;
}
#endif

static int
_dl_parse(struct elf_resolve *tpnt, struct r_scope_elem *scope,
	  unsigned long rel_addr, unsigned long rel_size,
	  int (*reloc_fnc) (struct elf_resolve *tpnt, struct r_scope_elem *scope,
			    ELF_RELOC *rpnt, ElfW(Sym) *symtab, char *strtab))
{
	int i;
	char *strtab;
	int goof = 0;
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

		if (unlikely(res <0))
		{
		        int reloc_type = ELF_R_TYPE(rpnt->r_info);
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

#if 0
static unsigned long
fix_bad_pc24 (unsigned long *const reloc_addr, unsigned long value)
{
  static void *fix_page;
  static unsigned int fix_offset;
  unsigned int *fix_address;
  if (! fix_page)
    {
      fix_page = _dl_mmap (NULL,  PAGE_SIZE   , PROT_READ | PROT_WRITE | PROT_EXEC,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      fix_offset = 0;
    }

  fix_address = (unsigned int *)(fix_page + fix_offset);
  fix_address[0] = 0xe51ff004;  /* ldr pc, [pc, #-4] */
  fix_address[1] = value;

  fix_offset += 8;
  if (fix_offset >= PAGE_SIZE)
    fix_page = NULL;

  return (unsigned long)fix_address;
}
#endif

static int
_dl_do_reloc (struct elf_resolve *tpnt,struct r_scope_elem *scope,
	      ELF_RELOC *rpnt, ElfW(Sym) *symtab, char *strtab)
{
	int reloc_type;
	int symtab_index;
	char *symname;
	unsigned long *reloc_addr;
	unsigned long symbol_addr;
	struct symbol_ref sym_ref;
	struct elf_resolve *def_mod = 0;
	int goof = 0;

	reloc_addr = (unsigned long *) DL_RELOC_ADDR(tpnt->loadaddr, rpnt->r_offset);

	reloc_type = ELF_R_TYPE(rpnt->r_info);
	symtab_index = ELF_R_SYM(rpnt->r_info);
	symbol_addr = 0;
	sym_ref.sym = &symtab[symtab_index];
	sym_ref.tpnt = NULL;
	symname = strtab + symtab[symtab_index].st_name;

	if (symtab_index) {
		if (ELF_ST_BIND (symtab[symtab_index].st_info) == STB_LOCAL) {
			symbol_addr = (unsigned long) DL_RELOC_ADDR(tpnt->loadaddr, symtab[symtab_index].st_value);
			def_mod = tpnt;
		} else {
			symbol_addr =  (unsigned long)_dl_find_hash(symname, scope, tpnt,
							elf_machine_type_class(reloc_type), &sym_ref);

			/*
			 * We want to allow undefined references to weak symbols - this might
			 * have been intentional.  We should not be linking local symbols
			 * here, so all bases should be covered.
			 */
			if (!symbol_addr && (ELF_ST_TYPE(symtab[symtab_index].st_info) != STT_TLS)
				&& (ELF_ST_BIND(symtab[symtab_index].st_info) != STB_WEAK)) {
				/* This may be non-fatal if called from dlopen.  */
				return 1;

			}
			if (_dl_trace_prelink) {
				_dl_debug_lookup (symname, tpnt, &symtab[symtab_index],
						&sym_ref, elf_machine_type_class(reloc_type));
			}
			def_mod = sym_ref.tpnt;
		}
	} else {
		/*
		 * Relocs against STN_UNDEF are usually treated as using a
		 * symbol value of zero, and using the module containing the
		 * reloc itself.
		 */
		symbol_addr = symtab[symtab_index].st_value;
		def_mod = tpnt;
	}

#if defined (__SUPPORT_LD_DEBUG__)
	{
		unsigned long old_val;

		if (reloc_type != R_ARM_NONE)
			old_val = *reloc_addr;
#endif
		switch (reloc_type) {
			case R_ARM_NONE:
				break;
			case R_ARM_ABS32:
				*reloc_addr += symbol_addr;
				break;
			case R_ARM_PC24:
#if 0
				{
					unsigned long addend;
					long newvalue, topbits;

					addend = *reloc_addr & 0x00ffffff;
					if (addend & 0x00800000) addend |= 0xff000000;

					newvalue = symbol_addr - (unsigned long)reloc_addr + (addend << 2);
					topbits = newvalue & 0xfe000000;
					if (topbits != 0xfe000000 && topbits != 0x00000000)
					{
						newvalue = fix_bad_pc24(reloc_addr, symbol_addr)
							- (unsigned long)reloc_addr + (addend << 2);
						topbits = newvalue & 0xfe000000;
						if (unlikely(topbits != 0xfe000000 && topbits != 0x00000000))
						{
							_dl_dprintf(2,"symbol '%s': R_ARM_PC24 relocation out of range.",
								symtab[symtab_index].st_name);
							_dl_exit(1);
						}
					}
					newvalue >>= 2;
					symbol_addr = (*reloc_addr & 0xff000000) | (newvalue & 0x00ffffff);
					*reloc_addr = symbol_addr;
					break;
				}
#else
				_dl_dprintf(2,"R_ARM_PC24: Compile shared libraries with -fPIC!\n");
				_dl_exit(1);
#endif
			case R_ARM_GLOB_DAT:
			case R_ARM_JUMP_SLOT:
				*reloc_addr = symbol_addr;
				break;
			case R_ARM_RELATIVE:
				*reloc_addr = DL_RELOC_ADDR(tpnt->loadaddr, *reloc_addr);
				break;
			case R_ARM_COPY:
				_dl_memcpy((void *) reloc_addr,
					   (void *) symbol_addr, symtab[symtab_index].st_size);
				break;
#ifdef __FDPIC__
			case R_ARM_FUNCDESC_VALUE:
				{
					struct funcdesc_value funcval;
					struct funcdesc_value *dst = (struct funcdesc_value *) reloc_addr;

					funcval.entry_point = (void*)symbol_addr;
					/* Add offset to section address for local symbols.  */
					if (ELF_ST_BIND(symtab[symtab_index].st_info) == STB_LOCAL)
					  funcval.entry_point += *reloc_addr;
					funcval.got_value = def_mod->loadaddr.got_value;
					*dst = funcval;
				}
				break;
			case R_ARM_FUNCDESC:
				{
				  unsigned long reloc_value = *reloc_addr;

				  if (symbol_addr)
					reloc_value = (unsigned long) _dl_funcdesc_for(symbol_addr + reloc_value, sym_ref.tpnt->loadaddr.got_value);
				  else
					/* Relocation against an
					   undefined weak symbol:
					   set funcdesc to zero.  */
					reloc_value = 0;

				  *reloc_addr = reloc_value;
				}
				break;
#endif
#if defined USE_TLS && USE_TLS
			case R_ARM_TLS_DTPMOD32:
				*reloc_addr = def_mod->l_tls_modid;
				break;

			case R_ARM_TLS_DTPOFF32:
				*reloc_addr += symbol_addr;
				break;

			case R_ARM_TLS_TPOFF32:
				CHECK_STATIC_TLS ((struct link_map *) def_mod);
				*reloc_addr += (symbol_addr + def_mod->l_tls_offset);
				break;
#endif
			default:
				return -1; /*call _dl_exit(1) */
		}
#if defined (__SUPPORT_LD_DEBUG__)
		if (_dl_debug_reloc && _dl_debug_detail && reloc_type != R_ARM_NONE)
			_dl_dprintf(_dl_debug_file, "\tpatch: %x ==> %x @ %x", old_val, *reloc_addr, reloc_addr);
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

	reloc_addr = (unsigned long *) DL_RELOC_ADDR(tpnt->loadaddr, rpnt->r_offset);
	reloc_type = ELF_R_TYPE(rpnt->r_info);

#if defined (__SUPPORT_LD_DEBUG__)
	{
		unsigned long old_val;

		if (reloc_type != R_ARM_NONE)
			old_val = *reloc_addr;
#endif
		switch (reloc_type) {
			case R_ARM_NONE:
				break;

			case R_ARM_JUMP_SLOT:
				*reloc_addr = DL_RELOC_ADDR(tpnt->loadaddr, *reloc_addr);
				break;
#ifdef __FDPIC__
			case R_ARM_FUNCDESC_VALUE:
				{
					struct funcdesc_value *dst = (struct funcdesc_value *) reloc_addr;

					dst->entry_point = DL_RELOC_ADDR(tpnt->loadaddr, dst->entry_point);
					dst->got_value = tpnt->loadaddr.got_value;
				}
				break;
#endif
			default:
				return -1; /*call _dl_exit(1) */
		}
#if defined (__SUPPORT_LD_DEBUG__)
		if (_dl_debug_reloc && _dl_debug_detail && reloc_type != R_ARM_NONE)
			_dl_dprintf(_dl_debug_file, "\tpatch: %x ==> %x @ %x", old_val, *reloc_addr, reloc_addr);
	}

#endif
	return 0;
}

void _dl_parse_lazy_relocation_information(struct dyn_elf *rpnt,
	unsigned long rel_addr, unsigned long rel_size)
{
	(void)_dl_parse(rpnt->dyn, NULL, rel_addr, rel_size, _dl_do_lazy_reloc);
}

int _dl_parse_relocation_information(struct dyn_elf *rpnt,
	struct r_scope_elem *scope, unsigned long rel_addr, unsigned long rel_size)
{
	return _dl_parse(rpnt->dyn, scope, rel_addr, rel_size, _dl_do_reloc);
}

#ifndef IS_IN_libdl
# include "../../libc/sysdeps/linux/arm/crtreloc.c"
#endif
