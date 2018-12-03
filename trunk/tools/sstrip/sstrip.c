/* sstrip: Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>
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

/* The online help text.
 */
static char const *yowzitch =
    "Usage: sstrip [OPTIONS] FILE...\n"
    "Remove all nonessential bytes from executable ELF files.\n\n"
    "  -z, --zeroes        Also discard trailing zero bytes.\n"
    "      --help          Display this help and exit.\n"
    "      --version       Display version information and exit.\n";

/* Version and license information.
 */
static char const *vourzhon =
    "sstrip, version 2.1\n"
    "Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>\n"
    "License GPLv2+: GNU GPL version 2 or later.\n"
    "This is free software; you are free to change and redistribute it.\n"
    "There is NO WARRANTY, to the extent permitted by law.\n";

/* The name of the program.
 */
static char const *theprogram;

/* TRUE if we should attempt to truncate zero bytes from the end of
 * the file.
 */
static int dozerotrunc = FALSE;

/* Information for each executable operated upon.
 */
static char const  *thefilename;	/* the name of the current file */
static FILE        *thefile;		/* the currently open file handle */
static Elf64_Ehdr   ehdr;		/* the current file's ELF header */
static Elf64_Phdr  *phdrs;		/* the program segment header table */
unsigned long       newsize;		/* the proposed new file size */

/* A simple error-handling function. FALSE is always returned for the
 * convenience of the caller.
 */
static int err(char const *errmsg)
{
    fprintf(stderr, "%s: %s: %s\n", theprogram, thefilename, errmsg);
    return FALSE;
}

/* A macro for I/O errors: The given error message is used only when
 * errno is not set.
 */
#define	ferr(msg) (err(ferror(thefile) ? strerror(errno) : (msg)))

/* readcmdline() attemps to parse the command line arguments, and only
 * returns if succeeded and there is work to do.
 */
static void readcmdline(int argc, char *argv[])
{
    static char const *optstring = "z";
    static struct option const options[] = {
	{ "zeros", no_argument, 0, 'z' },
	{ "zeroes", no_argument, 0, 'z' },
	{ "help", no_argument, 0, 'H' },
	{ "version", no_argument, 0, 'V' },
	{ 0, 0, 0, 0 }
    };

    int n;

    if (argc == 1) {
	fputs(yowzitch, stdout);
	exit(EXIT_SUCCESS);
    }

    theprogram = argv[0];
    while ((n = getopt_long(argc, argv, optstring, options, NULL)) != EOF) {
	switch (n) {
	  case 'z':
	    dozerotrunc = TRUE;
	    break;
	  case 'H':
	    fputs(yowzitch, stdout);
	    exit(EXIT_SUCCESS);
	  case 'V':
	    fputs(vourzhon, stdout);
	    exit(EXIT_SUCCESS);
	  default:
	    fputs("Try --help for more information.\n", stderr);
	    exit(EXIT_FAILURE);
	}
    }
}

/* readelfheader() reads the ELF header into our global variable, and
 * checks to make sure that this is in fact a file that we should be
 * munging.
 */
static int readelfheader(void)
{
    if (elfrw_read_Ehdr(thefile, &ehdr) != 1)
	return ferr("not a valid ELF file");

    if (ehdr.e_type != ET_EXEC && ehdr.e_type != ET_DYN)
	return err("not an executable or shared-object library.");

    return TRUE;
}

/* readphdrtable() loads the program segment header table into memory.
 */
static int readphdrtable(void)
{
    if (!ehdr.e_phoff || !ehdr.e_phnum)
	return err("ELF file has no program header table.");

    if (!(phdrs = realloc(phdrs, ehdr.e_phnum * sizeof *phdrs)))
	return err("Out of memory!");
    if (elfrw_read_Phdrs(thefile, phdrs, ehdr.e_phnum) != ehdr.e_phnum)
	return ferr("missing or incomplete program segment header table.");

    return TRUE;
}

/* getmemorysize() determines the offset of the last byte of the file
 * that is referenced by an entry in the program segment header table.
 * (Anything in the file after that point is not used when the program
 * is executing, and thus can be safely discarded.)
 */
static int getmemorysize(void)
{
    unsigned long size, n;
    int i;

    /* Start by setting the size to include the ELF header and the
     * complete program segment header table.
     */
    size = ehdr.e_phoff + ehdr.e_phnum * sizeof *phdrs;
    if (size < ehdr.e_ehsize)
	size = ehdr.e_ehsize;

    /* Then keep extending the size to include whatever data the
     * program segment header table references.
     */
    for (i = 0 ; i < ehdr.e_phnum ; ++i) {
	if (phdrs[i].p_type != PT_NULL) {
	    n = phdrs[i].p_offset + phdrs[i].p_filesz;
	    if (n > size)
		size = n;
	}
    }

    newsize = size;
    return TRUE;
}

/* truncatezeros() examines the bytes at the end of the file's
 * size-to-be, and reduces the size to exclude any trailing zero
 * bytes.
 */
static int truncatezeros(void)
{
    unsigned char contents[1024];
    unsigned long size, n;

    if (!dozerotrunc)
	return TRUE;

    size = newsize;
    do {
	n = sizeof contents;
	if (n > size)
	    n = size;
	if (fseek(thefile, size - n, SEEK_SET))
	    return ferr("cannot seek in file.");
	if (fread(contents, n, 1, thefile) != 1)
	    return ferr("cannot read file contents");
	while (n && !contents[--n])
	    --size;
    } while (size && !n);

    /* Sanity check.
     */
    if (!size)
	return err("ELF file is completely blank!");

    newsize = size;
    return TRUE;
}

/* modifyheaders() removes references to the section header table if
 * it was stripped, and reduces program header table entries that
 * included truncated bytes at the end of the file.
 */
static int modifyheaders(void)
{
    int i;

    /* If the section header table is gone, then remove all references
     * to it in the ELF header.
     */
    if (ehdr.e_shoff >= newsize) {
	ehdr.e_shoff = 0;
	ehdr.e_shnum = 0;
	ehdr.e_shstrndx = 0;
    }

    /* The program adjusts the file size of any segment that was
     * truncated. The case of a segment being completely stripped out
     * is handled separately.
     */
    for (i = 0 ; i < ehdr.e_phnum ; ++i) {
	if (phdrs[i].p_offset >= newsize) {
	    phdrs[i].p_offset = newsize;
	    phdrs[i].p_filesz = 0;
	} else if (phdrs[i].p_offset + phdrs[i].p_filesz > newsize) {
	    phdrs[i].p_filesz = newsize - phdrs[i].p_offset;
	}
    }

    return TRUE;
}

/* commitchanges() writes the new headers back to the original file
 * and sets the file to its new size.
 */
static int commitchanges(void)
{
    size_t n;

    /* Save the changes to the ELF header, if any.
     */
    if (fseek(thefile, 0, SEEK_SET))
	return ferr("could not rewind file");
    if (!elfrw_write_Ehdr(thefile, &ehdr))
	return ferr("could not modify file");

    /* Save the changes to the program segment header table, if any.
     */
    if (fseek(thefile, ehdr.e_phoff, SEEK_SET)) {
	ferr("could not seek in file");
	goto warning;
    }
    if (elfrw_write_Phdrs(thefile, phdrs, ehdr.e_phnum) != ehdr.e_phnum) {
	ferr("could not write to file");
	goto warning;
    }

    /* Eleventh-hour sanity check: don't truncate before the end of
     * the program segment header table.
     */
    n = ehdr.e_phnum * ehdr.e_phentsize;
    if (newsize < ehdr.e_phoff + n)
	newsize = ehdr.e_phoff + n;

    /* Chop off the end of the file.
     */
    if (ftruncate(fileno(thefile), newsize)) {
	err(errno ? strerror(errno) : "could not resize file");
	goto warning;
    }

    return TRUE;

  warning:
    return err("ELF file may have been corrupted!");
}

/* main() loops over the cmdline arguments, leaving all the real work
 * to the other functions.
 */
int main(int argc, char *argv[])
{
    int failures = 0;
    int i;

    readcmdline(argc, argv);

    for (i = optind ; i < argc ; ++i)
    {
	thefilename = argv[i];
	thefile = fopen(thefilename, "r+");
	if (!thefile) {
	    err(strerror(errno));
	    ++failures;
	    continue;
	}

	if (!(readelfheader() &&
	      readphdrtable() &&
	      getmemorysize() &&
	      truncatezeros() &&
	      modifyheaders() &&
	      commitchanges()))
	    ++failures;

	fclose(thefile);
    }

    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
