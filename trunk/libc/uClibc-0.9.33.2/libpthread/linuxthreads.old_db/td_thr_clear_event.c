/* Disable specific event for thread.
   Copyright (C) 1999, 2001, 2002, 2005 Free Software Foundation, Inc.
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
td_thr_clear_event (th, event)
     const td_thrhandle_t *th;
     td_thr_events_t *event;
{
  td_thr_events_t old_event;
  int i;

  LOG ("td_thr_clear_event");

  /* If the thread descriptor has not yet been constructed do not do
     anything.  */
  if (th->th_unique == NULL)
    return TD_OK;

  /* Write the new value into the thread data structure.  */
  if (ps_pdread (th->th_ta_p->ph,
		 ((char *) th->th_unique
		  + offsetof (struct _pthread_descr_struct,
			      p_eventbuf.eventmask)),
		 &old_event, sizeof (td_thr_events_t)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  /* Remove the set bits in.  */
  for (i = 0; i < TD_EVENTSIZE; ++i)
    old_event.event_bits[i] &= ~event->event_bits[i];

  /* Write the new value into the thread data structure.  */
  if (ps_pdwrite (th->th_ta_p->ph,
		  ((char *) th->th_unique
		   + offsetof (struct _pthread_descr_struct,
			       p_eventbuf.eventmask)),
		  &old_event, sizeof (td_thr_events_t)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  return TD_OK;
}
