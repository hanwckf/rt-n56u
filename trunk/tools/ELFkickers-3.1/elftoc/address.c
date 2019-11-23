/* address.c: Functions for identifying and handling memory addresses.
 * Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "gen.h"
#include "outbase.h"
#include "outitems.h"
#include "address.h"

/* Information collected on each address segment.
 */
struct addrinfo {
    long   address;	/* the address of a segment */
    long   from;	/* the lowest address used in this segment */
    long   to;	 	/* the highest address used in this segment */
    int    count;	/* how many addresses are in this segment */
    int    maxchunk;	/* size of the largest single chunk in this segment */
    char   name[64];	/* the preprocessor name assigned to this segment */
};

/* The list of memory segments.
 */
static struct addrinfo *addrs = NULL;
static int addrcount = 0;

/* Adds an address to the collection. addr is the address, offset is
 * the (presumed) offset within the memory segment, size is the size
 * of the chunk of memory starting at that address, and name is the
 * name associated with that chunk. The largest chunk in that memory
 * address will determine which name is used to name the memory
 * segment.
 */
void recordaddress(long address, long offset, long size, char const *name)
{
    static int allocated = 0;
    long base;
    int i;

    base = address - offset;
    for (i = 0 ; i < addrcount ; ++i)
	if (addrs[i].address == base)
	    break;
    if (i == addrcount) {
	if (addrcount == allocated) {
	    allocated = allocated ? 2 * allocated : 4;
	    addrs = reallocate(addrs, allocated * sizeof *addrs);
	    memset(addrs + addrcount, 0,
		   (allocated - addrcount) * sizeof *addrs);
	}
	++addrcount;
	addrs[i].address = base;
    }
    ++addrs[i].count;
    if (!*addrs[i].name || addrs[i].maxchunk < size) {
	strcpy(addrs[i].name, name);
	addrs[i].maxchunk = size;
    }
    if (!addrs[i].from || addrs[i].from > address)
	addrs[i].from = address;
    if (addrs[i].to < address + size)
	addrs[i].to = address + size;
}

/* Finalizes the list of addresses. This function weeds through the
 * collection of memory segments and picks out the ones that are
 * likely to be real (i.e., the ones that are used in more than one
 * place and/or begin on a page boundary). These addresses are then
 * assigned unique names.
 */
void setaddressnames(void)
{
    char *str;
    int len;
    int i, j, n;

    for (i = 0 ; i < addrcount ; ) {
	if (addrs[i].count == 1 && (addrs[i].address & 0x0FFF) > 0) {
	    addrs[i] = addrs[addrcount];
	    --addrcount;
	} else {
	    ++i;
	}
    }

    for (i = 0 ; i < addrcount ; ++i) {
	str = addrs[i].name;
	while (!isalnum(*str))
	    ++str;
	memmove(addrs[i].name + 5, str, strlen(str) + 1);
	memcpy(addrs[i].name, "ADDR_", 5);
	for (str = addrs[i].name + 5 ; *str ; ++str)
	    *str = isalnum(*str) ? toupper(*str) : '_';
    }
    for (i = 0 ; i < addrcount - 1 ; ++i) {
	n = 1;
	len = strlen(addrs[i].name);
	for (j = i + 1 ; j < addrcount ; ++j)
	    if (!memcmp(addrs[j].name, addrs[i].name, len + 1))
		sprintf(addrs[j].name + len, "%d", ++n);
	if (n > 1)
	    strcpy(addrs[i].name + len, "1");
    }
}

/* Returns the name and base address of the segment that contains the
 * given address.
 */
char const *getbaseaddress(long address, long *base)
{
    struct addrinfo const *candidate;
    int i;

    candidate = NULL;
    for (i = 0 ; i < addrcount ; ++i) {
	if (!candidate || addrs[i].address > candidate->address)
	    if (addrs[i].from <= address && address < addrs[i].to)
		candidate = addrs + i;
    }
    if (!candidate) {
	for (i = 0 ; i < addrcount ; ++i) {
	    if (address == addrs[i].to) {
		candidate = addrs + i;
		break;
	    }
	}
    }
    if (candidate) {
	if (base)
	    *base = candidate->address;
	return candidate->address ? candidate->name : "";
    } else {
	if (base)
	    *base = 0;
	return NULL;
    }
}

/* Translates a memory address back into a file offset. If the address
 * does not fall within any of the recorded memory segments, it is
 * assumed to already be a valid offset.
 */
long getaddressoffset(long address)
{
    int i;

    for (i = 0 ; i < addrcount ; ++i)
	if (addrs[i].from <= address && address < addrs[i].to)
	    return address - addrs[i].address;
    return address;
}

/* Outputs the list of recorded addresses as a series of macro
 * definitions.
 */
void outputaddresses(void)
{
    int width;
    int n, i;

    width = 0;
    for (i = 0 ; i < addrcount ; ++i) {
	if (addrs[i].address && addrs[i].name) {
	    n = strlen(addrs[i].name);
	    if (width < n)
		width = n;
	}
    }
    if (width == 0)
	return;
    for (i = 0 ; i < addrcount ; ++i)
	if (addrs[i].address && addrs[i].name)
	    outf("#define %-*s 0x%08lX\n",
		 width, addrs[i].name, addrs[i].address);
    out("\n");
}
