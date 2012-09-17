/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#define __FORCE_GLIBC
#include <features.h>
#include <stdio.h>
#include <assert.h>
#include "rpc_private.h"


#ifdef __UCLIBC_HAS_THREADS__

#include <bits/libc-tsd.h>
#include <bits/libc-lock.h>

/* Variable used in non-threaded applications or for the first thread.  */
static struct rpc_thread_variables __libc_tsd_RPC_VARS_mem;
__libc_tsd_define (, RPC_VARS)

/*
 * Task-variable destructor
 */
void
__rpc_thread_destroy (void)
{
	struct rpc_thread_variables *tvp = __libc_tsd_get (RPC_VARS);

	if (tvp != NULL && tvp != &__libc_tsd_RPC_VARS_mem) {
		__rpc_thread_svc_cleanup ();
		__rpc_thread_clnt_cleanup ();
		/*__rpc_thread_key_cleanup (); */
		free (tvp->authnone_private_s);
		free (tvp->clnt_perr_buf_s);
		free (tvp->clntraw_private_s);
		free (tvp->svcraw_private_s);
		free (tvp->authdes_cache_s);
		free (tvp->authdes_lru_s);
		free (tvp);
		__libc_tsd_set (RPC_VARS, NULL);
	}
}

/*
 * Initialize RPC multi-threaded operation
 */
static void
rpc_thread_multi (void)
{
  __libc_tsd_set (RPC_VARS, &__libc_tsd_RPC_VARS_mem);
}


struct rpc_thread_variables attribute_hidden *
__rpc_thread_variables (void)
{
	__libc_once_define (static, once);
	struct rpc_thread_variables *tvp;

	tvp = __libc_tsd_get (RPC_VARS);
	if (tvp == NULL) {
		__libc_once (once, rpc_thread_multi);
		tvp = __libc_tsd_get (RPC_VARS);
		if (tvp == NULL) {
			tvp = calloc (1, sizeof *tvp);
			if (tvp != NULL)
				__libc_tsd_set (RPC_VARS, tvp);
			else
				tvp = __libc_tsd_get (RPC_VARS);
		}
	}
	return tvp;
}


/* Global variables If we're single-threaded, or if this is the first
   thread using the variable, use the existing global variable.  This
   provides backwards compatability for existing applications which
   dynamically link against this code.  */
#undef svc_fdset
#undef rpc_createerr
#undef svc_pollfd
#undef svc_max_pollfd

fd_set *
__rpc_thread_svc_fdset (void)
{
	struct rpc_thread_variables *tvp;

	tvp = __rpc_thread_variables ();
	if (tvp == &__libc_tsd_RPC_VARS_mem)
		return &svc_fdset;
	return &tvp->svc_fdset_s;
}

struct rpc_createerr *
__rpc_thread_createerr (void)
{
	struct rpc_thread_variables *tvp;

	tvp = __rpc_thread_variables ();
	if (tvp == &__libc_tsd_RPC_VARS_mem)
		return &rpc_createerr;
	return &tvp->rpc_createerr_s;
}

struct pollfd **
__rpc_thread_svc_pollfd (void)
{
	struct rpc_thread_variables *tvp;

	tvp = __rpc_thread_variables ();
	if (tvp == &__libc_tsd_RPC_VARS_mem)
		return &svc_pollfd;
	return &tvp->svc_pollfd_s;
}

int *
__rpc_thread_svc_max_pollfd (void)
{
	struct rpc_thread_variables *tvp;

	tvp = __rpc_thread_variables ();
	if (tvp == &__libc_tsd_RPC_VARS_mem)
		return &svc_max_pollfd;
	return &tvp->svc_max_pollfd_s;
}
#else

#undef svc_fdset
#undef rpc_createerr
#undef svc_pollfd
#undef svc_max_pollfd

extern fd_set svc_fdset;
fd_set * __rpc_thread_svc_fdset (void)
{
    return &(svc_fdset);
}

extern struct rpc_createerr rpc_createerr;
struct rpc_createerr * __rpc_thread_createerr (void)
{
    return &(rpc_createerr);
}

extern struct pollfd *svc_pollfd;
struct pollfd ** __rpc_thread_svc_pollfd (void)
{
    return &(svc_pollfd);
}

extern int svc_max_pollfd;
int * __rpc_thread_svc_max_pollfd (void)
{
    return &(svc_max_pollfd);
}

#endif /* __UCLIBC_HAS_THREADS__ */

libc_hidden_def(__rpc_thread_svc_fdset)
libc_hidden_def(__rpc_thread_createerr)
libc_hidden_def(__rpc_thread_svc_pollfd)
libc_hidden_def(__rpc_thread_svc_max_pollfd)
