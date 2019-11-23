/* symtab.c: parts containing a symbol table.
 *
 * Copyright (C) 1999-2001 by Brian Raiter, under the GNU General
 * Public License. No warranty. See COPYING for details.
 */

#include	<stdlib.h>
#include	<string.h>
#include	<elf.h>
#include	"elfparts.h"
#include	"gen.h"

#ifndef	ELF32_ST_INFO
#define	ELF32_ST_INFO(bind, type)	(((bind) << 4) | ((type) & 0x0F))
#endif

/* Set up the elfpart structure and allocate an empty symbol table.
 */
static void new(elfpart *part)
{
    static Elf32_Sym	blanksym = { 0, 0, 0, 0, 0, SHN_UNDEF };
    Elf32_Sym	       *sym;

    part->shtype = SHT_SYMTAB;
    part->shname = ".symtab";
    part->info = 1;
    part->entsize = sizeof(Elf32_Sym);
    part->size = part->entsize;
    part->count = 1;
    sym = palloc(part);
    *sym = blanksym;
}

/* Translate the symbols' shndx fields from a part index to a section
 * header index.
 */
static void complete(elfpart *part, blueprint const *bp)
{
    Elf32_Sym  *sym;
    int		i, n;

    for (i = 0, sym = part->part ; i < part->count ; ++i, ++sym) {
	if (sym->st_shndx > 0 && sym->st_shndx < bp->partcount) {
	    n = sym->st_shndx;
	    sym->st_shndx = 1;
	    while (n--)
		if (bp->parts[n].shtype > 0)
		    ++sym->st_shndx;
	}
    }

    part->done = TRUE;
    (void)bp;
}

/* Set up the elfpart structure for a dynamic symbol table.
 */
static void dynnew(elfpart *part)
{
    new(part);
    part->shtype = SHT_DYNSYM;
    part->shname = ".dynsym";
    part->flags = PF_R;
}

/* If the parts list has a .dynamic section, then add a _DYNAMIC
 * symbol to the symbol table.
 */
static void dyninit(elfpart *part, blueprint const *bp)
{
    int	n;

    for (n = 0 ; n < bp->partcount ; ++n) {
	if (bp->parts[n].shtype == SHT_DYNAMIC) {
	    addtosymtab(part, NAME_DYNAMIC, STB_GLOBAL, STT_OBJECT, SHN_ABS);
	    break;
	}
    }

    part->done = TRUE;
    (void)bp;
}

/* Set the value of the _DYNAMIC symbol to point to the address of the
 * dynamic section.
 */
static void dyncomplete(elfpart *part, blueprint const *bp)
{
    int	n;

    for (n = 0 ; n < bp->partcount ; ++n) {
	if (bp->parts[n].shtype == SHT_DYNAMIC) {
	    setsymvalue(part, NAME_DYNAMIC, bp->parts[n].addr);
	    break;
	}
    }

    complete(part, bp);
}

/* The symbol table elfpart structures.
 */
elfpart	part_symtab = { new, NULL, NULL, complete },
	part_dynsym = { dynnew, dyninit, NULL, dyncomplete };


/* Adds a symbol to a symbol table. The symbol's value is initialized
 * to zero; the other data associated with the symbol are supplied by
 * the function's arguments. The symbol's name is automatically added
 * to the appropriate string table. The return value is an index of
 * the symbol in the table (see below for comments on the index
 * value).
 */
int addtosymtab(elfpart *part, char const *str, int bind, int type, int shndx)
{
    Elf32_Sym  *sym;
    int		n;

    assert(part->shtype == SHT_SYMTAB || part->shtype == SHT_DYNSYM);
    assert(part->link && part->link->shtype == SHT_STRTAB);

    part->size += part->entsize;
    sym = palloc(part);
    if (bind == STB_LOCAL) {
	sym += part->info;
	if (part->info < part->count)
	    memmove(sym + 1, sym, (part->count - part->info) * sizeof *sym);
	++part->count;
	n = part->info++;
    } else {
	sym += part->count++;
	n = part->info - part->count;
    }
    sym->st_name = addtostrtab(part->link, str);
    sym->st_value = 0;
    sym->st_size = 0;
    sym->st_info = ELF32_ST_INFO(bind, type);
    sym->st_other = 0;
    sym->st_shndx = shndx;
    return n;
}

/* Look up a symbol already in a symbol table by name. The return
 * value is the index of the symbol, or zero if the symbol is not in
 * the table. The index will be negative for local symbols, since the
 * true index cannot be determined until all the global symbols have
 * been added. After fill() has been called, a negative index can be
 * converted to a real index by subtracting it from the value in the
 * symbol table's info field, less one.
 */
int getsymindex(elfpart *part, char const *name)
{
    Elf32_Sym  *sym;
    char const *strtab;
    int		i;

    assert(part->shtype == SHT_SYMTAB || part->shtype == SHT_DYNSYM);
    assert(part->link && part->link->shtype == SHT_STRTAB);

    strtab = part->link->part;
    sym = part->part;
    for (i = 1, ++sym ; i < part->count ; ++i, ++sym) {
	if (!strcmp(name, strtab + sym->st_name)) {
	    if (i >= part->info)
		return part->info - i - 1;
	    else
		return i;
	}
    }
    return 0;
}

/* Sets the value of a symbol in a symbol table. The return value is
 * FALSE if the given symbol could not be found in the table.
 */
int setsymvalue(elfpart *part, char const *name, unsigned int value)
{
    Elf32_Sym  *sym;
    char const *strtab;
    int		i;

    assert(part->shtype == SHT_SYMTAB || part->shtype == SHT_DYNSYM);
    assert(part->link && part->link->shtype == SHT_STRTAB);

    strtab = part->link->part;
    sym = part->part;
    for (i = 1, ++sym ; i < part->count ; ++i, ++sym) {
	if (!strcmp(name, strtab + sym->st_name)) {
	    sym->st_value = value;
	    return TRUE;
	}
    }
    return FALSE;
}
