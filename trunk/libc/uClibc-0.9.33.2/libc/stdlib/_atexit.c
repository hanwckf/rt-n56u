/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

/*
 * Dec 2000          Manuel Novoa III
 *
 *   Made atexit handling conform to standards... i.e. no args.
 *   Removed on_exit since it did not match gnu libc definition.
 *   Combined atexit and __do_exit into one object file.
 *
 * Feb 2001          Manuel Novoa III
 *
 *   Reworked file after addition of __uClibc_main.
 *   Changed name of __do_exit to atexit_handler.
 *   Changed name of __cleanup to __uClibc_cleanup.
 *   Moved declaration of __uClibc_cleanup to __uClibc_main
 *      where it is initialized with (possibly weak alias)
 *      _stdio_term.
 *
 * Jul 2001          Steve Thayer
 *
 *   Added an on_exit implementation (that now matches gnu libc definition.)
 *   Pulled atexit_handler out of the atexit object since it is now required by
 *   on_exit as well.  Renamed it to __exit_handler.
 *   Fixed a problem where exit functions stop getting called if one of
 *   them calls exit().
 *   As a side effect of these changes, abort() no longer calls the exit
 *   functions (it now matches the gnu libc definition).
 *
 * August 2002    Erik Andersen
 *   Added locking so atexit and friends can be thread safe
 *
 * August 2005    Stephen Warren
 *   Added __cxa_atexit and __cxa_finalize support
 *
 */

#include <features.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <atomic.h>

#include <bits/uClibc_mutex.h>
__UCLIBC_MUTEX_EXTERN(__atexit_lock);



typedef void (*aefuncp)(void);         /* atexit function pointer */
typedef void (*oefuncp)(int, void *);  /* on_exit function pointer */
typedef void (*cxaefuncp)(void *);     /* __cxa_atexit function pointer */
typedef enum {
    ef_free,
    ef_in_use,
    ef_on_exit,
    ef_cxa_atexit
} ef_type; /* exit function types */

/* this is in the L_exit object */
extern void (*__exit_cleanup)(int) attribute_hidden;

/* these are in the L___do_exit object */
extern int __exit_slots attribute_hidden;
extern int __exit_count attribute_hidden;
extern void __exit_handler(int) attribute_hidden;
struct exit_function {
        /*
         * 'type' should be of type of the 'enum ef_type' above but since we
         * need this element in an atomic operation we have to use 'long int'.
         */
        long int type; /* enum ef_type */
	union {
                struct {
                        oefuncp func;
                        void *arg;
                } on_exit;
                struct {
                        cxaefuncp func;
                        void *arg;
                        void* dso_handle;
                } cxa_atexit;
	} funcs;
};
#ifdef __UCLIBC_DYNAMIC_ATEXIT__
extern struct exit_function *__exit_function_table attribute_hidden;
#else
extern struct exit_function __exit_function_table[__UCLIBC_MAX_ATEXIT] attribute_hidden;
#endif
extern struct exit_function *__new_exitfn(void) attribute_hidden;

/* this is in the L___cxa_atexit object */
extern int __cxa_atexit(cxaefuncp, void *arg, void *dso_handle);


/* remove old_atexit after 0.9.29 */
#if defined(L_atexit) || defined(L_old_atexit)
extern void *__dso_handle __attribute__ ((__weak__));

/*
 * register a function to be called at normal program termination
 * (the registered function takes no arguments)
 */
#ifdef L_atexit
int attribute_hidden atexit(aefuncp func)
#else
int old_atexit(aefuncp func);
int old_atexit(aefuncp func)
#endif
{
    /*
     * glibc casts aefuncp to cxaefuncp.
     * This seems dodgy, but I guess calling a function with more
     * parameters than it needs will work everywhere?
     */
    return __cxa_atexit((cxaefuncp)func, NULL,
                        &__dso_handle == NULL ? NULL : __dso_handle);
}
#ifndef L_atexit
weak_alias(old_atexit,atexit)
#endif
#endif

#ifdef L_on_exit
/*
 * register a function to be called at normal program termination
 * the registered function takes two arguments:
 *     status - the exit status that was passed to the exit() function
 *     arg - generic argument
 */
int on_exit(oefuncp func, void *arg)
{
    struct exit_function *efp;

    if (func == NULL) {
        return 0;
    }

    efp = __new_exitfn();
    if (efp == NULL) {
        return -1;
    }

    efp->funcs.on_exit.func = func;
    efp->funcs.on_exit.arg = arg;
    /* assign last for thread safety, since we're now unlocked */
    efp->type = ef_on_exit;

    return 0;
}
#endif

#ifdef L___cxa_atexit
libc_hidden_proto(__cxa_atexit)
int __cxa_atexit(cxaefuncp func, void *arg, void *dso_handle)
{
    struct exit_function *efp;

    if (func == NULL) {
        return 0;
    }

    efp = __new_exitfn();
    if (efp == NULL) {
        return -1;
    }

    efp->funcs.cxa_atexit.func = func;
    efp->funcs.cxa_atexit.arg = arg;
    efp->funcs.cxa_atexit.dso_handle = dso_handle;
    /* assign last for thread safety, since we're now unlocked */
    efp->type = ef_cxa_atexit;

    return 0;
}
libc_hidden_def(__cxa_atexit)
#endif

#ifdef L___cxa_finalize
/*
 * If D is non-NULL, call all functions registered with `__cxa_atexit'
 *  with the same dso handle.  Otherwise, if D is NULL, call all of the
 *  registered handlers.
 */
void __cxa_finalize(void *dso_handle);
void __cxa_finalize(void *dso_handle)
{
    struct exit_function *efp;
    int exit_count_snapshot = __exit_count;

    /* In reverse order */
    while (exit_count_snapshot) {
        efp = &__exit_function_table[--exit_count_snapshot];

        /*
         * We check dso_handle match before we verify the type of the union entry.
         * However, the atomic_exchange will validate that we were really "allowed"
         * to read dso_handle...
         */
        if ((dso_handle == NULL || dso_handle == efp->funcs.cxa_atexit.dso_handle)
            /* We don't want to run this cleanup more than once. */
            && !atomic_compare_and_exchange_bool_acq(&efp->type, ef_free, ef_cxa_atexit)
           ) {
            /* glibc passes status (0) too, but that's not in the prototype */
            (*efp->funcs.cxa_atexit.func)(efp->funcs.cxa_atexit.arg);
        }
    }

#if 0 /* haven't looked into this yet... */
    /*
     * Remove the registered fork handlers. We do not have to
     * unregister anything if the program is going to terminate anyway.
     */
#ifdef UNREGISTER_ATFORK
    if (d != NULL) {
        UNREGISTER_ATFORK(d);
    }
#endif
#endif
}
#endif

#ifdef L___exit_handler
int __exit_count = 0; /* Number of registered exit functions */
#ifdef __UCLIBC_DYNAMIC_ATEXIT__
struct exit_function *__exit_function_table = NULL;
int __exit_slots = 0; /* Size of __exit_function_table */
#else
struct exit_function __exit_function_table[__UCLIBC_MAX_ATEXIT];
#endif

/*
 * Find and return a new exit_function pointer, for atexit,
 * onexit and __cxa_atexit to initialize
 */
struct exit_function attribute_hidden *__new_exitfn(void)
{
    struct exit_function *efp;

    __UCLIBC_MUTEX_LOCK(__atexit_lock);

	/*
	 * Reuse free slots at the end of the list.
	 * This avoids eating memory when dlopen and dlclose modules multiple times.
	*/
	while (__exit_count > 0) {
		if (__exit_function_table[__exit_count-1].type == ef_free) {
			--__exit_count;
		} else break;
	}

#ifdef __UCLIBC_DYNAMIC_ATEXIT__
    /* If we are out of function table slots, make some more */
    if (__exit_slots < __exit_count+1) {
        efp = realloc(__exit_function_table,
                    (__exit_slots+20)*sizeof(struct exit_function));
        if (efp == NULL) {
            __set_errno(ENOMEM);
	    goto DONE;
        }
        __exit_function_table = efp;
        __exit_slots += 20;
    }
#else
    if (__exit_count >= __UCLIBC_MAX_ATEXIT) {
        __set_errno(ENOMEM);
	efp = NULL;
	goto DONE;
    }
#endif

    __exit_cleanup = __exit_handler; /* enable cleanup */
    efp = &__exit_function_table[__exit_count++];
    efp->type = ef_in_use;

DONE:
    __UCLIBC_MUTEX_UNLOCK(__atexit_lock);
    return efp;
}

/*
 * Handle the work of executing the registered exit functions
 * This is called while we are locked, so no additional locking
 * is needed...
 */
void __exit_handler(int status)
{
	struct exit_function *efp;

	/* In reverse order */
	while (__exit_count) {
		efp = &__exit_function_table[--__exit_count];
		switch (efp->type) {
		case ef_on_exit:
			if (efp->funcs.on_exit.func) {
				(efp->funcs.on_exit.func)(status, efp->funcs.on_exit.arg);
			}
			break;
                case ef_cxa_atexit:
                        if (efp->funcs.cxa_atexit.func) {
                                /* glibc passes status too, but that's not in the prototype */
                                (efp->funcs.cxa_atexit.func)(efp->funcs.cxa_atexit.arg);
                        }
                        break;
		}
	}
#ifdef __UCLIBC_DYNAMIC_ATEXIT__
	/* Free up memory used by the __exit_function_table structure */
	free(__exit_function_table);
#endif
}
#endif

#ifdef L_exit
/* Defeat compiler optimization which assumes function addresses are never NULL */
static __always_inline int not_null_ptr(const void *p)
{
	const void *q;
	__asm__ (""
		: "=r" (q) /* output */
		: "0" (p) /* input */
	);
	return q != 0;
}

extern void weak_function _stdio_term(void) attribute_hidden;
attribute_hidden void (*__exit_cleanup)(int) = 0;
__UCLIBC_MUTEX_INIT(__atexit_lock, PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP);

extern void __uClibc_fini(void);
libc_hidden_proto(__uClibc_fini)

/*
 * Normal program termination
 */
void exit(int rv)
{
	/* Perform exit-specific cleanup (atexit and on_exit) */
	__UCLIBC_MUTEX_LOCK(__atexit_lock);
	if (not_null_ptr(__exit_cleanup)) {
		__exit_cleanup(rv);
	}
	__UCLIBC_MUTEX_UNLOCK(__atexit_lock);

	__uClibc_fini();

	/* If we are using stdio, try to shut it down.  At the very least,
	 * this will attempt to commit all buffered writes.  It may also
	 * unbuffer all writable files, or close them outright.
	 * Check the stdio routines for details. */
	if (not_null_ptr(_stdio_term))
		_stdio_term();

	_exit(rv);
}
libc_hidden_def(exit)
#endif
