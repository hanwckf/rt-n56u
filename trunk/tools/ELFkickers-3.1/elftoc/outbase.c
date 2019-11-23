/* outbase.c: The lowest-level output functions.
 * Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "gen.h"
#include "outbase.h"

static int indent = 2;		/* how much to indent per level */
static int rmargin = 76;	/* the right-hand margin's "bell"; output
				   should not go more than 3 chars past this */
static int xpos = 0;		/* the cursor's current x-position */
static int level = 0;		/* the current level of indentation */
static int pendingcomma = 0;	/* nonzero if a comma should separate the next
				   item that is output (a negative value means
				   that a line break follows the comma) */
static char blockstack[8];	/* for each level, true if the braces around
				   that block go on separate lines */
static FILE *outfile;		/* the destination file */

/* Set the current output file.
 */
void setoutputfile(void *file)
{
    outfile = file;
}

/* Set the output line width.
 */
void setoutputwidth(int width)
{
    if (width)
	rmargin = width - 4;
}

/* Set the output indentation width.
 */
void setoutputindent(int width)
{
    if (width)
	indent = width;
}

/* Outputs a string and updates xpos. All output ultimately passes
 * through either this function or linebreak(). If an error occurs
 * during output, an error message is displayed and the program exits.
 */
static void textout(char const *str)
{
    int n;

    n = strlen(str);
    if (fputs(str, outfile) == EOF)
	fail(NULL);
    xpos += n;
    str = strrchr(str, '\n');
    if (str)
	xpos = strlen(str + 1);
}

/* Moves the output to the beginning of the next line, if the current
 * cursor position is not already at the beginning of a line.
 */
void linebreak(void)
{
    int n;

    if (pendingcomma) {
	fputc(',', outfile);
	pendingcomma = 0;
    }
    n = level * indent;
    if (xpos > n) {
	fputc('\n', outfile);
	xpos = 0;
    }
    if (xpos < n) {
	fprintf(outfile, "%*s", n - xpos, "");
	xpos = n;
    }
}

/* Starts a new block: outputs a left brace and increases the current
 * indentation level by one. If brk is true, then the block's braces
 * will be followed by line breaks; otherwise, the block's contents
 * will appear on the same lines as the braces.
 */
void beginblock(int brk)
{
    linebreak();
    blockstack[level] = brk;
    ++level;
    pendingcomma = 0;
    if (brk) {
	textout("{\n");
	linebreak();
    } else {
	textout("{ ");
    }
}

/* Closes the current block: outputs a right brace and decrements the
 * level of indentation.
 */
void endblock(void)
{
    pendingcomma = 0;
    --level;
    if (blockstack[level])
	linebreak();
    else
	textout(" ");
    if (level) {
	textout("}");
	pendingcomma = -1;
    } else {
	textout("};\n");
    }
}

/* Outputs a string, and if the indentation level is not zero (i.e.,
 * if the current output is inside a block, a separator is set as
 * pending.
 */
void out(char const *str)
{
    int n;

    n = strlen(str);
    if (xpos + n >= rmargin)
	linebreak();
    if (pendingcomma) {
	if (pendingcomma > 0 && xpos + 2 < rmargin)
	    textout(", ");
	else
	    linebreak();
    }
    textout(str);
    pendingcomma = level > 0;
}

/* Outputs given text as a C comment, with no separator pending
 * afterwards.
 */
void outcomment(char const *text)
{
    int pendingbreak = 0;
    int length;

    if (strstr(text, "*/")) {
	char *copy = strallocate(text);
	char *p = copy;
	while ((p = strstr(p, "*/")) != NULL)
	    *p = '+';
	outcomment(copy);
	deallocate(copy);
	return;
    }

    length = strlen(text);
    if (pendingcomma) {
	if (xpos + length + 9 >= rmargin) {
	    linebreak();
	} else {
	    textout(", ");
	    pendingbreak = (pendingcomma < 0);
	}
	pendingcomma = 0;
    } else {
	if (xpos + length + 7 >= rmargin)
	    linebreak();
    }
    textout("/* ");
    textout(text);
    textout(" */");
    if (pendingbreak)
	linebreak();
}

/* Returns the C representation for a character value, as it should
 * appear within a literal string constant.
 */
static char const *stringchar(char const *ptr)
{
    static char buf[8];

    switch (*ptr) {
      case '\a': return "\\a";		case '\r': return "\\r";
      case '\b': return "\\b";		case '\t': return "\\t";
      case '\f': return "\\f";		case '\v': return "\\v";
      case '\n': return "\\n";		case '\\': return "\\\\";
      case '"':  return "\\\"";
    }

    if (!*ptr && !(ptr[1] >= '0' && ptr[1] <= '7'))
	return "\\0";
    else if (isprint(*ptr) || *ptr == ' ') {
	buf[0] = *ptr;
	buf[1] = '\0';
    } else {
	buf[0] = '\\';
	buf[1] = '0' + ((unsigned char)*ptr / 64);
	buf[2] = '0' + (((unsigned char)*ptr / 8) % 8);
	buf[3] = '0' + ((unsigned char)*ptr % 8);
	buf[4] = '\0';
    }
    return buf;
}

/* Given a series of length bytes at str, returns the number of
 * characters that would be required to output those bytes within a
 * literal C string.
 */
long outstringsize(char const *str, long length)
{
    long size, i;

    size = 0;
    for (i = 0 ; i < length ; ++i) {
	switch (str[i]) {
	  case '\0':
	    if (i < length - 1 && str[i + 1] >= '0' && str[i + 1] <= '7')
		size += 4;
	    else
		size += 2;
	    break;
	  case '\a': case '\r':
	  case '\b': case '\t':
	  case '\f': case '\v':
	  case '\n': case '\\':
	  case '"':
	    size += 2;
	    break;
	  case ' ':
	    ++size;
	    break;
	  default:
	    if (isprint(str[i]))
		++size;
	    else
		size += 4;
	    break;
	}
    }
    return size;
}

/* Takes a string and outputs it as a literal C string, as a single
 * item.
 */
void outstring(char const *str, long length)
{
    char const *s;
    int pc, i;

    if (xpos >= rmargin || (length > 4 && xpos + 8 >= rmargin))
	linebreak();
    out("\"");
    pc = pendingcomma;
    pendingcomma = 0;
    ++level;
    for (i = 0, s = str ; i < length ; ++i, ++s) {
	textout(stringchar(s));
	if (xpos >= rmargin && i + 2 < length) {
	    textout("\"");
	    linebreak();
	    textout("\"");
	}
    }
    --level;
    textout("\"");
    pendingcomma = pc;
}
