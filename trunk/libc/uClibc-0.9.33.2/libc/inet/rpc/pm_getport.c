/* @(#)pmap_getport.c	2.2 88/08/01 4.0 RPCSRC */
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
#if 0
static char sccsid[] = "@(#)pmap_getport.c 1.9 87/08/11 Copyr 1984 Sun Micro";
#endif

/*
 * pmap_getport.c
 * Client interface to pmap rpc service.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#include <stdbool.h>
#include <unistd.h>
#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <rpc/pmap_clnt.h>


static const struct timeval timeout =
{5, 0};
static const struct timeval tottimeout =
{60, 0};

/*
 * Find the mapped port for program,version.
 * Calls the pmap service remotely to do the lookup.
 * Returns 0 if no map exists.
 */
u_short
pmap_getport (struct sockaddr_in *address, u_long program, u_long version,
			  u_int protocol)
{
  u_short port = 0;
  int _socket = -1;
  CLIENT *client;
  struct pmap parms;

  address->sin_port = htons (PMAPPORT);
  client = clntudp_bufcreate (address, PMAPPROG,
	      PMAPVERS, timeout, &_socket, RPCSMALLMSGSIZE, RPCSMALLMSGSIZE);
  if (client != (CLIENT *) NULL)
    {
      struct rpc_createerr *ce = &get_rpc_createerr ();
      parms.pm_prog = program;
      parms.pm_vers = version;
      parms.pm_prot = protocol;
      parms.pm_port = 0;	/* not needed or used */
      if (CLNT_CALL (client, PMAPPROC_GETPORT, (xdrproc_t)xdr_pmap,
		     (caddr_t)&parms, (xdrproc_t)xdr_u_short,
		     (caddr_t)&port, tottimeout) != RPC_SUCCESS)
	{
	  ce->cf_stat = RPC_PMAPFAILURE;
	  clnt_geterr (client, &ce->cf_error);
	}
      else if (port == 0)
	{
	  ce->cf_stat = RPC_PROGNOTREGISTERED;
	}
      CLNT_DESTROY (client);
    }
  /* (void)__close(_socket); CLNT_DESTROY already closed it */
  address->sin_port = 0;
  return port;
}
libc_hidden_def(pmap_getport)
