/* vi: set sw=4 ts=4: */
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

/* include the arch-specific _dl_reltypes_tab */
#include "dl-debug.h"

static const char *_dl_reltypes(int type)
{
	static char buf[50];
	const char *str;
	int tabsize;

	tabsize = (int)(sizeof(_dl_reltypes_tab) / sizeof(_dl_reltypes_tab[0]));

	if (type >= tabsize || (str = _dl_reltypes_tab[type]) == NULL)
		str = _dl_simple_ltoa(buf, (unsigned long)type);

	return str;
}
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

	_dl_dprintf(_dl_debug_file, "%s\toffset=%x",
		_dl_reltypes(ELF_R_TYPE(rpnt->r_info)),
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
