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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <link.h>
#include "thread_dbP.h"

/* Value used for dtv entries for which the allocation is delayed.  */
# define TLS_DTV_UNALLOCATED	((void *) -1l)


td_err_e
td_thr_tls_get_addr (const td_thrhandle_t *th __attribute__ ((unused)),
		     void *map_address __attribute__ ((unused)),
		     size_t offset __attribute__ ((unused)),
		     void **address __attribute__ ((unused)))
{
#if defined(USE_TLS) && USE_TLS
  size_t modid;
  union dtv pdtv, *dtvp;

  LOG ("td_thr_tls_get_addr");

  /* Get the DTV pointer from the thread descriptor.  */
  if (ps_pdread (th->th_ta_p->ph,
		 &((struct _pthread_descr_struct *) th->th_unique)->p_header.data.dtvp,
		 &dtvp, sizeof dtvp) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  /* Read the module ID from the link_map.  */
  if (ps_pdread (th->th_ta_p->ph,
		 &((struct link_map *) map_address)->l_tls_modid,
		 &modid, sizeof modid) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  /* Get the corresponding entry in the DTV.  */
  if (ps_pdread (th->th_ta_p->ph, dtvp + modid,
		 &pdtv, sizeof (union dtv)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  /* It could be that the memory for this module is not allocated for
     the given thread.  */
  if (pdtv.pointer == TLS_DTV_UNALLOCATED)
    /* There is not much we can do.  */
    return TD_NOTALLOC;

  *address = (char *) pdtv.pointer + offset;

  return TD_OK;
#else
  return TD_ERR;
#endif
}
