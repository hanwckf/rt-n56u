/* Iterate over a process's threads.
   Copyright (C) 1999,2000,2001,2002,2003 Free Software Foundation, Inc.
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
#include <alloca.h>

static int
handle_descr (const td_thragent_t *ta, td_thr_iter_f *callback,
	      void *cbdata_p, td_thr_state_e state, int ti_pri,
	      size_t cnt, pthread_descr descr)
{
  struct _pthread_descr_struct pds;
  size_t sizeof_descr = ta->sizeof_descr;
  td_thrhandle_t th;

  if (descr == NULL)
    {
      /* No descriptor (yet).  */
      if (cnt == 0)
	{
	  /* This is the main thread.  Create a fake descriptor.  */
	  memset (&pds, '\0', sizeof (pds));

	  /* Empty thread descriptor the thread library would create.  */
#if !defined USE_TLS || !TLS_DTV_AT_TP
	  pds.p_header.data.self = &pds;
#endif
	  pds.p_nextlive = pds.p_prevlive = &pds;
	  pds.p_tid = PTHREAD_THREADS_MAX;
	  /* The init code also sets up p_lock, p_errnop, p_herrnop, and
	     p_userstack but this should not be necessary here.  */

	  th.th_ta_p = (td_thragent_t *) ta;
	  th.th_unique = NULL;
	  if (callback (&th, cbdata_p) != 0)
	    return TD_DBERR;

	  /* All done successfully.  */
	  return TD_OK;
	}
      else if (cnt == 1)
	/* The manager is not yet started.  No big deal.  */
	return TD_OK;
      else
	/* For every other thread this should not happen.  */
	return TD_ERR;
    }

  if (ps_pdread (ta->ph, descr, &pds, sizeof_descr) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  /* The manager thread must be handled special.  The descriptor
     exists but the thread only gets created when the first
     `pthread_create' call is issued.  A clear indication that this
     happened is when the p_pid field is non-zero.  */
  if (cnt == 1 && pds.p_pid == 0)
    return TD_OK;

  /* Now test whether this thread matches the specified
     conditions.  */

  /* Only if the priority level is as high or higher.  */
  if (pds.p_priority < ti_pri)
    return TD_OK;

  /* Test the state.
     XXX This is incomplete.  */
  if (state != TD_THR_ANY_STATE)
    return TD_OK;

  /* XXX For now we ignore threads which are not running anymore.
     The reason is that gdb tries to get the registers and fails.
     In future we should have a special mode of the thread library
     in which we keep the process around until the actual join
     operation happened.  */
  if (pds.p_exited != 0)
    return TD_OK;

  /* Yep, it matches.  Call the callback function.  */
  th.th_ta_p = (td_thragent_t *) ta;
  th.th_unique = descr;
  if (callback (&th, cbdata_p) != 0)
    return TD_DBERR;

  /* All done successfully.  */
  return TD_OK;
}


td_err_e
td_ta_thr_iter (const td_thragent_t *ta, td_thr_iter_f *callback,
		void *cbdata_p, td_thr_state_e state, int ti_pri,
		sigset_t *ti_sigmask_p, unsigned int ti_user_flags)
{
  int pthread_threads_max;
  struct pthread_handle_struct *phc;
  td_err_e result = TD_OK;
  int cnt;
#ifdef ALL_THREADS_STOPPED
  int num;
#else
# define num 1
#endif

  LOG ("td_ta_thr_iter");

  /* Test whether the TA parameter is ok.  */
  if (! ta_ok (ta))
    return TD_BADTA;

  pthread_threads_max = ta->pthread_threads_max;
  phc = (struct pthread_handle_struct *) alloca (sizeof (phc[0])
						 * pthread_threads_max);

  /* First read only the main thread and manager thread information.  */
  if (ps_pdread (ta->ph, ta->handles, phc,
		 sizeof (struct pthread_handle_struct) * 2) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  /* Now handle these descriptors.  */
  result = handle_descr (ta, callback, cbdata_p, state, ti_pri, 0,
			 phc[0].h_descr);
  if (result != TD_OK)
    return result;
  result = handle_descr (ta, callback, cbdata_p, state, ti_pri, 1,
			 phc[1].h_descr);
  if (result != TD_OK)
    return result;

  /* Read all the descriptors.  */
  if (ps_pdread (ta->ph, ta->handles + 2, &phc[2],
		 (sizeof (struct pthread_handle_struct)
		  * (pthread_threads_max - 2))) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

#ifdef ALL_THREADS_STOPPED
  /* Read the number of currently active threads.  */
  if (ps_pdread (ta->ph, ta->pthread_handles_num, &num, sizeof (int)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */
#endif

  /* Now get all descriptors, one after the other.  */
  for (cnt = 2; cnt < pthread_threads_max && num > 0; ++cnt)
    if (phc[cnt].h_descr != NULL)
      {
#ifdef ALL_THREADS_STOPPED
	/* First count this active thread.  */
	--num;
#endif

	result = handle_descr (ta, callback, cbdata_p, state, ti_pri, cnt,
			       phc[cnt].h_descr);
	if (result != TD_OK)
	  break;
      }

  return result;
}
