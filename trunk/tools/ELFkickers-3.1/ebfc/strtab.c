/* strtab.c: parts containing a string table.
 *
 * Copyright (C) 1999-2001 by Brian Raiter, under the GNU General
 * Public License. No warranty. See COPYING for details.
 */

#include	<stdlib.h>
#include	<elf.h>
#include	<string.h>
#include	"elfparts.h"
#include	"gen.h"

/* Set up the elfpart structure and allocate an empty string table.
 */
static void new(elfpart *part)
{
    part->shtype = SHT_STRTAB;
    part->shname = ".strtab";
    part->size = 1;
    palloc(part);
    *(char*)part->part = '\0';
}

/* Set up the elfpart structure for a section header string table.
 */
static void shnew(elfpart *part)
{
    new(part);
    part->shname = ".shstrtab";
}

/* Set up the elfpart structure for a dynamic string table.
 */
static void dynnew(elfpart *part)
{
    new(part);
    part->shname = ".dynstr";
    part->flags = PF_R;
}

/* The string table elfpart structures.
 */
elfpart	part_strtab   = { new, NULL, NULL, NULL },
	part_shstrtab = { shnew, NULL, NULL, NULL },
	part_dynstr   = { dynnew, NULL, NULL, NULL };


/* Adds a string to a string table. The return value is the index
 * of the string in the table.
 */
int addtostrtab(elfpart *part, char const *str)
{
    char       *strtab;
    size_t	pos;

    assert(part->shtype == SHT_STRTAB);

    if (!str || !*str)
	return 0;
    pos = part->size;
    part->size += strlen(str) + 1;
    strtab = palloc(part);
    strcpy(strtab + pos, str);
    return pos;
}
