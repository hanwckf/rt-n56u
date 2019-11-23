/* ehdr.c: part containing the ELF header.
 *
 * Copyright (C) 1999-2001 by Brian Raiter, under the GNU General
 * Public License. No warranty. See COPYING for details.
 */

#include	<stdlib.h>
#include	<elf.h>
#include	"elfparts.h"
#include	"gen.h"

/* Set up the elfpart structure, and create an empty ELF header.
 */
static void new(elfpart *part)
{
    Elf32_Ehdr	blankehdr = {
	{ 0x7F, 'E', 'L', 'F', ELFCLASS32, ELFDATA2LSB, EV_CURRENT },
	0, 0, EV_CURRENT, 0, 0, 0, 0, sizeof(Elf32_Ehdr), 0, 0, 0, 0, 0
    };

    part->shtype = SHT_OTHER;
    part->flags = PF_R;
    part->size = sizeof(Elf32_Ehdr);
    palloc(part);
    *(Elf32_Ehdr*)part->part = blankehdr;
}

/* Fill in the ELF file type.
 */
static void init(elfpart *part, blueprint const *bp)
{
    ((Elf32_Ehdr*)part->part)->e_type = (Elf32_Half)bp->filetype;

    part->done = TRUE;
}

/* Fill in the information for the program and section header tables.
 */
static void complete(elfpart *part, blueprint const *bp)
{
    Elf32_Ehdr *ehdr = part->part;
    elfpart    *p;

    if (ehdr->e_phoff) {
	p = (elfpart*)(long)ehdr->e_phoff;
	ehdr->e_phoff = p->offset;
	ehdr->e_phnum = p->count;
	ehdr->e_phentsize = p->entsize;
    }
    if (ehdr->e_shoff) {
	p = (elfpart*)(long)ehdr->e_shoff;
	ehdr->e_shoff = p->offset;
	ehdr->e_shnum = p->count;
	ehdr->e_shentsize = p->entsize;
    }

    if (!ehdr->e_machine)
	ehdr->e_machine = EM_386;

    part->done = TRUE;
    (void)bp;
}


elfpart	part_ehdr = { new, init, NULL, complete };
