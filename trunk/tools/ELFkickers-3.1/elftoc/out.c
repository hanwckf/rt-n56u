/* out.c: Top-level (and bitness-neutral) output functions.
 * Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include <stdio.h>
#include <elf.h>
#include "gen.h"
#include "readelf.h"
#include "pieces.h"
#include "address.h"
#include "shdrtab.h"
#include "outbase.h"
#include "outitems.h"
#include "outelf64.h"
#include "outelf32.h"
#include "out.h"

/* The output function for a piece not actually present in the ELF
 * file image.
 */
static void outnothing(void const *ptr, long size, int ndx)
{
    (void)ptr;
    (void)size;
    (void)ndx;
    out("{ }");
}

/* The output function for pieces of type P_UNCLAIMED, P_SECTION,
 * P_BYTES, or P_STRINGS. The contents are output either as a literal
 * string, an array of character values, or an array of hexadecimal
 * byte values. The last will be used if the contents contain an
 * excess of non-graphic, non-ASCII characters. Otherwise, one of the
 * first two representations will be selected based on whether or not
 * the contents appear to be NUL-terminated.
 */
static void outbytes(void const *ptr, long size, int ndx)
{
    unsigned char const *bytes = ptr;
    long zeroes, n;
    long i;

    (void)ndx;
    for (zeroes = 0 ; zeroes < size && !bytes[size - zeroes - 1] ; ++zeroes) ;
    if (zeroes == size) {
	out("{ 0 }");
	return;
    }
    if (zeroes < 255 || zeroes * 4 < size)
	zeroes = 0;
    zeroes &= ~7;

    n = outstringsize((char const*)bytes, size);

    if (n * 2 > size * 3) {
	beginblock(TRUE);
	for (i = 0 ; i < size - zeroes ; ++i)
	    outf("0x%02X", bytes[i]);
	if (zeroes)
	    outcomment(strf("0x00 x %d", zeroes));
	endblock();
    } else if (zeroes || bytes[size - 1]) {
	beginblock(TRUE);
	for (i = 0 ; i < size - zeroes ; ++i)
	    outchar(bytes[i]);
	if (zeroes)
	    outcomment(strf("0x00 x %ld", zeroes));
	endblock();
    } else {
	outstring((char const*)bytes, size - 1);
    }
}

/* The output function for an array of 16-bit values. The values are
 * printed in hexadecimal unless they are all relatively small.
 */
static void outhalves(void const *ptr, long size, int ndx)
{
    Elf64_Half const *halves = ptr;
    long count = size / sizeof *halves;
    char fmtbuf[16];
    char const *fmt = NULL;
    unsigned int max;
    long zeroes, i;

    (void)ndx;
    for (zeroes = 0 ; zeroes < count ; ++zeroes)
	if (halves[count - zeroes - 1])
	    break;
    if (zeroes == count) {
	out("{ 0 }");
	return;
    }
    if (zeroes < 255 || zeroes * 4 < count)
	zeroes = 0;
    zeroes &= ~3;
    count -= zeroes;

    max = 0;
    for (i = 0 ; i < count ; ++i) {
	if (max < halves[i])
	    max = halves[i];
    }
    fmt = max > 0xFF ? "0x%04X" : max > 99 ? "0x%02X" : "%u";

    beginblock(TRUE);
    for (i = 0 ; i < count ; ++i)
	outf(fmt, halves[i]);
    if (zeroes) {
	sprintf(fmtbuf, "%s x %%ld", fmt);
	outcomment(strf(fmtbuf, 0, zeroes));
    }
    endblock();
}

/* The output function for an array of 32-bit values.
 */
static void outwords(void const *ptr, long size, int ndx)
{
    Elf64_Word const *words = ptr;
    long count = size / sizeof *words;
    char fmtbuf[16];
    char const *fmt = NULL;
    unsigned long max;
    long zeroes, i;

    (void)ndx;
    for (zeroes = 0 ; zeroes < count ; ++zeroes)
	if (words[count - zeroes - 1])
	    break;
    if (zeroes == count) {
	out("{ 0 }");
	return;
    }
    if (zeroes < 255 || zeroes * 4 < count)
	zeroes = 0;
    zeroes &= ~3;
    count -= zeroes;

    max = 0;
    for (i = 0 ; i < count ; ++i) {
	if (max < words[i])
	    max = words[i];
    }
    fmt = max > 0xFFFF ? "0x%08X" :
	  max > 0x00FF ? "0x%04X" : max > 99 ? "0x%02X" : "%u";

    beginblock(TRUE);
    for (i = 0 ; i < count ; ++i)
	outf(fmt, words[i]);
    if (zeroes) {
	sprintf(fmtbuf, "%s x %%ld", fmt);
	outcomment(strf(fmtbuf, 0, zeroes));
    }
    endblock();
}

/* The output function for an array of 64-bit values.
 */
static void outxwords(void const *ptr, long size, int ndx)
{
    Elf64_Xword const *xwords = ptr;
    long count = size / sizeof *xwords;
    char fmtbuf[16];
    char const *fmt = NULL;
    unsigned long max;
    long zeroes, i;

    (void)ndx;
    for (zeroes = 0 ; zeroes < count ; ++zeroes)
	if (xwords[count - zeroes - 1])
	    break;
    if (zeroes == count) {
	out("{ 0 }");
	return;
    }
    if (zeroes < 255 || zeroes * 4 < count)
	zeroes = 0;
    zeroes &= ~3;
    count -= zeroes;

    max = 0;
    for (i = 0 ; i < count ; ++i) {
	if (max < xwords[i])
	    max = xwords[i];
    }
    fmt = max > 0xFFFFFFFF ? "0x%016lX" :
	  max > 0x0000FFFF ? "0x%08lX"  :
	  max > 0x000000FF ? "0x%04lX"  : max > 99 ? "0x%02lX" : "%lu";

    beginblock(TRUE);
    for (i = 0 ; i < count ; ++i)
	outf(fmt, xwords[i]);
    if (zeroes) {
	sprintf(fmtbuf, "%s x %%ld", fmt);
	outcomment(strf(fmtbuf, 0, zeroes));
    }
    endblock();
}

/* The output function for an ELF hash table. The hash table is an
 * array of 32-bit values. The first two array elements indicate the
 * length of the following subsections. Line breaks are inserted to
 * indicate where the subsections start and end.
 */
static void outhashtable(void const *ptr, long size, int ndx)
{
    Elf64_Word const *entries = ptr;
    long count = size / sizeof *entries;
    long i;

    (void)ndx;
    beginblock(TRUE);
    if (count <= 2 || (long)(entries[0] + entries[1] + 2) != count) {
	for (i = 0 ; i < count ; ++i)
	    outdec(entries[i]);
    } else {
	outdec(entries[0]);
	outdec(entries[1]);
	linebreak();
	i = 2;
	if (entries[0] > 0) {
	    for ( ; i < (long)(2 + entries[0]) ; ++i)
		outdec(entries[i]);
	    linebreak();
	}
	for ( ; i < count ; ++i)
	    outdec(entries[i]);
    }
    endblock();
}

/* The GNU hash table is an array of 32-bit values. (Although in a
 * 64-bit ELF file the mask subpart consists of 64-bit values. But an
 * array cannot change its type midway through, so in this case the
 * mask values are split across two array entries.)
 */
static void outgnuhash(void const *ptr, long size, int ndx)
{
    Elf64_Word const *entries = ptr;
    long count = size / sizeof *entries;
    int buckets, masks;
    int i, n;

    (void)ndx;
    if (count < 4) {
	outwords(ptr, size, ndx);
	return;
    }
    buckets = entries[0];
    masks = entries[2];
    if (iself64())
	masks *= 2;
    if (4 + masks + buckets > count) {
	outwords(ptr, size, ndx);
	return;
    }

    beginblock(TRUE);
    for (i = 0 ; i < 4 ; ++i)
	outdec(entries[i]);
    n = i;
    if (masks) {
	linebreak();
	for (i = 0 ; i < masks ; ++i)
	    outf("0x%08lX", entries[n + i]);
	n += i;
    }
    if (buckets) {
	linebreak();
	for (i = 0 ; i < buckets ; ++i)
	    outdec(entries[n + i]);
	n += i;
    }
    if (n < count) {
	linebreak();
	for (i = n ; i < count ; ++i)
	    outf("0x%08lX", entries[i]);
    }
    endblock();
}

/* The output function for note sections. The format of a note section
 * varies with the type of note, the only constant part being that it
 * starts with an Elf*_Nhdr struct, and is usually followed by a
 * string. However, each subpart is guaranteed to be a multiple of 4
 * bytes, so notes are displayed as arrays of 32-bit values. Line
 * breaks are inserted to indicate the beginning of a note header.
 * Note sections in core files are rather different, so in that case a
 * completely different function is called instead.
 */
void outnote(void const *ptr, long size, int ndx)
{
    Elf64_Word const *words = ptr;
    long count = size / sizeof *words;
    long i;
    int namesize, descsize, tag;

    (void)ndx;
    if (iscorefile()) {
	iself64() ? outnote64(ptr, size, ndx)
		  : outnote32(ptr, size, ndx);
	return;
    }

    beginblock(TRUE);
    i = 0;
    while (i < count) {
	namesize = (words[i] + 3) / 4;
	descsize = (words[i + 1] + 3) / 4;
	linebreak();
	outdec(words[i++]);
	outdec(words[i++]);
	tag = words[i++];
	outdefint(tag, "NT_GNU_");
	while (namesize-- && i < count)
	    outhex(words[i++]);
	if (descsize == 0)
	    continue;
#ifdef NT_GNU_ABI_TAG
	if (tag == NT_GNU_ABI_TAG) {
	    outdefint(words[i++], "ELF_NOTE_OS_");
	    --descsize;
	}
#endif
	while (descsize-- && i < count)
	    outhex(words[i++]);
    }
    endblock();
}

/* The output functions for each type of piece in a 32-bit ELF file.
 */
static void (*outputfunctions32[P_COUNT])(void const *, long, int) = {
    outbytes,		/* P_UNCLAIMED */  
    outbytes,		/* P_SECTION */	   
    outbytes,		/* P_BYTES */	   
    outhalves,		/* P_HALVES */	   
    outwords,		/* P_WORDS */	   
    outxwords,		/* P_XWORDS */	   
    outaddr32,		/* P_ADDRS */	   
    outbytes,		/* P_STRINGS */	   
    outnote,		/* P_NOTE */	   
    outhashtable,	/* P_HASH */
    outgnuhash,		/* P_GNUHASH */	   
    outsym32,		/* P_SYMTAB */	   
    outsyminfo32,	/* P_SYMINFO */
    outrel32,		/* P_REL */	   
    outrela32,		/* P_RELA */	   
    outmove32,		/* P_MOVE */	   
    outdyn32,		/* P_DYNAMIC */	   
    outshdr32,		/* P_SHDRTAB */	   
    outphdr32,		/* P_PHDRTAB */	   
    outehdr32,		/* P_EHDR */	   
    outnothing		/* P_NONEXISTENT */
};

/* The output functions for each type of piece in a 64-bit ELF file.
 */
static void (*outputfunctions64[P_COUNT])(void const*, long, int) = {
    outbytes,		/* P_UNCLAIMED */  
    outbytes,		/* P_SECTION */	   
    outbytes,		/* P_BYTES */	   
    outhalves,		/* P_HALVES */	   
    outwords,		/* P_WORDS */	   
    outxwords,		/* P_XWORDS */	   
    outaddr64,		/* P_ADDRS */	   
    outbytes,		/* P_STRINGS */	   
    outnote,		/* P_NOTE */	   
    outhashtable,	/* P_HASH */	   
    outgnuhash,		/* P_GNUHASH */	   
    outsym64,		/* P_SYMTAB */	   
    outsyminfo64,	/* P_SYMINFO */	   
    outrel64,		/* P_REL */	   
    outrela64,		/* P_RELA */	   
    outmove64,		/* P_MOVE */	   
    outdyn64,		/* P_DYNAMIC */	   
    outshdr64,		/* P_SHDRTAB */	   
    outphdr64,		/* P_PHDRTAB */	   
    outehdr64,		/* P_EHDR */	   
    outnothing		/* P_NONEXISTENT */
};

/* Calls the appropriate output function for the indicated type.
 */
void outtypedblock(int type, long offset, long size, int ndx)
{
    void const *ptr;

    ptr = getptrto(offset, &size);
    if (iself64())
	outputfunctions64[type](ptr, size, ndx);
    else
	outputfunctions32[type](ptr, size, ndx);
}

/* Outputs everything, from start to finish.
 */
void output(void)
{
    out("#include <stddef.h>\n");
    out("#include <elf.h>\n");
    out("\n");
    outputaddresses();
    outputenum();
    outputstruct();
    outputpieces();
}
