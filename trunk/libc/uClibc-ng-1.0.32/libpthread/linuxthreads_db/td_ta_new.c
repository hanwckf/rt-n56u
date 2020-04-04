/* Attach to target process.
   Copyright (C) 1999, 2001, 2002, 2003 Free Software Foundation, Inc.
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
#include <stdlib.h>
#include <string.h>

#include "thread_dbP.h"


/* Datatype for the list of known thread agents.  Normally there will
   be exactly one so we don't spend much though on making it fast.  */
struct agent_list *__td_agent_list;


td_err_e
td_ta_new (struct ps_prochandle *ps, td_thragent_t **ta)
{
  psaddr_t addr;
  psaddr_t versaddr;
  char versbuf[sizeof (VERSION)];
  struct agent_list *elemp;

  LOG ("td_ta_new");

  /* Get the global event mask.  This is one of the variables which
     are new in the thread library to enable debugging.  If it is
     not available we cannot debug.  */
  if (td_lookup (ps, PTHREAD_THREADS_EVENTS, &addr) != PS_OK)
    return TD_NOLIBTHREAD;

  /* Check whether the versions match.  */
  if (td_lookup (ps, LINUXTHREADS_VERSION, &versaddr) != PS_OK)
    return TD_VERSION;
  if (ps_pdread (ps, versaddr, versbuf, sizeof (versbuf)) != PS_OK)
    return TD_ERR;

  versbuf[sizeof (versbuf) - 1] = '\0';
  if (strcmp (versbuf, VERSION) != 0)
    /* Not the right version.  */
    return TD_VERSION;

  /* Fill in the appropriate information.  */
  *ta = (td_thragent_t *) malloc (sizeof (td_thragent_t));
  if (*ta == NULL)
    return TD_MALLOC;

  /* Store the proc handle which we will pass to the callback functions
     back into the debugger.  */
  (*ta)->ph = ps;

  /* Remember the address.  */
  (*ta)->pthread_threads_eventsp = (td_thr_events_t *) addr;

  /* Get the pointer to the variable pointing to the thread descriptor
     with the last event.  */
  if (td_lookup (ps, PTHREAD_LAST_EVENT, &(*ta)->pthread_last_event) != PS_OK)
    {
    free_return:
      free (*ta);
      return TD_ERR;
    }

  /* Get the pointer to the variable containing the number of active
     threads.  */
  if (td_lookup (ps, PTHREAD_HANDLES_NUM, &(*ta)->pthread_handles_num)
      != PS_OK)
    goto free_return;

  /* See whether the library contains the necessary symbols.  */
  if (td_lookup (ps, PTHREAD_HANDLES, &addr) != PS_OK)
    goto free_return;

  (*ta)->handles = (struct pthread_handle_struct *) addr;


  if (td_lookup (ps, PTHREAD_KEYS, &addr) != PS_OK)
    goto free_return;

  /* Cast to the right type.  */
  (*ta)->keys = (struct pthread_key_struct *) addr;

  /* Find out about the maximum number of threads.  Old implementations
     don't provide this information.  In this case we assume that the
     debug  library is compiled with the same values.  */
  if (td_lookup (ps, LINUXTHREADS_PTHREAD_THREADS_MAX, &addr) != PS_OK)
    (*ta)->pthread_threads_max = PTHREAD_THREADS_MAX;
  else
    {
      if (ps_pdread (ps, addr, &(*ta)->pthread_threads_max, sizeof (int))
	  != PS_OK)
	goto free_return;
    }

  /* Similar for the maximum number of thread local data keys.  */
  if (td_lookup (ps, LINUXTHREADS_PTHREAD_KEYS_MAX, &addr) != PS_OK)
    (*ta)->pthread_keys_max = PTHREAD_KEYS_MAX;
  else
    {
      if (ps_pdread (ps, addr, &(*ta)->pthread_keys_max, sizeof (int))
	  != PS_OK)
	goto free_return;
    }

  /* And for the size of the second level arrays for the keys.  */
  if (td_lookup (ps, LINUXTHREADS_PTHREAD_SIZEOF_DESCR, &addr) != PS_OK)
    (*ta)->sizeof_descr = sizeof (struct _pthread_descr_struct);
  else
    {
      if (ps_pdread (ps, addr, &(*ta)->sizeof_descr, sizeof (int)) != PS_OK)
	goto free_return;
      /* Don't let bogons in the inferior make us mess ourselves.  */
      if ((*ta)->sizeof_descr > sizeof (struct _pthread_descr_struct))
	(*ta)->sizeof_descr = sizeof (struct _pthread_descr_struct);
    }

  /* Now add the new agent descriptor to the list.  */
  elemp = (struct agent_list *) malloc (sizeof (struct agent_list));
  if (elemp == NULL)
    {
      /* Argh, now that everything else worked...  */
      free (*ta);
      return TD_MALLOC;
    }

  /* We don't care for thread-safety here.  */
  elemp->ta = *ta;
  elemp->next = __td_agent_list;
  __td_agent_list = elemp;

  return TD_OK;
}
