/* Which thread is running on an lwp?
   Copyright (C) 1999, 2001, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1999.

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

#include "thread_dbP.h"
#include "../linuxthreads/internals.h"


td_err_e
td_ta_map_lwp2thr (const td_thragent_t *ta, lwpid_t lwpid, td_thrhandle_t *th)
{
  int pthread_threads_max = ta->pthread_threads_max;
  size_t sizeof_descr = ta->sizeof_descr;
  struct pthread_handle_struct phc[pthread_threads_max];
  size_t cnt;
#ifdef ALL_THREADS_STOPPED
  int num;
#else
# define num 1
#endif

  LOG ("td_ta_map_lwp2thr");

  /* Test whether the TA parameter is ok.  */
  if (! ta_ok (ta))
    return TD_BADTA;

  /* Read all the descriptors.  */
  if (ps_pdread (ta->ph, ta->handles, phc,
		 sizeof (struct pthread_handle_struct) * pthread_threads_max)
      != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

#ifdef ALL_THREADS_STOPPED
  /* Read the number of currently active threads.  */
  if (ps_pdread (ta->ph, ta->pthread_handles_num, &num, sizeof (int)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */
#endif

  /* Get the entries one after the other and find out whether the ID
     matches.  */
  for (cnt = 0; cnt < pthread_threads_max && num > 0; ++cnt)
    if (phc[cnt].h_descr != NULL)
      {
	struct _pthread_descr_struct pds;

#ifdef ALL_THREADS_STOPPED
	/* First count this active thread.  */
	--num;
#endif

	if (ps_pdread (ta->ph, phc[cnt].h_descr, &pds, sizeof_descr) != PS_OK)
	  return TD_ERR;	/* XXX Other error value?  */

	if ((pds.p_pid ?: ps_getpid (ta->ph)) == lwpid)
	  {
	    /* Found it.  Now fill in the `td_thrhandle_t' object.  */
	    th->th_ta_p = (td_thragent_t *) ta;
	    th->th_unique = phc[cnt].h_descr;

	    return TD_OK;
	  }
      }
    else if (cnt == 0)
      {
	/* The initial thread always exists.  But it might not yet be
	   initialized.  Construct a value.  */
	th->th_ta_p = (td_thragent_t *) ta;
	th->th_unique = NULL;

	return TD_OK;
      }

  return TD_NOLWP;
}
