/* rebind: Copyright (C) 2001,2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <elf.h>
#include "elfrw.h"

#ifndef TRUE
#define	TRUE	1
#define	FALSE	0
#endif

/* The memory-allocation macro.
 */
#define alloc(p, n) \
    (((p) = realloc(p, n)) || (fputs("Out of memory.\n", stderr), exit(1), 0))

/* The online help text.
 */
static char const *yowzitch =
    "Usage: rebind [OPTIONS] FILE [SYMBOL...]\n"
    "Change the binding/visibility of symbols in an ELF object file.\n\n"
    "  -b, --binding=BIND    Change symbol binding to BIND.\n"
    "  -v, --visibility=VIS  Change symbol visibility to VIS.\n"
    "  -w, --weaken          Short for \"--binding=weak\".\n"
    "  -h, --hide            Short for \"--visibility=hidden\".\n"
    "  -i, --verbose         Describe which symbols are changed.\n"
    "      --help            Display this help and exit.\n"
    "      --version         Display version information and exit.\n\n"
    "BIND can be either \"global\" or \"weak\".\n"
    "VIS can be \"hidden\", \"internal\", \"protected\", or \"default\".\n"
    "If no symbol names are given on the command line, the program\n"
    "reads the symbol names from standard input.\n";

/* The version text.
 */
static char const *vourzhon =
    "rebind: version 1.2\n"
    "Copyright (C) 2001,2011 by Brian Raiter <breadbox@muppetlabs.com>\n"
    "License GPLv2+: GNU GPL version 2 or later.\n"
    "This is free software; you are free to change and redistribute it.\n"
    "There is NO WARRANTY, to the extent permitted by law.\n";

static unsigned char	chgbind;	/* true if the binding is changing */
static unsigned char	chgvisibility;	/* true if visibility is changing */
static unsigned char	tobind;		/* the binding to change to */
static unsigned char	tovisibility;	/* the visibility to change to */
static int		verbose;	/* whether to tell the user */

static char	      **namelist;	/* the list of symbol names to seek */
static int		namecount;	/* the number of symbols in namelist */

static char const      *theprogram;	/* the name of this executable */
static char const      *thefilename;	/* the current file name */
static FILE	       *thefile;	/* the current file handle */

static Elf64_Ehdr	ehdr;		/* the file's ELF header */

/* Standard qsort/bsearch string comparison function.
 */
static int qstrcmp(void const *a, void const *b)
{
    return strcmp(*(char const**)a, *(char const**)b);
}

/* An error-handling function. The given error message is used only
 * when errno is not set.
 */
static int err(char const *errmsg)
{
    fprintf(stderr, "%s: %s: %s\n", theprogram, thefilename,
				    errno ? strerror(errno) : errmsg);
    return FALSE;
}

/* An error-handling function specifically for errors in the
 * command-line syntax.
 */
static int badcmdline(char const *errmsg)
{
    fprintf(stderr, "%s: %s\nTry --help for more information.\n",
		    theprogram, errmsg);
    exit(EXIT_FAILURE);
}

/* namelistfromfile() builds an array of all the non-whitespace strings
 * in the given file.
 */
static int namelistfromfile(FILE *fp)
{
    char	word[256];
    int		allocated = 0;
    int		n;

    while (fscanf(fp, "%255s", word) > 0) {
	if (namecount >= allocated) {
	    allocated = allocated ? allocated * 2 : 16;
	    alloc(namelist, allocated * sizeof *namelist);
	}
	namelist[namecount] = NULL;
	n = strlen(word) + 1;
	alloc(namelist[namecount], n);
	memcpy(namelist[namecount], word, n);
	++namecount;
    }
    return TRUE;
}


/* readheader() checks to make sure that this is in fact a proper ELF
 * object file that we're proposing to munge.
 */
static int readheader(void)
{
    if (elfrw_read_Ehdr(thefile, &ehdr) != 1) {
	if (ferror(thefile))
	    return err("not an ELF file.");
	fprintf(stderr, "%s: unrecognized ELF file type.\n", thefilename);
	return FALSE;
    }
    if (!ehdr.e_shoff)
	return err("no section header table.");
    if (ehdr.e_shentsize != sizeof(Elf32_Shdr) &&
			ehdr.e_shentsize != sizeof(Elf64_Shdr))
	return err("unrecognized section header size");

    return TRUE;
}

/* changesymbols() finds all symbols in a given symbol table that
 * appear in the namelist and alters their binding and/or visibility.
 * Note that the program will refuse to change the binding of local
 * symbols, as that can easily render a symbol table invalid.
 */
static int changesymbols(Elf64_Sym *symtab, char const *strtab, int count)
{
    Elf64_Sym  *sym;
    char const *name;
    int		touched;
    int		i;

    touched = FALSE;
    for (i = 0, sym = symtab ; i < count ; ++i, ++sym) {
	name = strtab + sym->st_name;
	if (!bsearch(&name, namelist, namecount, sizeof *namelist, qstrcmp))
	    continue;
	if (chgbind) {
	    if (ELF64_ST_BIND(sym->st_info) == STB_LOCAL) {
		fprintf(stderr, "warning: cannot rebind local symbol \"%s\"\n",
				name);
		continue;
	    }
	    sym->st_info = ELF64_ST_INFO(tobind, ELF64_ST_TYPE(sym->st_info));
	}
	if (chgvisibility)
	    sym->st_other = ELF64_ST_VISIBILITY(tovisibility);
	if (verbose)
	    printf("%s: \"%s\" altered.\n", thefilename, name);
	touched = TRUE;
    }
    return touched;
}

/* rebind() does the grunt work of locating the symbol tables. After
 * checking over the ELF headers, the function iterates through the
 * sections, looking for symbol tables containing non-local symbol.
 * When it finds one, it loads the non-local part of the table and the
 * associated string table into memory, and calls changesymbols(). If
 * changesymbols() actually changes anything, the altered symbol table
 * is written back out to the object file.
 */
static int rebind(void)
{
    Elf64_Shdr *shdrs = NULL;
    Elf64_Sym *symtab = NULL;
    char *strtab = NULL;
    unsigned long offset;
    int count;
    int changed;
    int i, n;

    if (!readheader())
	return FALSE;
    changed = FALSE;
    alloc(shdrs, ehdr.e_shnum * sizeof *shdrs);
    if (fseek(thefile, ehdr.e_shoff, SEEK_SET) ||
		elfrw_read_Shdrs(thefile, shdrs, ehdr.e_shnum) != ehdr.e_shnum)
	return err("invalid section header table.");
    for (i = 0 ; i < ehdr.e_shnum ; ++i) {
	if (shdrs[i].sh_type != SHT_SYMTAB && shdrs[i].sh_type != SHT_DYNSYM)
	    continue;
	if (shdrs[i].sh_entsize != sizeof(Elf32_Sym) &&
			shdrs[i].sh_entsize != sizeof(Elf64_Sym)) {
	    err("symbol table of unrecognized structure ignored.");
	    continue;
	}
	offset = shdrs[i].sh_offset + shdrs[i].sh_info * shdrs[i].sh_entsize;
	count = shdrs[i].sh_size / shdrs[i].sh_entsize - shdrs[i].sh_info;
	if (!count)
	    continue;
	n = shdrs[shdrs[i].sh_link].sh_size;
	alloc(symtab, count * sizeof *symtab);
	alloc(strtab, n);
	if (fseek(thefile, offset, SEEK_SET) ||
			elfrw_read_Syms(thefile, symtab, count) != count)
	    return err("invalid symbol table");
	if (fseek(thefile, shdrs[shdrs[i].sh_link].sh_offset, SEEK_SET) ||
			fread(strtab, n, 1, thefile) != 1)
	    return err("invalid associated string table");
	if (changesymbols(symtab, strtab, count)) {
	    if (fseek(thefile, offset, SEEK_SET) ||
			elfrw_write_Syms(thefile, symtab, count) != count)
		return err("unable to write to the object file");
	    changed = TRUE;
	}
    }

    if (verbose && !changed)
	printf("%s: nothing changed.\n", thefilename);
    free(strtab);
    free(symtab);
    free(shdrs);
    return TRUE;
}

/* readoptions() parses the command-line arguments. It only returns if
 * the syntax is valid and there is work to do.
 */
static void readcmdline(int argc, char *argv[])
{
    static char const *optstring = "b:hiv:w";
    static struct option const options[] = {
	{ "binding", required_argument, 0, 'b' },
	{ "visibility", required_argument, 0, 'v' },
	{ "weaken", no_argument, 0, 'w' },
	{ "hide", no_argument, 0, 'h' },
	{ "verbose", no_argument, 0, 'i' },
	{ "help", no_argument, 0, 'H' },
	{ "version", no_argument, 0, 'V' },
	{ 0, 0, 0, 0 }
    };

    int n, t;

    if (argc == 1) {
	fputs(yowzitch, stdout);
	exit(EXIT_SUCCESS);
    }

    verbose = FALSE;
    chgbind = FALSE;
    chgvisibility = FALSE;

    theprogram = argv[0];
    while ((n = getopt_long(argc, argv, optstring, options, NULL)) != EOF) {
	switch (n) {
	  case 'b':
	    if (!strcmp(optarg, "global") || !strcmp(optarg, "strong"))
		t = STB_GLOBAL;
	    else if (!strcmp(optarg, "weak"))
		t = STB_WEAK;
	    else
		badcmdline("invalid bind argument");
	    if (chgbind && tobind != t)
		badcmdline("conflicting binding arguments");
	    tobind = t;
	    chgbind = TRUE;
	    break;
	  case 'v':
	    if (!strcmp(optarg, "internal"))
		t = STV_INTERNAL;
	    else if (!strcmp(optarg, "hidden"))
		t = STV_HIDDEN;
	    else if (!strcmp(optarg, "protected"))
		t = STV_PROTECTED;
	    else if (!strcmp(optarg, "default"))
		t = STV_DEFAULT;
	    else
		badcmdline("invalid visibility argument");
	    if (chgvisibility && tovisibility != t)
		badcmdline("conflicting visibility arguments");
	    tovisibility = t;
	    chgvisibility = TRUE;
	    break;
	  case 'w':
	    if (chgbind && tobind != STB_WEAK)
		badcmdline("conflicting binding arguments");
	    chgbind = TRUE;
	    tobind = STB_WEAK;
	    break;
	  case 'h':
	    if (chgvisibility && tovisibility != STV_HIDDEN)
		badcmdline("conflicting visibility arguments");
	    chgvisibility = TRUE;
	    tobind = STV_HIDDEN;
	    break;
	  case 'i':
	    verbose = TRUE;
	    break;
	  case 'H':
	    fputs(yowzitch, stdout);
	    exit(EXIT_SUCCESS);
	  case 'V':
	    fputs(vourzhon, stdout);
	    exit(EXIT_SUCCESS);
	}
    }
    if (optind == argc)
	badcmdline("no input files");
    if (!chgbind && !chgvisibility)
	badcmdline("nothing to do");

    thefilename = argv[optind];
    ++optind;
}

/* main() builds the array of symbol names, opens the object file, and
 * calls rebind().
 */
int main(int argc, char *argv[])
{
    int r;

    readcmdline(argc, argv);

    if (!(thefile = fopen(thefilename, "rb+"))) {
	err("unable to open.");
	return EXIT_FAILURE;
    }

    if (optind == argc) {
	if (!namelistfromfile(stdin))
	    return EXIT_FAILURE;
    } else {
	namelist = argv + optind;
	namecount = argc - optind;
    }
    qsort(namelist, namecount, sizeof *namelist, qstrcmp);

    r = rebind();

    fclose(thefile);
    return r ? EXIT_SUCCESS : EXIT_FAILURE;
}
