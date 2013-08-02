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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

/* Use SVID search */
#define __USE_GNU
#include <search.h>

/* CGI hash table */
static struct hsearch_data htab;

static void
unescape(char *s)
{
	unsigned int c;

	while ((s = strpbrk(s, "%+"))) {
		/* Parse %xx */
		if (*s == '%') {
			sscanf(s + 1, "%02x", &c);
			*s++ = (char) c;
			strncpy(s, s + 2, strlen(s) + 1);
		}
		/* Space is special */
		else if (*s == '+')
			*s++ = ' ';
	}
}

char *
get_cgi(char *name)
{
	ENTRY e, *ep;

	if (!htab.table)
		return NULL;

	e.key = name;
	hsearch_r(e, FIND, &ep, &htab);

	return ep ? ep->data : NULL;
}

void
set_cgi(char *name, char *value)
{
	ENTRY e, *ep;

	if (!htab.table)
		return;

	e.key = name;
	hsearch_r(e, FIND, &ep, &htab);
	if (ep)
		ep->data = value;
	else {
		e.data = value;
		hsearch_r(e, ENTER, &ep, &htab);
	}
}

void
init_cgi(char *query)
{
	int len, nel;
	char *q, *name, *value;

	/* Clear variables */
	if (!query) {
		hdestroy_r(&htab);
		return;
	}

	/* Parse into individual assignments */
	q = query;
	len = strlen(query);
	nel = 1;
	while (strsep(&q, "&;"))
		nel++;
	hcreate_r(nel, &htab);

	for (q = query; q < (query + len);) {
		/* Unescape each assignment */
		unescape(name = value = q);

		/* Skip to next assignment */
		for (q += strlen(q); q < (query + len) && !*q; q++);

		/* Assign variable */
		name = strsep(&value, "=");
		if (value)
			set_cgi(name, value);
	}
}

