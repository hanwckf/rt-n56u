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
 * svc_unix.c, Server side for TCP/IP based RPC.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 *
 * Actually implements two flavors of transporter -
 * a unix rendezvouser (a listener and connection establisher)
 * and a record/unix stream.
 */

#define __FORCE_GLIBC
#include <features.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <rpc/rpc.h>
#include <rpc/svc.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/poll.h>
#include <errno.h>
#include <stdlib.h>

#ifdef USE_IN_LIBIO
# include <wchar.h>
#endif


/*
 * Ops vector for AF_UNIX based rpc service handle
 */
static bool_t svcunix_recv (SVCXPRT *, struct rpc_msg *);
static enum xprt_stat svcunix_stat (SVCXPRT *);
static bool_t svcunix_getargs (SVCXPRT *, xdrproc_t, caddr_t);
static bool_t svcunix_reply (SVCXPRT *, struct rpc_msg *);
static bool_t svcunix_freeargs (SVCXPRT *, xdrproc_t, caddr_t);
static void svcunix_destroy (SVCXPRT *);

static const struct xp_ops svcunix_op =
{
  svcunix_recv,
  svcunix_stat,
  svcunix_getargs,
  svcunix_reply,
  svcunix_freeargs,
  svcunix_destroy
};

/*
 * Ops vector for AF_UNIX rendezvous handler
 */
static bool_t rendezvous_request (SVCXPRT *, struct rpc_msg *);
static enum xprt_stat rendezvous_stat (SVCXPRT *);
static void svcunix_rendezvous_abort (void) attribute_noreturn;

/* This function makes sure abort() relocation goes through PLT
   and thus can be lazy bound.  */
static void
svcunix_rendezvous_abort (void)
{
  abort ();
};

static const struct xp_ops svcunix_rendezvous_op =
{
  rendezvous_request,
  rendezvous_stat,
  (bool_t (*) (SVCXPRT *, xdrproc_t, caddr_t)) svcunix_rendezvous_abort,
  (bool_t (*) (SVCXPRT *, struct rpc_msg *)) svcunix_rendezvous_abort,
  (bool_t (*) (SVCXPRT *, xdrproc_t, caddr_t)) svcunix_rendezvous_abort,
  svcunix_destroy
};

static int readunix (char*, char *, int);
static int writeunix (char *, char *, int);
static SVCXPRT *makefd_xprt (int, u_int, u_int) internal_function;

struct unix_rendezvous {        /* kept in xprt->xp_p1 */
  u_int sendsize;
  u_int recvsize;
};

struct unix_conn {		/* kept in xprt->xp_p1 */
  enum xprt_stat strm_stat;
  u_long x_id;
  XDR xdrs;
  char verf_body[MAX_AUTH_BYTES];
};

/*
 * Usage:
 *      xprt = svcunix_create(sock, send_buf_size, recv_buf_size);
 *
 * Creates, registers, and returns a (rpc) unix based transporter.
 * Once *xprt is initialized, it is registered as a transporter
 * see (svc.h, xprt_register).  This routine returns
 * a NULL if a problem occurred.
 *
 * If sock<0 then a socket is created, else sock is used.
 * If the socket, sock is not bound to a port then svcunix_create
 * binds it to an arbitrary port.  The routine then starts a unix
 * listener on the socket's associated port.  In any (successful) case,
 * xprt->xp_sock is the registered socket number and xprt->xp_port is the
 * associated port number.
 *
 * Since unix streams do buffered io similar to stdio, the caller can specify
 * how big the send and receive buffers are via the second and third parms;
 * 0 => use the system default.
 */
SVCXPRT *
svcunix_create (int sock, u_int sendsize, u_int recvsize, char *path)
{
  bool_t madesock = FALSE;
  SVCXPRT *xprt;
  struct unix_rendezvous *r;
  struct sockaddr_un addr;
  socklen_t len = sizeof (struct sockaddr_in);

  if (sock == RPC_ANYSOCK)
    {
      if ((sock = socket (AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
	  perror (_("svc_unix.c - AF_UNIX socket creation problem"));
	  return (SVCXPRT *) NULL;
	}
      madesock = TRUE;
    }
  memset (&addr, '\0', sizeof (addr));
  addr.sun_family = AF_UNIX;
  len = strlen (path) + 1;
  memcpy (addr.sun_path, path, len);
  len += sizeof (addr.sun_family);

  bind (sock, (struct sockaddr *) &addr, len);

  if (getsockname (sock, (struct sockaddr *) &addr, &len) != 0
      || listen (sock, 2) != 0)
    {
      perror (_("svc_unix.c - cannot getsockname or listen"));
      if (madesock)
	close (sock);
      return (SVCXPRT *) NULL;
    }

  r = (struct unix_rendezvous *) mem_alloc (sizeof (*r));
  xprt = (SVCXPRT *) mem_alloc (sizeof (SVCXPRT));
  if (r == NULL || xprt == NULL)
    {
#ifdef USE_IN_LIBIO
      if (_IO_fwide (stderr, 0) > 0)
	__fwprintf (stderr, L"%s", _("svcunix_create: out of memory\n"));
      else
#endif
	fputs (_("svcunix_create: out of memory\n"), stderr);
      mem_free (r, sizeof (*r));
      mem_free (xprt, sizeof (SVCXPRT));
      return NULL;
    }
  r->sendsize = sendsize;
  r->recvsize = recvsize;
  xprt->xp_p2 = NULL;
  xprt->xp_p1 = (caddr_t) r;
  xprt->xp_verf = _null_auth;
  xprt->xp_ops = &svcunix_rendezvous_op;
  xprt->xp_port = -1;
  xprt->xp_sock = sock;
  xprt_register (xprt);
  return xprt;
}

/*
 * Like svunix_create(), except the routine takes any *open* UNIX file
 * descriptor as its first input.
 */
SVCXPRT *
svcunixfd_create (int fd, u_int sendsize, u_int recvsize);
SVCXPRT *
svcunixfd_create (int fd, u_int sendsize, u_int recvsize)
{
  return makefd_xprt (fd, sendsize, recvsize);
}

static SVCXPRT *
internal_function
makefd_xprt (int fd, u_int sendsize, u_int recvsize)
{
  SVCXPRT *xprt;
  struct unix_conn *cd;

  xprt = (SVCXPRT *) mem_alloc (sizeof (SVCXPRT));
  cd = (struct unix_conn *) mem_alloc (sizeof (struct unix_conn));
  if (xprt == (SVCXPRT *) NULL || cd == (struct unix_conn *) NULL)
    {
#ifdef USE_IN_LIBIO
      if (_IO_fwide (stderr, 0) > 0)
	(void) __fwprintf (stderr, L"%s",
			   _("svc_unix: makefd_xprt: out of memory\n"));
      else
#endif
	(void) fputs (_("svc_unix: makefd_xprt: out of memory\n"), stderr);
      mem_free (xprt, sizeof (SVCXPRT));
      mem_free (cd, sizeof (struct unix_conn));
      return NULL;
    }
  cd->strm_stat = XPRT_IDLE;
  xdrrec_create (&(cd->xdrs), sendsize, recvsize,
		 (caddr_t) xprt, readunix, writeunix);
  xprt->xp_p2 = NULL;
  xprt->xp_p1 = (caddr_t) cd;
  xprt->xp_verf.oa_base = cd->verf_body;
  xprt->xp_addrlen = 0;
  xprt->xp_ops = &svcunix_op;	/* truly deals with calls */
  xprt->xp_port = 0;		/* this is a connection, not a rendezvouser */
  xprt->xp_sock = fd;
  xprt_register (xprt);
  return xprt;
}

static bool_t
rendezvous_request (SVCXPRT *xprt, struct rpc_msg *errmsg attribute_unused)
{
  int sock;
  struct unix_rendezvous *r;
  struct sockaddr_un addr;
  struct sockaddr_in in_addr;
  socklen_t len;

  r = (struct unix_rendezvous *) xprt->xp_p1;
again:
  len = sizeof (struct sockaddr_un);
  if ((sock = accept (xprt->xp_sock, (struct sockaddr *) &addr, &len)) < 0)
    {
      if (errno == EINTR)
	goto again;
      return FALSE;
    }
  /*
   * make a new transporter (re-uses xprt)
   */
  memset (&in_addr, '\0', sizeof (in_addr));
  in_addr.sin_family = AF_UNIX;
  xprt = makefd_xprt (sock, r->sendsize, r->recvsize);
  memcpy (&xprt->xp_raddr, &in_addr, sizeof (in_addr));
  xprt->xp_addrlen = len;
  return FALSE;		/* there is never an rpc msg to be processed */
}

static enum xprt_stat
rendezvous_stat (SVCXPRT *xprt attribute_unused)
{
  return XPRT_IDLE;
}

static void
svcunix_destroy (SVCXPRT *xprt)
{
  struct unix_conn *cd = (struct unix_conn *) xprt->xp_p1;

  xprt_unregister (xprt);
  close (xprt->xp_sock);
  if (xprt->xp_port != 0)
    {
      /* a rendezvouser socket */
      xprt->xp_port = 0;
    }
  else
    {
      /* an actual connection socket */
      XDR_DESTROY (&(cd->xdrs));
    }
  mem_free ((caddr_t) cd, sizeof (struct unix_conn));
  mem_free ((caddr_t) xprt, sizeof (SVCXPRT));
}

#ifdef SCM_CREDENTIALS
struct cmessage {
  struct cmsghdr cmsg;
  struct ucred cmcred;
  /* hack to make sure we have enough memory */
  char dummy[(CMSG_ALIGN (sizeof (struct ucred)) - sizeof (struct ucred) + sizeof (long))];
};

/* XXX This is not thread safe, but since the main functions in svc.c
   and the rpcgen generated *_svc functions for the daemon are also not
   thread safe and uses static global variables, it doesn't matter. */
static struct cmessage cm;
#endif

static int
__msgread (int sock, void *data, size_t cnt)
{
  struct iovec iov;
  struct msghdr msg;
  int len;

  iov.iov_base = data;
  iov.iov_len = cnt;

  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_name = NULL;
  msg.msg_namelen = 0;
#ifdef SCM_CREDENTIALS
  msg.msg_control = (caddr_t) &cm;
  msg.msg_controllen = sizeof (struct cmessage);
#endif
  msg.msg_flags = 0;

#ifdef SO_PASSCRED
  {
    int on = 1;
    if (setsockopt (sock, SOL_SOCKET, SO_PASSCRED, &on, sizeof (on)))
      return -1;
  }
#endif

 restart:
  len = recvmsg (sock, &msg, 0);
  if (len >= 0)
    {
      if (msg.msg_flags & MSG_CTRUNC || len == 0)
        return 0;
      else
        return len;
    }
  if (errno == EINTR)
    goto restart;
  return -1;
}

static int
__msgwrite (int sock, void *data, size_t cnt)
{
#ifndef SCM_CREDENTIALS
  /* We cannot implement this reliably.  */
  __set_errno (ENOSYS);
  return -1;
#else
  struct iovec iov;
  struct msghdr msg;
  struct cmsghdr *cmsg = &cm.cmsg;
  struct ucred cred;
  int len;

  /* XXX I'm not sure, if gete?id() is always correct, or if we should use
     get?id(). But since keyserv needs geteuid(), we have no other chance.
     It would be much better, if the kernel could pass both to the server. */
  cred.pid = getpid ();
  cred.uid = geteuid ();
  cred.gid = getegid ();

  memcpy (CMSG_DATA(cmsg), &cred, sizeof (struct ucred));
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_CREDENTIALS;
  cmsg->cmsg_len = sizeof(*cmsg) + sizeof(struct ucred);

  iov.iov_base = data;
  iov.iov_len = cnt;

  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_control = cmsg;
  msg.msg_controllen = CMSG_ALIGN(cmsg->cmsg_len);
  msg.msg_flags = 0;

 restart:
  len = sendmsg (sock, &msg, 0);
  if (len >= 0)
    return len;
  if (errno == EINTR)
    goto restart;
  return -1;

#endif
}

/*
 * reads data from the unix connection.
 * any error is fatal and the connection is closed.
 * (And a read of zero bytes is a half closed stream => error.)
 */
static int
readunix (char *xprtptr, char *buf, int len)
{
  SVCXPRT *xprt = (SVCXPRT *) xprtptr;
  int sock = xprt->xp_sock;
  int milliseconds = 35 * 1000;
  struct pollfd pollfd;

  do
    {
      pollfd.fd = sock;
      pollfd.events = POLLIN;
      switch (poll (&pollfd, 1, milliseconds))
	{
	case -1:
	  if (errno == EINTR)
	    continue;
	  /*FALLTHROUGH*/
	case 0:
	  goto fatal_err;
	default:
	  if ((pollfd.revents & POLLERR) || (pollfd.revents & POLLHUP)
	      || (pollfd.revents & POLLNVAL))
	    goto fatal_err;
	  break;
	}
    }
  while ((pollfd.revents & POLLIN) == 0);

  if ((len = __msgread (sock, buf, len)) > 0)
    return len;

 fatal_err:
  ((struct unix_conn *) (xprt->xp_p1))->strm_stat = XPRT_DIED;
  return -1;
}

/*
 * writes data to the unix connection.
 * Any error is fatal and the connection is closed.
 */
static int
writeunix (char *xprtptr, char * buf, int len)
{
  SVCXPRT *xprt = (SVCXPRT *) xprtptr;
  int i, cnt;

  for (cnt = len; cnt > 0; cnt -= i, buf += i)
    {
      if ((i = __msgwrite (xprt->xp_sock, buf, cnt)) < 0)
	{
	  ((struct unix_conn *) (xprt->xp_p1))->strm_stat = XPRT_DIED;
	  return -1;
	}
    }
  return len;
}

static enum xprt_stat
svcunix_stat (SVCXPRT *xprt)
{
  struct unix_conn *cd =
  (struct unix_conn *) (xprt->xp_p1);

  if (cd->strm_stat == XPRT_DIED)
    return XPRT_DIED;
  if (!xdrrec_eof (&(cd->xdrs)))
    return XPRT_MOREREQS;
  return XPRT_IDLE;
}

static bool_t
svcunix_recv (SVCXPRT *xprt, struct rpc_msg *msg)
{
  struct unix_conn *cd = (struct unix_conn *) (xprt->xp_p1);
  XDR *xdrs = &(cd->xdrs);

  xdrs->x_op = XDR_DECODE;
  xdrrec_skiprecord (xdrs);
  if (xdr_callmsg (xdrs, msg))
    {
      cd->x_id = msg->rm_xid;
      /* set up verifiers */
#ifdef SCM_CREDENTIALS
      msg->rm_call.cb_verf.oa_flavor = AUTH_UNIX;
      msg->rm_call.cb_verf.oa_base = (caddr_t) &cm;
      msg->rm_call.cb_verf.oa_length = sizeof (cm);
#endif
      return TRUE;
    }
  cd->strm_stat = XPRT_DIED;	/* XXXX */
  return FALSE;
}

static bool_t
svcunix_getargs (SVCXPRT *xprt, xdrproc_t xdr_args, caddr_t args_ptr)
{
  return (*xdr_args) (&(((struct unix_conn *) (xprt->xp_p1))->xdrs),
		      args_ptr);
}

static bool_t
svcunix_freeargs (SVCXPRT *xprt, xdrproc_t xdr_args, caddr_t args_ptr)
{
  XDR *xdrs = &(((struct unix_conn *) (xprt->xp_p1))->xdrs);

  xdrs->x_op = XDR_FREE;
  return (*xdr_args) (xdrs, args_ptr);
}

static bool_t
svcunix_reply (SVCXPRT *xprt, struct rpc_msg *msg)
{
  struct unix_conn *cd = (struct unix_conn *) (xprt->xp_p1);
  XDR *xdrs = &(cd->xdrs);
  bool_t stat;

  xdrs->x_op = XDR_ENCODE;
  msg->rm_xid = cd->x_id;
  stat = xdr_replymsg (xdrs, msg);
  (void) xdrrec_endofrecord (xdrs, TRUE);
  return stat;
}
