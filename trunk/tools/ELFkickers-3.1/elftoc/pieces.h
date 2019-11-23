/* pieces.h: Functions for handling the file as a sequence of pieces.
 * Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#ifndef	_pieces_h_
#define	_pieces_h_

/* The complete list of types of file pieces.
 */
enum
{
    P_UNCLAIMED = 0,	/* not referenced anywhere */
    P_SECTION,		/* generic section; no information */
    P_BYTES,		/* array of generic values */
    P_HALVES,
    P_WORDS,
    P_XWORDS,
    P_ADDRS,		/* array of memory addresses */
    P_STRINGS,		/* collection of strings */
    P_NOTE,		/* specific section types */
    P_HASH,
    P_GNUHASH,
    P_SYMTAB,
    P_SYMINFO,
    P_REL,
    P_RELA,
    P_MOVE,
    P_DYNAMIC,
    P_SHDRTAB,
    P_PHDRTAB,
    P_EHDR,
    P_NONEXISTENT,	/* enforced struct padding */
    P_COUNT		/* the number of types */
};

/* Changes output behavior so that misaligned pieces are displayed as
 * their original type, instead of as byte arrays.
 */
extern void permitmisalignment(void);

/* Changes the behavior on inclusion of a _end field. A false value
 * suppresses the field, a true value forces it to appear. The default
 * behavior is to only include it when its size is nonzero.
 */
extern void include_end(int include);

/* Changes the name of the struct and the variable being initialized,
 * respectively, in the generated source.
 */
extern void setstructname(char const *name);
extern void setvarname(char const *name);

/* Records a piece of the file, along with its known type, associated
 * section header table entry (if any), and a suggested name. align
 * gives the pieces alignment requirements, if any, and is used to
 * identify padding. If the piece's name is prefixed with a tilde,
 * then the name shall be treated as a weak suggestion, to be used if
 * nothing better comes along. The return value is the piece's index,
 * and can be used to refer to the piece in the following set
 * functions. The return value is negative if no piece was created.
 */
extern int recordpiece(long offset, long size, int type,
		       char const *name, int align);

/* Associates data with the piece, specifically a section header table
 * index, a string table, and a table of unspecified type. These data
 * will stay with the piece and can be retrieved during the output
 * phase.
 */
extern void setpieceshndx(int ndx, int shndx);
extern void setpiecestrtable(int ndx, long offset, long size);
extern void setpiecemisctable(int ndx, long offset, long size);

/* This function normalizes and finalizes the list of pieces that
 * comprise the ELF image. All redundancies and overlaps are removed,
 * after which every byte of the ELF image will belong to exactly one
 * piece. Each piece will then correspond to a field in the generated
 * source code's struct. No more pieces can be added after calling
 * this function, and all preexisting piece indexes are invalidated by
 * calling this function.
 */
extern void arrangepieces(void);

/* Verifies the alignment and sizing of each piece. If the size or
 * alignment is inappropriate for that piece's type, it will be marked
 * as such in the final output.
 */
extern void verifypiecetypes(void);

/* Assigns each piece a final name, ensuring that each name is unique.
 */
extern void setpiecenames(void);

/* Retrieves data previously associated with a piece. The piece index
 * should be one that was provided as a parameter to one of the output
 * functions, or else obtained from one of the functions following.
 */
extern int getpieceshndx(int ndx);
extern void const *getpiecestrtable(int ndx, long *size);
extern void const *getpiecemisctable(int ndx, long *size);

/* Returns the piece index associated with a section header table
 * index. The return value is negative if no piece is associated with
 * the given section.
 */
extern int getindexfromsection(int shndx);

/* Returns the piece index that contains the given position in the ELF
 * file image.
 */
extern int getindexfromoffset(long offset);

/* Returns a string representation of a file offset value. ndx is the
 * index of a piece that is presumed to be associated with the offset
 * value (typically obtained by calling getindexfromoffset()).
 */
extern char const *getoffsetstr(long offset, int ndx);

/* Returns a string representation of a size. ndx is the index of a
 * piece that is presumably associated with the size.
 */
extern char const *getsizestr(long size, int ndx);

/* Returns a string representation of a size, one that is expected to
 * be the size of a type rather than the size of an entire piece. ndx
 * is the index of a piece that is presumed to be of the expected
 * type.
 */
extern char const *getentsizestr(long size, int ndx);

/* Returns a string representation of a count value. ndx is the index
 * of a presumably related piece.
 */
extern char const *getcountstr(int count, int ndx);

/* Outputs the struct declaration that represents the structure of the
 * ELF image.
 */
extern void outputstruct(void);

/* Outputs the struct variable's definition and initialization,
 * representing the contents of the ELF image.
 */
extern void outputpieces(void);

#endif
