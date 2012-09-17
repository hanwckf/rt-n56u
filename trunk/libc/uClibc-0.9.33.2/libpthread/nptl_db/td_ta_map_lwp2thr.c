/* Which thread is running on an LWP?
   Copyright (C) 2003, 2004 Free Software Foundation, Inc.
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
#include <stdlib.h>
#include <byteswap.h>
#include <sys/procfs.h>


td_err_e
td_ta_map_lwp2thr (const td_thragent_t *ta_arg,
		   lwpid_t lwpid, td_thrhandle_t *th)
{
  td_thragent_t *const ta = (td_thragent_t *) ta_arg;
  ps_err_e err;
  td_err_e terr;
  prgregset_t regs;
  psaddr_t addr;

  LOG ("td_ta_map_lwp2thr");

  /* Test whether the TA parameter is ok.  */
  if (! ta_ok (ta))
    return TD_BADTA;

  if (ta->ta_howto == ta_howto_unknown)
    {
      /* We need to read in from the inferior the instructions what to do.  */
      psaddr_t howto;

      err = td_lookup (ta->ph, SYM_TH_UNIQUE_CONST_THREAD_AREA, &howto);
      if (err == PS_OK)
	{
	  err = ps_pdread (ta->ph, howto,
			   &ta->ta_howto_data.const_thread_area,
			   sizeof ta->ta_howto_data.const_thread_area);
	  if (err != PS_OK)
	    return TD_ERR;
	  ta->ta_howto = ta_howto_const_thread_area;
	  if (ta->ta_howto_data.const_thread_area & 0xff000000U)
	    ta->ta_howto_data.const_thread_area
	      = bswap_32 (ta->ta_howto_data.const_thread_area);
	}
      else
	{
	  switch (sizeof (regs[0]))
	    {
	    case 8:
	      err = td_lookup (ta->ph, SYM_TH_UNIQUE_REGISTER64, &howto);
	      if (err == PS_OK)
		ta->ta_howto = ta_howto_reg;
	      else if (err == PS_NOSYM)
		{
		  err = td_lookup (ta->ph,
				   SYM_TH_UNIQUE_REGISTER64_THREAD_AREA,
				   &howto);
		  if (err == PS_OK)
		    ta->ta_howto = ta_howto_reg_thread_area;
		}
	      break;

	    case 4:
	      err = td_lookup (ta->ph, SYM_TH_UNIQUE_REGISTER32, &howto);
	      if (err == PS_OK)
		ta->ta_howto = ta_howto_reg;
	      else if (err == PS_NOSYM)
		{
		  err = td_lookup (ta->ph,
				   SYM_TH_UNIQUE_REGISTER32_THREAD_AREA,
				   &howto);
		  if (err == PS_OK)
		    ta->ta_howto = ta_howto_reg_thread_area;
		}
	      break;

	    default:
	      abort ();
	      return TD_DBERR;
	    }

	  if (err != PS_OK)
	    return TD_DBERR;

	  /* For either of these methods we read in the same descriptor.  */
	  err = ps_pdread (ta->ph, howto,
			   ta->ta_howto_data.reg, DB_SIZEOF_DESC);
	  if (err != PS_OK)
	    return TD_ERR;
	  if (DB_DESC_SIZE (ta->ta_howto_data.reg) == 0)
	    return TD_DBERR;
	  if (DB_DESC_SIZE (ta->ta_howto_data.reg) & 0xff000000U)
	    {
	      /* Byte-swap these words, though we leave the size word
		 in native order as the handy way to distinguish.  */
	      DB_DESC_OFFSET (ta->ta_howto_data.reg)
		= bswap_32 (DB_DESC_OFFSET (ta->ta_howto_data.reg));
	      DB_DESC_NELEM (ta->ta_howto_data.reg)
		= bswap_32 (DB_DESC_NELEM (ta->ta_howto_data.reg));
	    }
	}
    }

  switch (ta->ta_howto)
    {
    case ta_howto_unknown:
      return TD_DBERR;

    default:
      return TD_DBERR;

    case ta_howto_reg:
      /* On most machines, we are just looking at a register.  */
      if (ps_lgetregs (ta->ph, lwpid, regs) != PS_OK)
	return TD_ERR;
      terr = _td_fetch_value_local (ta, ta->ta_howto_data.reg, -1,
				    0, regs, &addr);
      if (terr != TD_OK)
	return terr;
      /* In this descriptor the nelem word is overloaded as the bias.  */
      addr += (int32_t) DB_DESC_NELEM (ta->ta_howto_data.reg);
      th->th_unique = addr;
      break;

    case ta_howto_const_thread_area:
      /* Some hosts don't have this call and this case won't be used.  */
# pragma weak ps_get_thread_area
      if (&ps_get_thread_area == NULL)
	return TD_NOCAPAB;

       /* A la x86-64, there is a constant magic index for get_thread_area.  */
       if (ps_get_thread_area (ta->ph, lwpid,
			       ta->ta_howto_data.const_thread_area,
			       &th->th_unique) != PS_OK)
	 return TD_ERR;	/* XXX Other error value?  */
       break;

     case ta_howto_reg_thread_area:
      if (&ps_get_thread_area == NULL)
	return TD_NOCAPAB;

       /* A la i386, there is a register with an index for get_thread_area.  */
       if (ps_lgetregs (ta->ph, lwpid, regs) != PS_OK)
	 return TD_ERR;
       terr = _td_fetch_value_local (ta, ta->ta_howto_data.reg_thread_area, -1,
				     0, regs, &addr);
      if (terr != TD_OK)
	return terr;
      /* In this descriptor the nelem word is overloaded as scale factor.  */
      if (ps_get_thread_area
	  (ta->ph, lwpid,
	   ((addr - (psaddr_t) 0)
	    >> DB_DESC_NELEM (ta->ta_howto_data.reg_thread_area)),
	   &th->th_unique) != PS_OK)
	return TD_ERR;	/* XXX Other error value?  */
      break;
    }

  /* Found it.  Now complete the `td_thrhandle_t' object.  */
  th->th_ta_p = (td_thragent_t *) ta;

  return TD_OK;
}
