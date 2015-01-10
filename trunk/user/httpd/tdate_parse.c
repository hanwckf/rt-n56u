/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* tdate_parse - parse string dates into internal form, stripped-down version
**
** Copyright © 1995 by Jef Poskanzer <jef@mail.acme.com>.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/

/* This is a stripped-down version of date_parse.c, available at
** http://www.acme.com/software/date_parse/
*/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct strlong {
	char* s;
	long l;
};

static void
pound_case( char* str )
{
	for ( ; *str != '\0'; ++str ) {
		if ( isupper( (int) *str ) )
			*str = tolower( (int) *str );
	}
}

static int
strlong_compare( const void* v1, const void* v2 )
{
	const struct strlong* s1 = (const struct strlong*) v1;
	const struct strlong* s2 = (const struct strlong*) v2;
	return strcmp( s1->s, s2->s );
}

static int
strlong_search( char* str, struct strlong* tab, int n, long* lP )
{
	int i, h, l, r;

	l = 0;
	h = n - 1;
	for (;;) {
		i = ( h + l ) / 2;
		r = strcmp( str, tab[i].s );
		if ( r < 0 )
			h = i - 1;
		else if ( r > 0 )
			l = i + 1;
		else {
			*lP = tab[i].l;
			return 1;
		}
		if ( h < l )
			return 0;
	}
}

static int
scan_wday( char* str_wday, long* tm_wdayP )
{
	static struct strlong wday_tab[] = {
		{ "sun", 0 }, { "sunday", 0 },
		{ "mon", 1 }, { "monday", 1 },
		{ "tue", 2 }, { "tuesday", 2 },
		{ "wed", 3 }, { "wednesday", 3 },
		{ "thu", 4 }, { "thursday", 4 },
		{ "fri", 5 }, { "friday", 5 },
		{ "sat", 6 }, { "saturday", 6 },
	};
	static int sorted = 0;

	if ( ! sorted ) {
		(void) qsort(
		    wday_tab, sizeof(wday_tab)/sizeof(struct strlong),
		    sizeof(struct strlong), strlong_compare );
		sorted = 1;
	}
	pound_case( str_wday );
	return strlong_search(str_wday, wday_tab, sizeof(wday_tab)/sizeof(struct strlong), tm_wdayP );
}

static int
scan_mon( char* str_mon, long* tm_monP )
{
	static struct strlong mon_tab[] = {
		{ "jan", 0 }, { "january", 0 },
		{ "feb", 1 }, { "february", 1 },
		{ "mar", 2 }, { "march", 2 },
		{ "apr", 3 }, { "april", 3 },
		{ "may", 4 },
		{ "jun", 5 }, { "june", 5 },
		{ "jul", 6 }, { "july", 6 },
		{ "aug", 7 }, { "august", 7 },
		{ "sep", 8 }, { "september", 8 },
		{ "oct", 9 }, { "october", 9 },
		{ "nov", 10 }, { "november", 10 },
		{ "dec", 11 }, { "december", 11 },
	};
	static int sorted = 0;

	if ( ! sorted ) {
		(void) qsort(
		    mon_tab, sizeof(mon_tab)/sizeof(struct strlong),
		    sizeof(struct strlong), strlong_compare );
		sorted = 1;
	}
	pound_case( str_mon );
	return strlong_search(str_mon, mon_tab, sizeof(mon_tab)/sizeof(struct strlong), tm_monP );
}

static int
is_leap( int year )
{
	return year % 400? ( year % 100 ? ( year % 4 ? 0 : 1 ) : 0 ) : 1;
}

/* Basically the same as mktime(). */
static time_t
tm_to_time( struct tm* tmP )
{
	time_t t;
	static int monthtab[12] = {
		0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

	/* Years since epoch, converted to days. */
	t = ( tmP->tm_year - 70 ) * 365;
	/* Leap days for previous years - this will break in 2100! */
	t += ( tmP->tm_year - 69 ) / 4;
	/* Days for the beginning of this month. */
	t += monthtab[tmP->tm_mon];
	/* Leap day for this year. */
	if ( tmP->tm_mon >= 2 && is_leap( tmP->tm_year + 1900 ) )
	++t;
	/* Days since the beginning of this month. */
	t += tmP->tm_mday - 1;	/* 1-based field */
	/* Hours, minutes, and seconds. */
	t = t * 24 + tmP->tm_hour;
	t = t * 60 + tmP->tm_min;
	t = t * 60 + tmP->tm_sec;

	return t;
}

time_t
tdate_parse( char *str )
{
	struct tm tm;
	char str_mon[500], str_wday[500];
	int tm_sec, tm_min, tm_hour, tm_mday, tm_year;
	long tm_mon, tm_wday;
	time_t t;

	/* Initialize. */
	memset( &tm, 0, sizeof(struct tm) );

	/* wdy, DD-mth-YY HH:MM:SS GMT */
	if ( sscanf( str, "%400[a-zA-Z], %d-%400[a-zA-Z]-%d %d:%d:%d GMT",
		str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 7 &&
		scan_wday( str_wday, &tm_wday ) && scan_mon( str_mon, &tm_mon ) )
	{
		tm.tm_wday = tm_wday;
		tm.tm_mday = tm_mday;
		tm.tm_mon = tm_mon;
		tm.tm_year = tm_year;
		tm.tm_hour = tm_hour;
		tm.tm_min = tm_min;
		tm.tm_sec = tm_sec;
	}
	/* wdy, DD mth YY HH:MM:SS GMT */
	else if ( sscanf( str, "%400[a-zA-Z], %d %400[a-zA-Z] %d %d:%d:%d GMT",
		str_wday, &tm_mday, str_mon, &tm_year, &tm_hour, &tm_min, &tm_sec ) == 7 &&
		scan_wday( str_wday, &tm_wday ) && scan_mon( str_mon, &tm_mon ) )
	{
		tm.tm_wday = tm_wday;
		tm.tm_mday = tm_mday;
		tm.tm_mon = tm_mon;
		tm.tm_year = tm_year;
		tm.tm_hour = tm_hour;
		tm.tm_min = tm_min;
		tm.tm_sec = tm_sec;
	}
	else
		return (time_t) -1;

	if ( tm.tm_year > 1900 )
		tm.tm_year -= 1900;
	else if ( tm.tm_year < 70 )
		tm.tm_year += 100;

	t = tm_to_time( &tm );

	return t;
}
