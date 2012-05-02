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
 * clnt_unix.c, Implements a TCP/IP based, client side RPC.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 *
 * TCP based RPC supports 'batched calls'.
 * A sequence of calls may be batched-up in a send buffer.  The rpc call
 * return immediately to the client even though the call was not necessarily
 * sent.  The batching occurs if the results' xdr routine is NULL (0) AND
 * the rpc timeout value is zero (see clnt.h, rpc).
 *
 * Clients should NOT casually batch calls that in fact return results; that is,
 * the server side should be aware that a call is batched and not produce any
 * return message.  Batched calls that produce many result messages can
 * deadlock (netlock) the client and the server....
 *
 * Now go hang yourself.
 */

#define __FORCE_GLIBC
#include <features.h>

#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <rpc/rpc.h>
#include <sys/uio.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <rpc/pmap_clnt.h>
#ifdef USE_IN_LIBIO
# include <wchar.h>
#endif

extern u_long _create_xid (void);

#define MCALL_MSG_SIZE 24

struct ct_data
  {
    int ct_sock;
    bool_t ct_closeit;
    struct timeval ct_wait;
    bool_t ct_waitset;		/* wait set by clnt_control? */
    struct sockaddr_un ct_addr;
    struct rpc_err ct_error;
    char ct_mcall[MCALL_MSG_SIZE];	/* marshalled callmsg */
    u_int ct_mpos;		/* pos after marshal */
    XDR ct_xdrs;
  };

static int readunix (char *, char *, int);
static int writeunix (char *, char *, int);

static enum clnt_stat clntunix_call (CLIENT *, u_long, xdrproc_t, caddr_t,
				    xdrproc_t, caddr_t, struct timeval);
static void clntunix_abort (void);
static void clntunix_geterr (CLIENT *, struct rpc_err *);
static bool_t clntunix_freeres (CLIENT *, xdrproc_t, caddr_t);
static bool_t clntunix_control (CLIENT *, int, char *);
static void clntunix_destroy (CLIENT *);

static struct clnt_ops unix_ops =
{
  clntunix_call,
  clntunix_abort,
  clntunix_geterr,
  clntunix_freeres,
  clntunix_destroy,
  clntunix_control
};

/*
 * Create a client handle for a tcp/ip connection.
 * If *sockp<0, *sockp is set to a newly created TCP socket and it is
 * connected to raddr.  If *sockp non-negative then
 * raddr is ignored.  The rpc/tcp package does buffering
 * similar to stdio, so the client must pick send and receive buffer sizes,];
 * 0 => use the default.
 * If raddr->sin_port is 0, then a binder on the remote machine is
 * consulted for the right port number.
 * NB: *sockp is copied into a private area.
 * NB: It is the clients responsibility to close *sockp.
 * NB: The rpch->cl_auth is set null authentication.  Caller may wish to set this
 * something more useful.
 */
CLIENT *
clntunix_create (struct sockaddr_un *raddr, u_long prog, u_long vers,
		 int *sockp, u_int sendsz, u_int recvsz)
{
  CLIENT *h;
  struct ct_data *ct = (struct ct_data *) mem_alloc (sizeof (*ct));
  struct rpc_msg call_msg;
  int len;

  h = (CLIENT *) mem_alloc (sizeof (*h));
  if (h == NULL || ct == NULL)
    {
      struct rpc_createerr *ce = &get_rpc_createerr ();
#ifdef USE_IN_LIBIO
      if (_IO_fwide (stderr, 0) > 0)
	(void) __fwprintf (stderr, L"%s",
			   _("clntunix_create: out of memory\n"));
      else
#endif
	(void) fputs (_("clntunix_create: out of memory\n"), stderr);
      ce->cf_stat = RPC_SYSTEMERROR;
      ce->cf_error.re_errno = ENOMEM;
      goto fooy;
    }

  /*
   * If no socket given, open one
   */
  if (*sockp < 0)
    {
      *sockp = socket (AF_UNIX, SOCK_STREAM, 0);
      len = strlen (raddr->sun_path) + sizeof (raddr->sun_family) + 1;
      if (*sockp < 0
	  || connect (*sockp, (struct sockaddr *) raddr, len) < 0)
	{
	  struct rpc_createerr *ce = &get_rpc_createerr ();
	  ce->cf_stat = RPC_SYSTEMERROR;
	  ce->cf_error.re_errno = errno;
	  if (*sockp != -1)
	    close (*sockp);
	  goto fooy;
	}
      ct->ct_closeit = TRUE;
    }
  else
    {
      ct->ct_closeit = FALSE;
    }

  /*
   * Set up private data struct
   */
  ct->ct_sock = *sockp;
  ct->ct_wait.tv_usec = 0;
  ct->ct_waitset = FALSE;
  ct->ct_addr = *raddr;

  /*
   * Initialize call message
   */
  call_msg.rm_xid = _create_xid ();
  call_msg.rm_direction = CALL;
  call_msg.rm_call.cb_rpcvers = RPC_MSG_VERSION;
  call_msg.rm_call.cb_prog = prog;
  call_msg.rm_call.cb_vers = vers;

  /*
   * pre-serialize the static part of the call msg and stash it away
   */
  xdrmem_create (&(ct->ct_xdrs), ct->ct_mcall, MCALL_MSG_SIZE, XDR_ENCODE);
  if (!xdr_callhdr (&(ct->ct_xdrs), &call_msg))
    {
      if (ct->ct_closeit)
	close (*sockp);
      goto fooy;
    }
  ct->ct_mpos = XDR_GETPOS (&(ct->ct_xdrs));
  XDR_DESTROY (&(ct->ct_xdrs));

  /*
   * Create a client handle which uses xdrrec for serialization
   * and authnone for authentication.
   */
  xdrrec_create (&(ct->ct_xdrs), sendsz, recvsz,
		 (caddr_t) ct, readunix, writeunix);
  h->cl_ops = &unix_ops;
  h->cl_private = (caddr_t) ct;
  h->cl_auth = authnone_create ();
  return h;

fooy:
  /*
   * Something goofed, free stuff and barf
   */
  mem_free ((caddr_t) ct, sizeof (struct ct_data));
  mem_free ((caddr_t) h, sizeof (CLIENT));
  return (CLIENT *) NULL;
}

static enum clnt_stat
clntunix_call (h, proc, xdr_args, args_ptr, xdr_results, results_ptr, timeout)
     CLIENT *h;
     u_long proc;
     xdrproc_t xdr_args;
     caddr_t args_ptr;
     xdrproc_t xdr_results;
     caddr_t results_ptr;
     struct timeval timeout;
{
  struct ct_data *ct = (struct ct_data *) h->cl_private;
  XDR *xdrs = &(ct->ct_xdrs);
  struct rpc_msg reply_msg;
  u_long x_id;
  u_int32_t *msg_x_id = (u_int32_t *) (ct->ct_mcall);	/* yuk */
  bool_t shipnow;
  int refreshes = 2;

  if (!ct->ct_waitset)
    {
      ct->ct_wait = timeout;
    }

  shipnow =
    (xdr_results == (xdrproc_t) 0 && ct->ct_wait.tv_sec == 0
     && ct->ct_wait.tv_usec == 0) ? FALSE : TRUE;

call_again:
  xdrs->x_op = XDR_ENCODE;
  ct->ct_error.re_status = RPC_SUCCESS;
  x_id = ntohl (--(*msg_x_id));
  if ((!XDR_PUTBYTES (xdrs, ct->ct_mcall, ct->ct_mpos)) ||
      (!XDR_PUTLONG (xdrs, (long *) &proc)) ||
      (!AUTH_MARSHALL (h->cl_auth, xdrs)) ||
      (!(*xdr_args) (xdrs, args_ptr)))
    {
      if (ct->ct_error.re_status == RPC_SUCCESS)
	ct->ct_error.re_status = RPC_CANTENCODEARGS;
      (void) xdrrec_endofrecord (xdrs, TRUE);
      return ct->ct_error.re_status;
    }
  if (!xdrrec_endofrecord (xdrs, shipnow))
    return ct->ct_error.re_status = RPC_CANTSEND;
  if (!shipnow)
    return RPC_SUCCESS;
  /*
   * Hack to provide rpc-based message passing
   */
  if (ct->ct_wait.tv_sec == 0 && ct->ct_wait.tv_usec == 0)
    return ct->ct_error.re_status = RPC_TIMEDOUT;


  /*
   * Keep receiving until we get a valid transaction id
   */
  xdrs->x_op = XDR_DECODE;
  while (TRUE)
    {
      reply_msg.acpted_rply.ar_verf = _null_auth;
      reply_msg.acpted_rply.ar_results.where = NULL;
      reply_msg.acpted_rply.ar_results.proc = (xdrproc_t)xdr_void;
      if (!xdrrec_skiprecord (xdrs))
	return ct->ct_error.re_status;
      /* now decode and validate the response header */
      if (!xdr_replymsg (xdrs, &reply_msg))
	{
	  if (ct->ct_error.re_status == RPC_SUCCESS)
	    continue;
	  return ct->ct_error.re_status;
	}
      if (reply_msg.rm_xid == x_id)
	break;
    }

  /*
   * process header
   */
  _seterr_reply (&reply_msg, &(ct->ct_error));
  if (ct->ct_error.re_status == RPC_SUCCESS)
    {
      if (!AUTH_VALIDATE (h->cl_auth, &reply_msg.acpted_rply.ar_verf))
	{
	  ct->ct_error.re_status = RPC_AUTHERROR;
	  ct->ct_error.re_why = AUTH_INVALIDRESP;
	}
      else if (!(*xdr_results) (xdrs, results_ptr))
	{
	  if (ct->ct_error.re_status == RPC_SUCCESS)
	    ct->ct_error.re_status = RPC_CANTDECODERES;
	}
      /* free verifier ... */
      if (reply_msg.acpted_rply.ar_verf.oa_base != NULL)
	{
	  xdrs->x_op = XDR_FREE;
	  (void) xdr_opaque_auth (xdrs, &(reply_msg.acpted_rply.ar_verf));
	}
    }				/* end successful completion */
  else
    {
      /* maybe our credentials need to be refreshed ... */
      if (refreshes-- && AUTH_REFRESH (h->cl_auth))
	goto call_again;
    }				/* end of unsuccessful completion */
  return ct->ct_error.re_status;
}

static void
clntunix_geterr (CLIENT *h, struct rpc_err *errp)
{
  struct ct_data *ct = (struct ct_data *) h->cl_private;

  *errp = ct->ct_error;
}

static bool_t
clntunix_freeres (cl, xdr_res, res_ptr)
     CLIENT *cl;
     xdrproc_t xdr_res;
     caddr_t res_ptr;
{
  struct ct_data *ct = (struct ct_data *) cl->cl_private;
  XDR *xdrs = &(ct->ct_xdrs);

  xdrs->x_op = XDR_FREE;
  return (*xdr_res) (xdrs, res_ptr);
}

static void
clntunix_abort ()
{
}

static bool_t
clntunix_control (CLIENT *cl, int request, char *info)
{
  struct ct_data *ct = (struct ct_data *) cl->cl_private;


  switch (request)
    {
    case CLSET_FD_CLOSE:
      ct->ct_closeit = TRUE;
      break;
    case CLSET_FD_NCLOSE:
      ct->ct_closeit = FALSE;
      break;
    case CLSET_TIMEOUT:
      ct->ct_wait = *(struct timeval *) info;
      break;
    case CLGET_TIMEOUT:
      *(struct timeval *) info = ct->ct_wait;
      break;
    case CLGET_SERVER_ADDR:
      *(struct sockaddr_un *) info = ct->ct_addr;
      break;
    case CLGET_FD:
      *(int *)info = ct->ct_sock;
      break;
    case CLGET_XID:
      /*
       * use the knowledge that xid is the
       * first element in the call structure *.
       * This will get the xid of the PREVIOUS call
       */
      *(u_long *) info = ntohl (*(u_long *)ct->ct_mcall);
      break;
    case CLSET_XID:
      /* This will set the xid of the NEXT call */
      *(u_long *) ct->ct_mcall =  htonl (*(u_long *)info - 1);
      /* decrement by 1 as clntunix_call() increments once */
    case CLGET_VERS:
      /*
       * This RELIES on the information that, in the call body,
       * the version number field is the fifth field from the
       * begining of the RPC header. MUST be changed if the
       * call_struct is changed
       */
      *(u_long *) info = ntohl (*(u_long *) (ct->ct_mcall
					     + 4 * BYTES_PER_XDR_UNIT));
      break;
    case CLSET_VERS:
      *(u_long *) (ct->ct_mcall + 4 * BYTES_PER_XDR_UNIT)
	= htonl (*(u_long *) info);
      break;
    case CLGET_PROG:
      /*
       * This RELIES on the information that, in the call body,
       * the program number field is the  field from the
       * begining of the RPC header. MUST be changed if the
       * call_struct is changed
       */
      *(u_long *) info = ntohl (*(u_long *) (ct->ct_mcall
					     + 3 * BYTES_PER_XDR_UNIT));
      break;
    case CLSET_PROG:
      *(u_long *) (ct->ct_mcall + 3 * BYTES_PER_XDR_UNIT)
	= htonl(*(u_long *) info);
      break;
    /* The following are only possible with TI-RPC */
    case CLGET_RETRY_TIMEOUT:
    case CLSET_RETRY_TIMEOUT:
    case CLGET_SVC_ADDR:
    case CLSET_SVC_ADDR:
    case CLSET_PUSH_TIMOD:
    case CLSET_POP_TIMOD:
    default:
      return FALSE;
    }
  return TRUE;
}


static void
clntunix_destroy (CLIENT *h)
{
  struct ct_data *ct =
  (struct ct_data *) h->cl_private;

  if (ct->ct_closeit)
    {
      (void) close (ct->ct_sock);
    }
  XDR_DESTROY (&(ct->ct_xdrs));
  mem_free ((caddr_t) ct, sizeof (struct ct_data));
  mem_free ((caddr_t) h, sizeof (CLIENT));
}

static int
__msgread (int sock, void *data, size_t cnt)
{
  struct iovec iov;
  struct msghdr msg;
#ifdef SCM_CREDENTIALS
  static char cm[CMSG_SPACE(sizeof (struct ucred))];
#endif
  int len;

  iov.iov_base = data;
  iov.iov_len = cnt;

  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_name = NULL;
  msg.msg_namelen = 0;
#ifdef SCM_CREDENTIALS
  msg.msg_control = (caddr_t) &cm;
  msg.msg_controllen = CMSG_SPACE(sizeof (struct ucred));
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
  struct cmsghdr *cmsg = alloca (CMSG_SPACE(sizeof (struct ucred)));
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
 * Interface between xdr serializer and unix connection.
 * Behaves like the system calls, read & write, but keeps some error state
 * around for the rpc level.
 */
static int
readunix (char *ctptr, char *buf, int len)
{
  struct ct_data *ct = (struct ct_data *) ctptr;
  struct pollfd fd;
  int milliseconds = ((ct->ct_wait.tv_sec * 1000)
		      + (ct->ct_wait.tv_usec / 1000));

  if (len == 0)
    return 0;

  fd.fd = ct->ct_sock;
  fd.events = POLLIN;
  while (TRUE)
    {
      switch (poll (&fd, 1, milliseconds))
        {
        case 0:
          ct->ct_error.re_status = RPC_TIMEDOUT;
          return -1;

        case -1:
          if (errno == EINTR)
            continue;
          ct->ct_error.re_status = RPC_CANTRECV;
          ct->ct_error.re_errno = errno;
          return -1;
        }
      break;
    }
  switch (len = __msgread (ct->ct_sock, buf, len))
    {

    case 0:
      /* premature eof */
      ct->ct_error.re_errno = ECONNRESET;
      ct->ct_error.re_status = RPC_CANTRECV;
      len = -1;			/* it's really an error */
      break;

    case -1:
      ct->ct_error.re_errno = errno;
      ct->ct_error.re_status = RPC_CANTRECV;
      break;
    }
  return len;
}

static int
writeunix (char *ctptr, char *buf, int len)
{
  int i, cnt;
  struct ct_data *ct = (struct ct_data *) ctptr;

  for (cnt = len; cnt > 0; cnt -= i, buf += i)
    {
      if ((i = __msgwrite (ct->ct_sock, buf, cnt)) == -1)
	{
	  ct->ct_error.re_errno = errno;
	  ct->ct_error.re_status = RPC_CANTSEND;
	  return -1;
	}
    }
  return len;
}
