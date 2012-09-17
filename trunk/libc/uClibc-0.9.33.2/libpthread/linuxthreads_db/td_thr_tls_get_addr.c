/* Get address of thread local variable.
   Copyright (C) 2002,2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 2002.

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

#include <link.h>
#include "thread_dbP.h"

td_err_e
td_thr_tls_get_addr (const td_thrhandle_t *th __attribute__ ((unused)),
		     void *map_address __attribute__ ((unused)),
		     size_t offset __attribute__ ((unused)),
		     void **address __attribute__ ((unused)))
{
#if USE_TLS
  /* Read the module ID from the link_map.  */
  size_t modid;
  if (ps_pdread (th->th_ta_p->ph,
		 &((struct link_map *) map_address)->l_tls_modid,
		 &modid, sizeof modid) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  td_err_e result = td_thr_tlsbase (th, modid, address);
  if (result == TD_OK)
    *address += offset;
  return result;
#else
  return TD_ERR;
#endif
}
