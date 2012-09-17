/* @(#)getrpcport.c	2.1 88/07/29 4.0 RPCSRC */
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
static  char sccsid[] = "@(#)getrpcport.c 1.3 87/08/11 SMI";
#endif

/*
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#define __FORCE_GLIBC
#include <features.h>

#include <alloca.h>
#include <errno.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <rpc/rpc.h>
#include <rpc/clnt.h>
#include <rpc/pmap_clnt.h>
#include <sys/socket.h>


int
getrpcport (const char *host, u_long prognum, u_long versnum, u_int proto)
{
  struct sockaddr_in addr;
  struct hostent hostbuf, *hp;
  size_t buflen;
  char *buffer;
  int herr;

  buflen = 1024;
  buffer = alloca (buflen);
  while (gethostbyname_r (host, &hostbuf, buffer, buflen, &hp, &herr) != 0
	 || hp == NULL)
    if (herr != NETDB_INTERNAL || errno != ERANGE)
      return 0;
    else
      {
	/* Enlarge the buffer.  */
	buflen *= 2;
	buffer = alloca (buflen);
      }

  memcpy ((char *) &addr.sin_addr, hp->h_addr, hp->h_length);
  addr.sin_family = AF_INET;
  addr.sin_port = 0;
  return pmap_getport (&addr, prognum, versnum, proto);
}
