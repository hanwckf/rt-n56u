/* vi: set sw=4 ts=4: */
/*
 * Test application for functions defined in ctype.h
 *
 * Copyright (C) 2000 by Lineo, inc. and Erik Andersen
 * Copyright (C) 2000,2001 by Erik Andersen <andersen@uclibc.org>
 * Written by Erik Andersen <andersen@uclibc.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include "../testsuite.h"

int got_abort;

void aborthandler(int junk)
{
	got_abort=1;
}

int main( int argc, char **argv)
{
	signal(SIGABRT, aborthandler);
	
	init_testsuite("Testing functions defined in assert.h:\n\t");

	got_abort=0;
	assert(0==0);
	TEST_NUMERIC(got_abort, 0);

#define  NDEBUG
	got_abort=0;
	printf("Don't worry -- This next test is supposed to print an assert message:\n");
	fprintf(stderr, "\t");
	assert(0==1);
	TEST_NUMERIC(got_abort, 0);

#undef  NDEBUG
	got_abort=0;
	assert(0==1);
	TEST_NUMERIC(got_abort, 1);

	exit(0);
}
