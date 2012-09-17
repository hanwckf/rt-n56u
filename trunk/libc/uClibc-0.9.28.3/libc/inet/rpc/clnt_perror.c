/* @(#)clnt_perror.c	2.1 88/07/29 4.0 RPCSRC */
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
static char sccsid[] = "@(#)clnt_perror.c 1.15 87/10/07 Copyr 1984 Sun Micro";
#endif

/*
 * clnt_perror.c
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 *
 */
#define __FORCE_GLIBC
#include <features.h>

#include <stdio.h>
#include <string.h>
#include "rpc_private.h"

#ifdef USE_IN_LIBIO
# include <wchar.h>
# include <libio/iolibio.h>
# define fputs(s, f) _IO_fputs (s, f)
#endif

static char *auth_errmsg (enum auth_stat stat) internal_function;

#ifdef __UCLIBC_HAS_THREADS__
/*
 * Making buf a preprocessor macro requires renaming the local
 * buf variable in a few functions.  Overriding a global variable
 * with a local variable of the same name is a bad idea, anyway.
 */
#define buf (*(char **)&RPC_THREAD_VARIABLE(clnt_perr_buf_s))
#else
static char *buf;
#endif

static char *
_buf (void)
{
  if (buf == NULL)
    buf = (char *) malloc (256);
  return buf;
}

/*
 * Print reply error info
 */
char *
clnt_sperror (CLIENT * rpch, const char *msg)
{
  char chrbuf[1024];
  struct rpc_err e;
  char *err;
  char *str = _buf ();
  char *strstart = str;
  int len;

  if (str == NULL)
    return NULL;
  CLNT_GETERR (rpch, &e);

  len = sprintf (str, "%s: ", msg);
  str += len;

  (void) strcpy(str, clnt_sperrno(e.re_status));
  str += strlen(str);

  switch (e.re_status)
    {
    case RPC_SUCCESS:
    case RPC_CANTENCODEARGS:
    case RPC_CANTDECODERES:
    case RPC_TIMEDOUT:
    case RPC_PROGUNAVAIL:
    case RPC_PROCUNAVAIL:
    case RPC_CANTDECODEARGS:
    case RPC_SYSTEMERROR:
    case RPC_UNKNOWNHOST:
    case RPC_UNKNOWNPROTO:
    case RPC_PMAPFAILURE:
    case RPC_PROGNOTREGISTERED:
    case RPC_FAILED:
      break;

    case RPC_CANTSEND:
    case RPC_CANTRECV:
      strerror_r (e.re_errno, chrbuf, sizeof chrbuf);
      len = sprintf (str, "; errno = %s", chrbuf); 
      str += len;
      break;

    case RPC_VERSMISMATCH:
      len= sprintf (str, _("; low version = %lu, high version = %lu"),
		    e.re_vers.low, e.re_vers.high);
      str += len;
      break;

    case RPC_AUTHERROR:
      err = auth_errmsg (e.re_why);
      (void) strcpy(str, _("; why = "));
      str += strlen(str);

      if (err != NULL)
	{
	  (void) strcpy(str, err);
	  str += strlen(str);
	}
      else
	{
	  len = sprintf (str, _("(unknown authentication error - %d)"),
			 (int) e.re_why);
	  str += len;
	}
      break;

    case RPC_PROGVERSMISMATCH:
      len = sprintf (str, _("; low version = %lu, high version = %lu"),
		     e.re_vers.low, e.re_vers.high);
      str += len;
      break;

    default:			/* unknown */
      len = sprintf (str, "; s1 = %lu, s2 = %lu", e.re_lb.s1, e.re_lb.s2);
      str += len;
      break;
    }
  *str = '\n';
  *++str = '\0';
  return (strstart);
}

void
clnt_perror (CLIENT * rpch, const char *msg)
{
#ifdef USE_IN_LIBIO
  if (_IO_fwide (stderr, 0) > 0)
    (void) __fwprintf (stderr, L"%s", clnt_sperror (rpch, msg));
  else
#endif
    (void) fputs (clnt_sperror (rpch, msg), stderr);
}


struct rpc_errtab
{
  enum clnt_stat status;
  unsigned int message_off;
};

static const char rpc_errstr[] =
{
#define RPC_SUCCESS_IDX		0
  _("RPC: Success")
  "\0"
#define RPC_CANTENCODEARGS_IDX	(RPC_SUCCESS_IDX + sizeof "RPC: Success")
  _("RPC: Can't encode arguments")
  "\0"
#define RPC_CANTDECODERES_IDX	(RPC_CANTENCODEARGS_IDX \
				 + sizeof "RPC: Can't encode arguments")
  _("RPC: Can't decode result")
  "\0"
#define RPC_CANTSEND_IDX	(RPC_CANTDECODERES_IDX \
				 + sizeof "RPC: Can't decode result")
  _("RPC: Unable to send")
  "\0"
#define RPC_CANTRECV_IDX	(RPC_CANTSEND_IDX \
				 + sizeof "RPC: Unable to send")
  _("RPC: Unable to receive")
  "\0"
#define RPC_TIMEDOUT_IDX	(RPC_CANTRECV_IDX \
				 + sizeof "RPC: Unable to receive")
  _("RPC: Timed out")
  "\0"
#define RPC_VERSMISMATCH_IDX	(RPC_TIMEDOUT_IDX \
				 + sizeof "RPC: Timed out")
  _("RPC: Incompatible versions of RPC")
  "\0"
#define RPC_AUTHERROR_IDX	(RPC_VERSMISMATCH_IDX \
				 + sizeof "RPC: Incompatible versions of RPC")
  _("RPC: Authentication error")
  "\0"
#define RPC_PROGUNAVAIL_IDX		(RPC_AUTHERROR_IDX \
				 + sizeof "RPC: Authentication error")
  _("RPC: Program unavailable")
  "\0"
#define RPC_PROGVERSMISMATCH_IDX (RPC_PROGUNAVAIL_IDX \
				  + sizeof "RPC: Program unavailable")
  _("RPC: Program/version mismatch")
  "\0"
#define RPC_PROCUNAVAIL_IDX	(RPC_PROGVERSMISMATCH_IDX \
				 + sizeof "RPC: Program/version mismatch")
  _("RPC: Procedure unavailable")
  "\0"
#define RPC_CANTDECODEARGS_IDX	(RPC_PROCUNAVAIL_IDX \
				 + sizeof "RPC: Procedure unavailable")
  _("RPC: Server can't decode arguments")
  "\0"
#define RPC_SYSTEMERROR_IDX	(RPC_CANTDECODEARGS_IDX \
				 + sizeof "RPC: Server can't decode arguments")
  _("RPC: Remote system error")
  "\0"
#define RPC_UNKNOWNHOST_IDX	(RPC_SYSTEMERROR_IDX \
				 + sizeof "RPC: Remote system error")
  _("RPC: Unknown host")
  "\0"
#define RPC_UNKNOWNPROTO_IDX	(RPC_UNKNOWNHOST_IDX \
				 + sizeof "RPC: Unknown host")
  _("RPC: Unknown protocol")
  "\0"
#define RPC_PMAPFAILURE_IDX	(RPC_UNKNOWNPROTO_IDX \
				 + sizeof "RPC: Unknown protocol")
  _("RPC: Port mapper failure")
  "\0"
#define RPC_PROGNOTREGISTERED_IDX (RPC_PMAPFAILURE_IDX \
				   + sizeof "RPC: Port mapper failure")
  _("RPC: Program not registered")
  "\0"
#define RPC_FAILED_IDX		(RPC_PROGNOTREGISTERED_IDX \
				 + sizeof "RPC: Program not registered")
  _("RPC: Failed (unspecified error)")
};

static const struct rpc_errtab rpc_errlist[] =
{
  { RPC_SUCCESS, RPC_SUCCESS_IDX },
  { RPC_CANTENCODEARGS, RPC_CANTENCODEARGS_IDX },
  { RPC_CANTDECODERES, RPC_CANTDECODERES_IDX },
  { RPC_CANTSEND, RPC_CANTSEND_IDX },
  { RPC_CANTRECV, RPC_CANTRECV_IDX },
  { RPC_TIMEDOUT, RPC_TIMEDOUT_IDX },
  { RPC_VERSMISMATCH, RPC_VERSMISMATCH_IDX },
  { RPC_AUTHERROR, RPC_AUTHERROR_IDX },
  { RPC_PROGUNAVAIL, RPC_PROGUNAVAIL_IDX },
  { RPC_PROGVERSMISMATCH, RPC_PROGVERSMISMATCH_IDX },
  { RPC_PROCUNAVAIL, RPC_PROCUNAVAIL_IDX },
  { RPC_CANTDECODEARGS, RPC_CANTDECODEARGS_IDX },
  { RPC_SYSTEMERROR, RPC_SYSTEMERROR_IDX },
  { RPC_UNKNOWNHOST, RPC_UNKNOWNHOST_IDX },
  { RPC_UNKNOWNPROTO, RPC_UNKNOWNPROTO_IDX },
  { RPC_PMAPFAILURE, RPC_PMAPFAILURE_IDX },
  { RPC_PROGNOTREGISTERED, RPC_PROGNOTREGISTERED_IDX },
  { RPC_FAILED, RPC_FAILED_IDX }
};


/*
 * This interface for use by clntrpc
 */
char *
clnt_sperrno (enum clnt_stat stat)
{
  size_t i;

  for (i = 0; i < sizeof (rpc_errlist) / sizeof (struct rpc_errtab); i++)
    {
      if (rpc_errlist[i].status == stat)
	{
	  return (char*)_(rpc_errstr + rpc_errlist[i].message_off);
	}
    }
  return _("RPC: (unknown error code)");
}

void
clnt_perrno (enum clnt_stat num)
{
#ifdef USE_IN_LIBIO
  if (_IO_fwide (stderr, 0) > 0)
    (void) __fwprintf (stderr, L"%s", clnt_sperrno (num));
  else
#endif
    (void) fputs (clnt_sperrno (num), stderr);
}


char *
clnt_spcreateerror (const char *msg)
{
  char chrbuf[1024];
  char *str = _buf ();
  char *cp;
  int len;
  struct rpc_createerr *ce;

  if (str == NULL)
    return NULL;
  ce = &get_rpc_createerr ();
  len = sprintf (str, "%s: ", msg);
  cp = str + len;
  (void) strcpy(cp, clnt_sperrno (ce->cf_stat));
  cp += strlen(cp);

  switch (ce->cf_stat)
    {
    case RPC_PMAPFAILURE:
      (void) strcpy(cp, " - ");
      cp += strlen(cp);

      (void) strcpy(cp, clnt_sperrno (ce->cf_error.re_status));
      cp += strlen(cp);

      break;

    case RPC_SYSTEMERROR:
      (void) strcpy(cp, " - ");
      cp += strlen(cp);

      strerror_r (ce->cf_error.re_errno, chrbuf, sizeof chrbuf);
      (void) strcpy(cp, chrbuf);
      cp += strlen(cp);
      break;
    default:
      break;
    }
  *cp = '\n';
  *++cp = '\0';
  return str;
}

void
clnt_pcreateerror (const char *msg)
{
#ifdef USE_IN_LIBIO
  if (_IO_fwide (stderr, 0) > 0)
    (void) __fwprintf (stderr, L"%s", clnt_spcreateerror (msg));
  else
#endif
    (void) fputs (clnt_spcreateerror (msg), stderr);
}

struct auth_errtab
{
  enum auth_stat status;
  unsigned int message_off;
};

static const char auth_errstr[] =
{
#define AUTH_OK_IDX		0
   _("Authentication OK")
   "\0"
#define AUTH_BADCRED_IDX	(AUTH_OK_IDX + sizeof "Authentication OK")
   _("Invalid client credential")
   "\0"
#define AUTH_REJECTEDCRED_IDX	(AUTH_BADCRED_IDX \
				 + sizeof "Invalid client credential")
   _("Server rejected credential")
   "\0"
#define AUTH_BADVERF_IDX	(AUTH_REJECTEDCRED_IDX \
				 + sizeof "Server rejected credential")
   _("Invalid client verifier")
   "\0"
#define AUTH_REJECTEDVERF_IDX	(AUTH_BADVERF_IDX \
				 + sizeof "Invalid client verifier")
   _("Server rejected verifier")
   "\0"
#define AUTH_TOOWEAK_IDX	(AUTH_REJECTEDVERF_IDX \
				 + sizeof "Server rejected verifier")
   _("Client credential too weak")
   "\0"
#define AUTH_INVALIDRESP_IDX	(AUTH_TOOWEAK_IDX \
				 + sizeof "Client credential too weak")
   _("Invalid server verifier")
   "\0"
#define AUTH_FAILED_IDX		(AUTH_INVALIDRESP_IDX \
				 + sizeof "Invalid server verifier")
   _("Failed (unspecified error)")
};

static const struct auth_errtab auth_errlist[] =
{
  { AUTH_OK, AUTH_OK_IDX },
  { AUTH_BADCRED, AUTH_BADCRED_IDX },
  { AUTH_REJECTEDCRED, AUTH_REJECTEDCRED_IDX },
  { AUTH_BADVERF, AUTH_BADVERF_IDX },
  { AUTH_REJECTEDVERF, AUTH_REJECTEDVERF_IDX },
  { AUTH_TOOWEAK, AUTH_TOOWEAK_IDX },
  { AUTH_INVALIDRESP, AUTH_INVALIDRESP_IDX },
  { AUTH_FAILED, AUTH_FAILED_IDX }
};

static char *
internal_function
auth_errmsg (enum auth_stat stat)
{
  size_t i;

  for (i = 0; i < sizeof (auth_errlist) / sizeof (struct auth_errtab); i++)
    {
      if (auth_errlist[i].status == stat)
	{
	  return (char*)_(auth_errstr + auth_errlist[i].message_off);
	}
    }
  return NULL;
}


static void __attribute_used__
free_mem (void)
{
  free (buf);
}
