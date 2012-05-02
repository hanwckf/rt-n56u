#ifndef _RPC_RPC_H
#include <rpc/rpc.h>

/* Now define the internal interfaces.  */
extern unsigned long _create_xid (void);

/*
 * Multi-threaded support
 * Group all global and static variables into a single spot.
 * This area is allocated on a per-thread basis
 */
#ifdef __UCLIBC_HAS_THREADS__
#include <pthread.h>
struct rpc_thread_variables {
	fd_set		svc_fdset_s;		/* Global, rpc_common.c */
	struct rpc_createerr rpc_createerr_s;	/* Global, rpc_common.c */
	struct pollfd	*svc_pollfd_s;		/* Global, rpc_common.c */
	int		svc_max_pollfd_s;	/* Global, rpc_common.c */

	void		*authnone_private_s;	/* auth_none.c */

	void		*clnt_perr_buf_s;	/* clnt_perr.c */

	void		*clntraw_private_s;	/* clnt_raw.c */

	void		*callrpc_private_s;	/* clnt_simp.c */

	void		*key_call_private_s;	/* key_call.c */

	void		*authdes_cache_s;	/* svcauth_des.c */
	void		*authdes_lru_s;		/* svcauth_des.c */

	void		*svc_xports_s;		/* svc.c */
	void		*svc_head_s;		/* svc.c */

	void		*svcraw_private_s;	/* svc_raw.c */

	void		*svcsimple_proglst_s;	/* svc_simple.c */
	void		*svcsimple_transp_s;	/* svc_simple.c */
};

extern struct rpc_thread_variables *__rpc_thread_variables(void)
     __attribute__ ((const));
extern void __rpc_thread_svc_cleanup (void);
extern void __rpc_thread_clnt_cleanup (void);
extern void __rpc_thread_key_cleanup (void);

extern void __rpc_thread_destroy (void);

#define RPC_THREAD_VARIABLE(x) (__rpc_thread_variables()->x)

#endif /* __UCLIBC_HAS_THREADS__ */

#endif
