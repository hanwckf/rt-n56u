/* Copyright (C) 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <string.h>
#include <utmp.h>
#include <sys/time.h>

int
logout (const char *line)
{
  struct utmp tmp;
  struct utmp *ut;
  int result = 0;

  /* Tell that we want to use the UTMP file.  */
  utmpname (_PATH_UTMP);

  /* Open UTMP file.  */
  setutent ();

  /* Fill in search information.  */
#if _HAVE_UT_TYPE - 0
  tmp.ut_type = USER_PROCESS;
#endif
  strncpy (tmp.ut_line, line, sizeof tmp.ut_line);

  /* Read the record.  */
  if( (ut =  getutline(&tmp)) )
    {
      /* Clear information about who & from where.  */
      memset (ut->ut_name, 0, sizeof ut->ut_name);
#if _HAVE_UT_HOST - 0
      memset (ut->ut_host, 0, sizeof ut->ut_host);
#endif
#if _HAVE_UT_TV - 0
      gettimeofday (&ut->ut_tv, NULL);
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
