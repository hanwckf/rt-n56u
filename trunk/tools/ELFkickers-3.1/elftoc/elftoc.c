/* elftoc.c: Top-level functions.
 * Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <elf.h>
#include "gen.h"
#include "readelf.h"
#include "pieces.h"
#include "address.h"
#include "outbase.h"
#include "out.h"

/* Online help information.
 */
static char const *yowzitch =
    "Usage: elftoc [OPTIONS] FILENAME\n"
    "Translates an ELF file into C source code for a struct definition\n"
    "with the file's contents as its initialized value.\n\n"
    "  -e, --force-end         Force the inclusion of a final _end field.\n"
    "  -E, --exclude-end       Force the _end field to never be included.\n"
    "  -m, --allow-misaligned  Ignore alignment problems in fields.\n"
    "  -q, --quiet             Suppress warning messages.\n"
    "  -o, --output=FILE       Write source to FILE instead of stdout.\n"
    "  -w, --width=N           Set the right-hand margin to N [default=80].\n"
    "  -i, --indent=N          Set indentation to N spaces [default=2].\n"
    "  -s, --structname=NAME   Use NAME as the struct name [default=elf].\n"
    "  -v, --varname=NAME      Use NAME as the variable name [default=foo].\n"
    "      --help              Display this help and exit.\n"
    "      --version           Display version information and exit.\n";

/* A suffix for command-line parsing errors.
 */
static char const *helpsuffix = "Try --help for more information.";

/* The program's version information.
 */
static char const *vourzhon =
    "elftoc: version 2.\n"
    "Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>\n"
    "License GPLv2+: GNU GPL version 2 or later.\n"
    "This is free software; you are free to change and redistribute it.\n"
    "There is NO WARRANTY, to the extent permitted by law.\n";

static char const *inputfilename = NULL;	/* the ELF file to examine */
static char const *outputfilename = "-";	/* the file to write to */

/* Returns true if the string is a valid C identifier.
 */
static int isvalididentifier(char const *name)
{
    if (!isalpha(*name) && *name != '_')
	return 0;
    while (*++name)
	if (!isalnum(*name) && *name != '_')
	    return 0;
    return 1;
}

/* Reads and applies the command-line options. Returns only if the
 * syntax is valid, with the names of the input and output files
 * initialized.
 */
static void parsecmdline(int argc, char *argv[])
{
    static char const *optstring = "Eei:mo:qs:v:w:";
    static struct option options[] = {
	{ "force-end", no_argument, NULL, 'e' },
	{ "exclude-end", no_argument, NULL, 'E' },
	{ "allow-misaligned", no_argument, NULL, 'm' },
	{ "quiet", no_argument, NULL, 'q' },
	{ "indent", required_argument, NULL, 'i' },
	{ "width", required_argument, NULL, 'w' },
	{ "output", required_argument, NULL, 'o' },
	{ "structname", required_argument, NULL, 's' },
	{ "varname", required_argument, NULL, 'v' },
	{ "help", no_argument, NULL, 'H' },
	{ "version", no_argument, NULL, 'V' },
	{ 0, 0, 0, 0 }
    };

    char const *structname = NULL;
    char const *varname = NULL;
    char const *width = NULL;
    char const *indent = NULL;
    int n;

    setprogramname(argv[0]);
    while ((n = getopt_long(argc, argv, optstring, options, NULL)) != EOF) {
	switch (n) {
	  case 'e':	include_end(TRUE);		break;
	  case 'E':	include_end(FALSE);		break;
	  case 'm':	permitmisalignment();		break;
	  case 'q':	enablewarnings(FALSE);		break;
	  case 'i':	indent = optarg;		break;
	  case 'w':	width = optarg;			break;
	  case 'o':	outputfilename = optarg;	break;
	  case 's':	structname = optarg;		break;
	  case 'v':	varname = optarg;		break;
	  case 'H':	fputs(yowzitch, stdout);	exit(EXIT_SUCCESS);
	  case 'V':	fputs(vourzhon, stdout);	exit(EXIT_SUCCESS);
	  default:
	    fprintf(stderr, "%s", helpsuffix);
	    exit(EXIT_FAILURE);
	}
    }

    if (optind == argc)
	fail("no input file.\n%s", helpsuffix);
    if (optind + 1 < argc)
	fail("unrecognized extra argments.\n%s", helpsuffix);
    inputfilename = argv[optind];

    if (width) {
	if (sscanf(width, "%d", &n) != 1 || n < 4)
	    fail("invalid width parameter: %s.", width);
	setoutputwidth(n);
    }
    if (indent) {
	if (sscanf(indent, "%d", &n) != 1 || n < 0)
	    fail("invalid indent parameter: %s.", indent);
	setoutputindent(n);
    }
    if (varname) {
	if (!isvalididentifier(varname))
	    fail("invalid variable name: \"%s\".", varname);
	setvarname(varname);
    }
    if (structname) {
	if (!isvalididentifier(structname))
	    fail("invalid structure name: \"%s\".", structname);
	setstructname(structname);
    }
}

/* Maps the input file into memory and runs the initial examination of
 * the ELF structures.
 */
static int readinputfile(void)
{
    struct stat stat;
    void const *ptr;
    int fd;

    setfilename(inputfilename);
    fd = open(inputfilename, O_RDONLY);
    if (fd < 0)
	fail(NULL);
    if (fstat(fd, &stat))
	fail(NULL);
    if (!S_ISREG(stat.st_mode))
	fail("not a valid input file.");
    ptr = mmap(NULL, stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (ptr == MAP_FAILED)
	fail(NULL);
    close(fd);
    return readelf(ptr, stat.st_size);
}

/* Assembles a coherent image of the various parts of the input file
 * and assign names to all of the identifiers in the source code.
 */
static void processcontents(void)
{
    arrangepieces();
    verifypiecetypes();
    setpiecenames();
    setaddressnames();
}

/* Opens the output file and writes to it.
 */
static void writesource(void)
{
    FILE *file;

    if (strcmp(outputfilename, "-")) {
	setfilename(outputfilename);
	file = fopen(outputfilename, "w");
	if (!file)
	    fail(NULL);
    } else {
	setfilename("stdout");
	file = stdout;
    }
    setoutputfile(file);
    output();
    if (fclose(file))
	err(NULL);
}

/* main().
 */
int main(int argc, char *argv[])
{
    parsecmdline(argc, argv);
    if (!readinputfile())
	return EXIT_FAILURE;
    processcontents();
    writesource();
    return 0;
}
