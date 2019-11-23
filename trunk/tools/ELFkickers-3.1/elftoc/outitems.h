/* outitems.h: Functions to output primitive elements.
 * Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#ifndef _outitems_h_
#define _outitems_h_

/*
 * The functions in this module create a string representation of the
 * input and return a pointer to it. The pointer returned is typically
 * to a private buffer, which will be reused on a subsequent call to
 * the same function.
 */

/* Renders an arbitrary formatted string and returns it.
 */
extern char const *strf(char const *fmt, ...);

/* Renders an arbitrary formatted string and outputs it as a single
 * item.
 */
extern void outf(char const *fmt, ...);

/* Renders a number as a decimal literal.
 */
extern char const *strdec(unsigned long number);

/* Renders a number as a hexadecimal literal.
 */
extern char const *strhex(unsigned long number);

/* Renders a byte value as a C character literal.
 */
extern char const *strchar(unsigned char byte);

/* Renders an address value.
 */
extern char const *straddress(long address);

/* Renders a byte value as a predefined macro with the given prefix
 * (if possible, otherwise renders it as a character).
 */
extern char const *strdefbyte(unsigned char byte, char const *prefix);

/* Renders a numerical value as a predefined macro with the given
 * prefix, or as a decimal literal.
 */
extern char const *strdefint(unsigned long number, char const *prefix);

/* Renders a numerical value as a series of bitwise flags ORed
 * together, and/or a hexadecimal value if necessary.
 */
extern char const *strdefflags(unsigned long flags, char const *prefix);

/*
 * Convenience macros for outputting items.
 */

#define outdec(n)	    (out(strdec(n)))
#define outhex(n)	    (out(strhex(n)))
#define outchar(b)	    (out(strchar(b)))
#define outaddress(a)	    (out(straddress(a)))
#define outdefbyte(b, p)    (out(strdefbyte(b, p)))
#define outdefint(n, p)	    (out(strdefint(n, p)))
#define outdefflags(f, p)   (out(strdefflags(f, p)))

#endif
