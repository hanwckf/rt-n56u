/* Detach to target process.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <stdlib.h>

#include "thread_dbP.h"


td_err_e
td_ta_delete (td_thragent_t *ta)
{
  LOG ("td_ta_delete");

  /* Safety check.  */
  if (ta == NULL || __td_agent_list == NULL)
    return TD_BADTA;

  /* Remove the handle from the list.  */
  if (ta == __td_agent_list->ta)
    /* It's the first element of the list.  */
    __td_agent_list = __td_agent_list->next;
  else
    {
      /* We have to search for it.  */
      struct agent_list *runp = __td_agent_list;

      while (runp->next != NULL && runp->next->ta != ta)
	runp = runp->next;

      if (runp->next == NULL)
	/* It's not a valid decriptor since it is not in the list.  */
	return TD_BADTA;

      runp->next = runp->next->next;
    }

  /* The handle was allocated in `td_ta_new'.  */
  free (ta);

  return TD_OK;
}
