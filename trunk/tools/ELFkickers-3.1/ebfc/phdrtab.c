/* phdrtab.c: part containing the program header table.
 *
 * Copyright (C) 1999-2001 by Brian Raiter, under the GNU General
 * Public License. No warranty. See COPYING for details.
 */

#include	<stdlib.h>
#include	<elf.h>
#include	"elfparts.h"
#include	"gen.h"

/* The different possible entries in the program header table.
 */
enum { PH_TEXT = 0, PH_DATA, PH_DYNAMIC, PH_COUNT };

/* Set up the elfpart structure.
 */
static void new(elfpart *part)
{
    part->shtype = SHT_OTHER;
    part->flags = PF_R;
    part->entsize = sizeof(Elf32_Phdr);
}

/* Note the existence of the program header table in the ELF header.
 */
static void init(elfpart *part, blueprint const *bp)
{
    ((Elf32_Ehdr*)bp->parts[0].part)->e_phoff = (Elf32_Off)(long)part;

    part->done = TRUE;
    (void)bp;
}

/* Add entries in the program header table for the text segment, the
 * data segment, and (if present) the dynamic section.
 */
static void fill(elfpart *part, blueprint const *bp)
{
    static Elf32_Phdr	blankphdr = { PT_NULL, 0, 0, 0, 0, 0, 0, 0 };
    Elf32_Phdr	       *phdr;
    int			n;

    part->count = PH_COUNT;
    for (n = bp->partcount - 1 ; n > 0 ; --n)
	if (bp->parts[n].shtype == SHT_DYNAMIC)
	    break;
    if (!n)
	--part->count;
    part->size = part->count * part->entsize;
    phdr = palloc(part);

    phdr[PH_TEXT] = blankphdr;
    phdr[PH_TEXT].p_type = PT_LOAD;
    phdr[PH_TEXT].p_flags = PF_R | PF_X;
    phdr[PH_TEXT].p_align = 0x1000;

    phdr[PH_DATA] = blankphdr;
    phdr[PH_DATA].p_type = PT_LOAD;
    phdr[PH_DATA].p_flags = PF_R | PF_W;
    phdr[PH_DATA].p_align = 0x1000;

    if (n) {
	phdr[PH_DYNAMIC] = blankphdr;
	phdr[PH_DYNAMIC].p_type = PT_DYNAMIC;
	phdr[PH_DYNAMIC].p_flags = PF_R | PF_W;
	phdr[PH_DYNAMIC].p_align = 4;
	phdr[PH_DYNAMIC].p_offset = (Elf32_Off)(long)(bp->parts + n);
    }

    part->done = TRUE;
    (void)bp;
}

/* Fill in all the necessary information in the program header table.
 */
static void complete(elfpart *part, blueprint const *bp)
{
    Elf32_Phdr *phdr = part->part;
    elfpart    *p;
    int		i, n;

    phdr[PH_TEXT].p_offset = phdr[PH_DATA].p_offset = 0xFFFFFFFF;
    phdr[PH_TEXT].p_memsz = phdr[PH_DATA].p_memsz = 0;
    for (i = 0, p = bp->parts ; i < bp->partcount ; ++i, ++p) {
	if (!(p->flags & PF_R))
	    continue;
	n = p->flags & PF_W ? PH_DATA : PH_TEXT;
	if (phdr[n].p_offset > p->offset) {
	    phdr[n].p_offset = p->offset;
	    phdr[n].p_vaddr = p->addr;
	}
	if (phdr[n].p_memsz < p->offset + p->size)
	    phdr[n].p_memsz = p->offset + p->size;
    }
    phdr[PH_TEXT].p_memsz -= phdr[PH_TEXT].p_offset;
    phdr[PH_DATA].p_memsz -= phdr[PH_DATA].p_offset;

    if (part->count > PH_DYNAMIC) {
	p = (elfpart*)(long)phdr[PH_DYNAMIC].p_offset;
	phdr[PH_DYNAMIC].p_offset = p->offset;
	phdr[PH_DYNAMIC].p_vaddr = p->addr;
	phdr[PH_DYNAMIC].p_memsz = p->size;
    }

    for (i = 0 ; i < part->count ; ++i) {
	phdr[i].p_paddr = phdr[i].p_vaddr;
	phdr[i].p_filesz = phdr[i].p_memsz;
    }

    part->done = TRUE;
    (void)bp;
}

/* The program header table elfpart structure.
 */
elfpart	part_phdrtab = { new, init, fill, complete };
