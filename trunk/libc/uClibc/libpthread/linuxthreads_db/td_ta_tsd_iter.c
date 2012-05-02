/* Iterate over a process's thread-specific data.
   Copyright (C) 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
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
#include <alloca.h>

td_err_e
td_ta_tsd_iter (const td_thragent_t *ta, td_key_iter_f *callback,
		void *cbdata_p)
{
  struct pthread_key_struct *keys;
  int pthread_keys_max;
  int cnt;

  LOG ("td_ta_tsd_iter");

  /* Test whether the TA parameter is ok.  */
  if (! ta_ok (ta))
    return TD_BADTA;

  pthread_keys_max = ta->pthread_keys_max;
  keys = (struct pthread_key_struct *) alloca (sizeof (keys[0])
					       * pthread_keys_max);

  /* Read all the information about the keys.  */
  if (ps_pdread (ta->ph, ta->keys, keys,
		 sizeof (keys[0]) * pthread_keys_max) != PS_OK)
	return TD_ERR;	/* XXX Other error value?  */

  /* Now get all descriptors, one after the other.  */
  for (cnt = 0; cnt < pthread_keys_max; ++cnt)
    if (keys[cnt].in_use
	/* Return with an error if the callback returns a nonzero value.  */
	&& callback (cnt, keys[cnt].destr, cbdata_p) != 0)
      return TD_DBERR;

  return TD_OK;
}
