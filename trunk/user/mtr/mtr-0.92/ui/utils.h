/*
    mtr  --  a network diagnostic tool
    Copyright (C) 1997,1998  Matt Kimball
    Copyright (C) 2005 R.E.Wolff@BitWizard.nl

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

enum {
    STRTO_INT,
    STRTO_U32INT
};

extern char *trim(
    char *s,
    const char c);
extern int strtonum_or_err(
    const char *str,
    const char *errmesg,
    const int type);
extern float strtofloat_or_err(
    const char *str,
    const char *errmesg);

/* Like strncpy(3) but ensure null termination. */
static inline void xstrncpy(
    char *dest,
    const char *src,
    size_t n)
{
    strncpy(dest, src, n - 1);
    dest[n - 1] = 0;
}

extern void *xmalloc(
    const size_t size);
extern char *xstrdup(
    const char *str);

extern void close_stdout(
    void);

extern const char *iso_time(
    const time_t * t);
