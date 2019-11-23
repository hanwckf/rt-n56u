/* objres: Copyright (C) 2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>
#include <getopt.h>
#include <elf.h>
#include "elfrw.h"

/*
 * objres produces an object file with the following layout:
 *
 * 1. An ELF header.
 * 2. A section header table with four entries: one for each section
 *    following this one.
 * 3. A symbol table with two entries for each binary object (one
 *    local symbol for the source file name and one global symbol for
 *    the data).
 * 4. A string table accompanying the symbol table.
 * 5. A string table accompanying the section header table.
 * 6. A .data section, containing all of the actual data.
 *
 * This layout is reflected in the ordering of the pieces array.
 */

/* The online help text.
 */
static char const *yowzitch =
    "Usage: objres [OPTIONS] [SYMBOL=]FILENAME ...\n"
    "Compile arbitrary binary data as an object file.\n\n"
    "  -o, --output=FILENAME   Set the name of the object file to create.\n"
    "  -h, --header=FILENAME   Set the name of the C header file to create.\n"
    "  -m, --machine=[32|64]   Set the object file to be 32-bit or 64-bit.\n"
    "  -R, --reference=OBJFILE Use an existing object file as a template.\n"
    "  -r, --read-only         Make the exported objects const.\n"
    "      --help              Display this help and exit.\n"
    "      --version           Display version information and exit.\n\n"
    "An input filename can be prefixed with an explicit name to use as the\n"
    "exported object's symbol. If no symbol name is given, its name will be\n"
    "derived from the input filename. Similarly, if the names of the object\n"
    "file and/or header file are not explicitly declared, their names will\n"
    "be derived from the (first) symbol name.\n";

/* Version and license information.
 */
static char const *vourzhon =
    "objres, version 1.1\n"
    "Copyright (C) 2011 by Brian Raiter <breadbox@muppetlabs.com>\n"
    "License GPLv2+: GNU GPL version 2 or later.\n"
    "This is free software; you are free to change and redistribute it.\n"
    "There is NO WARRANTY, to the extent permitted by law.\n";

/* Data about each binary object being added to the ELF object file.
 */
struct object {
    char const *filename;		/* filename for the binary data */
    char const *objectname;		/* name of the variable to create */
    int		filenamestrpos;		/* filename position in .strtab */
    int		objectnamestrpos;	/* objectname position in .strtab */
    long	offset;			/* offset of the object in .data */
    long	size;			/* size of the object */
    void const *data;			/* the actual binary data */
};

/* Data about each piece of the ELF object file to be created.
 */
struct elfpiece {
    int		shndx;			/* section index (0 if not in shdr) */
    char const *name;			/* name of the section */
    int		namestrpos;		/* position of name in .shstrtab */
    int		type;			/* section type ID */
    long	offset;			/* offset of the section in the file */
    long	size;			/* total size of the section */
    int		entsize;		/* size of each section entry */
    int		entcount;		/* number of entries */
    int		align;			/* the section's alignment */
    int		link;			/* the section's shdr link value */
    int		info;			/* the section's shdr info value */
    void      (*output)(void);		/* pointer to the output function */
};

/*
 * Global variables.
 */

static char const *programname;		/* the name of this program */
static char const *outputfile = NULL;	/* name of the object file to create */
static char const *headerfile = NULL;	/* name of the header file to create */
static FILE *destfile = NULL;		/* the currently-open output file */
static Elf64_Ehdr refehdr;		/* a reference ELF header to output */
static int output64;			/* true if building a 64-bit target */
static int readonly = 0;		/* true if exporting const objects */

/* The list of binary objects that will go into the object file's data.
 */
static int objectcount = 0;
static struct object *objects = NULL;

/* The list of pieces that will comprise the ELF object file.
 */
static int const piececount = 6;
static struct elfpiece pieces[6];

/* Direct pointers to the individual pieces.
 */
static struct elfpiece *piece_ehdr = &pieces[0];
static struct elfpiece *piece_shtab = &pieces[1];
static struct elfpiece *piece_symtab = &pieces[2];
static struct elfpiece *piece_strtab = &pieces[3];
static struct elfpiece *piece_shstrtab = &pieces[4];
static struct elfpiece *piece_data = &pieces[5];

/*
 * General-purpose functions.
 */

/* Report an error and exit.
 */
static void fail(char const *fmt, ...)
{
    va_list args;

    fprintf(stderr, "%s: ", programname);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
    exit(EXIT_FAILURE);
}

/* Report a I/O error on the output file and exit.
 */
static void ferr()
{
    fail("%s: %s", outputfile,
		   ferror(destfile) ? strerror(errno) : "write error");
}

/* Two simple memory-allocation wrappers that exit upon failure.
 */
static void *allocate(size_t size)
{
    void *p = malloc(size);
    if (!p)
	fail(strerror(ENOMEM));
    return p;
}

static void *reallocate(void *p, size_t size)
{
    p = realloc(p, size);
    if (!p)
	fail(strerror(ENOMEM));
    return p;
}

/* Copy the contents of the given file to the current output file.
 */
static void fcopy(char const *srcfilename)
{
    FILE *srcfile;
    char buf[BUFSIZ];
    int n;

    srcfile = fopen(srcfilename, "rb");
    if (!srcfile)
	fail("%s: %s", srcfilename, strerror(errno));
    for (;;) {
	n = fread(buf, 1, sizeof buf, srcfile);
	if (n == 0) {
	    if (ferror(srcfile))
		fail("%s: %s", srcfilename, strerror(errno));
	    break;
	}
	if (fwrite(buf, n, 1, destfile) != 1)
	    ferr();
    }
    fclose(srcfile);
}

/*
 * ELF section output functions.
 */

/* Examine a pre-existing object file and extract its ELF header.
 */
static void readreferenceheader(char const *filename)
{
    FILE *file;

    file = fopen(filename, "rb");
    if (!file)
	fail("%s: %s", filename, strerror(errno));
    if (!elfrw_read_Ehdr(file, &refehdr))
	fail("%s: %s", filename, strerror(errno));
    fclose(file);
    output64 = refehdr.e_ident[EI_CLASS] == ELFCLASS64;
}

/* Create an empty ELF header template for our object file. By default
 * an x86 header is created with the same bitness as the current
 * program.
 */
static void makedefaultheader(void)
{
    memset(&refehdr, 0, sizeof refehdr);
    refehdr.e_ident[EI_MAG0] = ELFMAG0;
    refehdr.e_ident[EI_MAG1] = ELFMAG1;
    refehdr.e_ident[EI_MAG2] = ELFMAG2;
    refehdr.e_ident[EI_MAG3] = ELFMAG3;
    refehdr.e_ident[EI_CLASS] = output64 ? ELFCLASS64 : ELFCLASS32;
    refehdr.e_ident[EI_DATA] = ELFDATA2LSB;
    refehdr.e_ident[EI_VERSION] = EV_CURRENT;
    refehdr.e_ident[EI_OSABI] = ELFOSABI_NONE;
    refehdr.e_machine = output64 ? EM_X86_64 : EM_386;
    refehdr.e_version = EV_CURRENT;
}

/* Output the ELF header for the object file.
 */
static void outputehdr(void)
{
    Elf64_Ehdr ehdr;

    ehdr = refehdr;
    ehdr.e_type = ET_REL;
    memset(&ehdr, 0, sizeof ehdr);
    ehdr.e_ident[EI_MAG0] = ELFMAG0;
    ehdr.e_ident[EI_MAG1] = ELFMAG1;
    ehdr.e_ident[EI_MAG2] = ELFMAG2;
    ehdr.e_ident[EI_MAG3] = ELFMAG3;
    ehdr.e_ident[EI_CLASS] = refehdr.e_ident[EI_CLASS];
    ehdr.e_ident[EI_DATA] = refehdr.e_ident[EI_DATA];
    ehdr.e_ident[EI_VERSION] = refehdr.e_ident[EI_VERSION];
    ehdr.e_ident[EI_OSABI] = refehdr.e_ident[EI_OSABI];
    ehdr.e_machine = refehdr.e_machine;
    ehdr.e_version = refehdr.e_version;
    ehdr.e_entry = 0;
    ehdr.e_phoff = 0;
    ehdr.e_shoff = piece_shtab->offset;
    ehdr.e_flags = 0;
    ehdr.e_ehsize = piece_ehdr->size;
    ehdr.e_phentsize = 0;
    ehdr.e_phnum = 0;
    ehdr.e_shentsize = piece_shtab->entsize;
    ehdr.e_shnum = piece_shtab->entcount;
    ehdr.e_shstrndx = piece_shstrtab->shndx;
    if (!elfrw_write_Ehdr(destfile, &ehdr))
	ferr();
}

/* Output the object file's string table, containing the names of the
 * exported objects and the input files.
 */
static void outputstrtab(void)
{
    int n, i;

    fputc(0, destfile);
    for (i = 0 ; i < objectcount ; ++i) {
	n = strlen(objects[i].filename) + 1;
	if (fwrite(objects[i].filename, n, 1, destfile) != 1)
	    ferr();
	n = strlen(objects[i].objectname) + 1;
	if (fwrite(objects[i].objectname, n, 1, destfile) != 1)
	    ferr();
    }
}

/* Output the object file's section header string table.
 */
static void outputshstrtab(void)
{
    int n, i;

    fputc(0, destfile);
    for (i = 0 ; i < piececount ; ++i) {
	if (!pieces[i].name)
	    continue;
	n = strlen(pieces[i].name) + 1;
	if (fwrite(pieces[i].name, n, 1, destfile) != 1)
	    ferr();
    }
}

/* Output the object file's data section, containing the binary data
 * for each object.
 */
static void outputdata(void)
{
    long pos;
    int i;

    pos = 0;
    for (i = 0 ; i < objectcount ; ++i) {
	for ( ; pos < objects[i].offset ; ++pos)
	    fputc(0, destfile);
	if (objects[i].data) {
	    if (fwrite(objects[i].data, objects[i].size, 1, destfile) != 1)
		ferr();
	    free((void*)objects[i].data);
	    objects[i].data = NULL;
	} else {
	    fcopy(objects[i].filename);
	}
	pos += objects[i].size;
    }
}

/* Output the object file's section header table, one entry for each
 * piece with a nonzero index.
 */
static void outputshtab(void)
{
    Elf64_Shdr shdr;
    int n, i;

    memset(&shdr, 0, sizeof shdr);
    if (!elfrw_write_Shdr(destfile, &shdr))
	ferr();
    for (n = 1 ; n <= piececount ; ++n) {
	for (i = 0 ; i < piececount ; ++i)
	    if (pieces[i].shndx == n)
		break;
	if (i == piececount)
	    continue;
	shdr.sh_name = pieces[i].namestrpos;
	shdr.sh_type = pieces[i].type;
	if (pieces[i].type == SHT_PROGBITS)
	    shdr.sh_flags = readonly ? SHF_ALLOC : SHF_ALLOC | SHF_WRITE;
	else
	    shdr.sh_flags = 0;
	shdr.sh_addr = 0;
	shdr.sh_offset = pieces[i].offset;
	shdr.sh_size = pieces[i].size;
	shdr.sh_link = pieces[i].link;
	shdr.sh_info = pieces[i].info;
	shdr.sh_addralign = pieces[i].align;
	shdr.sh_entsize = pieces[i].entsize;
	if (!elfrw_write_Shdr(destfile, &shdr))
	    ferr();
    }
}

/* Output the object file's symbol table, containing one section
 * symbol, one file symbol for each input file, and one exported
 * symbol for each binary object.
 */
static void outputsymtab(void)
{
    Elf64_Sym sym;
    int i;

    sym.st_name = 0;
    sym.st_info = 0;
    sym.st_other = 0;
    sym.st_shndx = SHN_UNDEF;
    sym.st_value = 0;
    sym.st_size = 0;
    if (!elfrw_write_Sym(destfile, &sym))
	ferr();
    sym.st_info = ELF64_ST_INFO(STB_LOCAL, STT_SECTION);
    sym.st_shndx = piece_data->shndx;
    if (!elfrw_write_Sym(destfile, &sym))
	ferr();
    sym.st_shndx = SHN_UNDEF;
    for (i = 0 ; i < objectcount ; ++i) {
	sym.st_name = objects[i].filenamestrpos;
	sym.st_info = ELF64_ST_INFO(STB_LOCAL, STT_FILE);
	if (!elfrw_write_Sym(destfile, &sym))
	    ferr();
    }
    for (i = 0 ; i < objectcount ; ++i) {
	sym.st_name = objects[i].objectnamestrpos;
	sym.st_info = ELF64_ST_INFO(STB_GLOBAL, STT_OBJECT);
	sym.st_shndx = piece_data->shndx;
	sym.st_value = objects[i].offset;
	sym.st_size = objects[i].size;
	if (!elfrw_write_Sym(destfile, &sym))
	    ferr();
    }
}

/*
 * Data collection functions.
 */

/* Fill in the standard initial values for each section.
 */
static void initpieces(void)
{
    piece_ehdr->shndx = 0;
    piece_ehdr->output = outputehdr;
    piece_ehdr->entsize = output64 ? sizeof(Elf64_Ehdr) : sizeof(Elf32_Ehdr);
    piece_ehdr->align = 1;

    piece_shtab->shndx = 0;
    piece_shtab->output = outputshtab;
    piece_shtab->entsize = output64 ? sizeof(Elf64_Shdr) : sizeof(Elf32_Shdr);
    piece_shtab->align = output64 ? sizeof(Elf64_Addr) : sizeof(Elf32_Addr);

    piece_symtab->shndx = 1;
    piece_symtab->type = SHT_SYMTAB;
    piece_symtab->name = ".symtab";
    piece_symtab->output = outputsymtab;
    piece_symtab->entsize = output64 ? sizeof(Elf64_Sym) : sizeof(Elf32_Sym);
    piece_symtab->align = output64 ? sizeof(Elf64_Addr) : sizeof(Elf32_Addr);

    piece_strtab->shndx = 2;
    piece_strtab->type = SHT_STRTAB;
    piece_strtab->name = ".strtab";
    piece_strtab->output = outputstrtab;
    piece_strtab->entsize = 0;
    piece_strtab->align = 1;
    piece_strtab->link = 0;
    piece_strtab->info = 0;

    piece_shstrtab->shndx = 3;
    piece_shstrtab->type = SHT_STRTAB;
    piece_shstrtab->name = ".shstrtab";
    piece_shstrtab->output = outputshstrtab;
    piece_shstrtab->entsize = 0;
    piece_shstrtab->align = 1;
    piece_shstrtab->link = 0;
    piece_shstrtab->info = 0;

    piece_data->shndx = 4;
    piece_data->type = SHT_PROGBITS;
    piece_data->name = readonly ? ".rodata" : ".data";
    piece_data->output = outputdata;
    piece_data->entsize = 0;
    piece_data->align = 32;
    piece_data->link = 0;
    piece_data->info = 0;
}

/* Determine the size of the input files. Seekable files are simply
 * measured; non-seekable files are read completely and stored in
 * memory.
 */
static void measureobjects(void)
{
    FILE *srcfile;
    char *p;
    int i, n;

    for (i = 0 ; i < objectcount ; ++i) {
	srcfile = fopen(objects[i].filename, "rb");
	if (!srcfile)
	    fail("%s: %s", objects[i].filename, strerror(errno));
	if (fseek(srcfile, 0, SEEK_END) != -1) {
	    objects[i].size = ftell(srcfile);
	    fclose(srcfile);
	    continue;
	}
	if (errno != ESPIPE)
	    fail("%s: %s", objects[i].filename, strerror(errno));
	objects[i].size = 0;
	p = NULL;
	for (;;) {
	    p = reallocate(p, objects[i].size + BUFSIZ);
	    n = fread(p + objects[i].size, 1, BUFSIZ, srcfile);
	    objects[i].size += n;
	    if (n < BUFSIZ) {
		if (ferror(srcfile))
		    fail("%s: %s", objects[i].filename, strerror(errno));
		break;
	    }
	}
	objects[i].data = reallocate(p, objects[i].size);
	fclose(srcfile);
    }
}

/* Determine the size and position of each ELF section, and fill in
 * any remaining metadata values.
 */
static void measurepieces(void)
{
    long pos;
    int i;

    piece_ehdr->size = piece_ehdr->entsize;

    piece_shtab->entcount = 1;
    for (i = 0 ; i < piececount ; ++i) {
	if (pieces[i].shndx > 0)
	    ++piece_shtab->entcount;
    }
    piece_shtab->size = piece_shtab->entcount * piece_shtab->entsize;

    piece_symtab->entcount = 2 + 2 * objectcount;
    piece_symtab->size = piece_symtab->entcount * piece_symtab->entsize;
    piece_symtab->info = 2 + objectcount;
    piece_symtab->link = piece_strtab->shndx;

    piece_strtab->size = 1;
    for (i = 0 ; i < objectcount ; ++i) {
	objects[i].filenamestrpos = piece_strtab->size;
	piece_strtab->size += 1 + strlen(objects[i].filename);
	objects[i].objectnamestrpos = piece_strtab->size;
	piece_strtab->size += 1 + strlen(objects[i].objectname);
    }

    piece_shstrtab->size = 1;
    for (i = 0 ; i < piececount ; ++i) {
	if (pieces[i].name) {
	    pieces[i].namestrpos = piece_shstrtab->size;
	    piece_shstrtab->size += 1 + strlen(pieces[i].name);
	}
    }

    piece_data->size = 0;
    for (i = 0 ; i < objectcount ; ++i) {
	piece_data->size = (piece_data->size + 31) & ~31;
	objects[i].offset = piece_data->size;
	piece_data->size += objects[i].size;
    }

    pos = 0;
    for (i = 0 ; i < piececount ; ++i) {
	pos = (pos + pieces[i].align - 1) & ~(pieces[i].align - 1);
	pieces[i].offset = pos;
	pos += pieces[i].size;
    }
}

/*
 * Top-level output functions.
 */

/* Create the object file by calling each section's output function.
 */
static void outputelf(void)
{
    long pos;
    int i;

    destfile = fopen(outputfile, "wb");
    if (!destfile)
	ferr();

    pos = 0;
    for (i = 0 ; i < piececount ; ++i) {
	for ( ; pos < pieces[i].offset ; ++pos)
	    fputc(0, destfile);
	pieces[i].output();
	pos += pieces[i].size;
    }

    if (fclose(destfile))
	ferr();
    destfile = NULL;
}

/* Output a C header file that declares each object as an array of
 * unsigned char.
 */
static void outputheader(void)
{
    FILE *file;
    char const *name;
    char *ppsymbol;
    int i;

    file = fopen(headerfile, "wt");
    if (!file)
	fail("%s: %s", headerfile, strerror(errno));

    name = strrchr(headerfile, '/');
    name = name ? name + 1 : headerfile;
    ppsymbol = allocate(strlen(name) + 3);
    ppsymbol[0] = '_';
    for (i = 0 ; name[i] ; ++i)
	ppsymbol[i + 1] = isalnum(name[i]) ? name[i] : '_';
    ppsymbol[i + 1] = '_';
    ppsymbol[i + 2] = '\0';

    fprintf(file, "#ifndef %s\n#define %s\n\n", ppsymbol, ppsymbol);
    for (i = 0 ; i < objectcount ; ++i)
	fprintf(file, "extern unsigned char %s%s[%ld];\n",
		      (readonly ? "const " : ""),
		      objects[i].objectname, objects[i].size);
    fprintf(file, "\n#endif\n");

    if (fclose(file))
	fail("%s: %s", headerfile, strerror(errno));
    free(ppsymbol);
}

/* Generate an object filename and/or a header filename if the user
 * did not provide one.
 */
static void pickfilenames(void)
{
    char const *filename;
    char *str, *p;
    int n;

    if (!outputfile) {
	filename = strrchr(objects[0].objectname, ':');
	filename = filename ? filename + 1 : objects[0].objectname;
	if (!*filename) {
	    outputfile = "a.o";
	} else {
	    n = strlen(filename) + 1;
	    str = allocate(n + 2);
	    memcpy(str, filename, n);
	    p = strrchr(str, '.');
	    if (p)
		memcpy(p, ".o", 3);
	    else
		memcpy(str + n - 1, ".o", 3);
	    outputfile = str;
	}
    }

    if (!headerfile) {
	n = strlen(outputfile) + 1;
	str = allocate(n + 2);
	memcpy(str, outputfile, n);
	if (memcmp(str + n - 3, ".o", 3))
	    memcpy(str + n - 1, ".h", 3);
	else
	    memcpy(str + n - 3, ".h", 3);
	headerfile = str;
    }
}

/* Parse the command-line arguments and initialize the objects array.
 */
static void readcmdline(int argc, char *argv[])
{
    static char const *optstring = "h:m:o:rR:";
    static struct option options[] = {
	{ "header", required_argument, NULL, 'h' },
	{ "output", required_argument, NULL, 'o' },
	{ "machine", required_argument, NULL, 'm' },
	{ "reference", required_argument, NULL, 'R' },
	{ "read-only", no_argument, NULL, 'r' },
	{ "help", no_argument, NULL, 'H' },
	{ "version", no_argument, NULL, 'V' },
	{ 0, 0, 0, 0 }
    };

    char const *filename;
    char const *refobjfilename = NULL;
    char *p;
    int i, n;

    programname = argv[0];
    output64 = -1;

    while ((n = getopt_long(argc, argv, optstring, options, NULL)) != EOF) {
	switch (n) {
	  case 'h':
	    if (headerfile)
		fail("multiple header files specified");
	    headerfile = optarg;
	    break;
	  case 'o':
	    if (outputfile)
		fail("multiple output files specified");
	    outputfile = optarg;
	    break;
	  case 'm':
	    if (!strcmp(optarg, "32"))
		output64 = 0;
	    else if (!strcmp(optarg, "64"))
		output64 = 1;
	    else
		fail("invalid machine word size: must be either 32 or 64");
	    break;
	  case 'R':
	    refobjfilename = optarg;
	    break;
	  case 'r':
	    readonly = 1;
	    break;
	  case 'H':
	    fputs(yowzitch, stdout);
	    exit(EXIT_SUCCESS);
	  case 'V':
	    fputs(vourzhon, stdout);
	    exit(EXIT_SUCCESS);
	  default:
	    fail("Try --help for more information.");
	}
    }
    if (optind >= argc)
	fail("no input files");

    if (refobjfilename) {
	if (output64 >= 0)
	    fail("--machine conflicts with --reference; use only one");
	readreferenceheader(refobjfilename);
    } else {
	if (output64 < 0)
	    output64 = sizeof(void*) > 4;
	makedefaultheader();
    }

    objectcount = argc - optind;
    objects = allocate(objectcount * sizeof *objects);
    for (i = 0 ; i < objectcount ; ++i) {
	p = strchr(argv[optind + i], '=');
	if (p) {
	    *p = '\0';
	    objects[i].objectname = argv[optind + i];
	    objects[i].filename = p + 1;
	} else {
	    objects[i].filename = argv[optind + i];
	    objects[i].objectname = NULL;
	}
	if (!*objects[i].filename)
	    fail("object name with no input file: \"%s\"", argv[optind + i]);
	if (!objects[i].objectname || !*objects[i].objectname) {
	    p = strrchr(objects[i].filename, '/');
	    filename = p ? p + 1 : objects[i].filename;
	    n = strlen(filename) + 1;
	    p = allocate(n + 1);
	    if (isdigit(*filename)) {
		*p = '_';
		memcpy(p + 1, filename, n);
	    } else {
		memcpy(p, filename, n);
	    }
	    objects[i].objectname = p;
	    for ( ; *p ; ++p)
		if (!isalnum(*p))
		    *p = '_';
	}
    }
}

/* main().
 */
int main(int argc, char *argv[])
{
    readcmdline(argc, argv);
    measureobjects();

    initpieces();
    measurepieces();

    pickfilenames();
    outputelf();
    outputheader();

    return 0;
}
