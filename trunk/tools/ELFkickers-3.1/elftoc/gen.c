/* gen.c: General-purpose functions.
 * Copyright (C) 1999,2011 by Brian Raiter <breadbox@muppetlabs.com>
 * License GPLv2+: GNU GPL version 2 or later.
 * This is free software; you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include "gen.h"

static char const *programname = "elftoc";	/* the program name */
static char const *filename;			/* the current file's name */
static int showwarnings = TRUE;			/* true to display warnings */

/* Sets the program name prefixing error messages.
 */
void setprogramname(char const *str)
{
    programname = str;
}

/* Sets a filename to display as a prefix to error messages.
 */
void setfilename(char const *str)
{
    filename = str;
}

/* Turns warnings on or off.
 */
void enablewarnings(int flag)
{
    showwarnings = flag;
}

/* Displays a warning message (unless warnings are disabled) and
 * returns zero.
 */
int warn(char const *fmt, ...)
{
    va_list args;

    if (showwarnings) {
	fprintf(stderr, "%s: ", programname);
	if (filename)
	    fprintf(stderr, "%s: ", filename);
	fputs("warning: ", stderr);
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fputc('\n', stderr);
    }
    return FALSE;
}

/* Displays an error message and returns zero.
 */
int err(char const *fmt, ...)
{
    va_list args;

    fprintf(stderr, "%s: ", programname);
    if (filename)
	fprintf(stderr, "%s: ", filename);
    if (fmt) {
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
    } else {
	fprintf(stderr, "%s", strerror(errno));
    }
    fputc('\n', stderr);
    return FALSE;
}

/* Displays an error message and exits.
 */
void fail(char const *fmt, ...)
{
    va_list args;

    fprintf(stderr, "%s: ", programname);
    if (filename)
	fprintf(stderr, "%s: ", filename);
    if (fmt) {
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
    } else {
	fprintf(stderr, "%s", strerror(errno));
    }
    fputc('\n', stderr);
    exit(EXIT_FAILURE);
}

/* Allocates memory, or exits if the memory is unavailable.
 */
void *allocate(unsigned int size)
{
    void *ptr;

    ptr = malloc(size);
    if (!ptr)
	fail(strerror(ENOMEM));
    return ptr;
}

/* Changes an existing memory allocation, or exits on failure.
 */
void *reallocate(void *ptr, unsigned int size)
{
    ptr = realloc(ptr, size);
    if (!ptr)
	fail(strerror(ENOMEM));
    return ptr;
}

/* Copies a string to newly allocated memory, or exits on failure.
 */
char *strallocate(char const *str)
{
    char *copy;
    size_t size;

    size = strlen(str) + 1;
    copy = allocate(size);
    memcpy(copy, str, size);
    return copy;
}

/* Deallocates memory.
 */
void deallocate(void *ptr)
{
    free(ptr);
}
