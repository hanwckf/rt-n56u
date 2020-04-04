/* Get thread information.
   Copyright (C) 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <stddef.h>
#include <string.h>

#include "thread_dbP.h"


td_err_e
td_thr_get_info (const td_thrhandle_t *th, td_thrinfo_t *infop)
{
  struct _pthread_descr_struct pds;

  LOG ("td_thr_get_info");

  /* Handle the case when the thread library is not yet initialized.  */
  if (th->th_unique == NULL)
    {
      memset (&pds, '\0', sizeof (pds));
      pds.p_tid = PTHREAD_THREADS_MAX;
    }
  else
    /* Get the thread descriptor.  */
    if (ps_pdread (th->th_ta_p->ph, th->th_unique, &pds,
		   th->th_ta_p->sizeof_descr) != PS_OK)
      return TD_ERR;	/* XXX Other error value?  */

  /* Fill in information.  Clear first to provide reproducable
     results for the fields we do not fill in.  */
  memset (infop, '\0', sizeof (td_thrinfo_t));

  /* We have to handle the manager thread special since the thread
     descriptor in older versions is not fully initialized.  */
  if (pds.p_nr == 1)
    {
      infop->ti_tid = th->th_ta_p->pthread_threads_max * 2 + 1;
      infop->ti_type = TD_THR_SYSTEM;
      infop->ti_state = TD_THR_ACTIVE;
    }
  else
    {
      infop->ti_tid = pds.p_tid;
      infop->ti_tls = (char *) pds.p_specific;
      infop->ti_pri = pds.p_priority;
      infop->ti_type = TD_THR_USER;

      if (! pds.p_terminated)
	/* XXX For now there is no way to get more information.  */
	infop->ti_state = TD_THR_ACTIVE;
      else if (! pds.p_detached)
	infop->ti_state = TD_THR_ZOMBIE;
      else
	infop->ti_state = TD_THR_UNKNOWN;
    }

  /* Initialization which are the same in both cases.  */
  infop->ti_lid = pds.p_pid ?: ps_getpid (th->th_ta_p->ph);
  infop->ti_ta_p = th->th_ta_p;
  infop->ti_startfunc = pds.p_start_args.start_routine;
  memcpy (&infop->ti_events, &pds.p_eventbuf.eventmask,
	  sizeof (td_thr_events_t));
  infop->ti_traceme = pds.p_report_events != 0;

  return TD_OK;
}
