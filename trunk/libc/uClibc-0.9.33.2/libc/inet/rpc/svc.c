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
 * svc.c, Server-side remote procedure call interface.
 *
 * There are two sets of procedures here.  The xprt routines are
 * for handling transport handles.  The svc routines handle the
 * list of service routines.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#define __FORCE_GLIBC
#include <features.h>

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "rpc_private.h"
#include <rpc/svc.h>
#include <rpc/pmap_clnt.h>
#include <sys/poll.h>

/* used by svc_[max_]pollfd */
/* used by svc_fdset */

#ifdef __UCLIBC_HAS_THREADS__
#define xports (*(SVCXPRT ***)&RPC_THREAD_VARIABLE(svc_xports_s))
#else
static SVCXPRT **xports;
#endif

#define NULL_SVC ((struct svc_callout *)0)
#define	RQCRED_SIZE	400	/* this size is excessive */

/* The services list
   Each entry represents a set of procedures (an rpc program).
   The dispatch routine takes request structs and runs the
   appropriate procedure. */
struct svc_callout {
  struct svc_callout *sc_next;
  rpcprog_t sc_prog;
  rpcvers_t sc_vers;
  void (*sc_dispatch) (struct svc_req *, SVCXPRT *);
};
#ifdef __UCLIBC_HAS_THREADS__
#define svc_head (*(struct svc_callout **)&RPC_THREAD_VARIABLE(svc_head_s))
#else
static struct svc_callout *svc_head;
#endif

/* ***************  SVCXPRT related stuff **************** */

/* Activate a transport handle. */
void
xprt_register (SVCXPRT *xprt)
{
  register int sock = xprt->xp_sock;
  register int i;

  if (xports == NULL)
    {
      xports = (SVCXPRT **) malloc (_rpc_dtablesize () * sizeof (SVCXPRT *));
      if (xports == NULL) /* Don´t add handle */
	return;
    }

  if (sock < _rpc_dtablesize ())
    {
      xports[sock] = xprt;
      if (sock < FD_SETSIZE)
	FD_SET (sock, &svc_fdset);

      /* Check if we have an empty slot */
      for (i = 0; i < svc_max_pollfd; ++i)
	if (svc_pollfd[i].fd == -1)
	  {
	    svc_pollfd[i].fd = sock;
	    svc_pollfd[i].events = (POLLIN | POLLPRI |
				    POLLRDNORM | POLLRDBAND);
	    return;
	  }

      ++svc_max_pollfd;
      svc_pollfd = realloc (svc_pollfd,
			    sizeof (struct pollfd) * svc_max_pollfd);
      if (svc_pollfd == NULL) /* Out of memory */
	return;

      svc_pollfd[svc_max_pollfd - 1].fd = sock;
      svc_pollfd[svc_max_pollfd - 1].events = (POLLIN | POLLPRI |
					       POLLRDNORM | POLLRDBAND);
    }
}
libc_hidden_def(xprt_register)

/* De-activate a transport handle. */
void
xprt_unregister (SVCXPRT *xprt)
{
  register int sock = xprt->xp_sock;
  register int i;

  if ((sock < _rpc_dtablesize ()) && (xports[sock] == xprt))
    {
      xports[sock] = (SVCXPRT *) 0;

      if (sock < FD_SETSIZE)
	FD_CLR (sock, &svc_fdset);

      for (i = 0; i < svc_max_pollfd; ++i)
	if (svc_pollfd[i].fd == sock)
	  svc_pollfd[i].fd = -1;
    }
}
libc_hidden_def(xprt_unregister)


/* ********************** CALLOUT list related stuff ************* */

/* Search the callout list for a program number, return the callout
   struct. */
static struct svc_callout *
svc_find (rpcprog_t prog, rpcvers_t vers, struct svc_callout **prev)
{
  register struct svc_callout *s, *p;

  p = NULL_SVC;
  for (s = svc_head; s != NULL_SVC; s = s->sc_next)
    {
      if ((s->sc_prog == prog) && (s->sc_vers == vers))
	goto done;
      p = s;
    }
done:
  *prev = p;
  return s;
}

/* Add a service program to the callout list.
   The dispatch routine will be called when a rpc request for this
   program number comes in. */
bool_t
svc_register (SVCXPRT * xprt, rpcprog_t prog, rpcvers_t vers,
	      void (*dispatch) (struct svc_req *, SVCXPRT *),
	      rpcproc_t protocol)
{
  struct svc_callout *prev;
  register struct svc_callout *s;

  if ((s = svc_find (prog, vers, &prev)) != NULL_SVC)
    {
      if (s->sc_dispatch == dispatch)
	goto pmap_it;		/* he is registering another xptr */
      return FALSE;
    }
  s = (struct svc_callout *) mem_alloc (sizeof (struct svc_callout));
  if (s == (struct svc_callout *) 0)
    return FALSE;

  s->sc_prog = prog;
  s->sc_vers = vers;
  s->sc_dispatch = dispatch;
  s->sc_next = svc_head;
  svc_head = s;

pmap_it:
  /* now register the information with the local binder service */
  if (protocol)
    return pmap_set (prog, vers, protocol, xprt->xp_port);

  return TRUE;
}
libc_hidden_def(svc_register)

/* Remove a service program from the callout list. */
void
svc_unregister (rpcprog_t prog, rpcvers_t vers)
{
  struct svc_callout *prev;
  register struct svc_callout *s;

  if ((s = svc_find (prog, vers, &prev)) == NULL_SVC)
    return;

  if (prev == NULL_SVC)
    svc_head = s->sc_next;
  else
    prev->sc_next = s->sc_next;

  s->sc_next = NULL_SVC;
  mem_free ((char *) s, (u_int) sizeof (struct svc_callout));
  /* now unregister the information with the local binder service */
  pmap_unset (prog, vers);
}
libc_hidden_def(svc_unregister)

/* ******************* REPLY GENERATION ROUTINES  ************ */

/* Send a reply to an rpc request */
bool_t
svc_sendreply (register SVCXPRT *xprt, xdrproc_t xdr_results,
	       caddr_t xdr_location)
{
  struct rpc_msg rply;

  rply.rm_direction = REPLY;
  rply.rm_reply.rp_stat = MSG_ACCEPTED;
  rply.acpted_rply.ar_verf = xprt->xp_verf;
  rply.acpted_rply.ar_stat = SUCCESS;
  rply.acpted_rply.ar_results.where = xdr_location;
  rply.acpted_rply.ar_results.proc = xdr_results;
  return SVC_REPLY (xprt, &rply);
}
libc_hidden_def(svc_sendreply)

/* No procedure error reply */
void
svcerr_noproc (register SVCXPRT *xprt)
{
  struct rpc_msg rply;

  rply.rm_direction = REPLY;
  rply.rm_reply.rp_stat = MSG_ACCEPTED;
  rply.acpted_rply.ar_verf = xprt->xp_verf;
  rply.acpted_rply.ar_stat = PROC_UNAVAIL;
  SVC_REPLY (xprt, &rply);
}

/* Can't decode args error reply */
void
svcerr_decode (register SVCXPRT *xprt)
{
  struct rpc_msg rply;

  rply.rm_direction = REPLY;
  rply.rm_reply.rp_stat = MSG_ACCEPTED;
  rply.acpted_rply.ar_verf = xprt->xp_verf;
  rply.acpted_rply.ar_stat = GARBAGE_ARGS;
  SVC_REPLY (xprt, &rply);
}
libc_hidden_def(svcerr_decode)

/* Some system error */
void
svcerr_systemerr (register SVCXPRT *xprt)
{
  struct rpc_msg rply;

  rply.rm_direction = REPLY;
  rply.rm_reply.rp_stat = MSG_ACCEPTED;
  rply.acpted_rply.ar_verf = xprt->xp_verf;
  rply.acpted_rply.ar_stat = SYSTEM_ERR;
  SVC_REPLY (xprt, &rply);
}

/* Authentication error reply */
void
svcerr_auth (SVCXPRT *xprt, enum auth_stat why)
{
  struct rpc_msg rply;

  rply.rm_direction = REPLY;
  rply.rm_reply.rp_stat = MSG_DENIED;
  rply.rjcted_rply.rj_stat = AUTH_ERROR;
  rply.rjcted_rply.rj_why = why;
  SVC_REPLY (xprt, &rply);
}
libc_hidden_def(svcerr_auth)

/* Auth too weak error reply */
void
svcerr_weakauth (SVCXPRT *xprt)
{
  svcerr_auth (xprt, AUTH_TOOWEAK);
}

/* Program unavailable error reply */
void
svcerr_noprog (register SVCXPRT *xprt)
{
  struct rpc_msg rply;

  rply.rm_direction = REPLY;
  rply.rm_reply.rp_stat = MSG_ACCEPTED;
  rply.acpted_rply.ar_verf = xprt->xp_verf;
  rply.acpted_rply.ar_stat = PROG_UNAVAIL;
  SVC_REPLY (xprt, &rply);
}
libc_hidden_def(svcerr_noprog)

/* Program version mismatch error reply */
void
svcerr_progvers (register SVCXPRT *xprt, rpcvers_t low_vers,
		 rpcvers_t high_vers)
{
  struct rpc_msg rply;

  rply.rm_direction = REPLY;
  rply.rm_reply.rp_stat = MSG_ACCEPTED;
  rply.acpted_rply.ar_verf = xprt->xp_verf;
  rply.acpted_rply.ar_stat = PROG_MISMATCH;
  rply.acpted_rply.ar_vers.low = low_vers;
  rply.acpted_rply.ar_vers.high = high_vers;
  SVC_REPLY (xprt, &rply);
}
libc_hidden_def(svcerr_progvers)

/* ******************* SERVER INPUT STUFF ******************* */

/*
 * Get server side input from some transport.
 *
 * Statement of authentication parameters management:
 * This function owns and manages all authentication parameters, specifically
 * the "raw" parameters (msg.rm_call.cb_cred and msg.rm_call.cb_verf) and
 * the "cooked" credentials (rqst->rq_clntcred).
 * However, this function does not know the structure of the cooked
 * credentials, so it make the following assumptions:
 *   a) the structure is contiguous (no pointers), and
 *   b) the cred structure size does not exceed RQCRED_SIZE bytes.
 * In all events, all three parameters are freed upon exit from this routine.
 * The storage is trivially management on the call stack in user land, but
 * is mallocated in kernel land.
 */

void
svc_getreq_common (const int fd)
{
  enum xprt_stat stat;
  struct rpc_msg msg;
  register SVCXPRT *xprt;
  char cred_area[2 * MAX_AUTH_BYTES + RQCRED_SIZE];
  msg.rm_call.cb_cred.oa_base = cred_area;
  msg.rm_call.cb_verf.oa_base = &(cred_area[MAX_AUTH_BYTES]);

  xprt = xports[fd];
  /* Do we control fd? */
  if (xprt == NULL)
     return;

  /* now receive msgs from xprtprt (support batch calls) */
  do
    {
      if (SVC_RECV (xprt, &msg))
	{
	  /* now find the exported program and call it */
	  struct svc_callout *s;
	  struct svc_req r;
	  enum auth_stat why;
	  rpcvers_t low_vers;
	  rpcvers_t high_vers;
	  int prog_found;

	  r.rq_clntcred = &(cred_area[2 * MAX_AUTH_BYTES]);
	  r.rq_xprt = xprt;
	  r.rq_prog = msg.rm_call.cb_prog;
	  r.rq_vers = msg.rm_call.cb_vers;
	  r.rq_proc = msg.rm_call.cb_proc;
	  r.rq_cred = msg.rm_call.cb_cred;

	  /* first authenticate the message */
	  /* Check for null flavor and bypass these calls if possible */

	  if (msg.rm_call.cb_cred.oa_flavor == AUTH_NULL)
	    {
	      r.rq_xprt->xp_verf.oa_flavor = _null_auth.oa_flavor;
	      r.rq_xprt->xp_verf.oa_length = 0;
	    }
	  else if ((why = _authenticate (&r, &msg)) != AUTH_OK)
	    {
	      svcerr_auth (xprt, why);
	      goto call_done;
	    }

	  /* now match message with a registered service */
	  prog_found = FALSE;
	  low_vers = 0 - 1;
	  high_vers = 0;

	  for (s = svc_head; s != NULL_SVC; s = s->sc_next)
	    {
	      if (s->sc_prog == r.rq_prog)
		{
		  if (s->sc_vers == r.rq_vers)
		    {
		      (*s->sc_dispatch) (&r, xprt);
		      goto call_done;
		    }
		  /* found correct version */
		  prog_found = TRUE;
		  if (s->sc_vers < low_vers)
		    low_vers = s->sc_vers;
		  if (s->sc_vers > high_vers)
		    high_vers = s->sc_vers;
		}
	      /* found correct program */
	    }
	  /* if we got here, the program or version
	     is not served ... */
	  if (prog_found)
	    svcerr_progvers (xprt, low_vers, high_vers);
	  else
	    svcerr_noprog (xprt);
	  /* Fall through to ... */
	}
    call_done:
      if ((stat = SVC_STAT (xprt)) == XPRT_DIED)
	{
	  SVC_DESTROY (xprt);
	  break;
	}
    }
  while (stat == XPRT_MOREREQS);
}
libc_hidden_def(svc_getreq_common)

void
svc_getreqset (fd_set *readfds)
{
  register u_int32_t mask;
  register u_int32_t *maskp;
  register int setsize;
  register int sock;
  register int bit;

  setsize = _rpc_dtablesize ();
  maskp = (u_int32_t *) readfds->fds_bits;
  for (sock = 0; sock < setsize; sock += 32)
    for (mask = *maskp++; (bit = ffs (mask)); mask ^= (1 << (bit - 1)))
      svc_getreq_common (sock + bit - 1);
}
libc_hidden_def(svc_getreqset)

void
svc_getreq (int rdfds)
{
  fd_set readfds;

  FD_ZERO (&readfds);
  readfds.fds_bits[0] = rdfds;
  svc_getreqset (&readfds);
}
libc_hidden_def(svc_getreq)

void
svc_getreq_poll (struct pollfd *pfdp, int pollretval)
{
  register int i;
  register int fds_found;

  for (i = fds_found = 0; i < svc_max_pollfd && fds_found < pollretval; ++i)
    {
      register struct pollfd *p = &pfdp[i];

      if (p->fd != -1 && p->revents)
	{
	  /* fd has input waiting */
	  ++fds_found;

	  if (p->revents & POLLNVAL)
	    xprt_unregister (xports[p->fd]);
	  else
	    svc_getreq_common (p->fd);
	}
    }
}
libc_hidden_def(svc_getreq_poll)

#ifdef __UCLIBC_HAS_THREADS__

void attribute_hidden __rpc_thread_svc_cleanup (void)
{
  struct svc_callout *svcp;

  while ((svcp = svc_head) != NULL)
    svc_unregister (svcp->sc_prog, svcp->sc_vers);
}

#endif /* __UCLIBC_HAS_THREADS__ */
