/* Copyright (C) 1994, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sys/timeb.h>
#include <sys/time.h>

int ftime(timebuf)
struct timeb *timebuf;
{
	struct timeval tv;
	struct timezone tz;

	if (gettimeofday (&tv, &tz) < 0)
		return -1;

	timebuf->time = tv.tv_sec;
	timebuf->millitm = (tv.tv_usec + 999) / 1000;
	timebuf->timezone = tz.tz_minuteswest;
	timebuf->dstflag = tz.tz_dsttime;
	return 0;
}
