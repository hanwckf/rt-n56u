/* ebfc.c: The central module.
 *
 * Copyright (C) 1999-2001 by Brian Raiter, under the GNU General
 * Public License. No warranty. See COPYING for details.
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<errno.h>
#include	<stdarg.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<sys/stat.h>
#include	<getopt.h>
#include	<elf.h>
#include	"elfparts.h"
#include	"ebfc.h"

/* Allocates memory (or aborts).
 */
#define	xalloc(p, n) \
    (((p) = realloc((p), (n))) ? (p) : (fputs("Out of memory!\n", stderr), \
					exit(ENOMEM), NULL))

/* Allocates memory for a string and forces null-termination.
 */
#define	xstrndup(d, s, n) \
    (((char*)memcpy(xalloc((d), (n) + 1), (s), (n)))[n] = '\0')

/* The online help text.
 */
static char const      *yowzitch =
	"Usage: ebfc [-hvlxcsz] [-o OBJFILE] [-i SRCNAME] [-f FUNCTION] FILE\n"
	"   -h  Display this help\n"
	"   -v  Display version information\n"
	"   -x  Compile to a standalone executable\n"
	"   -l  Compile to a shared library\n"
	"   -c  Compile to an object file (the default)\n"
	"   -xc Compile to an object file for a standalone executable\n"
	"   -lc Compile to an object file for a shared library\n"
	"   -s  Smaller output: omit extra features in the object file\n"
	"   -z  Read a compressed source file\n"
	"   -o  Output the object code to OBJFILE\n"
	"   -f  Set the program's function name to FUNCTION\n"
	"   -i  Record the source filename as being SRCNAME\n";

/* The version text.
 */
static char const      *vourzhon =
	"ebfc, version 1.1: Copyright (C) 1999 by Brian Raiter\n";

/* The contents of the .comment section.
 */
static char const	comment[] = "\0ELF Brainfuck Compiler 1.0";

/* The full collection of elfparts used by this program. This list
 * must match up with the enum that appears in the header file.
 */
static elfpart const   *parttable[P_COUNT] = {
    &part_ehdr, &part_phdrtab, &part_hash, &part_dynsym, &part_dynstr,
    &part_text, &part_rel, &part_got, &part_dynamic, &part_data,
    &part_shstrtab, &part_progbits, &part_symtab, &part_strtab, &part_shdrtab
};

/* Masks indicating which elfparts are used with which file types.
 */
static char		partlists[][P_COUNT] = {
    { 0 },
    { 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1 },	/* ET_REL */
    { 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0 },	/* ET_EXEC */
    { 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1 }	/* ET_DYN */
};

/* The name of this program, taken from argv[0]. Used for error messages.
 */
static char const      *thisprog;

/* The name of the currently open file. Used for error messages.
 */
static char const      *thefilename;

/* The filename to insert in the object file as the source filename.
 */
static char	       *srcfilename = NULL;

/* The filename to receive the object code.
 */
static char	       *outfilename = NULL;

/* The name of the function to put the Brainfuck program under.
 */
static char	       *functionname = NULL;

/* Whether or not to add extra, unnecessary items to the object file.
 */
static int		addextras = TRUE;

/* Outputs a formatted error message on stderr. If fmt is NULL, then
 * uses the error message supplied by perror(). Returns false.
 */
int err(char const *fmt, ...)
{
    va_list	args;

    if (!fmt) {
	perror(thefilename);
	return FALSE;
    }

    fprintf(stderr, "%s: ", thefilename ? thefilename : thisprog);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fputc('\n', stderr);
    va_end(args);
    return FALSE;
}

/* Derives the names of the source file, the object file, and the
 * function from the pathname of the actual source file, if any of
 * these were not already specified by the user.
 */
static int setnames(blueprint const *bp, int codetype, char const *filename)
{
    char const *base;
    int		n, i;

    base = strrchr(filename, '/');
    base = base ? base + 1 : filename;
    n = strlen(base);

    if (!srcfilename)
	xstrndup(srcfilename, base, n);

    if (base[n - 2] == '.' && base[n - 1] == 'b' && n > 2)
	n -= 2;

    if (!outfilename) {
	if (bp->filetype == ET_REL) {
	    xalloc(outfilename, n + 3);
	    sprintf(outfilename, "%*.*s.o", n, n, base);
	} else if (bp->filetype == ET_DYN) {
	    xalloc(outfilename, n + 7);
	    sprintf(outfilename, "lib%*.*s.so", n, n, base);
	} else if (bp->filetype == ET_EXEC) {
	    if (!base[n])
		xstrndup(outfilename, "a.out", sizeof "a.out");
	    else
		xstrndup(outfilename, base, n);
	} else
	    xstrndup(outfilename, "a.out", sizeof "a.out");
    }

    if (!functionname) {
	if (codetype == ET_EXEC)
	    xstrndup(functionname, "_start", 6);
	else {
	    xstrndup(functionname, base, n);
	    for (i = 0 ; i < n ; ++i)
		if (!isalnum(functionname[i]))
		    functionname[i] = '_';
	}
    }
    if (!isalpha(*functionname) && *functionname != '_') {
	err("`%s' is not a valid symbol name", functionname);
	return FALSE;
    }

    return TRUE;
}

/* Creates and initializes the various parts of the object file.
 */
static int createparts(blueprint const *bp)
{
    newparts(bp);

    if (bp->parts[P_HASH].shtype)
	bp->parts[P_HASH].link = &bp->parts[P_DYNSYM];
    if (bp->parts[P_DYNSYM].shtype)
	bp->parts[P_DYNSYM].link = &bp->parts[P_DYNSTR];
    if (bp->parts[P_REL].shtype) {
	bp->parts[P_REL].link = &bp->parts[P_SYMTAB];
	bp->parts[P_REL].info = P_TEXT;
    }
    if (bp->parts[P_SYMTAB].shtype)
	bp->parts[P_SYMTAB].link = &bp->parts[P_STRTAB];
    if (bp->parts[P_SHDRTAB].shtype && bp->parts[P_SHSTRTAB].shtype)
	bp->parts[P_SHDRTAB].link = &bp->parts[P_SHSTRTAB];

    initparts(bp);

    return TRUE;
}

/* Creates the contents of the various parts of the object file.
 */
static int populateparts(blueprint const *bp, int codetype,
			 char const *filename, int compressed)
{
    if (addextras) {
	if (bp->parts[P_COMMENT].shtype) {
	    bp->parts[P_COMMENT].shname = ".comment";
	    bp->parts[P_COMMENT].size = sizeof comment;
	    xstrndup(bp->parts[P_COMMENT].part, comment, sizeof comment);
	}
	if (bp->parts[P_SYMTAB].shtype)
	    addtosymtab(bp->parts + P_SYMTAB, srcfilename,
			STB_LOCAL, STT_FILE, SHN_ABS);
    }

    thefilename = filename;
    if (!translatebrainfuck(filename, bp, codetype, functionname, compressed))
	return FALSE;
    thefilename = NULL;

    fillparts(bp);
    if (!computeoffsets(bp))
	return FALSE;

    createfixups(bp, codetype, functionname);

    completeparts(bp);

    return TRUE;
}

/* The top-level compiling function. Runs through the stages of
 * creating the object file image, compiling the source code, and
 * writing out the file.
 */
static int compile(blueprint *bp, int codetype,
		   char const *filename, int compressed)
{
    struct stat	s;

    if (!createparts(bp))
	return FALSE;
    if (!setnames(bp, codetype, filename))
	return FALSE;
    if (!populateparts(bp, codetype, filename, compressed))
	return FALSE;

    thefilename = outfilename;
    if (!outputelf(bp, outfilename)) {
	err(NULL);
	remove(outfilename);
	return FALSE;
    }
    if (!stat(outfilename, &s)) {
	if (bp->filetype == ET_EXEC)
	    s.st_mode |= S_IXUSR | S_IXGRP | S_IXOTH;
	else
	    s.st_mode &= ~(S_IXUSR | S_IXGRP | S_IXOTH);
	chmod(outfilename, s.st_mode);
    } else
	err(NULL);
    thefilename = NULL;

    return TRUE;
}

/* main() sets up the blueprint as per the command-line options and
 * begins the compiling process.
 */
int main(int argc, char *argv[])
{
    blueprint	b = { 0 };
    int		codetype = 0;
    int		compressed = FALSE;
    int		n;

    thisprog = argv[0];

    while ((n = getopt(argc, argv, "cf:hi:lo:svxz")) != EOF) {
	switch (n) {
	  case 'z':	compressed = TRUE;				break;
	  case 'c':	b.filetype = ET_REL;				break;
	  case 'x':	codetype = ET_EXEC;				break;
	  case 'l':	codetype = ET_DYN;				break;
	  case 's':	addextras = FALSE;				break;
	  case 'f':	xstrndup(functionname, optarg, strlen(optarg));	break;
	  case 'i':	xstrndup(srcfilename, optarg, strlen(optarg));	break;
	  case 'o':	xstrndup(outfilename, optarg, strlen(optarg));	break;
	  case 'h':	fputs(yowzitch, stdout);	return EXIT_SUCCESS;
	  case 'v':	fputs(vourzhon, stdout);	return EXIT_SUCCESS;
	  default:	fputs(yowzitch, stderr);	return EXIT_FAILURE;
	}
    }

    if (!b.filetype) {
	if (!codetype)
	    codetype = ET_EXEC;
	b.filetype = codetype;
    } else if (!codetype)
	codetype = ET_REL;
    b.partcount = P_COUNT;
    xalloc(b.parts, P_COUNT * sizeof *b.parts);
    for (n = 0 ; n < P_COUNT ; ++n) {
	if (partlists[b.filetype][n])
	    b.parts[n] = *parttable[n];
	else
	    b.parts[n].shtype = 0;
    }
    if (!addextras)
	b.parts[P_COMMENT].shtype = 0;

    if (optind + 1 != argc) {
	err(optind == argc ? "no input file specified."
			   : "multiple input files specified.");
	return EXIT_FAILURE;
    }

    return compile(&b, codetype, argv[optind], compressed) ? EXIT_SUCCESS
							   : EXIT_FAILURE;
}
