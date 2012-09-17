/* @(#)svc_udp.c	2.2 88/07/29 4.0 RPCSRC */
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
static char sccsid[] = "@(#)svc_udp.c 1.24 87/08/11 Copyr 1984 Sun Micro";
#endif

/*
 * svc_udp.c,
 * Server side for UDP/IP based RPC.  (Does some caching in the hopes of
 * achieving execute-at-most-once semantics.)
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#define __FORCE_GLIBC
#define _GNU_SOURCE
#include <features.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <rpc/rpc.h>
#include <sys/socket.h>
#include <errno.h>

#ifdef IP_PKTINFO
#include <sys/uio.h>
#endif

#ifdef USE_IN_LIBIO
# include <wchar.h>
# include <libio/iolibio.h>
# define fputs(s, f) _IO_fputs (s, f)
#endif

#define rpc_buffer(xprt) ((xprt)->xp_p1)
#ifndef MAX
#define MAX(a, b)     ((a > b) ? a : b)
#endif

static bool_t svcudp_recv (SVCXPRT *, struct rpc_msg *);
static bool_t svcudp_reply (SVCXPRT *, struct rpc_msg *);
static enum xprt_stat svcudp_stat (SVCXPRT *);
static bool_t svcudp_getargs (SVCXPRT *, xdrproc_t, caddr_t);
static bool_t svcudp_freeargs (SVCXPRT *, xdrproc_t, caddr_t);
static void svcudp_destroy (SVCXPRT *);

static const struct xp_ops svcudp_op =
{
  svcudp_recv,
  svcudp_stat,
  svcudp_getargs,
  svcudp_reply,
  svcudp_freeargs,
  svcudp_destroy
};

static int cache_get (SVCXPRT *, struct rpc_msg *, char **replyp,
		      u_long *replylenp);
static void cache_set (SVCXPRT *xprt, u_long replylen);

/*
 * kept in xprt->xp_p2
 */
struct svcudp_data
  {
    u_int su_iosz;		/* byte size of send.recv buffer */
    u_long su_xid;		/* transaction id */
    XDR su_xdrs;		/* XDR handle */
    char su_verfbody[MAX_AUTH_BYTES];	/* verifier body */
    char *su_cache;		/* cached data, NULL if no cache */
  };
#define	su_data(xprt)	((struct svcudp_data *)(xprt->xp_p2))

/*
 * Usage:
 *      xprt = svcudp_create(sock);
 *
 * If sock<0 then a socket is created, else sock is used.
 * If the socket, sock is not bound to a port then svcudp_create
 * binds it to an arbitrary port.  In any (successful) case,
 * xprt->xp_sock is the registered socket number and xprt->xp_port is the
 * associated port number.
 * Once *xprt is initialized, it is registered as a transporter;
 * see (svc.h, xprt_register).
 * The routines returns NULL if a problem occurred.
 */
SVCXPRT *
svcudp_bufcreate (sock, sendsz, recvsz)
     int sock;
     u_int sendsz, recvsz;
{
  bool_t madesock = FALSE;
  SVCXPRT *xprt;
  struct svcudp_data *su;
  struct sockaddr_in addr;
  socklen_t len = sizeof (struct sockaddr_in);
  int pad;
  void *buf;

  if (sock == RPC_ANYSOCK)
    {
      if ((sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
	  perror (_("svcudp_create: socket creation problem"));
	  return (SVCXPRT *) NULL;
	}
      madesock = TRUE;
    }
  memset ((char *) &addr, 0, sizeof (addr));
  addr.sin_family = AF_INET;
  if (bindresvport (sock, &addr))
    {
      addr.sin_port = 0;
      (void) bind (sock, (struct sockaddr *) &addr, len);
    }
  if (getsockname (sock, (struct sockaddr *) &addr, &len) != 0)
    {
      perror (_("svcudp_create - cannot getsockname"));
      if (madesock)
	(void) close (sock);
      return (SVCXPRT *) NULL;
    }
  xprt = (SVCXPRT *) mem_alloc (sizeof (SVCXPRT));
  su = (struct svcudp_data *) mem_alloc (sizeof (*su));
  buf = mem_alloc (((MAX (sendsz, recvsz) + 3) / 4) * 4);
  if (xprt == NULL || su == NULL || buf == NULL)
    {
#ifdef USE_IN_LIBIO
      if (_IO_fwide (stderr, 0) > 0)
	(void) __fwprintf (stderr, L"%s", _("svcudp_create: out of memory\n"));
      else
#endif
	(void) fputs (_("svcudp_create: out of memory\n"), stderr);
      mem_free (xprt, sizeof (SVCXPRT));
      mem_free (su, sizeof (*su));
      mem_free (buf, ((MAX (sendsz, recvsz) + 3) / 4) * 4);
      return NULL;
    }
  su->su_iosz = ((MAX (sendsz, recvsz) + 3) / 4) * 4;
  rpc_buffer (xprt) = buf;
  xdrmem_create (&(su->su_xdrs), rpc_buffer (xprt), su->su_iosz, XDR_DECODE);
  su->su_cache = NULL;
  xprt->xp_p2 = (caddr_t) su;
  xprt->xp_verf.oa_base = su->su_verfbody;
  xprt->xp_ops = &svcudp_op;
  xprt->xp_port = ntohs (addr.sin_port);
  xprt->xp_sock = sock;

#ifdef IP_PKTINFO
  if ((sizeof (struct iovec) + sizeof (struct msghdr)
       + sizeof(struct cmsghdr) + sizeof (struct in_pktinfo))
      > sizeof (xprt->xp_pad))
    {
# ifdef USE_IN_LIBIO
      if (_IO_fwide (stderr, 0) > 0)
	(void) __fwprintf (stderr, L"%s",
			   _("svcudp_create: xp_pad is too small for IP_PKTINFO\n"));
      else
# endif
	(void) fputs (_("svcudp_create: xp_pad is too small for IP_PKTINFO\n"),
		      stderr);
      return NULL;
    }
  pad = 1;
  if (setsockopt (sock, SOL_IP, IP_PKTINFO, (void *) &pad,
		  sizeof (pad)) == 0)
    /* Set the padding to all 1s. */
    pad = 0xff;
  else
#endif
    /* Clear the padding. */
    pad = 0;
  memset (&xprt->xp_pad [0], pad, sizeof (xprt->xp_pad));

  xprt_register (xprt);
  return xprt;
}

SVCXPRT *
svcudp_create (sock)
     int sock;
{

  return svcudp_bufcreate (sock, UDPMSGSIZE, UDPMSGSIZE);
}

static enum xprt_stat
svcudp_stat (xprt)
     SVCXPRT *xprt;
{

  return XPRT_IDLE;
}

static bool_t
svcudp_recv (xprt, msg)
     SVCXPRT *xprt;
     struct rpc_msg *msg;
{
  struct svcudp_data *su = su_data (xprt);
  XDR *xdrs = &(su->su_xdrs);
  int rlen;
  char *reply;
  u_long replylen;
  socklen_t len;

  /* It is very tricky when you have IP aliases. We want to make sure
     that we are sending the packet from the IP address where the
     incoming packet is addressed to. H.J. */
#ifdef IP_PKTINFO
  struct iovec *iovp;
  struct msghdr *mesgp;
#endif

again:
  /* FIXME -- should xp_addrlen be a size_t?  */
  len = (socklen_t) sizeof(struct sockaddr_in);
#ifdef IP_PKTINFO
  iovp = (struct iovec *) &xprt->xp_pad [0];
  mesgp = (struct msghdr *) &xprt->xp_pad [sizeof (struct iovec)];
  if (mesgp->msg_iovlen)
    {
      iovp->iov_base = rpc_buffer (xprt);
      iovp->iov_len = su->su_iosz;
      mesgp->msg_iov = iovp;
      mesgp->msg_iovlen = 1;
      mesgp->msg_name = &(xprt->xp_raddr);
      mesgp->msg_namelen = len;
      mesgp->msg_control = &xprt->xp_pad [sizeof (struct iovec)
					  + sizeof (struct msghdr)];
      mesgp->msg_controllen = sizeof(xprt->xp_pad)
			      - sizeof (struct iovec) - sizeof (struct msghdr);
      rlen = recvmsg (xprt->xp_sock, mesgp, 0);
      if (rlen >= 0)
	len = mesgp->msg_namelen;
    }
  else
#endif
    rlen = recvfrom (xprt->xp_sock, rpc_buffer (xprt),
		     (int) su->su_iosz, 0,
		     (struct sockaddr *) &(xprt->xp_raddr), &len);
  xprt->xp_addrlen = len;
  if (rlen == -1 && errno == EINTR)
    goto again;
  if (rlen < 16)		/* < 4 32-bit ints? */
    return FALSE;
  xdrs->x_op = XDR_DECODE;
  XDR_SETPOS (xdrs, 0);
  if (!xdr_callmsg (xdrs, msg))
    return FALSE;
  su->su_xid = msg->rm_xid;
  if (su->su_cache != NULL)
    {
      if (cache_get (xprt, msg, &reply, &replylen))
	{
#ifdef IP_PKTINFO
	  if (mesgp->msg_iovlen)
	    {
	      iovp->iov_base = reply;
	      iovp->iov_len = replylen;
	      (void) sendmsg (xprt->xp_sock, mesgp, 0);
	    }
	  else
#endif
	    (void) sendto (xprt->xp_sock, reply, (int) replylen, 0,
			   (struct sockaddr *) &xprt->xp_raddr, len);
	  return TRUE;
	}
    }
  return TRUE;
}

static bool_t
svcudp_reply (xprt, msg)
     SVCXPRT *xprt;
     struct rpc_msg *msg;
{
  struct svcudp_data *su = su_data (xprt);
  XDR *xdrs = &(su->su_xdrs);
  int slen, sent;
  bool_t stat = FALSE;
#ifdef IP_PKTINFO
  struct iovec *iovp;
  struct msghdr *mesgp;
#endif

  xdrs->x_op = XDR_ENCODE;
  XDR_SETPOS (xdrs, 0);
  msg->rm_xid = su->su_xid;
  if (xdr_replymsg (xdrs, msg))
    {
      slen = (int) XDR_GETPOS (xdrs);
#ifdef IP_PKTINFO
      mesgp = (struct msghdr *) &xprt->xp_pad [sizeof (struct iovec)];
      if (mesgp->msg_iovlen)
	{
	  iovp = (struct iovec *) &xprt->xp_pad [0];
	  iovp->iov_base = rpc_buffer (xprt);
	  iovp->iov_len = slen;
	  sent = sendmsg (xprt->xp_sock, mesgp, 0);
	}
      else
#endif
	sent = sendto (xprt->xp_sock, rpc_buffer (xprt), slen, 0,
		       (struct sockaddr *) &(xprt->xp_raddr),
		       xprt->xp_addrlen);
      if (sent == slen)
	{
	  stat = TRUE;
	  if (su->su_cache && slen >= 0)
	    {
	      cache_set (xprt, (u_long) slen);
	    }
	}
    }
  return stat;
}

static bool_t
svcudp_getargs (xprt, xdr_args, args_ptr)
     SVCXPRT *xprt;
     xdrproc_t xdr_args;
     caddr_t args_ptr;
{

  return (*xdr_args) (&(su_data (xprt)->su_xdrs), args_ptr);
}

static bool_t
svcudp_freeargs (xprt, xdr_args, args_ptr)
     SVCXPRT *xprt;
     xdrproc_t xdr_args;
     caddr_t args_ptr;
{
  XDR *xdrs = &(su_data (xprt)->su_xdrs);

  xdrs->x_op = XDR_FREE;
  return (*xdr_args) (xdrs, args_ptr);
}

static void
svcudp_destroy (xprt)
     SVCXPRT *xprt;
{
  struct svcudp_data *su = su_data (xprt);

  xprt_unregister (xprt);
  (void) close (xprt->xp_sock);
  XDR_DESTROY (&(su->su_xdrs));
  mem_free (rpc_buffer (xprt), su->su_iosz);
  mem_free ((caddr_t) su, sizeof (struct svcudp_data));
  mem_free ((caddr_t) xprt, sizeof (SVCXPRT));
}


/***********this could be a separate file*********************/

/*
 * Fifo cache for udp server
 * Copies pointers to reply buffers into fifo cache
 * Buffers are sent again if retransmissions are detected.
 */

#define SPARSENESS 4		/* 75% sparse */

#ifdef USE_IN_LIBIO
# define CACHE_PERROR(msg)	\
	if (_IO_fwide (stderr, 0) > 0)					      \
		(void) __fwprintf(stderr, L"%s\n", msg);		      \
	else								      \
		(void) fprintf(stderr, "%s\n", msg)
#else
# define CACHE_PERROR(msg)	\
	(void) fprintf(stderr,"%s\n", msg)
#endif

#define ALLOC(type, size)	\
	(type *) mem_alloc((unsigned) (sizeof(type) * (size)))

#define BZERO(addr, type, size)	 \
	memset((char *) addr, 0, sizeof(type) * (int) (size))

/*
 * An entry in the cache
 */
typedef struct cache_node *cache_ptr;
struct cache_node
  {
    /*
     * Index into cache is xid, proc, vers, prog and address
     */
    u_long cache_xid;
    u_long cache_proc;
    u_long cache_vers;
    u_long cache_prog;
    struct sockaddr_in cache_addr;
    /*
     * The cached reply and length
     */
    char *cache_reply;
    u_long cache_replylen;
    /*
     * Next node on the list, if there is a collision
     */
    cache_ptr cache_next;
  };



/*
 * The entire cache
 */
struct udp_cache
  {
    u_long uc_size;		/* size of cache */
    cache_ptr *uc_entries;	/* hash table of entries in cache */
    cache_ptr *uc_fifo;		/* fifo list of entries in cache */
    u_long uc_nextvictim;	/* points to next victim in fifo list */
    u_long uc_prog;		/* saved program number */
    u_long uc_vers;		/* saved version number */
    u_long uc_proc;		/* saved procedure number */
    struct sockaddr_in uc_addr;	/* saved caller's address */
  };


/*
 * the hashing function
 */
#define CACHE_LOC(transp, xid)	\
 (xid % (SPARSENESS*((struct udp_cache *) su_data(transp)->su_cache)->uc_size))


/*
 * Enable use of the cache.
 * Note: there is no disable.
 */
int
svcudp_enablecache (SVCXPRT *transp, u_long size)
{
  struct svcudp_data *su = su_data (transp);
  struct udp_cache *uc;

  if (su->su_cache != NULL)
    {
      CACHE_PERROR (_("enablecache: cache already enabled"));
      return 0;
    }
  uc = ALLOC (struct udp_cache, 1);
  if (uc == NULL)
    {
      CACHE_PERROR (_("enablecache: could not allocate cache"));
      return 0;
    }
  uc->uc_size = size;
  uc->uc_nextvictim = 0;
  uc->uc_entries = ALLOC (cache_ptr, size * SPARSENESS);
  if (uc->uc_entries == NULL)
    {
      CACHE_PERROR (_("enablecache: could not allocate cache data"));
      return 0;
    }
  BZERO (uc->uc_entries, cache_ptr, size * SPARSENESS);
  uc->uc_fifo = ALLOC (cache_ptr, size);
  if (uc->uc_fifo == NULL)
    {
      CACHE_PERROR (_("enablecache: could not allocate cache fifo"));
      return 0;
    }
  BZERO (uc->uc_fifo, cache_ptr, size);
  su->su_cache = (char *) uc;
  return 1;
}


/*
 * Set an entry in the cache
 */
static void
cache_set (SVCXPRT *xprt, u_long replylen)
{
  cache_ptr victim;
  cache_ptr *vicp;
  struct svcudp_data *su = su_data (xprt);
  struct udp_cache *uc = (struct udp_cache *) su->su_cache;
  u_int loc;
  char *newbuf;

  /*
   * Find space for the new entry, either by
   * reusing an old entry, or by mallocing a new one
   */
  victim = uc->uc_fifo[uc->uc_nextvictim];
  if (victim != NULL)
    {
      loc = CACHE_LOC (xprt, victim->cache_xid);
      for (vicp = &uc->uc_entries[loc];
	   *vicp != NULL && *vicp != victim;
	   vicp = &(*vicp)->cache_next)
	;
      if (*vicp == NULL)
	{
	  CACHE_PERROR (_("cache_set: victim not found"));
	  return;
	}
      *vicp = victim->cache_next;	/* remote from cache */
      newbuf = victim->cache_reply;
    }
  else
    {
      victim = ALLOC (struct cache_node, 1);
      if (victim == NULL)
	{
	  CACHE_PERROR (_("cache_set: victim alloc failed"));
	  return;
	}
      newbuf = mem_alloc (su->su_iosz);
      if (newbuf == NULL)
	{
	  CACHE_PERROR (_("cache_set: could not allocate new rpc_buffer"));
	  return;
	}
    }

  /*
   * Store it away
   */
  victim->cache_replylen = replylen;
  victim->cache_reply = rpc_buffer (xprt);
  rpc_buffer (xprt) = newbuf;
  xdrmem_create (&(su->su_xdrs), rpc_buffer (xprt), su->su_iosz, XDR_ENCODE);
  victim->cache_xid = su->su_xid;
  victim->cache_proc = uc->uc_proc;
  victim->cache_vers = uc->uc_vers;
  victim->cache_prog = uc->uc_prog;
  victim->cache_addr = uc->uc_addr;
  loc = CACHE_LOC (xprt, victim->cache_xid);
  victim->cache_next = uc->uc_entries[loc];
  uc->uc_entries[loc] = victim;
  uc->uc_fifo[uc->uc_nextvictim++] = victim;
  uc->uc_nextvictim %= uc->uc_size;
}

/*
 * Try to get an entry from the cache
 * return 1 if found, 0 if not found
 */
static int
cache_get (xprt, msg, replyp, replylenp)
     SVCXPRT *xprt;
     struct rpc_msg *msg;
     char **replyp;
     u_long *replylenp;
{
  u_int loc;
  cache_ptr ent;
  struct svcudp_data *su = su_data (xprt);
  struct udp_cache *uc = (struct udp_cache *) su->su_cache;

#define EQADDR(a1, a2)	(memcmp((char*)&a1, (char*)&a2, sizeof(a1)) == 0)

  loc = CACHE_LOC (xprt, su->su_xid);
  for (ent = uc->uc_entries[loc]; ent != NULL; ent = ent->cache_next)
    {
      if (ent->cache_xid == su->su_xid &&
	  ent->cache_proc == uc->uc_proc &&
	  ent->cache_vers == uc->uc_vers &&
	  ent->cache_prog == uc->uc_prog &&
	  EQADDR (ent->cache_addr, uc->uc_addr))
	{
	  *replyp = ent->cache_reply;
	  *replylenp = ent->cache_replylen;
	  return 1;
	}
    }
  /*
   * Failed to find entry
   * Remember a few things so we can do a set later
   */
  uc->uc_proc = msg->rm_call.cb_proc;
  uc->uc_vers = msg->rm_call.cb_vers;
  uc->uc_prog = msg->rm_call.cb_prog;
  memcpy (&uc->uc_addr, &xprt->xp_raddr, sizeof (uc->uc_addr));
  return 0;
}
