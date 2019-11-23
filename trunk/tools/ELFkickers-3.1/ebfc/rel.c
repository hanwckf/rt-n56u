/* rel.c: parts containing a relocation section.
 *
 * Copyright (C) 1999-2001 by Brian Raiter, under the GNU General
 * Public License. No warranty. See COPYING for details.
 */

#include	<stdlib.h>
#include	<string.h>
#include	<elf.h>
#include	"elfparts.h"
#include	"gen.h"

#ifndef	ELF32_R_INFO
#define	ELF32_R_INFO(sym, type)	(((sym) << 8) | ((type) & 0x00FF))
#endif

/* Set up the elfpart structure.
 */
static void relnew(elfpart *part)
{
    part->shtype = SHT_REL;
    part->shname = ".rel";
    part->entsize = sizeof(Elf32_Rel);
}

/* Name the relocation section, and translate its info field from a
 * part index to a section header index.
 */
static void init(elfpart *part, blueprint const *bp)
{
    char       *str;
    size_t	n;

    if (bp->parts[part->info].shtype != SHT_PROGBITS)
	assert(!".rel requires a valid info setting");

    if (bp->parts[part->info].shname) {
	n = strlen(part->shname);
	if (!(str = malloc(n + strlen(bp->parts[part->info].shname) + 1)))
	    assert(!"Out of memory!");
	memcpy(str, part->shname, n);
	strcpy(str + n, bp->parts[part->info].shname);
	part->shname = str;
    }
    n = part->info;
    part->info = 1;
    while (n--)
	if (bp->parts[n].shtype > 0)
	    ++part->info;

    part->done = TRUE;
}

/* Update the relocations to have the correct symbol table indices.
 */
static void complete(elfpart *part, blueprint const *bp)
{
    Elf32_Rel  *rel;
    int		sym, i;

    if (!part->link->done)
	return;

    for (i = 0, rel = part->part ; i < part->count ; ++i, ++rel) {
	sym = ELF32_R_SYM((int)rel->r_info);
	if (sym < 0)
	    sym = part->link->info - sym - 1;
	rel->r_info = ELF32_R_INFO(sym, ELF32_R_TYPE(rel->r_info));
    }

    part->done = TRUE;
    (void)bp;
}

/* Set up the elfpart structure for a rela section.
 */
static void relanew(elfpart *part)
{
    relnew(part);
    part->shtype = SHT_RELA;
    part->shname = ".rela";
}

/* The relocation elfpart structures.
 */
elfpart	part_rel = { relnew, init, NULL, complete };
elfpart	part_rela = { relanew, init, NULL, complete };


/* Adds a new entry to a relocation section. The return value is the
 * index of the entry.
 */
int addtorel(elfpart *part, unsigned int offset, int sym, int type)
{
    Elf32_Rel  *rel;

    assert(part->shtype == SHT_REL);

    part->size += part->entsize;
    rel = palloc(part);
    rel += part->count;
    rel->r_offset = offset;
    rel->r_info = ELF32_R_INFO(sym, type);
    return part->count++;
}

/* Adds a new entry to a relocation section. The index of the given
 * symbol is retrieved automatically; if the symbol is not already
 * in the table, it is added.
 */
int addrelsymbol(elfpart *part, unsigned int offset, int reltype,
		 char const *sym, int symbind, int symtype, int shndx)
{
    int	n;

    assert(part->shtype == SHT_REL);

    if (!sym || (n = getsymindex(part->link, sym)) == 0)
	n = addtosymtab(part->link, sym, symbind, symtype, shndx);
    return addtorel(part, offset, n, reltype);
}
