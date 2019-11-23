/* elfls: Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <elf.h>

#include "elfrw.h"

#ifndef TRUE
#define	TRUE	1
#define	FALSE	0
#endif

/* Memory allocation error message.
 */
#define nomem() (fputs("Out of memory!\n", stderr), exit(EXIT_FAILURE))

/* Structure used to organize strings to be displayed in a list.
 */
typedef	struct textline {
    char       *str;	/* the string to display */
    int		size;	/* the current length of the string */
    int		left;	/* how much more the string can grow */
} textline;

/* The online help text.
 */
static char const *yowzitch = 
    "Usage: elfls [OPTIONS] FILE...\n"
    "Display information about the contents of ELF files.\n\n"
    "  -c, --sources       Display list of source files.\n"
    "  -d, --dependencies  Display list of dependencies.\n"
    "  -P, --nophdr        Omit the program header table.\n"
    "  -S, --noshdr        Omit the section header table.\n"
    "  -i, --nostr         Don't display some section contents.\n"
    "  -p, --nopos         Omit file position column.\n"
    "  -w, --width=N       Set maximum width of output.\n"
    "      --help          Display this help and exit.\n"
    "      --version       Display version information and exit.\n";

/* The version text.
 */
static char const *vourzhon =
    "elfls: version 1.1\n"
    "Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>\n"
    "License GPLv2+: GNU GPL version 2 or later.\n"
    "This is free software; you are free to change and redistribute it.\n"
    "There is NO WARRANTY, to the extent permitted by law.\n";

/* The global variables.
 */
static Elf64_Ehdr	elffhdr;	/* ELF header of current file */
static Elf64_Phdr      *proghdr = NULL;	/* program header table */
static Elf64_Shdr      *secthdr = NULL;	/* section header table */
static char	       *sectstr = NULL;	/* sh string table */

static int		proghdrs;	/* FALSE if no ph table */
static int		secthdrs;	/* FALSE if no sh table */
static Elf64_Phdr      *phentry = NULL;	/* ph with the entry point */
static Elf64_Shdr      *shshstr = NULL;	/* sh with the sh string table */

static char const      *programname;	/* name of this program */
static char const      *thefilename;	/* name of current file */
static FILE	       *thefile;	/* handle to current file */

static int		phdrls = TRUE;	/* TRUE = show ph table */
static int		shdrls = TRUE;	/* TRUE = show sh table */
static int		srcfls = FALSE;	/* TRUE = show source files */
static int		ldepls = FALSE;	/* TRUE = show libraries */
static int		dostrs = TRUE;	/* TRUE = show entry strings */
static int		dooffs = TRUE;	/* TRUE = show file offsets */

static char		sizefmt[8];	/* num digits to show sizes */
static char		addrfmt[8];	/* num digits to show addresses */
static char		offsetfmt[8];	/* num digits to show offsets */
static int		outwidth;	/* maximum width of output */

/* The error-reporting function.
 */
static int err(char const *fmt, ...)
{
    va_list args;

    fprintf(stderr, "%s: ", programname);
    if (fmt) {
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
    } else {
	fprintf(stderr, "%s: %s", thefilename, strerror(errno));
    }
    fputc('\n', stderr);
    return 0;
}

/*
 * List output formatting functions.
 */

/* Allocate a textline array, including one initialized string for
 * each element.
 */
static textline *gettextlines(int count)
{
    char       *text;
    textline   *lines;
    int		width;
    int		i;

    width = outwidth ? outwidth : 256;
    if (!(lines = malloc(count * (sizeof *lines + width))))
	nomem();
    text = (char*)(lines + count);
    for (i = 0 ; i < count ; ++i) {
	lines[i].str = text;
	lines[i].str[0] = '\0';
	lines[i].size = 0;
	lines[i].left = width - 1;
	text += width;
    }
    return lines;
}

/* Concatenate formatted text to the given textline string.
 */
static int append(textline *line, char const *fmt, ...)
{
    va_list	args;
    int		n;

    if (line->left <= 0)
	return 0;
    va_start(args, fmt);
    n = vsnprintf(line->str + line->size, line->left, fmt, args);
    va_end(args);
    if (n < 0) {
	line->size += line->left;
	line->left = 0;
    } else {
	line->size += n;
	line->left -= n;
    }
    return line->left;
}

/* Function to pass to qsort().
 */
static int linesorter(const void *i1, const void *i2)
{
    return strcmp(((textline*)i1)->str, ((textline*)i2)->str);
}

/* Output the given textline array strings to a stream, nicely
 * formatted in columns, somewhat in the style of ls.
 */
static void formatlist(FILE *fp, textline *lines, int count)
{
    textline   *line;
    int		colx, dy;
    int		i, j;

    colx = 1;
    for (i = 0, line = lines ; i < count ; ++i, ++line) {
	if (line->size > colx)
	    colx = line->size;
    }
    if (outwidth) {
	i = (outwidth - 1) / colx;
	dy = i ? (count + i - 1) / i : count;
	colx = -(colx + 1);
    } else {
	dy = count;
	colx = 0;
    }

    for (j = 0 ; j < dy ; ++j) {
	for (i = j ; i < count - dy ; i += dy)
	    fprintf(fp, "%*s", colx, lines[i].str);
	fprintf(fp, "%s\n", lines[i].str);
    }
}

/* Output the given list and its header text on one line. If it
 * won't fit on one line, then use formatlist() instead.
 */
static void outputlist(FILE *fp, textline *lines, int count,
		       char const *header)
{
    int maxwidth;
    int	i, n;

    if (!count)
	return;
    fputs(header, fp);
    if (outwidth) {
	maxwidth = outwidth - strlen(header);
	for (i = n = 0 ; i < count ; ++i) {
	    if ((n += lines[i].size + 1) >= maxwidth) {
		fputc('\n', fp);
		formatlist(fp, lines, count);
		return;
	    }
	}
    }
    fputs(lines[0].str, fp);
    for (i = 1 ; i < count ; ++i) {
	fputc(' ', fp);
	fputs(lines[i].str, fp);
    }
    fputc('\n', fp);
}

/*
 * Generic file-reading functions.
 */

/* Read a piece of the current file into a freshly allocated buffer.
 */
static void *getarea(long offset, unsigned long size)
{
    void       *buf;

    if (fseek(thefile, offset, SEEK_SET))
	return NULL;
    if (!(buf = malloc(size)))
	nomem();
    if (fread(buf, size, 1, thefile) != 1) {
	free(buf);
	return NULL;
    }
    return buf;
}

/* Read a null-terminated string from the current file, up to a given
 * size, into a static buffer. If skip is nonzero, then skip over NULs
 * at the beginning of the string.
 */
static char const *getstring(unsigned long offset, unsigned long size,
			     int skip)
{
    static char	       *buf = NULL;
    static size_t	buflen = 0;
    char	       *str;
    int			n;

    if (!size)
	return "";
    if (size > buflen) {
	if (!(buf = realloc(buf, size + 1)))
	    nomem();
	buflen = size;
    }
    if (fseek(thefile, offset, SEEK_SET))
	return "";
    if (skip) {
	if ((n = fread(buf, 1, size, thefile)) <= 0)
	    return "";
	for (str = buf ; !*str && n ; ++str, --n) ;
    } else {
	if (!fgets(buf, size, thefile))
	    return "";
	n = size;
	str = buf;
    }
    if (!n)
	return "";
    str[n] = '\0';
    for (n = 0 ; str[n] ; ++n)
	if (str[n] < ' ' || str[n] > '~')
	    str[n] = '.';
    return str;
}

/*
 * Functions for examining ELF structures.
 */

/* Verify that the given ELF identifier is appropriate to our
 * expectations.
 */
static int checkelfident(unsigned char const id[EI_NIDENT])
{
    if (memcmp(id, ELFMAG, SELFMAG))
	return err("%s: not an ELF file.", thefilename);
    if (id[EI_CLASS] != ELFCLASS32 && id[EI_CLASS] != ELFCLASS64)
	return err("%s: unrecognized ELF class: %d.",
		   thefilename, (int)id[EI_CLASS]);
    if (id[EI_DATA] != ELFDATA2MSB && id[EI_DATA] != ELFDATA2LSB)
	return err("%s: unrecognized ELF data: %d.",
		   thefilename, id[EI_DATA]);
    if (id[EI_VERSION] != EV_CURRENT)
	return err("%s: unrecognized ELF version: %d.",
		   thefilename, (int)id[EI_VERSION]);

    return TRUE;
}

/* Read in the ELF header proper, and verify that its contents conform
 * to what the program can decipher.
 */
static int readelfhdr(void)
{
    if (elfrw_read_Ehdr(thefile, &elffhdr) != 1) {
	if (ferror(thefile))
	    return err(NULL);
	else
	    return err("%s: not an ELF file.", thefilename);
    }
    if (!checkelfident(elffhdr.e_ident))
	return FALSE;

    switch (elffhdr.e_type) {
      case ET_REL:
      case ET_EXEC:
      case ET_DYN:
      case ET_CORE:
	break;
      default:
	return err("%s: unknown ELF file type (type = %u).",
		   thefilename, elffhdr.e_type);
    }
    if (elffhdr.e_ehsize != sizeof(Elf32_Ehdr) &&
			elffhdr.e_ehsize != sizeof(Elf64_Ehdr))
	return err("%s: warning: unrecognized ELF header size: %d.",
		   thefilename, elffhdr.e_ehsize);
    if (elffhdr.e_version != EV_CURRENT)
	return err("%s: unrecognized ELF header version: %u.",
		   thefilename, (unsigned int)elffhdr.e_version);

    if (elffhdr.e_phoff != 0) {
	if (elffhdr.e_phentsize != sizeof(Elf32_Phdr) &&
			elffhdr.e_phentsize != sizeof(Elf64_Phdr))
	    err("%s: unrecognized program header entry size: %u.",
		thefilename, elffhdr.e_phentsize);
	else
	    proghdrs = 1;
    }
    if (elffhdr.e_shoff != 0) {
	if (elffhdr.e_shentsize != sizeof(Elf32_Shdr) &&
			elffhdr.e_shentsize != sizeof(Elf64_Shdr))
	    err("%s: unrecognized section header entry size: %u.",
		thefilename, elffhdr.e_shentsize);
	else
	    secthdrs = 1;
    }
    return TRUE;
}

/* Read in the program header table, if it is present. If the ELF
 * header lists an entry point, then compare the entry point with the
 * load address of the program header entries to determine which one
 * contains said point.
 */
static int readproghdrs(void)
{
    int i, n;

    if (!proghdrs)
	return TRUE;
    n = elffhdr.e_phnum;
    if (!(proghdr = malloc(n * sizeof *proghdr)))
	nomem();
    if (fseek(thefile, elffhdr.e_phoff, SEEK_SET) ||
			elfrw_read_Phdrs(thefile, proghdr, n) != n) {
	err("%s: invalid program header table offset.", thefilename);
	proghdrs = FALSE;
	return TRUE;
    }
    if (elffhdr.e_entry) {
	for (i = 0 ; i < n ; ++i) {
	    if (proghdr[i].p_type == PT_LOAD
			&& elffhdr.e_entry >= proghdr[i].p_vaddr
			&& elffhdr.e_entry < proghdr[i].p_vaddr
						 + proghdr[i].p_memsz) {
		phentry = proghdr + i;
		break;
	    }
	}
    }

    return TRUE;
}

/* Read in the section header table, if it is present. If a section
 * header string table is given in the ELF header, then also load the
 * string table contents.
 */
static int readsecthdrs(void)
{
    int n;

    if (!secthdrs)
	return TRUE;
    n = elffhdr.e_shnum;
    if (!(secthdr = malloc(n * sizeof *secthdr)))
	nomem();
    if (fseek(thefile, elffhdr.e_shoff, SEEK_SET) ||
			elfrw_read_Shdrs(thefile, secthdr, n) != n) {
	err("%s: warning: invalid section header table offset.", thefilename);
	secthdrs = FALSE;
	return TRUE;
    }
    if (elffhdr.e_shstrndx != SHN_UNDEF) {
	shshstr = secthdr + elffhdr.e_shstrndx;
	if (!(sectstr = getarea(shshstr->sh_offset, shshstr->sh_size))) {
	    err("%s: warning: invalid string table location.", thefilename);
	    free(sectstr);
	    sectstr = NULL;
	}
    }

    return TRUE;
}

/* Extract the list of source files, if present. The source files are
 * determined by loading the symbol table section (and its associated
 * string table) and looking up symbols of type STT_FILE.
 */
static int getsrcfiles(textline **plines)
{
    Elf64_Sym  *syms;
    char       *nmstr;
    textline   *lines;
    char       *str;
    unsigned	strtab, count, i;
    int		j, n;

    if (!secthdrs)
	return 0;
    for (i = 0 ; i < elffhdr.e_shnum ; ++i)
	if (secthdr[i].sh_type == SHT_SYMTAB)
	    break;
    if (i == elffhdr.e_shnum)
	return 0;

    count = secthdr[i].sh_size / secthdr[i].sh_entsize;
    strtab = secthdr[i].sh_link;
    if (!(syms = malloc(count * sizeof *syms)))
	nomem();
    if (fseek(thefile, secthdr[i].sh_offset, SEEK_SET))
	return err("%s: invalid symbol table offset.", thefilename);
    count = elfrw_read_Syms(thefile, syms, count);
    if (!count)
	return 0;
    nmstr = getarea(secthdr[strtab].sh_offset, secthdr[strtab].sh_size);
    if (!nmstr) {
	free(syms);
	return 0;
    }
    lines = gettextlines(count);
    n = 0;
    for (i = 0 ; i < count ; ++i) {
	if (ELF64_ST_TYPE(syms[i].st_info) != STT_FILE)
	    continue;
	str = nmstr + syms[i].st_name;
	for (j = 0 ; j < n ; ++j)
	    if (!strcmp(lines[j].str, str))
		break;
	if (j == n)
	    append(lines + n++, "%s", str);
    }
    free(syms);
    free(nmstr);
    if (n)
	*plines = lines;
    else
	free(lines);
    return n;
}

/* Extract the list of dependencies, if present. The program header
 * table entry of type PT_DYNAMIC contains the dynamic info, which
 * includes the dependent files, and the string table that contains
 * the file names. However, the string table is referenced by its
 * location in memory, as opposed to its location in the file, so
 * the program header table needs to be searched again to find the
 * part that will be loaded at that location.
 */
static int getlibraries(textline **plines)
{
    Elf64_Dyn  *dyns;
    char       *nmstr;
    textline   *lines;
    char       *str;
    unsigned long strtab = 0, strsz = 0;
    unsigned	count, i, j, n;

    if (!proghdrs)
	return 0;
    for (i = 0 ; i < elffhdr.e_phnum ; ++i)
	if (proghdr[i].p_type == PT_DYNAMIC)
	    break;
    if (i == elffhdr.e_phnum)
	return 0;

    count = elfrw_count_Dyns(proghdr[i].p_filesz);
    if (!(dyns = malloc(count * sizeof *dyns)))
	nomem();
    if (fseek(thefile, proghdr[i].p_offset, SEEK_SET))
	return err("%s: invalid dynamic table offset.", thefilename);
    count = elfrw_read_Dyns(thefile, dyns, count);
    if (!count)
	return 0;
    n = 0;
    for (i = 0 ; i < count ; ++i) {
	if (dyns[i].d_tag == DT_STRTAB)
	    strtab = dyns[i].d_un.d_ptr;
	else if (dyns[i].d_tag == DT_STRSZ)
	    strsz = dyns[i].d_un.d_val;
	else if (dyns[i].d_tag == DT_NEEDED)
	    ++n;
    }
    if (!strtab || !strsz)
	return 0;
    for (i = 0 ; i < elffhdr.e_phnum ; ++i)
	if (strtab >= proghdr[i].p_vaddr
		&& strtab < proghdr[i].p_vaddr + proghdr[i].p_filesz)
	    break;
    if (i == elffhdr.e_phnum)
	return 0;
    if (!(nmstr = getarea(proghdr[i].p_offset + (strtab - proghdr[i].p_vaddr),
			  strsz)))
	return 0;
    lines = gettextlines(n);
    n = 0;
    for (i = 0 ; i < count ; ++i) {
	if (dyns[i].d_tag != DT_NEEDED)
	    continue;
	str = nmstr + dyns[i].d_un.d_val;
	for (j = 0 ; j < n ; ++j)
	    if (!strcmp(lines[j].str, str))
		break;
	if (j == n)
	    append(lines + n++, "%s", str);
    }
    free(dyns);
    free(nmstr);
    if (n)
	*plines = lines;
    else
	free(lines);
    return n;
}

/*
 * Output-generating functions.
 */

/* Determine how wide the size and offset columns need to be.
 */
static void makenumberfmts(void)
{
    unsigned long maxsize, maxaddr, maxoffset;
    int i;

    maxsize = 0;
    maxoffset = 0;
    maxaddr = 0;
    for (i = 0 ; i < elffhdr.e_phnum ; ++i) {
	if (maxsize < proghdr[i].p_filesz)
	    maxsize = proghdr[i].p_filesz;
	if (maxoffset < proghdr[i].p_offset)
	    maxoffset = proghdr[i].p_offset;
	if (maxaddr < proghdr[i].p_vaddr)
	    maxaddr = proghdr[i].p_vaddr;
    }
    for (i = 0 ; i < elffhdr.e_shnum ; ++i) {
	if (maxsize < secthdr[i].sh_size)
	    maxsize = secthdr[i].sh_size;
	if (maxoffset < secthdr[i].sh_offset)
	    maxoffset = secthdr[i].sh_offset;
    }
    maxsize >>= 20;
    for (i = 6 ; maxsize ; ++i, maxsize >>= 4) ;
    sprintf(sizefmt, "%%%dlX", i);
    maxoffset >>= 20;
    for (i = 6 ; maxoffset ; ++i, maxoffset >>= 4) ;
    sprintf(offsetfmt, "%%%dlX", i);
    maxaddr >>= 16;
    maxaddr >>= 16;
    for (i = 8 ; maxaddr ; ++i, maxaddr >>= 4) ;
    sprintf(addrfmt, "%%0%dlX", i);
}

/* Display a one-line description of the ELF file.
 */
static void describeehdr(FILE *fp)
{
    fprintf(fp, "%s", thefilename);

    switch (elffhdr.e_type) {
      case ET_REL:						break;
      case ET_EXEC:	fputc('*', fp);				break;
      case ET_DYN:	fputc('&', fp);				break;
      case ET_CORE:	fputc('$', fp);				break;
      default:		fprintf(fp, "?(%u)", elffhdr.e_type);	break;
    }

    switch (elffhdr.e_machine) {
      case EM_M32:	fprintf(fp, " (AT&T M32)");		break;
      case EM_386:	fprintf(fp, " (Intel 386)");		break;
      case EM_X86_64:	fprintf(fp, " (Intel x86-64)");		break;
      case EM_860:	fprintf(fp, " (Intel 860)");		break;
      case EM_MIPS:	fprintf(fp, " (MIPS)");			break;
      case EM_68K:	fprintf(fp, " (Motorola 68k)");		break;
      case EM_88K:	fprintf(fp, " (Motorola 88k)");		break;
      case EM_SPARC:	fprintf(fp, " (SPARC)");		break;
      case EM_S390:	fprintf(fp, " (IBM S390)");		break;
      case EM_ALPHA:	fprintf(fp, " (Digital Alpha)");	break;
      case EM_ARM:	fprintf(fp, " (ARM)");			break;
      case EM_PARISC:	fprintf(fp, " (HPPA)");			break;
      case EM_PPC:	fprintf(fp, " (PowerPC)");		break;
      case EM_PPC64:	fprintf(fp, " (64-bit PowerPC)");	break;
      default:		fprintf(fp, " (?%u)", elffhdr.e_machine); break;
    }

    fputc('\n', fp);
}

/* Fill the given string with the terse description of the given
 * program header table entry. The description includes a character
 * that identifies the entry type, the segment permission flags, the
 * offset and size within the file, and the virtual address at which
 * to load the contents.
 */
static void describephdr(textline *line, Elf64_Phdr *phdr)
{
    char const *str;
    int		n;

    switch (phdr->p_type) {
      case PT_NULL:	    append(line, "(null)");	return;
      case PT_INTERP:	    append(line, "I ");		break;
      case PT_NOTE:	    append(line, "N ");		break;
      case PT_LOAD:	    append(line, "B ");		break;
      case PT_PHDR:	    append(line, "P ");		break;
      case PT_DYNAMIC:	    append(line, "D ");		break;
      case PT_TLS:	    append(line, "T ");		break;
#ifdef PT_GNU_EH_FRAME
      case PT_GNU_EH_FRAME: append(line, "U ");		break;
#endif
#ifdef PT_GNU_STACK
      case PT_GNU_STACK:    append(line, ". ");		break;
#endif
#ifdef PT_GNU_RELRO
      case PT_GNU_RELRO:    append(line, "R ");		break;
#endif
      default:		    append(line, "? ");		break;

    }

    if (dostrs) {
	switch (phdr->p_type) {
	  case PT_INTERP:
	    str = getstring(phdr->p_offset, phdr->p_filesz, FALSE);
	    if (str && *str) {
		n = strlen(str);
		if (n > 30)
		    append(line, "\"%.27s...\"", str);
		else
		    append(line, "\"%s\"", str);
		return;
	    }
	    break;
	  case PT_NOTE:
	    if (phdr->p_filesz > 12) {
		str = getstring(phdr->p_offset + 12,
				phdr->p_filesz - 12, FALSE);
		if (str && *str) {
		    n = strlen(str);
		    if (n > 30)
			append(line, "\"%.27s...\"", str);
		    else
			append(line, "\"%s\"", str);
		    return;
		}
	    }
	    break;
	}
    }

    append(line, "%c%c%c",
		 (phdr->p_flags & PF_R ? 'r' : '-'),
		 (phdr->p_flags & PF_W ? 'w' : '-'),
		 (phdr->p_flags & PF_X ? phdr == phentry ? 's' : 'x' : '-'));
    if (dooffs)
	append(line, sizefmt, phdr->p_offset);
    append(line, sizefmt, phdr->p_filesz);
    append(line, " ");
    append(line, addrfmt, phdr->p_vaddr);
    if (phdr->p_filesz != phdr->p_memsz)
	append(line, " +%lX", phdr->p_memsz - phdr->p_filesz);
}

/* Fill the given string with the terse description of the given
 * section header table entry. The description includes a character
 * that identifies the entry type, the flags, the offset and size
 * within the file, and the section name and the indices of any
 * related sections.
 */
static void describeshdr(textline *line, Elf64_Shdr *shdr)
{
    char const *str;
    int		n;

    switch (shdr->sh_type) {
      case SHT_NULL:
	append(line, "(null)");
	return;

      case SHT_PROGBITS:
	if (sectstr) {
	    str = sectstr + shdr->sh_name;
	    if (!strcmp(str, ".comment")) {
		append(line, "C ");
		if (dostrs) {
		    str = getstring(shdr->sh_offset, shdr->sh_size, TRUE);
		    if (str && *str) {
			n = strlen(str);
			if (n > 30)
			    append(line, "\"%.27s...\"", str);
			else
			    append(line, "\"%s\"", str);
			return;
		    }
		}
		break;
	    } else if (!strcmp(str, ".interp")) {
		append(line, "I ");
		if (dostrs) {
		    str = getstring(shdr->sh_offset, shdr->sh_size, FALSE);
		    if (str && *str) {
			n = strlen(str);
			if (n > 30)
			    append(line, "\"%.27s...\"", str);
			else
			    append(line, "\"%s\"", str);
			return;
		    }
		}
		break;
	    } else if (!strcmp(str, ".plt") || !strcmp(str, ".got.plt")) {
		append(line, "P ");
		break;
	    } else if (!strcmp(str, ".got")) {
		append(line, "O ");
		break;
	    } else if (!memcmp(str, ".debug_", 7)) {
		append(line, "G ");
		break;
	    } else if (!memcmp(str, ".eh_", 4)) {
		append(line, "U ");
		break;
	    }
	}
	append(line, "B ");
	break;

      case SHT_NOTE:		append(line, "N ");	break;
      case SHT_STRTAB:		append(line, "$ ");	break;
      case SHT_SYMTAB:		append(line, "S ");	break;
      case SHT_DYNSYM:		append(line, "S ");	break;
      case SHT_DYNAMIC:		append(line, "D ");	break;
      case SHT_REL:		append(line, "R ");	break;
      case SHT_RELA:		append(line, "R ");	break;
      case SHT_HASH:		append(line, "H ");	break;
      case SHT_GROUP:		append(line, "g ");	break;
      case SHT_NOBITS:		append(line, "0 ");	break;
#ifdef SHT_GNU_HASH
      case SHT_GNU_HASH:	append(line, "H ");	break;
#endif
#ifdef SHT_GNU_verdef
      case SHT_GNU_verdef:	append(line, "V ");	break;
      case SHT_GNU_verneed:	append(line, "V ");	break;
      case SHT_GNU_versym:	append(line, "V ");	break;
#endif
      default:			append(line, "? ");	break;
    }

    if (dostrs) {
	if (shdr->sh_type == SHT_NOTE) {
	    str = getstring(shdr->sh_offset + 12, shdr->sh_size - 12, FALSE);
	    if (str && *str) {
		n = strlen(str);
		if (n > 30)
		    append(line, "\"%.27s...\"", str);
		else
		    append(line, "\"%s\"", str);
		return;
	    }
	}
    }

    append(line, "%c%c%c",
		 (shdr->sh_flags & SHF_ALLOC ? 'r' : '-'),
		 (shdr->sh_flags & SHF_WRITE ? 'w' : '-'),
		 (shdr->sh_flags & SHF_EXECINSTR ? 'x' : '-'));
    if (dooffs)
	append(line, offsetfmt, shdr->sh_offset);
    append(line, sizefmt, shdr->sh_size);
    append(line, " %s", sectstr ? sectstr + shdr->sh_name : "(n/a)");
    if (shdr->sh_type == SHT_REL || shdr->sh_type == SHT_RELA)
	append(line, ":%lu", shdr->sh_info);
    if (shdr->sh_link)
	append(line, " [%lu]", shdr->sh_link);
    if (shdr->sh_type == SHT_STRTAB && shdr == shshstr)
	append(line, " [S]");
}

/*
 * Top-level functions.
 */

/* Parse the command-line options.
 */
static void readoptions(int argc, char *argv[])
{
    static char const *optstring = "cdiPpSw:";
    static struct option const options[] = {
	{ "sources", no_argument, NULL, 'c' },
	{ "dependencies", no_argument, NULL, 'd' },
	{ "nophdr", no_argument, NULL, 'P' },
	{ "noshdr", no_argument, NULL, 'S' },
	{ "nostr", no_argument, NULL, 'i' },
	{ "nopos", no_argument, NULL, 'p' },
	{ "width", required_argument, NULL, 'w' },
	{ "help", no_argument, NULL, 'H' },
	{ "version", no_argument, NULL, 'V' },
	{ 0, 0, 0, 0 }
    };

    char const *str;
    int n;

    programname = argv[0];
    if (argc == 1) {
	fputs(yowzitch, stdout);
	exit(EXIT_SUCCESS);
    }

    if (!(str = getenv("COLUMNS")) || (outwidth = atoi(str)) <= 0)
	outwidth = 80;

    while ((n = getopt_long(argc, argv, optstring, options, NULL)) != EOF) {
	switch (n) {
	  case 'c':	srcfls = TRUE;			break;
	  case 'd':	ldepls = TRUE;			break;
	  case 'P':	phdrls = FALSE;			break;
	  case 'S':	shdrls = FALSE;			break;
	  case 'i':	dostrs = FALSE;			break;
	  case 'p':	dooffs = FALSE;			break;
	  case 'w':	outwidth = atoi(optarg);	break;
	  case 'H':	fputs(yowzitch, stdout);	exit(EXIT_SUCCESS);
	  case 'V':	fputs(vourzhon, stdout);	exit(EXIT_SUCCESS);
	  default:
	    fprintf(stderr, "Try --help for more information.\n");
	    exit(EXIT_FAILURE);
	}
    }
    if (outwidth < 0) {
	err("invalid width parameter.");
	exit(EXIT_FAILURE);
    }
}

/* main().
 */
int main(int argc, char *argv[])
{
    textline   *lines = NULL;
    char      **arg;
    int		ret = 0;
    int		count, i;

    readoptions(argc, argv);
    if (optind == argc) {
	err("nothing to do.");
	exit(EXIT_FAILURE);
    }

    for (arg = argv + optind ; (thefilename = *arg) != NULL ; ++arg) {
	if (!(thefile = fopen(thefilename, "rb"))) {
	    perror(thefilename);
	    ++ret;
	    continue;
	}
	if (!readelfhdr() || !readproghdrs() || !readsecthdrs()) {
	    fclose(thefile);
	    ++ret;
	    continue;
	}

	describeehdr(stdout);
	if (ldepls && proghdrs) {
	    if ((count = getlibraries(&lines))) {
		outputlist(stdout, lines, count, "Dependencies: ");
		free(lines);
	    }
	}
	if (srcfls && secthdrs) {
	    if ((count = getsrcfiles(&lines))) {
		qsort(lines, count, sizeof *lines, linesorter);
		outputlist(stdout, lines, count, "Source files: ");
		free(lines);
	    }
	}

	makenumberfmts();
	if (phdrls && proghdrs) {
	    printf("Program header table entries: %d", elffhdr.e_phnum);
	    if (dooffs)
		printf(" (%lX - %lX)",
		       (unsigned long)elffhdr.e_phoff,
		       (unsigned long)elffhdr.e_phoff +
				elffhdr.e_phnum * elffhdr.e_phentsize);
	    putchar('\n');
	    lines = gettextlines(elffhdr.e_phnum);
	    for (i = 0 ; i < elffhdr.e_phnum ; ++i) {
		append(lines + i, "%2d ", i);
		describephdr(lines + i, proghdr + i);
	    }
	    formatlist(stdout, lines, elffhdr.e_phnum);
	    free(lines);
	}

	if (shdrls && secthdrs) {
	    printf("Section header table entries: %d", elffhdr.e_shnum);
	    if (dooffs)
		printf(" (%lX - %lX)",
		       (unsigned long)elffhdr.e_shoff,
		       (unsigned long)elffhdr.e_shoff +
					elffhdr.e_shnum * elffhdr.e_shentsize);
	    putchar('\n');
	    lines = gettextlines(elffhdr.e_shnum);
	    for (i = 0 ; i < elffhdr.e_shnum ; ++i) {
		append(lines + i, "%2d ", i);
		describeshdr(lines + i, secthdr + i);
	    }
	    formatlist(stdout, lines, elffhdr.e_shnum);
	    free(lines);
	}

	fclose(thefile);
	free(proghdr);
	free(secthdr);
	free(sectstr);
    }

    return ret;
}
