/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

#include "ldso.h"

unsigned long
_dl_linux_resolver(struct elf_resolve *tpnt, int reloc_entry)
{
    ELF_RELOC *this_reloc;
    int symtab_index;
    //char *rel_tab;
    Elf32_Sym *sym_tab;
    char *str_tab;
    char *sym_name;
    char *sym_addr;
    char **reloc_addr;

    this_reloc  = (ELF_RELOC *)tpnt->dynamic_info[DT_JMPREL];
    this_reloc += reloc_entry;
    //this_reloc = (ELF_RELOC *)(intptr_t)(rel_tab + reloc_entry);
    symtab_index = ELF32_R_SYM(this_reloc->r_info);

    sym_tab = (Elf32_Sym *)(intptr_t)tpnt->dynamic_info[DT_SYMTAB];
    str_tab = (char *)tpnt->dynamic_info[DT_STRTAB];
    sym_name = str_tab + sym_tab[symtab_index].st_name;

    reloc_addr = (char **)((unsigned long)this_reloc->r_offset +
                     (unsigned long)tpnt->loadaddr);

    sym_addr = _dl_find_hash(sym_name, &_dl_loaded_modules->symbol_scope, tpnt, ELF_RTYPE_CLASS_PLT, NULL);

    if (unlikely(!sym_addr)) {
        _dl_dprintf(2, "%s: 1can't resolve symbol '%s' in lib '%s'.\n", _dl_progname, sym_name, tpnt->libname);
        _dl_exit(1);
    }

    *reloc_addr = sym_addr;

    return (unsigned long)sym_addr;
}

static int
_dl_parse(struct elf_resolve *tpnt, struct r_scope_elem*scope,
      unsigned long rel_addr, unsigned long rel_size,
      int (*reloc_fnc)(struct elf_resolve *tpnt, struct r_scope_elem*scope,
               ELF_RELOC *rpnt, Elf32_Sym *symtab, char *strtab))
{
    unsigned int i;
    char *strtab;
    Elf32_Sym *symtab;
    ELF_RELOC *rpnt;
    int symtab_index;

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
            _dl_dprintf(2, "2can't handle reloc type '%s' in lib '%s'\n",
                    _dl_reltypes(reloc_type), tpnt->libname);
#else
            _dl_dprintf(2, "3can't handle reloc type %x in lib '%s'\n",
                    reloc_type, tpnt->libname);
#endif
            return res;
        } else if (unlikely(res > 0)) {
            _dl_dprintf(2, "4can't resolve symbol in lib '%s'.\n", tpnt->libname);
            return res;
        }
    }

    return 0;
}

static int
_dl_do_reloc(struct elf_resolve *tpnt, struct r_scope_elem *scope,
         ELF_RELOC *rpnt, Elf32_Sym *symtab, char *strtab)
{
    int reloc_type;
    int symtab_index;
    char *symname;
    unsigned long *reloc_addr;
    unsigned long symbol_addr;
    struct symbol_ref sym_ref;
#if defined (__SUPPORT_LD_DEBUG__)
    unsigned long old_val;
#endif
#if defined USE_TLS && USE_TLS
	struct elf_resolve *tls_tpnt = NULL;
#endif
#if defined(__CSKYABIV2__)
    unsigned int insn_opcode = 0x0;
    unsigned short *opcode16_addr;
#endif

    reloc_addr = (unsigned long *)(intptr_t)(tpnt->loadaddr + (unsigned long)rpnt->r_offset);
#if defined(__CSKYABIV2__)
    opcode16_addr = (unsigned short *)reloc_addr;
#endif
	reloc_type = ELF32_R_TYPE(rpnt->r_info);

	if (reloc_type == R_CKCORE_NONE)
		return 0;

    symtab_index = ELF32_R_SYM(rpnt->r_info);
    symbol_addr = 0;
    sym_ref.sym = &symtab[symtab_index];
    sym_ref.tpnt = NULL;
    symname = strtab + symtab[symtab_index].st_name;
    if (symtab_index) {
        symbol_addr = (unsigned long)_dl_find_hash(symname, scope, tpnt,
                               elf_machine_type_class(reloc_type), &sym_ref);
    /*
         * We want to allow undefined references to weak symbols - this
         * might have been intentional.  We should not be linking local
         * symbols here, so all bases should be covered.
         */
      //  if (unlikely(!symbol_addr && ELF32_ST_BIND(symtab[symtab_index].st_info) != STB_WEAK))
		if (!symbol_addr && (ELF_ST_TYPE(symtab[symtab_index].st_info) != STT_TLS)
			&& (ELF_ST_BIND(symtab[symtab_index].st_info) != STB_WEAK))
            return 1;
#if defined USE_TLS && USE_TLS
		tls_tpnt = sym_ref.tpnt;
#endif
    }else{
		/*
		 * Relocs against STN_UNDEF are usually treated as using a
		 * symbol value of zero, and using the module containing the
		 * reloc itself.
		 */
        symbol_addr = symtab[symtab_index].st_name;
#if defined USE_TLS && USE_TLS
		tls_tpnt = tpnt;
#endif
    }
#if defined (__SUPPORT_LD_DEBUG__)
    old_val = *reloc_addr;
#endif

    switch (reloc_type) {            /* need modify */
        case R_CKCORE_NONE:
		case R_CKCORE_PCRELJSR_IMM11BY2:
            break;
        case R_CKCORE_ADDR32:
            *reloc_addr = symbol_addr + rpnt->r_addend;
            break;
        case R_CKCORE_GLOB_DAT:
        case R_CKCORE_JUMP_SLOT:
            *reloc_addr = symbol_addr;
            break;
        case R_CKCORE_RELATIVE:
            *reloc_addr = (unsigned long)tpnt->loadaddr + rpnt->r_addend;
            break;
#if defined(__CSKYABIV2__)
        case R_CKCORE_ADDR_HI16:
            insn_opcode = (*opcode16_addr << 16) | (*(opcode16_addr + 1));
            insn_opcode = (insn_opcode & 0xffff0000)
                            | (((symbol_addr + rpnt->r_addend) >> 16) & 0xffff);
            *(opcode16_addr++) = (unsigned short)(insn_opcode >> 16);
            *opcode16_addr = (unsigned short)(insn_opcode & 0xffff);
            break;
        case R_CKCORE_ADDR_LO16:
            insn_opcode = (*opcode16_addr << 16) | (*(opcode16_addr + 1));
            insn_opcode = (insn_opcode & 0xffff0000)
                            | ((symbol_addr + rpnt->r_addend) & 0xffff);
            *(opcode16_addr++) = (unsigned short)(insn_opcode >> 16);
            *opcode16_addr = (unsigned short)(insn_opcode & 0xffff);
            break;
        case R_CKCORE_PCREL_IMM26BY2:
        {
            unsigned int offset = ((symbol_addr + rpnt->r_addend -
                                    (unsigned int)reloc_addr) >> 1);
            insn_opcode = (*opcode16_addr << 16) | (*(opcode16_addr + 1));
            if (offset > 0x3ffffff){
                _dl_dprintf(2, "%s:The reloc R_CKCORE_PCREL_IMM26BY2 cannot reach the symbol '%s'.\n", _dl_progname, symname);
                _dl_exit(1);
            }
            insn_opcode = (insn_opcode & ~0x3ffffff) | offset;
            *(opcode16_addr++) = (unsigned short)(insn_opcode >> 16);
            *opcode16_addr = (unsigned short)(insn_opcode & 0xffff);
            break;
        }
        case R_CKCORE_PCREL_JSR_IMM26BY2:
            break;
#endif
        case R_CKCORE_COPY:
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
#if defined USE_TLS && USE_TLS
		case R_CKCORE_TLS_DTPMOD32:
			*reloc_addr = tls_tpnt->l_tls_modid;
			break;
		case R_CKCORE_TLS_DTPOFF32:
			*reloc_addr += symbol_addr;
			break;
		case R_CKCORE_TLS_TPOFF32:
			CHECK_STATIC_TLS ((struct link_map *) tls_tpnt);
			*reloc_addr += tls_tpnt->l_tls_offset + symbol_addr;
			break;
#endif
        default:
            return -1;
    }

#if defined (__SUPPORT_LD_DEBUG__)
    if (_dl_debug_reloc && _dl_debug_detail)
        _dl_dprintf(_dl_debug_file, "\n\tpatched: %x ==> %x @ %x",
                old_val, *reloc_addr, reloc_addr);
#endif

    return 0;
}

static int
_dl_do_lazy_reloc(struct elf_resolve *tpnt, struct r_scope_elem *scope,
          ELF_RELOC *rpnt, Elf32_Sym *symtab, char *strtab)
{
    int reloc_type;
    unsigned long *reloc_addr;
#if defined (__SUPPORT_LD_DEBUG__)
    unsigned long old_val;
#endif

    reloc_addr = (unsigned long *)(intptr_t)(tpnt->loadaddr + (unsigned long)rpnt->r_offset);
    reloc_type = ELF32_R_TYPE(rpnt->r_info);

#if defined (__SUPPORT_LD_DEBUG__)
    old_val = *reloc_addr;
#endif

    switch (reloc_type) {
        case R_CKCORE_NONE:
		case R_CKCORE_PCRELJSR_IMM11BY2:
            break;
        case R_CKCORE_JUMP_SLOT:
            *reloc_addr = (unsigned long)tpnt->loadaddr + rpnt->r_addend;
            break;
        default:
            return -1;
    }

#if defined (__SUPPORT_LD_DEBUG__)
    if (_dl_debug_reloc && _dl_debug_detail)
        _dl_dprintf(_dl_debug_file, "\n\tpatched: %x ==> %x @ %x",
                old_val, *reloc_addr, reloc_addr);
#endif

    return 0;
}


void
_dl_parse_lazy_relocation_information(struct dyn_elf *rpnt,
                      unsigned long rel_addr,
                      unsigned long rel_size)
{
    (void)_dl_parse(rpnt->dyn, NULL, rel_addr, rel_size, _dl_do_lazy_reloc);
}

int
_dl_parse_relocation_information(struct dyn_elf *rpnt,
                 struct r_scope_elem *scope,
                 unsigned long rel_addr,
                 unsigned long rel_size)
{
    return _dl_parse(rpnt->dyn, scope, rel_addr, rel_size, _dl_do_reloc);
}
