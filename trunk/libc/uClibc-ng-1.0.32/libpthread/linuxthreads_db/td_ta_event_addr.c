/* Get event address.
   Copyright (C) 1999, 2001 Free Software Foundation, Inc.
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

#include "thread_dbP.h"


td_err_e
td_ta_event_addr (const td_thragent_t *ta, td_event_e event, td_notify_t *addr)
{
  td_err_e res = TD_NOEVENT;
  int idx = -1;

  LOG ("td_ta_event_addr");

  /* Test whether the TA parameter is ok.  */
  if (! ta_ok (ta))
    return TD_BADTA;

  switch (event)
    {
    case TD_CREATE:
      idx = LINUXTHREADS_CREATE_EVENT;
      break;

    case TD_DEATH:
      idx = LINUXTHREADS_DEATH_EVENT;
      break;

    case TD_REAP:
      idx = LINUXTHREADS_REAP_EVENT;
      break;

    default:
      /* Event cannot be handled.  */
      break;
    }

  /* Now get the address.  */
  if (idx != -1)
    {
      psaddr_t taddr;

      if (td_lookup (ta->ph, idx, &taddr) == PS_OK)
	{
	  /* Success, we got the address.  */
	  addr->type = NOTIFY_BPT;
	  addr->u.bptaddr = taddr;

	  res = TD_OK;
	}
      else
	res = TD_ERR;
    }

  return res;
}
