/* got.c: part containing a global offset table.
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
    part->shtype = SHT_PROGBITS;
    part->shname = ".got";
    part->flags = PF_R | PF_W;
    part->entsize = sizeof(Elf32_Addr);
}

/* Add the three required entries to the GOT, and add the GOT to
 * the dynamic symbol table.
 */
static void init(elfpart *part, blueprint const *bp)
{
    Elf32_Addr *got;
    elfpart    *p;
    int		i;

    part->count = 3;
    part->size = part->count * part->entsize;
    got = palloc(part);
    got[0] = 0;
    got[1] = 0;
    got[2] = 0;

    for (i = 0, p = bp->parts ; i < bp->partcount ; ++i, ++p) {
	if (p->shtype == SHT_DYNSYM) {
	    addtosymtab(p, NAME_GOT, STB_GLOBAL, STT_OBJECT, SHN_ABS);
	    break;
	}
    }

    part->done = TRUE;
    (void)bp;
}

/* Set the first entry in the GOT to point to the .dynamic section,
 * and set the address of the GOT in the dynamic symbol table.
 */
static void complete(elfpart *part, blueprint const *bp)
{
    int	n;

    for (n = 0 ; n < bp->partcount ; ++n) {
	if (bp->parts[n].shtype == SHT_DYNSYM) {
	    setsymvalue(bp->parts + n, NAME_GOT, part->addr);
	    break;
	}
    }
    for (n = 0 ; n < bp->partcount ; ++n) {
	if (bp->parts[n].shtype == SHT_DYNAMIC) {
	    ((Elf32_Addr*)part->part)[0] = bp->parts[n].addr;
	    break;
	}
    }

    part->done = TRUE;
    (void)bp;
}

/* The GOT elfpart structure.
 */
elfpart	part_got = { new, init, NULL, complete };
