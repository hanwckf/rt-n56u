/* Copyright (C) 1996, 1997, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include "internal/utmp.h"

int
logout (const char *line)
{
  struct UT tmp;
  struct UT *ut;
  int result = 0;

  /* if (utmpname (_PATH_UTMP) == -1) return 0; - why?
   * this makes it impossible for caller to use other file!
   * Does any standard or historical precedent says this must be done? */

  /* Open UTMP file.  */
  setutent ();

  /* Fill in search information.  */
#if _HAVE_UT_TYPE - 0
  tmp.ut_type = USER_PROCESS;
#endif
  strncpy (tmp.ut_line, line, sizeof tmp.ut_line);

  /* Read the record.  */
  if ((ut = getutline(&tmp)) != NULL)
    {
      /* Clear information about who & from where.  */
      memset (ut->ut_name, 0, sizeof ut->ut_name);
#if _HAVE_UT_HOST - 0
      memset (ut->ut_host, 0, sizeof ut->ut_host);
#endif
#if _HAVE_UT_TV - 0
# if !defined __WORDSIZE_TIME64_COMPAT32
      gettimeofday (&ut->ut_tv, NULL);
# else
      {
	struct timeval tv;
	gettimeofday (&tv, NULL);
	ut->ut_tv.tv_sec = tv.tv_sec;
	ut->ut_tv.tv_usec = tv.tv_usec;
      }
# endif
#else
      time (&ut->ut_time);
#endif
#if _HAVE_UT_TYPE - 0
      ut->ut_type = DEAD_PROCESS;
#endif

      if (pututline (ut) != NULL)
	result = 1;
    }

  /* Close UTMP file.  */
  endutent ();

  return result;
}
