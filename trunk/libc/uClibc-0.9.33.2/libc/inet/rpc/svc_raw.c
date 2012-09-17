/* @(#)svc_raw.c	2.1 88/07/29 4.0 RPCSRC */
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
static char sccsid[] = "@(#)svc_raw.c 1.15 87/08/11 Copyr 1984 Sun Micro";
#endif

/*
 * svc_raw.c,   This a toy for simple testing and timing.
 * Interface to create an rpc client and server in the same UNIX process.
 * This lets us simulate rpc and get rpc (round trip) overhead, without
 * any interference from the kernel.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#define __FORCE_GLIBC
#include <features.h>
#include "rpc_private.h"
#include <rpc/svc.h>


/*
 * This is the "network" that we will be moving data over
 */
struct svcraw_private_s
  {
    char _raw_buf[UDPMSGSIZE];
    SVCXPRT server;
    XDR xdr_stream;
    char verf_body[MAX_AUTH_BYTES];
  };
#ifdef __UCLIBC_HAS_THREADS__
#define svcraw_private (*(struct svcraw_private_s **)&RPC_THREAD_VARIABLE(svcraw_private_s))
#else
static struct svcraw_private_s *svcraw_private;
#endif

static bool_t svcraw_recv (SVCXPRT *, struct rpc_msg *);
static enum xprt_stat svcraw_stat (SVCXPRT *);
static bool_t svcraw_getargs (SVCXPRT *, xdrproc_t, caddr_t);
static bool_t svcraw_reply (SVCXPRT *, struct rpc_msg *);
static bool_t svcraw_freeargs (SVCXPRT *, xdrproc_t, caddr_t);
static void svcraw_destroy (SVCXPRT *);

static struct xp_ops server_ops =
{
  svcraw_recv,
  svcraw_stat,
  svcraw_getargs,
  svcraw_reply,
  svcraw_freeargs,
  svcraw_destroy
};

SVCXPRT *
svcraw_create (void)
{
  struct svcraw_private_s *srp = svcraw_private;

  if (srp == 0)
    {
      srp = (struct svcraw_private_s *) calloc (1, sizeof (*srp));
      if (srp == 0)
	return NULL;
    }
  srp->server.xp_sock = 0;
  srp->server.xp_port = 0;
  srp->server.xp_ops = &server_ops;
  srp->server.xp_verf.oa_base = srp->verf_body;
  xdrmem_create (&srp->xdr_stream, srp->_raw_buf, UDPMSGSIZE, XDR_FREE);
  return &srp->server;
}

static enum xprt_stat
svcraw_stat (SVCXPRT *xprt attribute_unused)
{
  return XPRT_IDLE;
}

static bool_t
svcraw_recv (SVCXPRT *xprt attribute_unused, struct rpc_msg *msg)
{
  struct svcraw_private_s *srp = svcraw_private;
  XDR *xdrs;

  if (srp == 0)
    return FALSE;
  xdrs = &srp->xdr_stream;
  xdrs->x_op = XDR_DECODE;
  XDR_SETPOS (xdrs, 0);
  if (!xdr_callmsg (xdrs, msg))
    return FALSE;
  return TRUE;
}

static bool_t
svcraw_reply (SVCXPRT *xprt attribute_unused, struct rpc_msg *msg)
{
  struct svcraw_private_s *srp = svcraw_private;
  XDR *xdrs;

  if (srp == 0)
    return FALSE;
  xdrs = &srp->xdr_stream;
  xdrs->x_op = XDR_ENCODE;
  XDR_SETPOS (xdrs, 0);
  if (!xdr_replymsg (xdrs, msg))
    return FALSE;
  (void) XDR_GETPOS (xdrs);	/* called just for overhead */
  return TRUE;
}

static bool_t
svcraw_getargs (SVCXPRT *xprt attribute_unused, xdrproc_t xdr_args, caddr_t args_ptr)
{
  struct svcraw_private_s *srp = svcraw_private;

  if (srp == 0)
    return FALSE;
  return (*xdr_args) (&srp->xdr_stream, args_ptr);
}

static bool_t
svcraw_freeargs (SVCXPRT *xprt attribute_unused, xdrproc_t xdr_args, caddr_t args_ptr)
{
  struct svcraw_private_s *srp = svcraw_private;
  XDR *xdrs;

  if (srp == 0)
    return FALSE;
  xdrs = &srp->xdr_stream;
  xdrs->x_op = XDR_FREE;
  return (*xdr_args) (xdrs, args_ptr);
}

static void
svcraw_destroy (SVCXPRT *xprt attribute_unused)
{
}
