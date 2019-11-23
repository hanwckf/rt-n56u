/* shdrtab.c: part containing the section header table.
 *
 * Copyright (C) 1999-2001 by Brian Raiter, under the GNU General
 * Public License. No warranty. See COPYING for details.
 */

#include	<stdlib.h>
#include	<elf.h>
#include	"elfparts.h"
#include	"gen.h"

/* Set up the elfpart structure.
 */
static void new(elfpart *part)
{
    part->shtype = SHT_OTHER;
    part->entsize = sizeof(Elf32_Shdr);
}

/* Create a blank section header table with enough entries for all the
 * appropriate parts in the blueprint.
 */
static void init(elfpart *part, blueprint const *bp)
{
    static Elf32_Shdr	blankshdr = { 0, SHT_NULL, 0, 0, 0, 0, 0, 0, 0, 0 };
    Elf32_Shdr	       *shdr;
    int			i;

    ((Elf32_Ehdr*)bp->parts[0].part)->e_shoff = (Elf32_Off)(long)part;

    for (i = 0 ; i < bp->partcount ; ++i)
	if (bp->parts[i].shtype > 0 && !bp->parts[i].done)
	    return;

    part->count = 1;
    for (i = 0 ; i < bp->partcount ; ++i) {
	if (bp->parts[i].shtype <= 0)
	    continue;
	++part->count;
    }

    part->size = part->count * part->entsize;
    shdr = palloc(part);
    for (i = 0 ; i < part->count ; ++i)
	shdr[i] = blankshdr;

    part->done = TRUE;
}

/* Add the section names to the section header string table, if one is
 * present.
 */
static void fill(elfpart *part, blueprint const *bp)
{
    Elf32_Shdr *shdr = part->part;
    int		i, n;

    if (part->link) {
	for (i = n = 0 ; i < bp->partcount ; ++i) {
	    if (bp->parts[i].shtype <= 0)
		continue;
	    ++n;
	    if (bp->parts[i].shname)
		shdr[n].sh_name = addtostrtab(part->link, bp->parts[i].shname);
	    if (bp->parts + i == part->link)
		((Elf32_Ehdr*)bp->parts[0].part)->e_shstrndx = n;
	}
	part->link = NULL;
    }

    part->done = TRUE;
}

/* Fill in the section header table, using the information from the
 * parts list in the blueprint.
 */
static void complete(elfpart *part, blueprint const *bp)
{
    Elf32_Shdr *shdr;
    elfpart    *p;
    int		i, n;

    for (i = 0 ; i < bp->partcount ; ++i)
	if (bp->parts[i].shtype > 0 && !bp->parts[i].done)
	    return;

    shdr = part->part;
    for (i = 0, p = bp->parts ; i < bp->partcount ; ++i, ++p) {
	if (p->shtype <= 0)
	    continue;
	++shdr;
	shdr->sh_type = p->shtype;
	shdr->sh_offset = p->offset;
	shdr->sh_size = p->size;
	shdr->sh_addr = p->addr;
	shdr->sh_entsize = p->entsize;
	if (p->flags) {
	    shdr->sh_addralign = 4;
	    shdr->sh_flags = SHF_ALLOC;
	    if (p->flags & PF_W)
		shdr->sh_flags |= SHF_WRITE;
	    if (p->flags & PF_X) {
		shdr->sh_flags |= SHF_EXECINSTR;
		shdr->sh_addralign = 16;
	    }
	} else if (shdr->sh_entsize)
	    shdr->sh_addralign = 4;
	if (p->info)
	    shdr->sh_info = p->info;
	if (p->link) {
	    shdr->sh_link = 1;
	    for (n = 0 ; n < bp->partcount ; ++n) {
		if (bp->parts + n == p->link)
		    break;
		if (bp->parts[n].shtype > 0)
		    ++shdr->sh_link;
	    }
	}
    }

    part->done = TRUE;
}

/* The section header table elfpart structure.
 */
elfpart	part_shdrtab = { new, init, fill, complete };
