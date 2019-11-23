/* elfparts.c: Global functions supplied by the elfparts library.
 *
 * Copyright (C) 1999-2001 by Brian Raiter, under the GNU General
 * Public License. No warranty. See COPYING for details.
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<errno.h>
#include	<elf.h>
#include	"elfparts.h"
#include	"gen.h"


/* Align a position to the next dword/page boundary.
 */
#define	filealign(off)	(((off) + 3) & ~3)
#define	memalign(off)	(((off) + 0x0FFF) & ~0x0FFF)


static int const	loadaddr = 0x08048000;

void newparts(blueprint const *bp)
{
    elfpart    *part;
    int		i;

    for (i = 0, part = bp->parts ; i < bp->partcount ; ++i, ++part)
	if (part->new)
	    (*part->new)(part);
}

void initparts(blueprint const *bp)
{
    elfpart    *part;
    int		i, m, n;

    for (i = 0, part = bp->parts ; i < bp->partcount ; ++i, ++part)
	part->done = !(part->shtype && part->init);

    m = bp->partcount;
    do {
	n = 0;
	for (i = 0, part = bp->parts ; i < bp->partcount ; ++i, ++part) {
	    if (!part->done) {
		(*part->init)(part, bp);
		if (!part->done)
		    ++n;
	    }
	}
	if (n == m)
	    assert(!"Mutually dependent ELF parts in init().");
	m = n;
    } while (n);
}

void fillparts(blueprint const *bp)
{
    elfpart    *part;
    int		i, m, n;

    for (i = 0, part = bp->parts ; i < bp->partcount ; ++i, ++part)
	part->done = !(part->shtype && part->fill);

    m = bp->partcount;
    do {
	n = 0;
	for (i = 0, part = bp->parts ; i < bp->partcount ; ++i, ++part) {
	    if (!part->done) {
		(*part->fill)(part, bp);
		if (!part->done)
		    ++n;
	    }
	}
	if (n == m)
	    assert(!"Mutually dependent ELF parts in fill().");
	m = n;
    } while (n);
}

void completeparts(blueprint const *bp)
{
    elfpart    *part;
    int		i, m, n;

    for (i = 0, part = bp->parts ; i < bp->partcount ; ++i, ++part)
	part->done = !(part->shtype && part->complete);

    m = bp->partcount;
    do {
	n = 0;
	for (i = 0, part = bp->parts ; i < bp->partcount ; ++i, ++part) {
	    if (!part->done) {
		(*part->complete)(part, bp);
		if (!part->done)
		    ++n;
	    }
	}
	if (n == m)
	    assert(!"Mutually dependent ELF parts in complete().");
	m = n;
    } while (n);
}

int computeoffsets(blueprint const *bp)
{
    Elf32_Off	off;
    elfpart    *part;
    int		i;

    off = 0;
    for (i = 0, part = bp->parts ; i < bp->partcount ; ++i, ++part) {
	if (part->shtype) {
	    if (part->entsize)
		part->size = part->count * part->entsize;
	    part->offset = off;
	    off = filealign(off + part->size);
	}
    }

    if (bp->filetype != ET_REL) {
	off = 0;
	for (i = 0, part = bp->parts ; i < bp->partcount ; ++i, ++part) {
	    if (part->flags) {
		part->addr = part->offset;
		if (!(part->flags & PF_W)) {
		    if (off < part->addr + part->size)
			off = part->addr + part->size;
		}
	    }
	}
	off = memalign(off);
	for (i = 0, part = bp->parts ; i < bp->partcount ; ++i, ++part)
	    if (part->flags & PF_W)
		part->addr += off;

	if (bp->filetype == ET_EXEC) {
	    for (i = 0, part = bp->parts ; i < bp->partcount ; ++i, ++part)
		if (part->flags)
		    part->addr += loadaddr;
	}
    }

    return TRUE;
}

#define	fout(fp, p, n)	((n) <= 0 || fwrite((p), (n), 1, (fp)) == 1)

int outputelf(blueprint const *bp, char const *filename)
{
    FILE       *fp;
    elfpart    *part;
    char	padding[16] = { 0 };
    Elf32_Off	off;
    int		i;

    if (!(fp = fopen(filename, "wb")))
	return FALSE;
    off = 0;
    for (i = 0, part = bp->parts ; i < bp->partcount ; ++i, ++part) {
	if (!part->shtype)
	    continue;
	if (!fout(fp, padding, part->offset - off)
			|| !fout(fp, part->part, part->size)) {
	    i = errno;
	    fclose(fp);
	    errno = i;
	    return FALSE;
	}
	off = part->offset + part->size;
    }
    if (fclose(fp))
	return FALSE;

    return TRUE;
}
