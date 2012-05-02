/* Get the number of threads in the process.
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

#include "thread_dbP.h"

td_err_e
td_ta_get_nthreads (const td_thragent_t *ta, int *np)
{
  psaddr_t addr;

  LOG ("td_ta_get_nthreads");

  /* Test whether the TA parameter is ok.  */
  if (! ta_ok (ta))
    return TD_BADTA;

  /* Access the variable `__pthread_handles_num'.  */
  if (td_lookup (ta->ph, PTHREAD_HANDLES_NUM, &addr) != PS_OK)
     return TD_ERR;	/* XXX Other error value?  */

  if (ps_pdread (ta->ph, addr, np, sizeof (int)) != PS_OK)
     return TD_ERR;	/* XXX Other error value?  */

  return TD_OK;
}
