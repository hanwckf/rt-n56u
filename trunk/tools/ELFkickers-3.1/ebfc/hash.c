/* hash.c: part containing a hash table.
 *
 * Copyright (C) 1999-2001 by Brian Raiter, under the GNU General
 * Public License. No warranty. See COPYING for details.
 */

#include	<stdlib.h>
#include	<string.h>
#include	<elf.h>
#include	"elfparts.h"
#include	"gen.h"

/* Set up the elfpart structure.
 */
static void new(elfpart *part)
{
    part->shtype = SHT_HASH;
    part->shname = ".hash";
    part->flags = PF_R;
    part->entsize = sizeof(Elf32_Word);
}

/* Determine the size of the hash table and allocate the necessary
 * memory.
 */
static void fill(elfpart *part, blueprint const *bp)
{
    static Elf32_Word const buckets[] = {
	1, 1, 3, 17, 37, 67, 97, 131, 197, 263, 521, 1031, 2053,
	4099, 8209, 16411, 32771
    };
    Elf32_Word *hash;
    Elf32_Word	symnum;
    int		i;

    assert(part->link);

    if (!part->link->done)
	return;

    symnum = part->link->count;
    for (i = 1 ; i < (int)(sizeof buckets / sizeof *buckets) ; ++i)
	if (buckets[i] > symnum)
	    break;
    --i;
    part->count = symnum + buckets[i] + 2;
    part->size = part->count * part->entsize;
    hash = palloc(part);
    hash[0] = buckets[i];
    hash[1] = symnum;
    memset(hash + 2, 0, (buckets[i] + symnum) * sizeof *hash);

    part->done = TRUE;
    (void)bp;
}

/* Create the hash table.
 */
static void complete(elfpart *part, blueprint const *bp)
{
    Elf32_Word		       *chain = part->part;
    Elf32_Sym		       *sym;
    unsigned char const	       *strtab;
    unsigned char const	       *name;
    Elf32_Word			bucketnum;
    Elf32_Word			hash;
    Elf32_Sword			n;
    int				i;

    if (!part->link->done)
	return;

    sym = part->link->part;
    strtab = part->link->link->part;
    bucketnum = chain[0];
    chain += 2 + bucketnum;
    for (i = 1, ++sym ; i < part->link->count ; ++i, ++sym) {
	name = strtab + sym->st_name;
	for (hash = 0 ; *name ; ++name) {
	    hash = (hash << 4) + *name;
	    hash = (hash ^ ((hash & 0xF0000000) >> 24)) & 0x0FFFFFFF;
	}
	hash = hash % bucketnum;
	n = hash - bucketnum;
	while (chain[n])
	    n = chain[n];
	chain[n] = i;
    }

    part->done = TRUE;
    (void)bp;
}

/* The hash elfpart structure.
 */
elfpart	part_hash = { new, NULL, fill, complete };
