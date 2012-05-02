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
#include <stdarg.h>
#include <ctype.h>
#include "../testsuite.h"


int main( int argc, char **argv)
{
	int i, c;


    init_testsuite("Testing functions defined in ctype.h\n");

	/* isalnum() */
	{
		int buffer[]={ '1', '4', 'a', 'z', 'A', 'Z', '5', -1};
		for(i=0; buffer[i]!=-1; i++) {
			c = buffer[i];
			TEST( isalnum(c)!=0);
		}
	}
	{
		int buffer[]={  2, 128, 254, '\n', -1};
		for(i=0; buffer[i]!=-1; i++) {
			c = buffer[i];
			TEST( isalnum(c)==0);
		}
	}



	/* isalpha() */
	{
		int buffer[]={ 'a', 'z', 'A', 'Z', -1};
		for(i=0; buffer[i]!=-1; i++) {
			c = buffer[i];
			TEST( isalpha(c)!=0);
		}
	}
	{
		int buffer[]={  2, 63, 128, 254, '\n', -1};
		for(i=0; buffer[i]!=-1; i++) {
			c = buffer[i];
			TEST( isalpha(c)==0);
		}
	}



	/* isascii() */
	{
		int buffer[]={ 'a', 'z', 'A', 'Z', '\n', -1};
		for(i=0; buffer[i]!=-1; i++) {
			c = buffer[i];
			TEST( isascii(c)!=0);
		}
	}
	{
		int buffer[]={  128, 254, -1};
		for(i=0; buffer[i]!=-1; i++) {
			c = buffer[i];
			TEST( isascii(c)==0);
		}
	}


	/* iscntrl() */
	{
		int buffer[]={ 0x7F, 6, '\t', '\n', 0x7F, -1};
		for(i=0; buffer[i]!=-1; i++) {
			c = buffer[i];
			TEST( iscntrl(c)!=0);
		}
	}
	{
		int buffer[]={  63, 128, 254, -1};
		for(i=0; buffer[i]!=-1; i++) {
			c = buffer[i];
			TEST( iscntrl(c)==0);
		}
	}


	/* isdigit() */
	{
		int buffer[]={ '1', '5', '7', '9', -1};
		for(i=0; buffer[i]!=-1; i++) {
			c = buffer[i];
			TEST( isdigit(c)!=0);
		}
	}
	{
		int buffer[]={  2, 'a', 'z', 'A', 'Z', 63, 128, 254, '\n', -1};
		for(i=0; buffer[i]!=-1; i++) {
			c = buffer[i];
			TEST( isdigit(c)==0);
		}
	}



	/* isgraph() */
	{
		int buffer[]={ ')', '~', '9', -1};
		for(i=0; buffer[i]!=-1; i++) {
			c = buffer[i];
			TEST( isgraph(c)!=0);
		}
	}
	{
		int buffer[]={ 9, ' ', '\t', '\n', 200, 0x7F, -1};
		for(i=0; buffer[i]!=-1; i++) {
			c = buffer[i];
			TEST( isgraph(c)==0);
		}
	}


	/* islower() */
	{
		int buffer[]={ 'a', 'g', 'z', -1};
		for(i=0; buffer[i]!=-1; i++) {
			c = buffer[i];
			TEST( islower(c)!=0);
		}
	}
	{
		int buffer[]={ 9, 'A', 'Z', 128, 254, ' ', '\t', '\n', 0x7F, -1};
		for(i=0; buffer[i]!=-1; i++) {
			c = buffer[i];
			TEST( islower(c)==0);
		}
	}


	/* isprint() */
	{
		int buffer[]={ ' ', ')', '~', '9', -1};
		for(i=0; buffer[i]!=-1; i++) {
			c = buffer[i];
			TEST( isprint(c)!=0);
		}
	}
	{
		int buffer[]={ '\b', '\t', '\n', 9, 128, 254, 200, 0x7F, -1};
		for(i=0; buffer[i]!=-1; i++) {
			c = buffer[i];
			TEST( isprint(c)==0);
		}
	}


	/* ispunct() */
	{
		int buffer[]={ '.', '#', '@', ';', -1};
		for(i=0; buffer[i]!=-1; i++) {
			c = buffer[i];
			TEST( ispunct(c)!=0);
		}
	}
	{
		int buffer[]={  2, 'a', 'Z', '1', 128, 254, '\n', -1};
		for(i=0; buffer[i]!=-1; i++) {
			c = buffer[i];
			TEST( ispunct(c)==0);
		}
	}


	/* isspace() */
	{
		int buffer[]={ ' ', '\t', '\r', '\v', '\n', -1};
		for(i=0; buffer[i]!=-1; i++) {
			c = buffer[i];
			TEST( isspace(c)!=0);
		}
	}
	{
		int buffer[]={  2, 'a', 'Z', '1', 128, 254, -1};
		for(i=0; buffer[i]!=-1; i++) {
			c = buffer[i];
			TEST( isspace(c)==0);
		}
	}


	/* isupper() */
	{
		int buffer[]={ 'A', 'G', 'Z', -1};
		for(i=0; buffer[i]!=-1; i++) {
			c = buffer[i];
			TEST( isupper(c)!=0);
		}
	}
	{
		int buffer[]={  2, 'a', 'z', '1', 128, 254, -1};
		for(i=0; buffer[i]!=-1; i++) {
			c = buffer[i];
			TEST( isupper(c)==0);
		}
	}



	/* isxdigit() */
	{
		int buffer[]={ 'f', 'A', '1', '8', -1};
		for(i=0; buffer[i]!=-1; i++) {
			c = buffer[i];
			TEST( isxdigit(c)!=0);
		}
	}
	{
		int buffer[]={  2, 'g', 'G', 'x', '\n', -1};
		for(i=0; buffer[i]!=-1; i++) {
			c = buffer[i];
			TEST( isxdigit(c)==0);
		}
	}


	/* tolower() */
	c='A';
	TEST_NUMERIC( tolower(c), 'a');
	c='a';
	TEST_NUMERIC( tolower(c), 'a');
	c='#';
	TEST_NUMERIC( tolower(c), c);

	/* toupper() */
	c='a';
	TEST_NUMERIC( toupper(c), 'A');
	c='A';
	TEST_NUMERIC( toupper(c), 'A');
	c='#';
	TEST_NUMERIC( toupper(c), c);

	exit(0);
}
