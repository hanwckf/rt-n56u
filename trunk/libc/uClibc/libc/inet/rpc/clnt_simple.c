/* @(#)clnt_simple.c	2.2 88/08/01 4.0 RPCSRC */
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
static char sccsid[] = "@(#)clnt_simple.c 1.35 87/08/11 Copyr 1984 Sun Micro";
#endif

/*
 * clnt_simple.c
 * Simplified front end to rpc.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#define __FORCE_GLIBC
#include <features.h>

#include <alloca.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include "rpc_private.h"
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

struct callrpc_private_s
  {
    CLIENT *client;
    int socket;
    u_long oldprognum, oldversnum, valid;
    char *oldhost;
  };
#ifdef __UCLIBC_HAS_THREADS__
#define callrpc_private (*(struct callrpc_private_s **)&RPC_THREAD_VARIABLE(callrpc_private_s))
#else
static struct callrpc_private_s *callrpc_private;
#endif

int
callrpc (const char *host, u_long prognum, u_long versnum, u_long procnum,
	 xdrproc_t inproc, const char *in, xdrproc_t outproc, char *out)
{
  struct callrpc_private_s *crp = callrpc_private;
  struct sockaddr_in server_addr;
  enum clnt_stat clnt_stat;
  struct hostent hostbuf, *hp;
  struct timeval timeout, tottimeout;

  if (crp == 0)
    {
      crp = (struct callrpc_private_s *) calloc (1, sizeof (*crp));
      if (crp == 0)
	return 0;
      callrpc_private = crp;
    }
  if (crp->oldhost == NULL)
    {
      crp->oldhost = malloc (256);
      crp->oldhost[0] = 0;
      crp->socket = RPC_ANYSOCK;
    }
  if (crp->valid && crp->oldprognum == prognum && crp->oldversnum == versnum
      && strcmp (crp->oldhost, host) == 0)
    {
      /* reuse old client */
    }
  else
    {
      size_t buflen;
      char *buffer;
      int herr;

      crp->valid = 0;
      if (crp->socket != RPC_ANYSOCK)
	{
	  (void) close (crp->socket);
	  crp->socket = RPC_ANYSOCK;
	}
      if (crp->client)
	{
	  clnt_destroy (crp->client);
	  crp->client = NULL;
	}

      buflen = 1024;
      buffer = alloca (buflen);
      while (gethostbyname_r (host, &hostbuf, buffer, buflen,
				&hp, &herr) != 0
	     || hp == NULL)
	if (herr != NETDB_INTERNAL || errno != ERANGE)
	  return (int) RPC_UNKNOWNHOST;
	else
	  {
	    /* Enlarge the buffer.  */
	    buflen *= 2;
	    buffer = alloca (buflen);
	  }

      timeout.tv_usec = 0;
      timeout.tv_sec = 5;
      memcpy ((char *) &server_addr.sin_addr, hp->h_addr, hp->h_length);
      server_addr.sin_family = AF_INET;
      server_addr.sin_port = 0;
      if ((crp->client = clntudp_create (&server_addr, (u_long) prognum,
			  (u_long) versnum, timeout, &crp->socket)) == NULL)
	return (int) get_rpc_createerr().cf_stat;
      crp->valid = 1;
      crp->oldprognum = prognum;
      crp->oldversnum = versnum;
      (void) strncpy (crp->oldhost, host, 255);
      crp->oldhost[255] = '\0';
    }
  tottimeout.tv_sec = 25;
  tottimeout.tv_usec = 0;
  clnt_stat = clnt_call (crp->client, procnum, inproc, (char *) in,
			 outproc, out, tottimeout);
  /*
   * if call failed, empty cache
   */
  if (clnt_stat != RPC_SUCCESS)
    crp->valid = 0;
  return (int) clnt_stat;
}

#ifdef __UCLIBC_HAS_THREADS__
void
__rpc_thread_clnt_cleanup (void)
{
	struct callrpc_private_s *rcp = RPC_THREAD_VARIABLE(callrpc_private_s);

	if (rcp) {
		if (rcp->client)
			CLNT_DESTROY (rcp->client);
		free (rcp);
	}
}
#endif /* __UCLIBC_HAS_THREADS__ */
