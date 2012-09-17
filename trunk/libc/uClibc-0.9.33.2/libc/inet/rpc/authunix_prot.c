/*
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user.
 *
 * SUN RPC IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 *
 * Sun RPC is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 *
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY SUN RPC
 * OR ANY PART THEREOF.
 *
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 *
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */
/*
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */
/*
 * authunix_prot.c
 * XDR for UNIX style authentication parameters for RPC
 */

#include <rpc/types.h>
#include <rpc/xdr.h>
#include <rpc/auth.h>
#include <rpc/auth_unix.h>


/*
 * XDR for unix authentication parameters.
 * Unfortunately, none of these can be declared const.
 */
bool_t
xdr_authunix_parms (XDR * xdrs, struct authunix_parms *p)
{
  if (xdr_u_long (xdrs, &(p->aup_time))
      && xdr_string (xdrs, &(p->aup_machname), MAX_MACHINE_NAME)
      && (sizeof (uid_t) == sizeof (short int)
	  ? xdr_u_short (xdrs, (u_short *) & (p->aup_uid))
	  : xdr_u_int (xdrs, (u_int *) & (p->aup_uid)))
      && (sizeof (gid_t) == sizeof (short int)
	  ? xdr_u_short (xdrs, (u_short *) & (p->aup_gid))
	  : xdr_u_int (xdrs, (u_int *) & (p->aup_gid)))
      && xdr_array (xdrs, (caddr_t *) & (p->aup_gids),
		    & (p->aup_len), NGRPS, sizeof (gid_t),
		      (sizeof (gid_t) == sizeof (short int)
		       ? (xdrproc_t) xdr_u_short : (xdrproc_t) xdr_u_int)))
    {
      return TRUE;
    }
  return FALSE;
}
libc_hidden_def(xdr_authunix_parms)
