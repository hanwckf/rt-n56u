/* Locate TLS data for a thread.
   Copyright (C) 2003, 2004, 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

/* Value used for dtv entries for which the allocation is delayed.  */
# define TLS_DTV_UNALLOCATED	((void *) -1l)

td_err_e
td_thr_tlsbase (const td_thrhandle_t *th,
		unsigned long int modid,
		psaddr_t *base)
{
  if (modid < 1)
    return TD_NOTLS;

#if USE_TLS
  union dtv pdtv, *dtvp;

  LOG ("td_thr_tlsbase");

  psaddr_t dtvpp = th->th_unique;
#if defined(TLS_TCB_AT_TP)
  dtvpp += offsetof (struct _pthread_descr_struct, p_header.data.dtvp);
#elif defined(TLS_DTV_AT_TP)
/* Special case hack.  If TLS_TCB_SIZE == 0 (on PowerPC), there is no TCB
   containing the DTV at the TP, but actually the TCB lies behind the TP,
   i.e. at the very end of the area covered by TLS_PRE_TCB_SIZE.  */
  dtvpp += TLS_PRE_TCB_SIZE + offsetof (tcbhead_t, dtv)
	   - (TLS_TCB_SIZE == 0 ? sizeof (tcbhead_t) : 0);
#else
# error "Either TLS_TCB_AT_TP or TLS_DTV_AT_TP must be defined."
#endif

  /* Get the DTV pointer from the thread descriptor.  */
  if (ps_pdread (th->th_ta_p->ph, dtvpp, &dtvp, sizeof dtvp) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  /* Get the corresponding entry in the DTV.  */
  if (ps_pdread (th->th_ta_p->ph, dtvp + modid,
		 &pdtv, sizeof (union dtv)) != PS_OK)
    return TD_ERR;	/* XXX Other error value?  */

  /* It could be that the memory for this module is not allocated for
     the given thread.  */
  if (pdtv.pointer.val == TLS_DTV_UNALLOCATED)
    return TD_TLSDEFER;

  *base = (char *) pdtv.pointer.val;

  return TD_OK;
#else
  return TD_ERR;
#endif
}
