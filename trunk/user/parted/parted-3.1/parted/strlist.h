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

#ifndef STRLIST_H_INCLUDED
#define STRLIST_H_INCLUDED

#include <wchar.h>

#ifndef ENABLE_NLS
#	define L_(str) str
#       ifdef wchar_t
#               undef wchar_t
#       endif
#       define wchar_t char
#endif

typedef struct _StrList StrList;
struct _StrList {
	StrList*	next;
	const wchar_t*	str;
};

extern char* language;
extern char* gettext_charset;
extern char* term_charset;

extern StrList* str_list_create (const char* first, ...);
extern StrList* str_list_create_unique (const char* first, ...);
extern void str_list_destroy (StrList* list);
extern void str_list_destroy_node (StrList* list);

extern StrList* str_list_duplicate (const StrList* list);
extern StrList* str_list_duplicate_node (const StrList* list);
extern StrList* str_list_insert (StrList* list, const char* str);
extern StrList* str_list_append (StrList* list, const char* str);
extern StrList* str_list_append_unique (StrList* list, const char* str);
extern StrList* str_list_join (StrList* a, StrList* b);
extern char* str_list_convert (const StrList* list);
extern char* str_list_convert_node (const StrList* list);

extern void str_list_print (const StrList* list, FILE *fp);
extern void str_list_print_wrap (const StrList* list, int line_length,
				 int offset, int indent, FILE *fp);
extern int str_list_match_any (const StrList* list, const char* str);
extern int str_list_match_node (const StrList* list, const char* str);
extern StrList* str_list_match (const StrList* list, const char* str);

extern int str_list_length (const StrList* list) _GL_ATTRIBUTE_PURE;

#endif /* STRLIST_H_INCLUDED */
