/*
 * TODO: - make right and centered alignment possible
 */
/*
    parted - a frontend to libparted
    Copyright (C) 2006-2012 Free Software Foundation, Inc.

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

#include <stdio.h>
#include <stdlib.h>

#include <assert.h>
#include <wchar.h>
#include <string.h>

#include "xalloc.h"
#include "strlist.h"

#ifdef ENABLE_NLS
#	define L_(str) L##str
#else
#	define L_(str) str
#       define wcslen strlen
#       define wcswidth strnlen
#       define wcscat strcat
#       define wcsdup xstrdup
#endif


static const unsigned int       MAX_WIDTH = 512;
static const wchar_t*           DELIMITER = L_("  ");
static const wchar_t*           COLSUFFIX = L_("\n");

typedef struct
{
        unsigned int    ncols;
        unsigned int    nrows;
        wchar_t***      rows;
        int*            widths;
} Table;


Table* table_new(int ncols)
{
        assert ( ncols >= 0 );

        Table *t = xmalloc (sizeof(*t));

        t->ncols = ncols;
        t->nrows = 0;
        t->rows = (wchar_t***)NULL;
        t->widths = NULL;

        return t;
}


void table_destroy (Table* t)
{
        unsigned int r, c;

        assert (t);
        assert (t->ncols > 0);

        for (r = 0; r < t->nrows; ++r)
        {
                for (c = 0; c < t->ncols; ++c)
                        free (t->rows[r][c]);
                free (t->rows[r]);
        }

        free (t->rows);
        free (t->widths);
        free (t);
}


static int max (int x, int y)
{
        return x > y ? x : y;
}


static void table_calc_column_widths (Table* t)
{
        unsigned int r, c;

        assert(t);
        assert(t->ncols > 0);

        if (!t->widths)
                t->widths = xmalloc (t->ncols * sizeof(t->widths[0]));

        for (c = 0; c < t->ncols; ++c)
                t->widths[c] = 0;

        for (r = 0; r < t->nrows; ++r)
                for (c = 0; c < t->ncols; ++c)
                {
                        t->widths[c] = max ( t->widths[c],
                                             wcswidth(t->rows[r][c],
                                                      MAX_WIDTH) );
                }
}


/*
 * add a row which is a string array of ncols elements.
 * 'row' will get freed by table_destroy;  you must not free it
 * yourself.
 */
void table_add_row (Table* t, wchar_t** row)
{
        assert(t);

        /*unsigned int i;
        fputs ("adding row: ", stdout);
        for (i = 0; i < t->ncols; ++i)
                printf("[%s]", row[i]);
        putchar ('\n');*/

        t->rows = xrealloc (t->rows, (t->nrows + 1) * sizeof(*(t->rows)));

        t->rows[t->nrows] = row;

        ++t->nrows;

        table_calc_column_widths (t);
}


void table_add_row_from_strlist (Table* t, StrList* list)
{
        wchar_t** row = xmalloc (str_list_length(list) * sizeof(*row));
        int i = 0;

        while (list)
        {
                row[i] = wcsdup (list->str);
                if (row[i] == NULL)
                        xalloc_die ();


                list = list->next;
                ++i;
        }

        table_add_row (t, row);
}


/* render a row */
static void table_render_row (Table* t, int rownum, int ncols, wchar_t** s)
{
        wchar_t** row = t->rows[rownum];
        size_t len = 1, i;
        size_t newsize;

        assert(t);
        assert(s != NULL);

        for (i = 0; i < ncols; ++i)
                len += t->widths[i] + wcslen(DELIMITER);

        len += wcslen(COLSUFFIX);

        newsize = (wcslen(*s) + len + 1) * sizeof(wchar_t);
        *s = xrealloc (*s, newsize);

        for (i = 0; i < ncols; ++i)
        {
                wcscat (*s, row[i]);
                if (ncols <= i + 1)
                        break;

                int j;
                int nspaces = max(t->widths[i] - wcswidth(row[i], MAX_WIDTH),
                                  0);
                wchar_t* pad = xmalloc ((nspaces + 1) * sizeof(*pad));

                for (j = 0; j < nspaces; ++j)
                       pad[j] = L' ';

                pad[nspaces] = L_('\0');

                wcscat (*s, pad);
                if (i + 1 < ncols)
                        wcscat (*s, DELIMITER);

                free (pad);
        }

        /* Remove any trailing blanks.  */
        wchar_t *p = *s;
        size_t k = wcslen (p);
        while (k && p[k-1] == L_(' '))
                --k;
        p[k] = L_('\0');


        wcscat (*s, COLSUFFIX);
}


/*
 * Render the rows.
 * \p s must be a null-terminated string.
 */
static void table_render_rows (Table* t, wchar_t** s)
{
        unsigned int i;

        assert (**s == L_('\0'));
        for (i = 0; i < t->nrows; ++i)
                table_render_row (t, i, t->ncols, s);
}

/*
 * Render the table to a string.
 * You are responsible for freeing the returned string.
 */
wchar_t* table_render(Table* t)
{
        wchar_t* s = xmalloc (sizeof(*s));

        *s = L_('\0');
        table_render_rows (t, &s);
        return s;
}
