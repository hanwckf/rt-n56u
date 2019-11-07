/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <stdio.h>
#include <string.h>
#include <netdb.h>


static const char error_msg[] = "Resolver error";
static const char *const h_errlist[] = {
	"Error 0",
	"Unknown host",			    /* 1 HOST_NOT_FOUND */
	"Host name lookup failure",	    /* 2 TRY_AGAIN */
	"Unknown server error",		    /* 3 NO_RECOVERY */
	"No address associated with name",  /* 4 NO_ADDRESS */
};
static const int h_nerr = { sizeof(h_errlist)/sizeof(h_errlist[0]) };

/*
 * herror -- print the error indicated by the h_errno value.
 */
void herror(const char *s)
{
	static const char colon_space[] = ": ";
	const char *p;
	const char *c;

	c = colon_space;
	if (!s || !*s) {
		c += 2;
	}
	p = error_msg;
	if ((h_errno >= 0) && (h_errno < h_nerr)) {
		p = h_errlist[h_errno];
	}
	fprintf(stderr, "%s%s%s\n", s, c, p);
}
libc_hidden_def(herror)


const char *hstrerror(int err)
{
	if ((unsigned)err < h_nerr)
		return(h_errlist[err]);

	return error_msg;
}
