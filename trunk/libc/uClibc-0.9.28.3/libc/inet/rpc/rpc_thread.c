#define __FORCE_GLIBC
#include <features.h>
#include <stdio.h>
#include <assert.h>
#include <bits/libc-tsd.h>
#include "rpc_private.h"

#ifdef __UCLIBC_HAS_THREADS__

/* Variable used in non-threaded applications or for the first thread.  */
static struct rpc_thread_variables __libc_tsd_RPC_VARS_mem;
static struct rpc_thread_variables *__libc_tsd_RPC_VARS_data =
     &__libc_tsd_RPC_VARS_mem;

/*
 * Task-variable destructor
 */
void
__rpc_thread_destroy (void)
{
	struct rpc_thread_variables *tvp = __rpc_thread_variables();

	if (tvp != NULL && tvp != &__libc_tsd_RPC_VARS_mem) {
		__rpc_thread_svc_cleanup ();
		__rpc_thread_clnt_cleanup ();
		//__rpc_thread_key_cleanup ();
		free (tvp->authnone_private_s);
		free (tvp->clnt_perr_buf_s);
		free (tvp->clntraw_private_s);
		free (tvp->svcraw_private_s);
		free (tvp->authdes_cache_s);
		free (tvp->authdes_lru_s);
		free (tvp);
	}
}


extern int weak_function __pthread_once (pthread_once_t *__once_control,
			   void (*__init_routine) (void));

# define __libc_once_define(CLASS, NAME) \
  CLASS pthread_once_t NAME = PTHREAD_ONCE_INIT

/* Call handler iff the first call.  */
#define __libc_once(ONCE_CONTROL, INIT_FUNCTION) \
  do {									      \
    if (__pthread_once != NULL)						      \
      __pthread_once (&(ONCE_CONTROL), (INIT_FUNCTION));		      \
    else if ((ONCE_CONTROL) == PTHREAD_ONCE_INIT) {			      \
      INIT_FUNCTION ();							      \
      (ONCE_CONTROL) = !PTHREAD_ONCE_INIT;				      \
    }									      \
  } while (0)

/*
 * Initialize RPC multi-threaded operation
 */
static void
rpc_thread_multi (void)
{
  __libc_tsd_set (RPC_VARS, &__libc_tsd_RPC_VARS_mem);
}


struct rpc_thread_variables *
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
				tvp = __libc_tsd_RPC_VARS_data;
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

fd_set * __rpc_thread_svc_fdset (void)
{
    extern fd_set svc_fdset;
    return &(svc_fdset);
}

struct rpc_createerr * __rpc_thread_createerr (void)
{
    extern struct rpc_createerr rpc_createerr;
    return &(rpc_createerr);
}

struct pollfd ** __rpc_thread_svc_pollfd (void)
{
    extern struct pollfd *svc_pollfd;
    return &(svc_pollfd);
}

int * __rpc_thread_svc_max_pollfd (void)
{
    extern int svc_max_pollfd;
    return &(svc_max_pollfd);
}

#endif /* __UCLIBC_HAS_THREADS__ */

