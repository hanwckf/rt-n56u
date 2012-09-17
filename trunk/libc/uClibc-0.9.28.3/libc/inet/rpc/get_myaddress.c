/* @(#)get_myaddress.c 2.1 88/07/29 4.0 RPCSRC */
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
static char sccsid[] = "@(#)get_myaddress.c 1.4 87/08/11 Copyr 1984 Sun Micro";
#endif

/*
 * get_myaddress.c
 *
 * Get client's IP address via ioctl.  This avoids using the yellowpages.
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#include <rpc/types.h>
#include <rpc/clnt.h>
#include <rpc/pmap_prot.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
/* Order of following two #includes reversed by roland@gnu */
#include <netinet/in.h>
#include <arpa/inet.h>

/*
 * don't use gethostbyname, which would invoke yellow pages
 *
 * Avoid loopback interfaces.  We return information from a loopback
 * interface only if there are no other possible interfaces.
 */
void
get_myaddress (struct sockaddr_in *addr)
{
  int s;
  char buf[BUFSIZ];
  struct ifconf ifc;
  struct ifreq ifreq, *ifr;
  int len, loopback = 0;

  if ((s = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      perror ("get_myaddress: socket");
      exit (1);
    }
  ifc.ifc_len = sizeof (buf);
  ifc.ifc_buf = buf;
  if (ioctl (s, SIOCGIFCONF, (char *) &ifc) < 0)
    {
      perror (_("get_myaddress: ioctl (get interface configuration)"));
      exit (1);
    }

 again:
  ifr = ifc.ifc_req;
  for (len = ifc.ifc_len; len; len -= sizeof ifreq)
    {
      ifreq = *ifr;
      if (ioctl (s, SIOCGIFFLAGS, (char *) &ifreq) < 0)
	{
          perror ("get_myaddress: ioctl");
          exit (1);
	}
      if ((ifreq.ifr_flags & IFF_UP) && (ifr->ifr_addr.sa_family == AF_INET)
	  && (!(ifreq.ifr_flags & IFF_LOOPBACK) ||
	      (loopback == 1 && (ifreq.ifr_flags & IFF_LOOPBACK))))
	{
	  *addr = *((struct sockaddr_in *) &ifr->ifr_addr);
	  addr->sin_port = htons (PMAPPORT);
	  close (s);
	  return;
	}
      ifr++;
    }
  if (loopback == 0)
    {
      loopback = 1;
      goto again;
    }
  close (s);
}
