/* Get a thread-specific data pointer for a thread.
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

#include "thread_dbP.h"
#include "../linuxthreads/internals.h"


td_err_e
td_thr_tsd (const td_thrhandle_t *th, const thread_key_t tk, void **data)
{
  struct _pthread_descr_struct pds;
  struct pthread_key_struct *keys = th->th_ta_p->keys;
  struct pthread_key_struct key;
  int pthread_keys_max = th->th_ta_p->pthread_keys_max;
  int pthread_key_2ndlevel_size = th->th_ta_p->pthread_key_2ndlevel_size;
  unsigned int idx1st;
  unsigned int idx2nd;
  void *p;

  LOG ("td_thr_tsd");

  /* If there is no thread descriptor there cannot be any thread
     specific data.  */
  if (th->th_unique == NULL)
    return TD_BADKEY;

  /* Get the thread descriptor.  */
  if (ps_pdread (th->th_ta_p->ph, th->th_unique, &pds,
		 sizeof (struct _pthread_descr_struct)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  /* Check correct value of key.  */
  if (tk >= pthread_keys_max)
    return TD_BADKEY;

  /* Get the key entry.  */
  if (ps_pdread (th->th_ta_p->ph, &keys[tk], &key,
		 sizeof (struct pthread_key_struct)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  /* Fail if this key is not at all used.  */
  if (! key.in_use)
    return TD_BADKEY;

  /* Compute the indeces.  */
  idx1st = tk / pthread_key_2ndlevel_size;
  idx2nd = tk % pthread_key_2ndlevel_size;

  /* Check the pointer to the second level array.  */
  if (pds.p_specific[idx1st] == NULL)
    return TD_NOTSD;

  /* Now get the real key.
     XXX I don't know whether it's correct but there is currently no
     easy way to determine whether a key was never set or the value
     is NULL.  We return an error whenever the value is NULL.  */
  if (ps_pdread (th->th_ta_p->ph, &pds.p_specific[idx1st][idx2nd], &p,
		 sizeof (void *)) != PS_OK)
    return TD_ERR;

  if (p != NULL)
    *data = p;

  return p != NULL ? TD_OK : TD_NOTSD;
}
