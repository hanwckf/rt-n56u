/* outitems.c: Functions to output primitive elements.
 * Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include "gen.h"
#include "outbase.h"
#include "names.h"
#include "pieces.h"
#include "address.h"
#include "outitems.h"

/* Formats a string into a local buffer.
 */
char const *strf(char const *fmt, ...)
{
    static char *buf = NULL;
    static int bufsize = 0;
    va_list args;
    int n;

    for (;;) {
	va_start(args, fmt);
	n = vsnprintf(buf, bufsize, fmt, args);
	va_end(args);
	if (n >= 0 && n < bufsize)
	    break;
	bufsize = n > 0 ? n + 1 : bufsize ? bufsize * 2 : 64;
	buf = reallocate(buf, bufsize);
    }
    return buf;
}

/* Creates a formatted string and passes it to out().
 */
void outf(char const *fmt, ...)
{
    static char *buf = NULL;
    static int bufsize = 0;
    va_list args;
    int n;

    for (;;) {
	va_start(args, fmt);
	n = vsnprintf(buf, bufsize, fmt, args);
	va_end(args);
	if (n >= 0 && n < bufsize)
	    break;
	bufsize = n > 0 ? n + 1 : bufsize ? bufsize * 2 : 64;
	buf = reallocate(buf, bufsize);
    }
    out(buf);
}

/* Renders an integer as a decimal literal.
 */
char const *strdec(unsigned long number)
{
    static char buf[24];

    sprintf(buf, "%lu", number);
    return buf;
}

/* Renders an integer in hexadecimal. (The use of ULONG_MAX instead of
 * a constant is done to avoid pointless warnings on 32-bit machines.)
 */
char const *strhex(unsigned long number)
{
    static char buf[20];

    if (sizeof number > 4 && number > 0xFFFFFFFFL) {
	sprintf(buf, number > (ULONG_MAX >> 4) ? "0x%016lX" : "0x%012lX",
		     number);
    } else {
	sprintf(buf,number > 0x00FFFFFFL ? "0x%08lX"  :
		    number > 0x0000FFFFL ? "0x%06lX"  :
		    number > 0x000000FFL ? "0x%04lX"  :
		    number > 0x0000000FL ? "0x%lX"    : "%lu", number);
    }
    return buf;
}

/* Returns the C representation for a character value as a literal
 * character constant.
 */
char const *strchar(unsigned char ch)
{
    static char buf[8];

    switch (ch) {
      case '\a': return "'\\a'";	case '\t': return "'\\t'";
      case '\b': return "'\\b'";	case '\v': return "'\\v'";
      case '\f': return "'\\f'";	case '\'': return "'\\''";
      case '\n': return "'\\n'";	case '\\': return "'\\\\'";
      case '\r': return "'\\r'";	case '\0': return "0";
    }

    if (isprint(ch) || ch == ' ') {
	buf[0] = '\'';
	buf[1] = ch;
	buf[2] = '\'';
	buf[3] = '\0';
    } else {
	sprintf(buf, "0x%02X", (unsigned char)ch);
    }
    return buf;
}

/* Renders a memory address, preferably as a base address plus a file
 * offset.
 */
char const *straddress(long address)
{
    char const *name, *offset;
    long base;

    if (address == 0)
	return "0";
    name = getbaseaddress(address, &base);
    if (!name)
	return strhex(address);
    if (!*name)
	return getoffsetstr(address, getindexfromoffset(address));
    if (address == base)
	return name;
    offset = getoffsetstr(address - base, getindexfromoffset(address - base));
    return strf("%s + %s", name, offset);
}

/* Render a character value, using a macro name with the given prefix
 * if possible.
 */
char const *strdefbyte(unsigned char byte, char const *prefix)
{
    char const *name;

    name = findname(byte, prefix);
    if (name)
	return name;
    else if (byte == 0)
	return "0";
    else
	return strchar(byte);
}

/* Render an integer value, using a macro name with the given prefix
 * if possible, otherwise using a decimal literal.
 */
char const *strdefint(unsigned long number, char const *prefix)
{
    char const *name;

    name = findname(number, prefix);
    if (name)
	return name;
    return strdec(number);
}

/* Output a value as a collection of bitwise flags, using macro names
 * with the given prefix where possible.
 */
char const *strdefflags(unsigned long flags, char const *prefix)
{
    static char *buf = NULL;
    static int bufsize = 0;
    char const *name;
    int size, n;

    if (flags == 0)
	return "0";
    size = 0;
    while (flags) {
	name = findflag(&flags, prefix);
	if (!name)
	    break;
	n = 3 + strlen(name);
	if (size + n > bufsize) {
	    bufsize += n;
	    buf = reallocate(buf, bufsize + 32);
	}
	sprintf(buf + size, " | %s", name);
	size += n;
    }

    if (size == 0)
	return strhex(flags);
    if (flags)
	sprintf(buf + size, " | %s", strhex(flags));
    return buf + 3;
}
