/* Copyright (C) 1999 Free Software Foundation, Inc.
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

/* The definitions in this file must correspond to those in the debugger.  */
#include <sys/procfs.h>

typedef enum
{
  PS_OK,          /* generic "call succeeded" */
  PS_ERR,         /* generic. */
  PS_BADPID,      /* bad process handle */
  PS_BADLID,      /* bad lwp identifier */
  PS_BADADDR,     /* bad address */
  PS_NOSYM,       /* p_lookup() could not find given symbol */
        PS_NOFREGS
  /*
   * FPU register set not available for given
   * lwp
   */
}       ps_err_e;


struct ps_prochandle;		/* user defined. */


extern ps_err_e ps_pdread(struct ps_prochandle *,
                        psaddr_t, void *, size_t);
extern ps_err_e ps_pdwrite(struct ps_prochandle *,
                        psaddr_t, const void *, size_t);
extern ps_err_e ps_ptread(struct ps_prochandle *,
                        psaddr_t, void *, size_t);
extern ps_err_e ps_ptwrite(struct ps_prochandle *,
                        psaddr_t, const void *, size_t);

extern ps_err_e ps_pglobal_lookup(struct ps_prochandle *,
        const char *object_name, const char *sym_name, psaddr_t *sym_addr);


extern ps_err_e ps_lgetregs(struct ps_prochandle *,
                        lwpid_t, prgregset_t);
extern ps_err_e ps_lsetregs(struct ps_prochandle *,
                        lwpid_t, const prgregset_t);
extern ps_err_e ps_lgetfpregs(struct ps_prochandle *,
                        lwpid_t, prfpregset_t *);
extern ps_err_e ps_lsetfpregs(struct ps_prochandle *,
                        lwpid_t, const prfpregset_t *);

extern pid_t ps_getpid (struct ps_prochandle *);


extern ps_err_e ps_pstop (const struct ps_prochandle *);
extern ps_err_e ps_pcontinue (const struct ps_prochandle *);

extern ps_err_e ps_lstop (const struct ps_prochandle *, lwpid_t);
extern ps_err_e ps_lcontinue (const struct ps_prochandle *, lwpid_t);
