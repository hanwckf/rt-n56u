/* elfparts.h: Definitions and functions supplied by the elfparts library.
 *
 * Copyright (C) 1999-2001 by Brian Raiter, under the GNU General
 * Public License. No warranty. See COPYING for details.
 */

#ifndef	_elfparts_h_
#define	_elfparts_h_

/* Used to identify a part that has no section header table entry.
 */
#define	SHT_OTHER	(-1)

/* Names of global objects that are used in the symbol tables.
 */
#define	NAME_DYNAMIC	"_DYNAMIC"
#define	NAME_GOT	"_GLOBAL_OFFSET_TABLE_"

typedef	struct blueprint	blueprint;
typedef	struct elfpart		elfpart;

/* Information used to specify the contents and structure of the ELF
 * file.
 */
struct blueprint {
    int			filetype;	/* the type of file to make */
    int			partcount;	/* the number of parts */
    elfpart	       *parts;		/* the parts list */
};

/* Information used to specify the contents of a specific part. The
 * first four fields are function pointers; the functions themselves
 * are supplied by the elfparts library. The other fields are filled
 * in by the library and/or by the calling program.
 */
struct elfpart {
    void	      (*new)(elfpart*);
    void	      (*init)(elfpart*, blueprint const*);
    void	      (*fill)(elfpart*, blueprint const*);
    void	      (*complete)(elfpart*, blueprint const*);
    int			shtype;		/* the section header entry type */
    char const	       *shname;		/* the section header entry name */
    int			entsize;	/* the size of the part's entries */
    int			count;		/* the number of the part's entries */
    int			size;		/* the total size */
    unsigned int	offset;		/* the part's location in the file */
    unsigned int	addr;		/* the part's load address */
    elfpart	       *link;		/* another part that this one uses */
    int			info;		/* a extra, multipurpose field */
    short		flags;		/* memory read-write-exec flags */
    short		done;		/* true when this part is completed */
    int			shindex;	/* the section header entry index */
    void	       *part;		/* the actual contents */
};

/* The different types of parts available for use. These structures
 * all have predefined values for the four function pointers.
 */
extern elfpart	part_ehdr,
		part_phdrtab, part_shdrtab,
		part_progbits, part_text, part_data, part_got, part_bss,
		part_strtab, part_dynstr, part_shstrtab,
		part_symtab, part_dynsym,
		part_hash, part_rel, part_rela, part_dynamic;

/* Calls new for all the parts in the blueprint's part list. This
 * function should be called once the part list has been created.
 */
extern void newparts(blueprint const *bp);

/* Calls init for all the parts in the blueprint's part list. This
 * function should be called after the caller has tailored the
 * information in the elfpart structures (e.g., the link and info
 * fields), but before using any of the part-specific functions listed
 * below.
 */
extern void initparts(blueprint const *bp);

/* Calls fill for all the parts in the blueprint's part list. This
 * function should be called after the caller has finished adding to
 * (or removing from) the parts. When this function returns, every
 * part will have determined its final size (but not necessarily its
 * actual contents).
 */
extern void fillparts(blueprint const *bp);

/* Calls complete for all the parts in the blueprint's part list. This
 * function should be called when the caller has completely finished
 * altering the contents of the parts. When this function returns, the
 * parts will be in their final state, and may be written out.
 */
extern void completeparts(blueprint const *bp);

/* Fills in the values for the addr and offset fields for all the
 * parts in the blueprint's part list. The function returns true to
 * indicate success. This function should be called after fill() and
 * before complete().
 */
extern int computeoffsets(blueprint const *bp);

/* Writes out the ELF file to the given filename. If the return value
 * is false, an error occurred and errno is set accordingly.
 */
extern int outputelf(blueprint const *bp, char const *filename);

/* Adds a string to a string table. The return value is the index
 * of the string in the table.
 */
extern int addtostrtab(elfpart *part, char const *str);

/* Adds a symbol to a symbol table. The symbol's value is initialized
 * to zero; the other data associated with the symbol are supplied by
 * the function's arguments. The symbol's name is automatically added
 * to the appropriate string table. The return value is an index of
 * the symbol in the table (see below for comments on the index
 * value).
 */
extern int addtosymtab(elfpart *part, char const *str,
		       int bind, int type, int shndx);

/* Look up a symbol already in a symbol table by name. The return
 * value is the index of the symbol, or zero if the symbol is not in
 * the table. The index will be negative for local symbols, since the
 * true index cannot be determined until all the global symbols have
 * been added. After fill() has been called, a negative index can be
 * converted to a real index by subtracting it from the value in the
 * symbol table's info field, less one.
 */
extern int getsymindex(elfpart *part, char const *name);

/* Sets the value of a symbol in a symbol table. The return value is
 * FALSE if the given symbol could not be found in the table.
 */
extern int setsymvalue(elfpart *part, char const *name, unsigned int value);

/* Adds an entry to a relocation section. The return value is the
 * index of the entry.
 */
extern int addtorel(elfpart *part, unsigned int offset, int sym, int type);

/* Adds an entry to a relocation section. The given symbol is added to
 * the appropriate symbol table if it is not already present. (If the
 * symbol is already present, its data is not changed, and the last
 * three arguments are ignored.) The return value is the index of the
 * entry.
 */
extern int addrelsymbol(elfpart *part, unsigned int offset, int reltype,
			char const *sym, int symbind, int symtype, int shndx);

/* Sets the value of an entry in a dynamic section. The entry is added
 * if it is not already present. The return value is the index of the
 * entry.
 */
extern int setdynvalue(elfpart *part, int tag, int value);

#endif
