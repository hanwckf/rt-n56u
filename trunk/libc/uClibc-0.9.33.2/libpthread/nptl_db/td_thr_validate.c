/* Validate a thread handle.
   Copyright (C) 1999, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 1999.

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
#include <stdbool.h>

static td_err_e
check_thread_list (const td_thrhandle_t *th, psaddr_t head, bool *uninit)
{
  td_err_e err;
  psaddr_t next, ofs;

  err = DB_GET_FIELD (next, th->th_ta_p, head, list_t, next, 0);
  if (err == TD_OK)
    {
      if (next == 0)
	{
	  *uninit = true;
	  return TD_NOTHR;
	}
      err = DB_GET_FIELD_ADDRESS (ofs, th->th_ta_p, 0, pthread, list, 0);
    }

  while (err == TD_OK)
    {
      if (next == head)
	return TD_NOTHR;

      if (next - (ofs - (psaddr_t) 0) == th->th_unique)
	return TD_OK;

      err = DB_GET_FIELD (next, th->th_ta_p, next, list_t, next, 0);
    }

  return err;
}


td_err_e
td_thr_validate (const td_thrhandle_t *th)
{
  td_err_e err;
  psaddr_t list = NULL;

  LOG ("td_thr_validate");

  /* First check the list with threads using user allocated stacks.  */
  bool uninit = false;
  err = DB_GET_SYMBOL (list, th->th_ta_p, __stack_user);
  if (err == TD_OK)
    err = check_thread_list (th, list, &uninit);

  /* If our thread is not on this list search the list with stack
     using implementation allocated stacks.  */
  if (err == TD_NOTHR)
    {
      err = DB_GET_SYMBOL (list, th->th_ta_p, stack_used);
      if (err == TD_OK)
	err = check_thread_list (th, list, &uninit);

      if (err == TD_NOTHR && uninit)
	{
	  /* __pthread_initialize_minimal has not run yet.
	     But the main thread still has a valid ID.  */
	  td_thrhandle_t main_th;
	  err = td_ta_map_lwp2thr (th->th_ta_p,
				   ps_getpid (th->th_ta_p->ph), &main_th);
	  if (err == TD_OK && th->th_unique != main_th.th_unique)
	    err = TD_NOTHR;
	}
    }

  return err;
}
