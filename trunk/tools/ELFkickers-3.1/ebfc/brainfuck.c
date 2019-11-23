/* brainfuck.c: The Brainfuck compiler proper.
 *
 * Copyright (C) 1999-2001 by Brian Raiter, under the GNU General
 * Public License. No warranty. See COPYING for details.
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stddef.h>
#include	<elf.h>
#include	"elfparts.h"
#include	"ebfc.h"

/*
 * The bits of machine language that, together, make up a compiled
 * program. The prolog and epilog are different, depending on the type
 * of program being created. In addition, the prologs are declared as
 * structures, so that internal offsets may be computed.
 */

#define	MLBIT(size)	static struct { unsigned char _dummy[size]; } const 

MLBIT(4) start =    { { 0x31, 0xC0,			/* xor  eax, eax   */
			0x99,				/* cdq             */
			0x42 } };			/* inc  edx        */

MLBIT(2) incchar =  { { 0xFE, 0x01 } };			/* inc  [ecx]      */
MLBIT(2) decchar =  { { 0xFE, 0x09 } };			/* dec  [ecx]      */
MLBIT(1) incptr =   { { 0x41 } };			/* inc  ecx        */
MLBIT(1) decptr =   { { 0x49 } };			/* dec  ecx        */

MLBIT(6) readchar = { { 0xB0, 0x03,			/* mov  al, 3      */
			0x31, 0xDB,			/* xor  ebx, ebx   */
			0xCD, 0x80 } };			/* int  0x80       */
MLBIT(6) writechar= { { 0xB0, 0x04,			/* mov  al, 4      */
			0x89, 0xD3,			/* mov  ebx, edx   */
			0xCD, 0x80 } };			/* int  0x80       */

MLBIT(5) jump =     { { 0xE9, 0, 0, 0, 0 } };		/* jmp  near ???   */
MLBIT(4) branch =   { { 0x3A, 0x31,			/* cmp  dh, [ecx]  */
			0x0F, 0x85 } };			/* jnz  near ...   */

MLBIT(2) addchar =  { { 0x80, 0x01 } };			/* add  [ecx], ... */
MLBIT(2) subchar =  { { 0x80, 0x29 } };			/* sub  [ecx], ... */
MLBIT(2) addptr =   { { 0x83, 0xC1 } };			/* add  ecx, ...   */
MLBIT(2) subptr =   { { 0x83, 0xE9 } };			/* sub  ecx, ...   */

MLBIT(5)
	epilog_ex = { { 0x92,				/* xchg eax, edx   */
			0x31, 0xDB,			/* xor  ebx, ebx   */
			0xCD, 0x80 } };			/* int  0x80       */
MLBIT(12)
	epilog_fn = { { 0xB9, 0x4C, 0x1D, 0x00, 0x00,	/* mov  ecx, 7500  */
			0x31, 0xC0,			/* xor  eax, eax   */
			0x5F,				/* pop  edi        */
			0xF3, 0xAB,			/* rep  stosd      */
			0x61,				/* popa            */
			0xC3 } };			/* ret             */

static struct prolog_ex
    { char _movecx[1], dataoff[4]; } const
	prolog_ex = { { 0xB9 }, { 0 } };		/* mov  ecx, ???   */

static struct prolog_fn
    { char _pusha[1];
      char _movecx[1], dataoff[4];
      char _pushecx[1]; } const
	prolog_fn = { { 0x60 },				/* pusha           */
		      { 0xB9 }, { 0 },			/* mov  ecx, ???   */
		      { 0x51 } };			/* push ecx        */

static struct prolog_so
    { char _pusha[1];
      char _callzero[5];
      char eipptr[1];
      char _addecx[2], gotpc[4];
      char _addecxagain[2], datagotoff[4];
      char _pushecx[1]; } const
	prolog_so = { { 0x60 },				/* pusha           */
		      { 0xE8, 0x00, 0x00, 0x00, 0x00 },	/* call $          */
		      { 0x59 },				/* pop  ecx        */
		      { 0x81, 0xC1 }, { 0 },		/* add  ecx, ???   */
		      { 0x81, 0xC1 }, { 0 },		/* add  ecx, ???   */
		      { 0x51 } };			/* push ecx        */

/* The size of a Brainfuck program's data segment.
 */
static int const	datafieldsize = 30000;

/* A pointer to the text buffer.
 */
static char	       *textbuf;

/* The amount of memory allocated for the text buffer.
 */
static int		textsize;

/* The first byte past the program being compiled in the text buffer.
 */
static int		pos;

/* Appends the given bytes to the program being compiled.
 */
static void emit(void const *bytes, int size)
{
    if (pos + size > textsize) {
	textsize += 4096;
	if (!(textbuf = realloc(textbuf, textsize))) {
	    fputs("Out of memory!\n", stderr);
	    exit(EXIT_FAILURE);
	}
    }
    memcpy(textbuf + pos, bytes, size);
    pos += size;
}

/* Macros to simplify calling emit with predefined bits of code.
 */
#define	emitobj(obj)		(emit(&(obj), sizeof (obj)))
#define	emitarg(seq, arg)	(emitobj(seq), emitobj(arg))

/* Modifies previously emitted code at the given position.
 */
#define	insertobj(at, obj)	(memcpy(textbuf + (at), &(obj), sizeof (obj)))

/* Translates a single, possibly repeated, Brainfuck command. Returns
 * false if a programmer error is detected.
 */
static int translatecmd(int cmd, char arg)
{
    static int	stack[256];
    static int *st;
    Elf32_Word	p;

    switch (cmd) {
      case '+':	   arg == 1 ? emitobj(incchar) : emitarg(addchar, arg);	break;
      case '-':	   arg == 1 ? emitobj(decchar) : emitarg(subchar, arg);	break;
      case '>':	   arg == 1 ? emitobj(incptr)  : emitarg(addptr, arg);	break;
      case '<':	   arg == 1 ? emitobj(decptr)  : emitarg(subptr, arg);	break;
      case ',':	while (arg--) emitobj(readchar);			break;
      case '.':	while (arg--) emitobj(writechar);			break;

      case '[':
	if (st - stack > (int)(sizeof stack / sizeof *stack)) {
	    err("too many levels of nested loops");
	    return FALSE;
	}
	while (arg--) {
	    emitobj(jump);
	    *st = pos;
	    ++st;
	}
	break;

      case ']':
	while (arg--) {
	    if (st == stack) {
		err("unmatched ]");
		return FALSE;
	    }
	    --st;
	    p = pos - *st;
	    insertobj(*st - sizeof p, p);
	    p = -(p + sizeof p + sizeof branch);
	    emitarg(branch, p);
	}
	break;

      case 0:
	st = stack;
	emitobj(start);
	break;

      case EOF:
	if (st != stack) {
	    err("unmatched [");
	    return FALSE;
	}
	break;
    }

    return TRUE;
}

/* Reads a Brainfuck source file and calls translatecmd() on the
 * contents. Repeated commands are conglomerated and translated into
 * single instructions.
 */
static int translateuncompressed(FILE *fp)
{
    static char		lkup[128] = { 0 };
    int			cmd;
    int			lastcmd;
    char		lastarg;

    lkup['+'] = lkup['-'] = lkup['<'] = lkup['>'] = 1;
    lkup[','] = lkup['.'] = lkup['['] = lkup[']'] = 1;

    lastcmd = 0;
    while ((cmd = fgetc(fp)) != EOF) {
	if ((unsigned int)cmd >= sizeof lkup || !lkup[cmd])
	    continue;
	if (cmd == lastcmd && lastarg < 127)
	    ++lastarg;
	else {
	    if (!translatecmd(lastcmd, lastarg))
		return FALSE;
	    lastcmd = cmd;
	    lastarg = 1;
	}
    }
    return translatecmd(lastcmd, lastarg) && translatecmd(EOF, 0);
}

/* Reads a compressed Brainfuck source file and calls translatecmd()
 * on the contents. Compression scheme is as follows:
 *
 * 000: +  001: -  010: <  011: >  100: [  101: ]  110: ,  111: .
 *
 * 00xxxXXX = singleton:  xxx (when xxx == XXX)
 * 00xxxyyy = pair:       xxx then yyy
 * 10xxyyzz = triple:     0xx then 0yy then 0zz
 * 01xxxyyy = repetition: yyy repeated 2+xxx times (2-9)
 * 11xxxxyy = repetition: 0yy repeated 2+xxxx times (2-17)
 */
static int translatecompressed(FILE *fp)
{
    char const *cmdlist = "+-<>[],.";
    int		byte;

    if (!translatecmd(0, 0))
	return FALSE;
    while ((byte = fgetc(fp)) != EOF) {
	switch (byte & 0xC0) {
	  case 0x00:
	    if (!translatecmd(cmdlist[(byte >> 3) & 7], 1))
		return FALSE;
	    if (((byte >> 3) & 7) != (byte & 7))
		if (!translatecmd(cmdlist[byte & 7], 1))
		    return FALSE;
	    break;
	  case 0x80:
	    if (!translatecmd(cmdlist[(byte >> 4) & 3], 1) ||
				!translatecmd(cmdlist[(byte >> 2) & 3], 1) ||
				!translatecmd(cmdlist[byte & 3], 1))
		return FALSE;
	    break;
	  case 0x40:
	    if (!translatecmd(cmdlist[byte & 7], 2 + ((byte >> 3) & 7)))
		return FALSE;
	    break;
	  case 0xC0:
	    if (!translatecmd(cmdlist[byte & 3], 2 + ((byte >> 2) & 15)))
		return FALSE;
	    break;
	}
    }
    return translatecmd(EOF, 0);
}

/* Adds entries to the relocation section and/or the symbol table,
 * depending on what kind of file is being generated.
 */
static void addrelocations(blueprint const *bp, int codetype,
			   char const *function)
{
    if (bp->filetype != ET_REL) {
	if (codetype == ET_DYN) {
	    addtosymtab(bp->parts + P_DYNSYM, function,
			STB_GLOBAL, STT_FUNC, P_TEXT);
	}
	return;
    }

    addtosymtab(bp->parts + P_SYMTAB, function,
		STB_GLOBAL, STT_FUNC, P_TEXT);

    switch (codetype) {
      case ET_EXEC:
	addrelsymbol(bp->parts + P_REL, offsetof(struct prolog_ex, dataoff),
		     R_386_32, NULL, STB_LOCAL, STT_OBJECT, P_DATA);
	break;
      case ET_REL:
	addrelsymbol(bp->parts + P_REL, offsetof(struct prolog_fn, dataoff),
		     R_386_32, NULL, STB_LOCAL, STT_OBJECT, P_DATA);
	break;
      case ET_DYN:
	addtorel(bp->parts + P_REL, offsetof(struct prolog_so, gotpc),
		 getsymindex(bp->parts + P_SYMTAB, NAME_GOT), R_386_GOTPC);
	addrelsymbol(bp->parts + P_REL, offsetof(struct prolog_so, datagotoff),
		     R_386_GOTOFF, NULL, STB_LOCAL, STT_OBJECT, P_DATA);
	break;
    }
}

/* Compiles the contents of the given file into the text segment of
 * the blueprint, as a global function with the given name.
 * Relocations and other fixups are added as appropriate, though with
 * incomplete values. The return value is false if a fatal error
 * occurs.
 */
int translatebrainfuck(char const *filename, blueprint const *bp,
		       int codetype, char const *function, int compressed)
{
    FILE       *fp;

    if (!(fp = fopen(filename, compressed ? "rb" : "r"))) {
	err(NULL);
	return FALSE;
    }

    pos = 0;
    textbuf = bp->parts[P_TEXT].part;
    textsize = bp->parts[P_TEXT].size;

    switch (codetype) {
      case ET_EXEC:	emitobj(prolog_ex);	break;
      case ET_REL:	emitobj(prolog_fn);	break;
      case ET_DYN:	emitobj(prolog_so);	break;
    }

    if (!(compressed ? translatecompressed(fp) : translateuncompressed(fp)))
	return FALSE;

    if (fclose(fp))
	err(NULL);

    switch (codetype) {
      case ET_EXEC:	emitobj(epilog_ex);	break;
      case ET_REL:	emitobj(epilog_fn);	break;
      case ET_DYN:	emitobj(epilog_fn);	break;
    }

    bp->parts[P_TEXT].size = pos;
    bp->parts[P_TEXT].part = textbuf;

    bp->parts[P_DATA].size = datafieldsize;
    if (!(bp->parts[P_DATA].part = calloc(datafieldsize, 1))) {
	err("Out of memory!");
	return FALSE;
    }

    addrelocations(bp, codetype, function);

    return TRUE;
}

/* Completes the relocations and other fixups that were added by
 * the previous function.
 */
void createfixups(blueprint const *bp, int codetype, char const *function)
{
    Elf32_Word	off, add;

    textbuf = bp->parts[P_TEXT].part;

    if (codetype == ET_DYN) {
	off = offsetof(struct prolog_so, gotpc);
	add = offsetof(struct prolog_so, eipptr);
	if (bp->filetype == ET_REL)
	    add = off - add;
	else
	    add = bp->parts[P_GOT].addr - (bp->parts[P_TEXT].addr + add);
	insertobj(off, add);
    }

    switch (codetype) {
      case ET_EXEC:	off = offsetof(struct prolog_ex, dataoff);	break;
      case ET_REL:	off = offsetof(struct prolog_fn, dataoff);	break;
      case ET_DYN:	off = offsetof(struct prolog_so, datagotoff);	break;
    }
    if (bp->filetype == ET_REL)
	add = 0;
    else if (codetype == ET_EXEC)
	add = bp->parts[P_DATA].addr;
    else if (codetype == ET_DYN)
	add = bp->parts[P_DATA].addr - bp->parts[P_GOT].addr;
    insertobj(off, add);

    if (bp->filetype == ET_REL)
	setsymvalue(bp->parts + P_SYMTAB, function, 0);
    else if (bp->filetype == ET_DYN)
	setsymvalue(bp->parts + P_DYNSYM, function,
					bp->parts[P_TEXT].addr);
    else if (bp->filetype == ET_EXEC)
	((Elf32_Ehdr*)bp->parts[P_EHDR].part)->e_entry =
					bp->parts[P_TEXT].addr;
}
