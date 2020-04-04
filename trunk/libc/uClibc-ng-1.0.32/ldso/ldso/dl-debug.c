/* common debug code for ELF shared library loader
 *
 * Copyright (c) 1994-2000 Eric Youngdale, Peter MacDonald,
 *                         David Engel, Hongjiu Lu and Mitch D'Souza
 * Copyright (C) 2001-2004 Erik Andersen
 * Copyright (C) 2002-2004, Axis Communications AB
 * Copyright (C) 2003, 2004 Red Hat, Inc.
 * Copyright (C) 2002, Steven J. Hill (sjhill@realitydiluted.com)
 * Copyright (C) 2001-2002 David A. Schleef
 * Copyright (C) 2004 Joakim Tjernlund
 * Copyright (C) 2002, Stefan Allius <allius@atecom.com> and
 *                     Eddie C. Dost <ecd@atecom.com>
 * Copyright (C) 2003, 2004, 2005  Paul Mundt <lethal@linux-sh.org>
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

#if defined (__SUPPORT_LD_DEBUG__)

static void debug_sym(ElfW(Sym) *symtab, char *strtab, int symtab_index)
{
	if (!_dl_debug_symbols || !symtab_index)
		return;

	_dl_dprintf(_dl_debug_file,
		"\n%s\n\tvalue=%x\tsize=%x\tinfo=%x\tother=%x\tshndx=%x",
		strtab + symtab[symtab_index].st_name,
		symtab[symtab_index].st_value,
		symtab[symtab_index].st_size,
		symtab[symtab_index].st_info,
		symtab[symtab_index].st_other,
		symtab[symtab_index].st_shndx);
}

static void debug_reloc(ElfW(Sym) *symtab, char *strtab, ELF_RELOC *rpnt)
{
	if (!_dl_debug_reloc)
		return;

	if (_dl_debug_symbols) {
		_dl_dprintf(_dl_debug_file, "\n\t");
	} else {
		int symtab_index;
		const char *sym;

		symtab_index = ELF_R_SYM(rpnt->r_info);
		sym = symtab_index ? strtab + symtab[symtab_index].st_name : "sym=0x0";

		_dl_dprintf(_dl_debug_file, "\n%s\n\t", sym);
	}

	_dl_dprintf(_dl_debug_file, "%x\toffset=%x",
		ELF_R_TYPE(rpnt->r_info),
		rpnt->r_offset);
#ifdef ELF_USES_RELOCA
	_dl_dprintf(_dl_debug_file, "\taddend=%x", rpnt->r_addend);
#endif
	_dl_dprintf(_dl_debug_file, "\n");
}

#else

#define debug_sym(symtab, strtab, symtab_index)
#define debug_reloc(symtab, strtab, rpnt)

#endif /* __SUPPORT_LD_DEBUG__ */

#ifdef __LDSO_PRELINK_SUPPORT__
static void
internal_function
_dl_debug_lookup (const char *undef_name, struct elf_resolve *undef_map,
					const ElfW(Sym) *ref, struct symbol_ref *value, int type_class)
{
#ifdef SHARED
  if (_dl_trace_prelink)
    {
      int conflict = 0;
      struct symbol_ref val = { ref, NULL };

      if ((_dl_trace_prelink_map == NULL
	   || _dl_trace_prelink_map == _dl_loaded_modules)
	  && undef_map != _dl_loaded_modules)
	{
	  _dl_find_hash(undef_name, &undef_map->symbol_scope,
			undef_map, type_class, &val);

	  if (val.sym != value->sym || val.tpnt != value->tpnt)
	    conflict = 1;
	}

      if (unlikely(value->sym && ELF_ST_TYPE(value->sym->st_info) == STT_TLS))
	type_class = 4;

      if (conflict
	  || _dl_trace_prelink_map == undef_map
	  || _dl_trace_prelink_map == NULL
	  || type_class == 4)
	{
	  _dl_dprintf (1, "%s %x %x -> %x %x ",
		      conflict ? "conflict" : "lookup",
		      (size_t) undef_map->mapaddr,
		      (size_t) (((ElfW(Addr)) ref) - undef_map->mapaddr),
		      (size_t) (value->tpnt ? value->tpnt->mapaddr : 0),
		      (size_t) (value->sym ? value->sym->st_value : 0));
	  if (conflict)
	    _dl_dprintf (1, "x %x %x ",
			(size_t) (val.tpnt ? val.tpnt->mapaddr : 0),
			(size_t) (val.sym ? val.sym->st_value : 0));
	  _dl_dprintf (1, "/%x %s\n", type_class, undef_name);
	}
}
#endif
}

#else
#define _dl_debug_lookup(undef_name, undef_map, ref, value, type_class)
#endif
