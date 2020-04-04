/* Retrieve event.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <stddef.h>
#include <string.h>

#include "thread_dbP.h"


td_err_e
td_thr_event_getmsg (const td_thrhandle_t *th, td_event_msg_t *msg)
{
  td_eventbuf_t event;

  LOG ("td_thr_event_getmsg");

  /* If the thread descriptor has not yet been created there cannot be
     any event.  */
  if (th->th_unique == NULL)
    return TD_NOMSG;

  /* Read the even structure from the target.  */
  if (ps_pdread (th->th_ta_p->ph,
		 ((char *) th->th_unique
		  + offsetof (struct _pthread_descr_struct, p_eventbuf)),
		 &event, sizeof (td_eventbuf_t)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  /* Check whether an event occurred.  */
  if (event.eventnum == TD_EVENT_NONE)
    /* Nothing.  */
    return TD_NOMSG;

  /* Fill the user's data structure.  */
  msg->event = event.eventnum;
  msg->th_p = th;
  msg->msg.data = (uintptr_t) event.eventdata;

  /* And clear the event message in the target.  */
  memset (&event, '\0', sizeof (td_eventbuf_t));
  if (ps_pdwrite (th->th_ta_p->ph,
		  ((char *) th->th_unique
		   + offsetof (struct _pthread_descr_struct, p_eventbuf)),
		  &event, sizeof (td_eventbuf_t)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  return TD_OK;
}
