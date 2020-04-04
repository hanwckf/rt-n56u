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
td_ta_event_getmsg (const td_thragent_t *ta, td_event_msg_t *msg)
{
  /* XXX I cannot think of another way but using a static variable.  */
  static td_thrhandle_t th;
  td_eventbuf_t event;
  psaddr_t addr;

  LOG ("td_ta_event_getmsg");

  /* Test whether the TA parameter is ok.  */
  if (! ta_ok (ta))
    return TD_BADTA;

  /* Get the pointer to the thread descriptor with the last event.  */
  if (ps_pdread (ta->ph, ta->pthread_last_event,
		 &addr, sizeof (void *)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  /* If the pointer is NULL no event occurred.  */
  if (addr == 0)
    return TD_NOMSG;

  /* Read the even structure from the target.  */
  if (ps_pdread (ta->ph,
		 ((char *) addr
		  + offsetof (struct _pthread_descr_struct, p_eventbuf)),
		 &event, sizeof (td_eventbuf_t)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  /* Check whether an event occurred.  */
  if (event.eventnum == TD_EVENT_NONE)
    {
      /* Oh well, this means the last event was already read.  So
	 we have to look for any other event.  */
      struct pthread_handle_struct handles[ta->pthread_threads_max];
      int num;
      int i;

      /* Read the number of currently active threads.  */
      if (ps_pdread (ta->ph, ta->pthread_handles_num, &num, sizeof (int))
	  != PS_OK)
	return TD_ERR;	/* XXX Other error value?  */

      /* Now read the handles.  */
      if (ps_pdread (ta->ph, ta->handles, handles,
		     ta->pthread_threads_max * sizeof (handles[0])) != PS_OK)
	return TD_ERR;	/* XXX Other error value?  */

      for (i = 0; i < ta->pthread_threads_max && num > 0; ++i)
	{
	  if (handles[i].h_descr == NULL)
	    /* No entry here.  */
	    continue;

	  /* First count this active thread.  */
	  --num;

	  if (handles[i].h_descr == addr)
	    /* We already handled this.  */
	    continue;

	  /* Read the event data for this thread.  */
	  if (ps_pdread (ta->ph,
			 ((char *) handles[i].h_descr
			  + offsetof (struct _pthread_descr_struct,
				      p_eventbuf)),
			 &event, sizeof (td_eventbuf_t)) != PS_OK)
	    return TD_ERR;

	  if (event.eventnum != TD_EVENT_NONE)
	    {
	      /* We found a thread with an unreported event.  */
	      addr = handles[i].h_descr;
	      break;
	    }
	}

      /* If we haven't found any other event signal this to the user.  */
      if (event.eventnum == TD_EVENT_NONE)
	return TD_NOMSG;
    }

  /* Generate the thread descriptor.  */
  th.th_ta_p = (td_thragent_t *) ta;
  th.th_unique = addr;

  /* Fill the user's data structure.  */
  msg->event = event.eventnum;
  msg->th_p = &th;
  msg->msg.data = (uintptr_t) event.eventdata;

  /* And clear the event message in the target.  */
  memset (&event, '\0', sizeof (td_eventbuf_t));
  if (ps_pdwrite (ta->ph,
		  ((char *) addr
		   + offsetof (struct _pthread_descr_struct, p_eventbuf)),
		  &event, sizeof (td_eventbuf_t)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  return TD_OK;
}
