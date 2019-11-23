/* dynamic.c: part containing a .dynamic section.
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
    part->shtype = SHT_DYNAMIC;
    part->shname = ".dynamic";
    part->flags = PF_R | PF_W;
    part->entsize = sizeof(Elf32_Dyn);
}

/* Add the minimally required entries to the dynamic section.
 */
static void init(elfpart *part, blueprint const *bp)
{
    setdynvalue(part, DT_HASH, 0);
    setdynvalue(part, DT_SYMTAB, 0);
    setdynvalue(part, DT_SYMENT, 0);
    setdynvalue(part, DT_STRTAB, 0);
    setdynvalue(part, DT_STRSZ, 0);

    part->done = TRUE;
    (void)bp;
}

/* Fill in the values for the required entries.
 */
static void complete(elfpart *part, blueprint const *bp)
{
    elfpart    *p;
    int		i;

    for (i = 0, p = bp->parts ; i < bp->partcount ; ++i, ++p)
	if (p->shtype == SHT_HASH)
	    break;
    if (i == bp->partcount)
	assert(!".dynamic requires a .hash section");

    setdynvalue(part, DT_HASH, p->addr);
    p = p->link;
    setdynvalue(part, DT_SYMTAB, p->addr);
    setdynvalue(part, DT_SYMENT, p->entsize);
    p = p->link;
    setdynvalue(part, DT_STRTAB, p->addr);
    setdynvalue(part, DT_STRSZ, p->size);

    part->done = TRUE;
    (void)bp;
}

/* The dynamic elfpart structure.
 */
elfpart	part_dynamic = { new, init, NULL, complete };


/* Sets the value of an entry in a dynamic section. The entry is added
 * to the section if it is not already present. The return value is the
 * index of the entry.
 */
int setdynvalue(elfpart *part, int tag, int value)
{
    Elf32_Dyn  *dyn;
    int		i;

    assert(part->shtype == SHT_DYNAMIC);

    for (i = 0, dyn = part->part ; i < part->count - 1 ; ++i, ++dyn)
	if (dyn->d_tag == tag)
	    break;
    if (i >= part->count - 1) {
	part->count = i + 2;
	part->size = part->count * part->entsize;
	dyn = palloc(part);
	dyn += i;
	dyn->d_tag = tag;
	dyn[1].d_tag = DT_NULL;
	dyn[1].d_un.d_val = 0;
    }
    dyn->d_un.d_val = value;
    return i;
}
