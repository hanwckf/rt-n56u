/* Enable event process-wide.
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

#include <stddef.h>

#include "thread_dbP.h"


td_err_e
td_thr_event_enable (th, onoff)
     const td_thrhandle_t *th;
     int onoff;
{
  LOG ("td_thr_event_enable");

  /* Write the new value into the thread data structure.  */
  if (th->th_unique == NULL)
    {
      psaddr_t addr;

      if (td_lookup (th->th_ta_p->ph, LINUXTHREADS_INITIAL_REPORT_EVENTS,
		     &addr) != PS_OK)
	/* Cannot read the symbol.  This should not happen.  */
	return TD_ERR;

      if (ps_pdwrite (th->th_ta_p->ph, addr, &onoff, sizeof (int)) != PS_OK)
	return TD_ERR;

      return TD_OK;
    }

  if (ps_pdwrite (th->th_ta_p->ph,
		  ((char *) th->th_unique
		   + offsetof (struct _pthread_descr_struct,
			       p_report_events)),
		  &onoff, sizeof (int)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  return TD_OK;
}
