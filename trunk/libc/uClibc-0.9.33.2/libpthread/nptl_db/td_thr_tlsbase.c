/* Locate TLS data for a thread.
   Copyright (C) 2003, 2006 Free Software Foundation, Inc.
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

td_err_e
td_thr_tlsbase (const td_thrhandle_t *th,
		unsigned long int modid,
		psaddr_t *base)
{
  td_err_e err;
  psaddr_t dtv, dtvslot, dtvptr;

  if (modid < 1)
    return TD_NOTLS;

  /* Get the DTV pointer from the thread descriptor.  */
  err = DB_GET_FIELD (dtv, th->th_ta_p, th->th_unique, pthread, dtvp, 0);
  if (err != TD_OK)
    return err;

  /* Find the corresponding entry in the DTV.  */
  err = DB_GET_FIELD_ADDRESS (dtvslot, th->th_ta_p, dtv, dtv, dtv, modid);
  if (err != TD_OK)
    return err;

  /* Extract the TLS block address from that DTV slot.  */
  err = DB_GET_FIELD (dtvptr, th->th_ta_p, dtvslot, dtv_t, pointer_val, 0);
  if (err != TD_OK)
    return err;

  /* It could be that the memory for this module is not allocated for
     the given thread.  */
  if ((uintptr_t) dtvptr & 1)
    return TD_TLSDEFER;

  *base = dtvptr;
  return TD_OK;
}
