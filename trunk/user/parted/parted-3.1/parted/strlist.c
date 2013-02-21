/*
    parted - a frontend to libparted
    Copyright (C) 1999-2001, 2007, 2009-2012 Free Software Foundation, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <config.h>

#include <parted/debug.h>

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "xalloc.h"

#ifdef ENABLE_NLS

#undef __USE_GNU
#define __USE_GNU

#include <wchar.h>
#include <wctype.h>

#endif /* !ENABLE_NLS */

#include "strlist.h"

#define MIN(a,b)	( (a<b)?  a : b )

static int _GL_ATTRIBUTE_PURE
wchar_strlen (const wchar_t* str)
{
#ifdef ENABLE_NLS
	return wcslen (str);
#else
	return strlen (str);
#endif
}

static wchar_t * _GL_ATTRIBUTE_PURE
wchar_strchr (const wchar_t* str, char ch)
{
#ifdef ENABLE_NLS
	return wcschr (str, ch);
#else
	return strchr (str, ch);
#endif
}

static int _GL_ATTRIBUTE_PURE
wchar_strcasecmp (const wchar_t* a, const wchar_t* b)
{
#ifdef ENABLE_NLS
	return wcscasecmp (a, b);
#else
	return strcasecmp (a, b);
#endif
}

static int _GL_ATTRIBUTE_PURE
wchar_strncasecmp (const wchar_t* a, const wchar_t* b, size_t n)
{
#ifdef ENABLE_NLS
	return wcsncasecmp (a, b, n);
#else
	return strncasecmp (a, b, n);
#endif
}

static wchar_t * _GL_ATTRIBUTE_PURE
wchar_strdup (const wchar_t* str)
{
#ifdef ENABLE_NLS
	return wcsdup (str);
#else
	return xstrdup (str);
#endif
}

/* converts a string from the encoding in the gettext catalogues to wide
 * character strings (of type wchar_t*).
 */
#ifdef ENABLE_NLS
static wchar_t*
gettext_to_wchar (const char* str)
{
	int		count;
	wchar_t*	result;
	size_t		status;
	mbstate_t	ps;

	count = strlen (str) + 1;
	result = malloc (count * sizeof (wchar_t));
	if (!result)
		goto error;

	memset(&ps, 0, sizeof (ps));
	status = mbsrtowcs(result, &str, count, &ps);
	if (status == (size_t) -1)
		goto error;

	result = xrealloc (result, (wcslen (result) + 1) * sizeof (wchar_t));
	return result;

error:
	fprintf (stderr, "Error during translation: %s\n", strerror (errno));
	exit (EXIT_FAILURE);
}

#else /* ENABLE_NLS */

static wchar_t*
gettext_to_wchar (const char* str)
{
	return xstrdup (str);
}

#endif /* !ENABLE_NLS */


#ifdef ENABLE_NLS
static char*
wchar_to_str (const wchar_t* str, size_t count)
{
	char*		result;
	char*		out_buf;
	size_t		status;
	mbstate_t	ps;
	size_t		i;

	if (count == 0 || wcslen(str) < count)
		count = wcslen (str);

	out_buf = result = malloc ((count + 1) *  MB_LEN_MAX);
	if (!result)
		goto error;

	memset(&ps, 0, sizeof(ps));

	for (i = 0; i < count; i++) {
		status = wcrtomb (out_buf, str[i], &ps);
		if (status == (size_t) -1)
			goto error;
		out_buf += status;
	}

	status = wcrtomb (out_buf, 0, &ps);
	if (status == (size_t) -1)
		goto error;

	result = realloc (result, strlen (result) + 1);
	return result;

error:
	fprintf (stderr, "Error during translation: %s\n", strerror (errno));
	exit (EXIT_FAILURE);
}

#else /* ENABLE_NLS */

static char*
wchar_to_str (const wchar_t* str, size_t count)
{
	char*		result;

	result = xstrdup (str);
	if (count && count < strlen (result))
		result [count] = 0;
	return result;
}

#endif /* !ENABLE_NLS */

static void
print_wchar (const wchar_t* str, size_t count, FILE *fp)
{
	char*	tmp = wchar_to_str (str, count);
	fprintf (fp, "%s", tmp);
	free (tmp);
}

static StrList*
str_list_alloc ()
{
	StrList*	list;

	list = xmalloc (sizeof (StrList));
	list->next = NULL;

	return list;
}

void
str_list_destroy (StrList* list)
{
	if (list) {
		str_list_destroy (list->next);
		str_list_destroy_node (list);
	}
}

void
str_list_destroy_node (StrList* list)
{
	void *p = (char *) (list->str); /* discard const */
	free (p);
	free (list);
}

StrList*
str_list_duplicate_node (const StrList* node)
{
	StrList*	result = str_list_alloc ();
	result->str = wchar_strdup (node->str);
	return result;
}

StrList*
str_list_duplicate (const StrList* list)
{
	if (list)
		return str_list_join (str_list_duplicate_node (list),
				      str_list_duplicate (list->next));
	else
		return NULL;
}

StrList*
str_list_join (StrList* a, StrList* b)
{
	StrList*	walk;

	for (walk = a; walk && walk->next; walk = walk->next);

	if (walk) {
		walk->next = b;
		return a;
	} else {
		return b;
	}
}

static StrList*
_str_list_append (StrList* list, const wchar_t* str)
{
	StrList*	walk;

	if (list) {
		for (walk = list; walk->next; walk = walk->next);
		walk->next = str_list_alloc ();
		walk = walk->next;
	} else {
		walk = list = str_list_alloc ();
	}
	walk->str = str;

	return list;
}

StrList*
str_list_append (StrList* list, const char* str)
{
	return _str_list_append (list, gettext_to_wchar (str));
}

StrList*
str_list_append_unique (StrList* list, const char* str)
{
	StrList*	walk;
	wchar_t*	new_str = gettext_to_wchar (str);

	for (walk=list; walk; walk=walk->next) {
		if (walk->str) {
			if (wchar_strcasecmp (new_str, walk->str) == 0) {
				free (new_str);
				return list;
			}
		}
	}

	return _str_list_append (list, new_str);
}

StrList*
str_list_insert (StrList* list, const char* str)
{
	return str_list_join (str_list_create (str, NULL), list);
}

StrList*
str_list_create (const char* first, ...)
{
	va_list		args;
	char*		str;
	StrList*	list;

	list = str_list_append (NULL, first);

	if (first) {
		va_start (args, first);
		while ( (str = va_arg (args, char*)) )
			str_list_append (list, str);
		va_end (args);
	}

	return list;
}

StrList*
str_list_create_unique (const char* first, ...)
{
	va_list		args;
	char*		str;
	StrList*	list;

	list = str_list_append (NULL, first);

	if (first) {
		va_start (args, first);
		while ( (str = va_arg (args, char*)) )
			str_list_append_unique (list, str);
		va_end (args);
	}

	return list;
}

char*
str_list_convert_node (const StrList* list)
{
	return wchar_to_str (list->str, 0);
}

char*
str_list_convert (const StrList* list)
{
	const StrList*	walk;
	int		pos = 0;
	int		length = 1;
	char*		str = xstrdup ("");

	for (walk = list; walk; walk = walk->next) {
		if (walk->str) {
			char*	tmp = wchar_to_str (walk->str, 0);

			length += strlen (tmp);

			str = realloc (str, length);
			strcpy (str + pos, tmp);

			pos = length - 1;
			free (tmp);
		}
	}

	return str;
}

void
str_list_print (const StrList* list, FILE *fp)
{
	const StrList*	walk;

	for (walk=list; walk; walk=walk->next) {
		if (walk->str)
                        print_wchar (walk->str, 0, fp);
	}
}

static int
str_search (const wchar_t* str, int n, wchar_t c)
{
	int	i;

	for (i=0; i<n; i++)
		if (str [i] == c)
			return i;
	return -1;
}


/* Japanese don't leave spaces between words, so ALL Japanese characters
 * are treated as delimiters.  Note: since the translations should already
 * be properly formatted (eg: spaces after commas), there should be no
 * need to include them.  Best not to avoid side effects, like 3.
14159 :-)
 * FIXME: how do we exclude "." and "(" ?
 * FIXME: glibc doesn't like umlaute.  i.e. \"o (TeX notation), which should
 * look like: ö
 */

static int
is_break_point (wchar_t c)
{
#ifdef ENABLE_NLS
	return !iswalnum (c) && !iswpunct (c);
#else
	return !isalnum (c) && !ispunct (c);
#endif
}

/* NOTE: this should not return '\n' as a space, because explicit '\n' may
 * be placed inside strings.
 */
static int
is_space (wchar_t c)
{
#ifdef ENABLE_NLS
	return c == (wchar_t) btowc(' ');
#else
	return c == ' ';
#endif
}

void
str_list_print_wrap (const StrList* list, int line_length, int offset,
		     int indent, FILE *fp)
{
	const StrList*	walk;
	const wchar_t*	str;
	int		str_len;
	int		cut_right;
	int		cut_left;
	int		line_left;
	int		search_result;
	int		line_break;

	PED_ASSERT (line_length - indent > 10);

	line_left = line_length - offset;

	for (walk=list; walk; walk=walk->next) {
		if (!walk->str)
			continue;
		str = walk->str;
		str_len = wchar_strlen (str);

		while (line_left < str_len || wchar_strchr (str, '\n')) {
			line_break = 0;

			cut_left = MIN (line_left - 1, str_len - 1);

			/* we can have a space "over", but not a comma */
			if (cut_left < str_len
					&& is_space (str [cut_left + 1]))
				cut_left++;

			while (cut_left && !is_break_point (str [cut_left]))
				cut_left--;
			while (cut_left && is_space (str [cut_left]))
				cut_left--;

		/* str [cut_left] is either the end of a word, or a
		 * Japanese character, or the start of a blank line.
		 */

			search_result = str_search (str, cut_left + 1, '\n');
			if (search_result != -1) {
				cut_left = search_result - 1;
				line_break = 1;
			}

			for (cut_right = cut_left + (line_break ? 2 : 1);
			     cut_right < str_len && is_space (str [cut_right]);
			     cut_right++);

			if (cut_left > 0)
				print_wchar (str, cut_left + 1, fp);

			str += cut_right;
			str_len -= cut_right;
			line_left = line_length - indent;

			if (walk->next || *str)
				fprintf (fp, "\n%*s", indent, "");
			else if (line_break)
				fputc ('\n', fp);
		}

		print_wchar (str, 0, fp);
		line_left -= wchar_strlen (str);
	}
}

static int
_str_list_match_node (const StrList* list, const wchar_t* str)
{
	if (wchar_strcasecmp (list->str, str) == 0)
		return 2;
	if (wchar_strncasecmp (list->str, str, wchar_strlen (str)) == 0)
		return 1;
	return 0;
}

int
str_list_match_node (const StrList* list, const char* str)
{
	wchar_t*	wc_str = gettext_to_wchar (str);	/* FIXME */
	int		status;

	status = _str_list_match_node (list, wc_str);
	free (wc_str);

	return status;
}

/* returns:  2 for full match
	     1 for partial match
	     0 for no match
 */
int
str_list_match_any (const StrList* list, const char* str)
{
	const StrList*	walk;
	int		best_status = 0;
	wchar_t*	wc_str = gettext_to_wchar (str);

	for (walk = list; walk; walk = walk->next) {
		int	this_status = _str_list_match_node (walk, wc_str);
		if (this_status > best_status)
	       		best_status = this_status;
	}

	free (wc_str);
	return best_status;
}

StrList*
str_list_match (const StrList* list, const char* str)
{
	const StrList*	walk;
	const StrList*	partial_match = NULL;
	int		ambiguous = 0;
	wchar_t*	wc_str = gettext_to_wchar (str);

	for (walk = list; walk; walk = walk->next) {
		switch (_str_list_match_node (walk, wc_str)) {
			case 2:
				free (wc_str);
				return (StrList*) walk;

			case 1:
				if (partial_match)
					ambiguous = 1;
				partial_match = walk;
		}
	}

	free (wc_str);
	return ambiguous ? NULL : (StrList*) partial_match;
}

int
str_list_length (const StrList* list)
{
	int		length = 0;
	const StrList*	walk;

	for (walk = list; walk; walk = walk->next)
		length++;

	return length;
}
