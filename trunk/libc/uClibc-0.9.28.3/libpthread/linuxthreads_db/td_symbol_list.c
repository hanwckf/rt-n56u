/* Return list of symbols the library can request.
   Copyright (C) 2001, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 2001.

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

#include <assert.h>
#include "thread_dbP.h"


static const char *symbol_list_arr[] =
{
  [PTHREAD_THREADS_EVENTS] = "__pthread_threads_events",
  [PTHREAD_LAST_EVENT] = "__pthread_last_event",
  [PTHREAD_HANDLES_NUM] = "__pthread_handles_num",
  [PTHREAD_HANDLES] = "__pthread_handles",
  [PTHREAD_KEYS] = "pthread_keys",
  [LINUXTHREADS_PTHREAD_THREADS_MAX] = "__linuxthreads_pthread_threads_max",
  [LINUXTHREADS_PTHREAD_KEYS_MAX] = "__linuxthreads_pthread_keys_max",
  [LINUXTHREADS_PTHREAD_SIZEOF_DESCR] = "__linuxthreads_pthread_sizeof_descr",
  [LINUXTHREADS_CREATE_EVENT] = "__linuxthreads_create_event",
  [LINUXTHREADS_DEATH_EVENT] = "__linuxthreads_death_event",
  [LINUXTHREADS_REAP_EVENT] = "__linuxthreads_reap_event",
  [LINUXTHREADS_INITIAL_REPORT_EVENTS] = "__linuxthreads_initial_report_events",
  [LINUXTHREADS_VERSION] = "__linuxthreads_version",
  [NUM_MESSAGES] = NULL
};


const char **
td_symbol_list (void)
{
  return symbol_list_arr;
}


int
td_lookup (struct ps_prochandle *ps, int idx, psaddr_t *sym_addr)
{
  assert (idx >= 0 && idx < NUM_MESSAGES);
  return ps_pglobal_lookup (ps, LIBPTHREAD_SO, symbol_list_arr[idx], sym_addr);
}
