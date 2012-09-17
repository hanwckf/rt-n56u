/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <unistd.h>
#include <errno.h>


/* Defined in brk.c.  */
extern void *__curbrk attribute_hidden;

/* Extend the process's data space by INCREMENT.
   If INCREMENT is negative, shrink data space by - INCREMENT.
   Return start of new space allocated, or -1 for errors.  */
void * sbrk (intptr_t increment)
{
    void *oldbrk;

    if (__curbrk == NULL)
	if (brk (NULL) < 0)	/* Initialize the break.  */
	    return (void *) -1;

    if (increment == 0)
	return __curbrk;

    oldbrk = __curbrk;
    if (brk (oldbrk + increment) < 0)
	return (void *) -1;

    return oldbrk;
}
libc_hidden_def(sbrk)
