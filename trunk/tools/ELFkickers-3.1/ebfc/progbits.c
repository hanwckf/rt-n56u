/* progbits.c: parts containing segments of the program.
 *
 * Copyright (C) 1999-2001 by Brian Raiter, under the GNU General
 * Public License. No warranty. See COPYING for details.
 */

#include	<stdlib.h>
#include	<elf.h>
#include	"elfparts.h"
#include	"gen.h"

/* Generic setup of the elfpart structure.
 */
static void new(elfpart *part)
{
    part->shtype = SHT_PROGBITS;
}

/* Set up the elfpart structure for a text segment.
 */
static void textnew(elfpart *part)
{
    new(part);
    part->flags = PF_R | PF_X;
    part->shname = ".text";
}

/* Set up the elfpart structure for a data segment.
 */
static void datanew(elfpart *part)
{
    new(part);
    part->flags = PF_R | PF_W;
    part->shname = ".data";
}

/* Set up the elfpart structure for a bss segment.
 */
static void bssnew(elfpart *part)
{
    datanew(part);
    part->shtype = SHT_NOBITS;
    part->shname = ".bss";
}

/* The progbits elfpart structures.
 */
elfpart part_progbits = { new, NULL, NULL, NULL };
elfpart	part_text = { textnew, NULL, NULL, NULL };
elfpart	part_data = { datanew, NULL, NULL, NULL };
elfpart part_bss = { bssnew, NULL, NULL, NULL };
