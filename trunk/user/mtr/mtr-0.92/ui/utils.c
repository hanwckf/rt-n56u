/*
    mtr  --  a network diagnostic tool
    Copyright (C) 1997,1998  Matt Kimball

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

#include "config.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef HAVE_ERROR_H
#include <error.h>
#else
#include "portability/error.h"
#endif

#ifdef HAVE_STDIO_EXT_H
#include <stdio_ext.h>
#endif

#include "utils.h"

char *trim(
    char *str,
    const char c)
{
    char *p = str;
    size_t len;

    /* left trim */
    while (*p && (isspace((unsigned char) *p) || (c && *p == c)))
        p++;
    if (str < p) {
        len = strlen(str);
        memmove(str, p, len + 1);
    }

    /* right trim */
    len = strlen(str);
    while (len) {
        len--;
        if (isspace((unsigned char) str[len]) || (c && str[len] == c)) {
            continue;
        }
        len++;
        break;
    }
    str[len] = '\0';
    return str;
}

/* Parse string, and return positive signed int. */
int strtonum_or_err(
    const char *str,
    const char *errmesg,
    const int type)
{
    unsigned long int num;
    char *end = NULL;

    if (str != NULL && *str != '\0') {
        errno = 0;
        num = strtoul(str, &end, 10);
        if (errno == 0 && str != end && end != NULL && *end == '\0') {
            switch (type) {
            case STRTO_INT:
                if (num < INT_MAX)
                    return num;
                break;
            case STRTO_U32INT:
                if (num < UINT32_MAX)
                    return num;
                break;
            }
        }
    }
    error(EXIT_FAILURE, errno, "%s: '%s'", errmesg, str);
    return 0;
}

float strtofloat_or_err(
    const char *str,
    const char *errmesg)
{
    double num;
    char *end = NULL;

    if (str != NULL && *str != '\0') {
        errno = 0;
        num = strtod(str, &end);
        if (errno == 0 && str != end && end != NULL && *end == '\0'
#ifdef FLT_MAX
            && num < FLT_MAX
#endif
            )
            return num;
    }
    error(EXIT_FAILURE, errno, "%s: '%s'", errmesg, str);
    return 0;
}

void *xmalloc(
    const size_t size)
{
    void *ret = malloc(size);

    if (!ret && size)
        error(EXIT_FAILURE, errno, "cannot allocate %zu bytes", size);
    return ret;
}

char *xstrdup(
    const char *str)
{
    char *ret;

    if (!str)
        return NULL;
    ret = strdup(str);
    if (!ret)
        error(EXIT_FAILURE, errno, "cannot duplicate string: %s", str);
    return ret;
}

#ifndef HAVE___FPENDING
static inline int __fpending(
    FILE * stream __attribute__ ((__unused__)))
{
    return 0;
}
#endif
static inline int close_stream(
    FILE * stream)
{
    const int some_pending = (__fpending(stream) != 0);
    const int prev_fail = (ferror(stream) != 0);
    const int fclose_fail = (fclose(stream) != 0);

    if (prev_fail || (fclose_fail && (some_pending || errno != EBADF))) {
        if (!fclose_fail && !(errno == EPIPE))
            errno = 0;
        return EOF;
    }
    return 0;
}

/* Meant to be used atexit(close_stdout); */
void close_stdout(
    void)
{
    if (close_stream(stdout) != 0 && !(errno == EPIPE)) {
        error(0, errno, "write error");
        _exit(EXIT_FAILURE);
    }
    if (close_stream(stderr) != 0)
        _exit(EXIT_FAILURE);
}

/* ctime() replacement that will reteturn ISO-8601 timestamp string such as:
 * 2016-08-29T19:25:02+01:00 */
const char *iso_time(
    const time_t * t)
{
    static char s[32];
    struct tm *tm;

    tm = localtime(t);
    strftime(s, sizeof(s), "%Y-%m-%dT%H:%M:%S%z", tm);
    return s;
}
