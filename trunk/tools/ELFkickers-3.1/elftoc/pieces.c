/* pieces.c: Functions for handling the file as a sequence of pieces.
 * Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include "gen.h"
#include "readelf.h"
#include "shdrtab.h"
#include "outbase.h"
#include "outitems.h"
#include "out.h"
#include "pieces.h"

/* Flags used to insert warnings in the generated source code.
 */
#define	PW_MISALIGNED	0x0001	/* piece misaligned for original type */
#define	PW_WRONGSIZE	0x0002	/* incorrect size for original type */

/* A struct representing an area in the file image.
 */
struct area { long offset, size; };

/* The information associated with each file piece.
 */
struct pieceinfo {
    long	from;		/* offset of start of piece */
    long	to;		/* one past end of piece */
    long	size;		/* (to - from) */
    short	type;		/* the piece type */
    short	badtype;	/* warning flags */
    int		align;		/* the piece's required alignment */
    int		shndx;		/* associated shdr index */
    struct area strtable;	/* an associated string table */
    struct area misctable;	/* an associated non-string table */
    char	name[64];	/* the piece's field name */
};

/* Information describing a C type.
 */
struct typeinfo {
    char const *name;		/* the name of the C type */
    int		size;		/* the type's size */
};

/* The names of the notional structure in the generated source code.
 */
static char const *structname = "elf";
static char const *varname = "foo";

#define	_(str) #str, sizeof(str)

/* The C types that go with each type of piece, for 32-bit ELF and
 * 64-bit ELF.
 */
static struct typeinfo const ctypes32[P_COUNT] = {
    { _(unsigned char) },	/* P_UNCLAIMED */
    { _(unsigned char) },	/* P_SECTION */
    { _(unsigned char) },	/* P_BYTES */
    { _(Elf32_Half) },		/* P_HALVES */
    { _(Elf32_Word) },		/* P_WORDS */
    { _(uint64_t) },		/* P_XWORDS */
    { _(Elf32_Addr) },		/* P_ADDRS */
    { _(char) },		/* P_STRINGS */
    { _(Elf32_Word) },		/* P_NOTE */
    { _(Elf32_Word) },		/* P_HASH */
    { _(Elf32_Word) },		/* P_GNUHASH */
    { _(Elf32_Sym) },		/* P_SYMTAB */
    { _(Elf32_Syminfo) },	/* P_SYMINFO */
    { _(Elf32_Rel) },		/* P_REL */
    { _(Elf32_Rela) },		/* P_RELA */
    { _(Elf32_Move) },		/* P_MOVE */
    { _(Elf32_Dyn) },		/* P_DYNAMIC */
    { _(Elf32_Shdr) },		/* P_SHDRTAB */
    { _(Elf32_Phdr) },		/* P_PHDRTAB */
    { _(Elf32_Ehdr) },		/* P_EHDR */
    { _(unsigned char) }	/* P_NONEXISTENT */
};

static struct typeinfo const ctypes64[P_COUNT] = {
    { _(unsigned char) },	/* P_UNCLAIMED */
    { _(unsigned char) },	/* P_SECTION */
    { _(unsigned char) },	/* P_BYTES */
    { _(Elf64_Half) },		/* P_HALVES */
    { _(Elf64_Word) },		/* P_WORDS */
    { _(Elf64_Xword) },		/* P_XWORDS */
    { _(Elf64_Addr) },		/* P_ADDRS */
    { _(char) },		/* P_STRINGS */
    { _(Elf64_Word) },		/* P_NOTE */
    { _(Elf64_Word) },		/* P_HASH */
    { _(Elf64_Word) },		/* P_GNUHASH */
    { _(Elf64_Sym) },		/* P_SYMTAB */
    { _(Elf64_Syminfo) },	/* P_SYMINFO */
    { _(Elf64_Rel) },		/* P_REL */
    { _(Elf64_Rela) },		/* P_RELA */
    { _(Elf64_Move) },		/* P_MOVE */
    { _(Elf64_Dyn) },		/* P_DYNAMIC */
    { _(Elf64_Shdr) },		/* P_SHDRTAB */
    { _(Elf64_Phdr) },		/* P_PHDRTAB */
    { _(Elf64_Ehdr) },		/* P_EHDR */
    { _(unsigned char) }	/* P_NONEXISTENT */
};

#undef _

/* Points to ctypes32 or ctypes64, depending on the input file.
 */
static struct typeinfo const *ctypes;

/* The collection of file pieces.
 */
static struct pieceinfo *pieces = NULL;
static int piececount = 0;

static int add_end = 0;			/* force _end to appear/not appear */
static int needalignment = TRUE;	/* ignore type on misaligned fields */

/* Forces the _end field to be included or excluded.
 */
void include_end(int include)
{
    add_end = include ? +1 : -1;
}

/* Causes misaligned fields in the struct to be output in their
 * original form, instead of forcing them to be represented as byte
 * arrays.
 */
void permitmisalignment(void)
{
    needalignment = FALSE;
}

/* Sets the name of the struct to use in the generated source.
 */
void setstructname(char const *name)
{
    structname = name;
}

/* Sets the name of the variable to use in the generated source.
 */
void setvarname(char const *name)
{
    varname = name;
}

/* Records an area of the file as being a unique piece. offset and
 * size identify the area of the file, type specifies the type of the
 * contents at that piece, align gives the field's alignment
 * requirements, if any, and name supplies a suggested name for that
 * piece, to be used as the field name for the structure definition in
 * the generated C code. A tilde prefixed to the name indicates that
 * the suggestion is a weak one, and other suggestions should take
 * precedence. The return value is the piece's index.
 */
int recordpiece(long offset, long size, int type, char const *name, int align)
{
    static int allocated = 0;

    if (offset < 0)
	return -1;
    if (type != P_NONEXISTENT) {
	(void)getptrto(offset, &size);
	if (size <= 0)
	    return -1;
    }

    if (piececount == allocated) {
	allocated = allocated ? 2 * allocated : 16;
	pieces = reallocate(pieces, allocated * sizeof *pieces);
    }
    memset(&pieces[piececount], 0, sizeof *pieces);
    pieces[piececount].from = offset;
    pieces[piececount].to = offset + size;
    pieces[piececount].type = type;
    pieces[piececount].align = align;
    sprintf(pieces[piececount].name, "%-.*s", (int)sizeof pieces->name - 1,
				     name && *name ? name : "~section");
    return piececount++;
}

/* Associates a section index with the given piece.
 */
void setpieceshndx(int ndx, int shndx)
{
    pieces[ndx].shndx = shndx;
}

/* Associates a string table location with the given piece.
 */
void setpiecestrtable(int ndx, long offset, long size)
{
    pieces[ndx].strtable.offset = offset;
    pieces[ndx].strtable.size = size;
}

/* Associates a table of any type with the given piece.
 */
void setpiecemisctable(int ndx, long offset, long size)
{
    pieces[ndx].misctable.offset = offset;
    pieces[ndx].misctable.size = size;
}

/* Retrieves the section index associated with the given piece.
 */
int getpieceshndx(int ndx)
{
    return pieces[ndx].shndx;
}

/* Retrieves the string table location associated with the given
 * piece.
 */
void const *getpiecestrtable(int ndx, long *size)
{
    *size = pieces[ndx].strtable.size;
    return *size == 0 ? NULL : getptrto(pieces[ndx].strtable.offset, size);
}

/* Retrieves the location of the unspecified table associated with the
 * given piece.
 */
void const *getpiecemisctable(int ndx, long *size)
{
    *size = pieces[ndx].misctable.size;
    return *size == 0 ? NULL : getptrto(pieces[ndx].misctable.offset, size);
}

/* Deletes one of the members of the pieces array.
 */
static void removepiece(struct pieceinfo *piece)
{
    int i;

    i = piece - pieces;
    --piececount;
    if (i < piececount)
	memmove(piece, piece + 1, (piececount - i) * sizeof *pieces);
}

/* An ordering function for arranging pieces by their starting
 * position.
 */
static int piececmp(const void *p1, const void *p2)
{
    int n;

    n = ((struct pieceinfo*)p1)->from - ((struct pieceinfo*)p2)->from;
    if (!n)
	n = ((struct pieceinfo*)p2)->to - ((struct pieceinfo*)p1)->to;
    return n;
}

/* Takes two overlapping pieces and alters the second piece so that it
 * no longer overlaps the first. The second piece will be either
 * reduced in size, split in two, or removed entirely. The return
 * value is true if the pieces array needs to be resorted afterwards.
 */
static int removeoverlap(struct pieceinfo *p1, struct pieceinfo *p2)
{
    char buf[64];
    long size;

    if (p1->from <= p2->from && p1->to >= p2->to) {
	if (p1->strtable.size == 0 && p2->strtable.size > 0)
	    p1->strtable = p2->strtable;
	if (p1->misctable.size == 0 && p2->misctable.size > 0)
	    p1->misctable = p2->misctable;
	removepiece(p2);
	return FALSE;
    } else if (p1->from <= p2->from && p1->to < p2->to) {
	p2->from = p1->to;
	return TRUE;
    } else if (p1->from > p2->from && p1->to >= p2->to) {
	p2->to = p1->from;
	return FALSE;
    } else if (p1->from > p2->from && p1->to < p2->to) {
	size = p2->to - p1->to;
	strcpy(buf, p2->name);
	p2->to = p1->from;
	recordpiece(p1->to, size, p2->type, buf, p2->align);
	return TRUE;
    }
    return FALSE;
}

/* Takes two pieces with shared area and determines which piece needs
 * to be modified in order to remove the overlap. The return value is
 * true if the pieces array needs to be resorted as a result.
 */
static int disentangle(struct pieceinfo *p1, struct pieceinfo *p2)
{
    if (*p1->name != '~' && *p2->name == '~')
	return removeoverlap(p1, p2);
    else if (*p1->name == '~' && *p2->name != '~')
	return removeoverlap(p2, p1);
    else if (p1->type > p2->type)
	return removeoverlap(p1, p2);
    else if (p1->type < p2->type)
	return removeoverlap(p2, p1);
    else
	return removeoverlap(p1, p2);
}

/* Arranges the collection of pieces in order of appearance within the
 * file, and removes redundancies and overlapping areas. The end
 * results is that the collection of pieces will be mutually exclusive
 * and (presuming that every byte in the file has been assigned to at
 * least one piece) collectively exhaustive. Finally, the function
 * optionally adds a trailing field to account for padding in the C
 * structure.
 */
void arrangepieces(void)
{
    int count = 0;
    int i, n;

    n = 0;
    for (;;) {
	++count;
	if (n == 0)
	    qsort(pieces, piececount, sizeof *pieces, piececmp);
	for (i = 0 ; i < piececount - 1 ; ++i)
	    if (pieces[i].to > pieces[i + 1].from)
		break;
	if (i == piececount - 1)
	    break;
	n = i - 1;
	if (disentangle(&pieces[i], &pieces[i + 1]))
	    n = 0;
    }

    if (add_end >= 0) {
	n = pieces[piececount - 1].to % sizeof_elf(Addr);
	if (n > 0)
	    n = sizeof_elf(Addr) - n;
	if (n > 0 || add_end > 0)
	    recordpiece(pieces[piececount - 1].to, n, P_NONEXISTENT,
			"_end", 0);
    }

    for (i = 0 ; i < piececount ; ++i)
	pieces[i].size = pieces[i].to - pieces[i].from;
}

/* Checks the offset and sizing of pieces that contain arrays of
 * anything other than simple characters. If either of these values is
 * inconsistent with the alignment requirements of the field's type,
 * the piece is marked as being invalid for its type.
 */
void verifypiecetypes(void)
{
    struct typeinfo const *type;
    int	n, i;

    ctypes = iself64() ? ctypes64 : ctypes32;

    for (i = 0 ; i < piececount ; ++i) {
	type = ctypes + pieces[i].type;
	n = type->size;
	if (n <= 1)
	    continue;
	if (n > ctypes[P_ADDRS].size || (n & (n - 1)))
	    n = ctypes[P_ADDRS].size;
	if (pieces[i].from % n)
	    pieces[i].badtype |= PW_MISALIGNED;
	if (pieces[i].size % type->size)
	    pieces[i].badtype |= PW_WRONGSIZE;
	if (pieces[i].type == P_EHDR && pieces[i].size != type->size)
	    pieces[i].badtype |= PW_WRONGSIZE;
    }
}

/* Forces each piece to have a unique name. Tilde prefixes are
 * removed, and conflicting names are given numerical suffixes. This
 * is also where padding sections are identified and renamed.
 */
void setpiecenames(void)
{
    int padsize;
    int suffix, len, i, j;

    padsize = sizeof_elf(Addr);
    for (i = 0 ; i < piececount ; ++i) {
	if (*pieces[i].name == '~') {
	    memmove(pieces[i].name, pieces[i].name + 1,
		    sizeof pieces->name - 1);
	    if (pieces[i].type == P_SECTION && strcmp(pieces[i].name, "pad")) {
		if (pieces[i].size < padsize && pieces[i].to % padsize == 0) {
		    strcpy(pieces[i].name, "pad");
		} else if (i + 1 < piececount) {
		    len = pieces[i + 1].align;
		    if (pieces[i].size < len && pieces[i].to % len == 0)
			strcpy(pieces[i].name, "pad");
		}
	    }
	}
    }

    for (i = 0 ; i < piececount - 1 ; ++i) {
	suffix = 1;
	len = strlen(pieces[i].name);
	for (j = i + 1 ; j < piececount ; ++j)
	    if (!strcmp(pieces[i].name, pieces[j].name))
		sprintf(pieces[j].name + len, "%d", ++suffix);
	if (suffix > 1)
	    strcpy(pieces[i].name + len, "1");
    }
}

/* Finds the piece associated with the given section index.
 */
int getindexfromsection(int shndx)
{
    int i;

    for (i = 0 ; i < piececount ; ++i)
	if (pieces[i].shndx == shndx)
	    return i;
    return -1;
}

/* Finds the piece that contains the byte at the given offset.
 */
int getindexfromoffset(long offset)
{
    int i;

    for (i = 0 ; i < piececount ; ++i)
	if (pieces[i].from <= offset && offset < pieces[i].to)
	    return i;
    return -1;
}

/* Returns a string representation of a file offset value that, if
 * possible, is expressed as an offset into the C structure. ndx is
 * the index of the piece located at that offset, typically obtained
 * by calling getindexfromoffset() first.
 */
char const *getoffsetstr(long offset, int ndx)
{
    static char	buf[128];

    if (!offset)
	return "0";

    if (ndx < 0 || ndx >= piececount) {
	if (offset == pieces[piececount - 1].to) {
	    sprintf(buf, "sizeof(%s)", structname);
	} else if (offset > pieces[piececount - 1].to) {
	    sprintf(buf, "sizeof(%s) + %s",
		    structname, strhex(offset - pieces[piececount - 1].to));
	} else {
	    strcpy(buf, strhex(offset));
	}
	return buf;
    }

    if (pieces[ndx].from == offset) {
	sprintf(buf, "offsetof(%s, %s)", structname, pieces[ndx].name);
    } else {
	offset -= pieces[ndx].from;
	if (ctypes[pieces[ndx].type].size > 1 &&
			offset % ctypes[pieces[ndx].type].size == 0) {
	    sprintf(buf, "offsetof(%s, %s[%ld])",
			 structname, pieces[ndx].name,
			 offset / ctypes[pieces[ndx].type].size);
	} else if (pieces[ndx].from > 0) {
	    sprintf(buf, "offsetof(%s, %s) + %s",
			 structname, pieces[ndx].name, strhex(offset));
	} else {
	    strcpy(buf, strhex(offset));
	}
    }
    return buf;
}

/* Returns a string representation of a size value that, if possible,
 * is represented as a sizeof some part of the C structure. If ndx is
 * not negative, it supplies a piece index that is hoped to be
 * associated with the given size.
 */
char const *getsizestr(long size, int ndx)
{
    static char	buf[256];
    struct pieceinfo const *piece;
    int i;

    if (!size)
	return "0";
    if (ndx < 0)
	return strdec(size);

    piece = pieces + ndx;

    if (size == piece->size) {
	sprintf(buf, "sizeof %s.%s", varname, piece->name);
	return buf;
    }

    if (size < piece->size) {
	if (ndx > 0 && ctypes[piece->type].size > 1) {
	    if (size == ctypes[piece->type].size) {
		sprintf(buf, "sizeof *%s.%s", varname, piece->name);
		return buf;
	    } else if (size % ctypes[piece->type].size == 0) {
		sprintf(buf, "sizeof *%s.%s * %ld",
			     varname, piece->name,
			     size / ctypes[piece->type].size);
		return buf;
	    }
	}
	if (size > 8 && piece->size % size == 0) {
	    sprintf(buf, "sizeof %s.%s / %ld",
			 varname, piece->name, piece->size / size);
	    return buf;
	}
	if (piece->size - size < piece->size / 8) {
	    sprintf(buf, "sizeof %s.%s - %ld",
			 varname, piece->name, piece->size - size);
	    return buf;
	}
	sprintf(buf, "%ld", size);
	return buf;
    }

    if (size == pieces[piececount - 1].to) {
	sprintf(buf, "sizeof(%s)", structname);
	return buf;
    } else if (size > pieces[piececount - 1].to) {
	sprintf(buf, "sizeof(%s) + %ld",
		structname, size - pieces[piececount - 1].to);
	return buf;
    } else if (size == pieces[piececount - 1].to - piece->from) {
	sprintf(buf, "sizeof(%s) - offsetof(%s, %s)",
		structname, structname, piece->name);
	return buf;
    } else if (size > pieces[piececount - 1].to - piece->from) {
	sprintf(buf, "%ld", size);
	return buf;
    }
    for (i = ndx + 1 ; i < piececount ; ++i) {
	if (size == pieces[i].from - piece->from) {
	    if (piece->from) {
		sprintf(buf, "offsetof(%s, %s) - offsetof(%s, %s)",
			     structname, pieces[i].name,
			     structname, piece->name);
	    } else {
		sprintf(buf, "offsetof(%s, %s)", structname, pieces[i].name);
	    }
	    return buf;
	} else if (size < pieces[i].from - piece->from) {
	    break;
	}
    }
    sprintf(buf, "sizeof %s.%s + %ld",
		 varname, piece->name, size - piece->size);
    return buf;
}

/* Returns a string representation of a count value that, if possible,
 * is related to the contents of some part of the C structure. ndx is
 * the index of a piece that may contain a table of the necessary
 * count.
 */
char const *getcountstr(int count, int ndx)
{
    struct pieceinfo const *piece;

    if (count <= 1 || ndx <= 0)
	return strdec(count);
    piece = pieces + ndx;
    if (ctypes[piece->type].size <= 1)
	return getsizestr(count, ndx);
    if (piece->size % ctypes[piece->type].size == 0)
	if (piece->size / ctypes[piece->type].size == count)
	    return strf("sizeof %s.%s / sizeof *%s.%s",
			varname, piece->name, varname, piece->name);
    return strdec(count);
}

/* Returns a string representation of a smaller size value, typically
 * the size of some ELF structure. If ndx is not negative, it refers
 * to a piece of the C structure whose contents may have the given
 * size.
 */
char const *getentsizestr(long size, int ndx)
{
    if (size <= 1)
	return strdec(size);
    if (ndx > 0 && size == ctypes[pieces[ndx].type].size)
	return strf("sizeof(%s)", ctypes[pieces[ndx].type].name);
    if (size == sizeof_elf(Addr))
	return iself64() ? "sizeof(Elf64_Addr)" : "sizeof(Elf32_Addr)";
    return strhex(size);
}

/* Outputs the C structure represented by the pieces array.
 */
void outputstruct(void)
{
    int i;

    outf("typedef struct %s\n{\n", structname);
    for (i = 0 ; i < piececount ; ++i) {
	if (pieces[i].badtype)
	    outf("  %-15s %s[%ld];\n",
		 "unsigned char", pieces[i].name, pieces[i].size);
	else if (pieces[i].type == P_EHDR)
	    outf("  %-15s %s;\n",
		 ctypes[pieces[i].type].name, pieces[i].name);
	else if (pieces[i].type == P_SHDRTAB)
	    outf("  %-15s %s[%s];\n",
		 ctypes[pieces[i].type].name, pieces[i].name,
		 getsectionid(pieces[i].size / ctypes[pieces[i].type].size));
	else
	    outf("  %-15s %s[%ld];\n",
		 ctypes[pieces[i].type].name, pieces[i].name,
		 pieces[i].size / ctypes[pieces[i].type].size);
    }
    outf("} %s;\n\n", structname);
}

/* Ouptuts the contents of the ELF image as a C structure.
 */
void outputpieces(void)
{
    int type, i;

    outf("%s %s =", structname, varname);
    beginblock(TRUE);
    for (i = 0 ; i < piececount ; ++i) {
	linebreak();
	outcomment(pieces[i].name);
	linebreak();
	type = pieces[i].type;
	if (pieces[i].badtype) {
	    outcomment(strf("This section is of type %s but has the wrong %s",
			    ctypes[type].name,
			    (pieces[i].badtype == PW_MISALIGNED ? "alignment" :
			     pieces[i].badtype == PW_WRONGSIZE ? "size" :
							"alignment/size")));
	    linebreak();
	    if ((pieces[i].badtype & PW_MISALIGNED) || needalignment)
		type = P_SECTION;
	}
	outtypedblock(type, pieces[i].from, pieces[i].size, i);
    }
    endblock();
}
