/* Validate a thread handle.
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
#include <linuxthreads/internals.h>


td_err_e
td_thr_validate (const td_thrhandle_t *th)
{
  struct pthread_handle_struct *handles = th->th_ta_p->handles;
  int pthread_threads_max = th->th_ta_p->pthread_threads_max;
  int cnt;
  struct pthread_handle_struct phc;

  LOG ("td_thr_validate");

  /* A special case: if the program just starts up the handle is
     NULL.  */
  if (th->th_unique == NULL)
    {
      /* Read the first handle.  If the pointer to the thread
	 descriptor is not NULL this is an error.  */
      if (ps_pdread (th->th_ta_p->ph, handles, &phc,
		     sizeof (struct pthread_handle_struct)) != PS_OK)
	return TD_ERR;	/* XXX Other error value?  */

      return phc.h_descr == NULL ? TD_OK : TD_NOTHR;
    }

  /* Now get all descriptors, one after the other.  */
  for (cnt = 0; cnt < pthread_threads_max; ++cnt, ++handles)
    {
      if (ps_pdread (th->th_ta_p->ph, handles, &phc,
		     sizeof (struct pthread_handle_struct)) != PS_OK)
	return TD_ERR;	/* XXX Other error value?  */

      if (phc.h_descr != NULL && phc.h_descr == th->th_unique)
	{
	  struct _pthread_descr_struct pds;

	  if (ps_pdread (th->th_ta_p->ph, phc.h_descr, &pds,
			 th->th_ta_p->sizeof_descr) != PS_OK)
	    return TD_ERR;	/* XXX Other error value?  */

	  /* XXX There should be another test using the TID but this is
	     currently not available.  */
	  return pds.p_terminated != 0 ? TD_NOTHR : TD_OK;
	}
    }

  return TD_ERR;
}
