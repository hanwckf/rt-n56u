/*
    mtr  --  a network diagnostic tool
    Copyright (C) 2016  Matt Kimball

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

#include "timeval.h"

/*
    Ensure that a timevalue has a microsecond value in the range
    [0.0, 1.0e6) microseconds by converting microseconds to full seconds.
*/
void normalize_timeval(
    struct timeval *timeval)
{
    int full_sec;

    /*
       If tv_usec has overflowed a full second, convert the overflow
       to tv_sec.
     */
    full_sec = timeval->tv_usec / 1000000;
    timeval->tv_sec += full_sec;
    timeval->tv_usec -= 1000000 * full_sec;

    /*  If tv_usec is negative, make it positive by rolling tv_sec back  */
    if (timeval->tv_usec < 0) {
        timeval->tv_sec--;
        timeval->tv_usec += 1000000;
    }

    /*  If the entire time value is negative, clamp to zero  */
    if (timeval->tv_sec < 0) {
        timeval->tv_sec = 0;
        timeval->tv_usec = 0;
    }
}

/*
    Compare two time values.  Return:

        -1 if a < b
         0 if a == b
         1 if a > b
*/
int compare_timeval(
    struct timeval a,
    struct timeval b)
{
    if (a.tv_sec > b.tv_sec) {
        return 1;
    }
    if (a.tv_sec < b.tv_sec) {
        return -1;
    }

    if (a.tv_usec > b.tv_usec) {
        return 1;
    }
    if (a.tv_usec < b.tv_usec) {
        return -1;
    }

    return 0;
}
