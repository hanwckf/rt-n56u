/*
 * Copyright (C) 2006 by Steven J. Hill <sjhill@realitydiluted.com>
 * Copyright (C) 2001 by Manuel Novoa III <mjn3@uclibc.org>
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 * __uClibc_main is the routine to be called by all the arch-specific
 * versions of crt1.S in uClibc.
 *
 * It is meant to handle any special initialization needed by the library
 * such as setting the global variable(s) __environ (environ) and
 * initializing the stdio package.  Using weak symbols, the latter is
 * avoided in the static library case.
 */

#include <features.h>
#ifndef __UCLIBC_HAS_THREADS_NATIVE__
#define	_ERRNO_H
#endif
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <link.h>
#include <bits/uClibc_page.h>
#include <paths.h>
#include <unistd.h>
#include <asm/errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#ifdef __UCLIBC_HAS_THREADS_NATIVE__
#include <errno.h>
#include <pthread-functions.h>
#include <not-cancel.h>
#include <atomic.h>
#endif
#ifdef __UCLIBC_HAS_THREADS__
#include <pthread.h>
#endif 

#ifndef SHARED
void *__libc_stack_end = NULL;

# ifdef __UCLIBC_HAS_SSP__
#  include <dl-osinfo.h>
static uintptr_t stack_chk_guard;
#  ifndef THREAD_SET_STACK_GUARD
/* Only exported for architectures that don't store the stack guard canary
 * in thread local area. */
/* for gcc-4.1 non-TLS */
uintptr_t __stack_chk_guard attribute_relro;
#  endif
/* for gcc-3.x + Etoh ssp */
#  ifdef __UCLIBC_HAS_SSP_COMPAT__
uintptr_t __guard attribute_relro;
#  endif
# endif

/*
 * Needed to initialize _dl_phdr when statically linked
 */

void internal_function _dl_aux_init (ElfW(auxv_t) *av);

#ifdef __UCLIBC_HAS_THREADS__
/*
 * uClibc internal locking requires that we have weak aliases
 * for dummy functions in case libpthread.a is not linked in.
 * This needs to be in compilation unit that is pulled always
 * in or linker will disregard these weaks.
 */

static int __pthread_return_0 (pthread_mutex_t *unused) { return 0; }
weak_alias (__pthread_return_0, __pthread_mutex_lock)
weak_alias (__pthread_return_0, __pthread_mutex_trylock)
weak_alias (__pthread_return_0, __pthread_mutex_unlock)

int weak_function
__pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
        return 0;
}

void weak_function
_pthread_cleanup_push_defer(struct _pthread_cleanup_buffer *__buffer,
                            void (*__routine) (void *), void *__arg)
{
        __buffer->__routine = __routine;
        __buffer->__arg = __arg;
}

void weak_function
_pthread_cleanup_pop_restore(struct _pthread_cleanup_buffer *__buffer,
                             int __execute)
{
        if (__execute)
                __buffer->__routine(__buffer->__arg);
}
#endif /* __UCLIBC_HAS_THREADS__ */

#endif /* !SHARED */

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

/*
 * Prototypes.
 */
extern int *weak_const_function __errno_location(void);
extern int *weak_const_function __h_errno_location(void);
extern void weak_function _stdio_init(void) attribute_hidden;
#ifdef __UCLIBC_HAS_LOCALE__
extern void weak_function _locale_init(void) attribute_hidden;
#endif
#ifdef __UCLIBC_HAS_THREADS__
#if !defined (__UCLIBC_HAS_THREADS_NATIVE__) || defined (SHARED)
extern void weak_function __pthread_initialize_minimal(void);
#else
extern void __pthread_initialize_minimal(void);
#endif
#endif

/* If __UCLIBC_FORMAT_SHARED_FLAT__, all array initialisation and finalisation
 * is handled by the routines passed to __uClibc_main().  */
#if defined (__UCLIBC_CTOR_DTOR__) && !defined (__UCLIBC_FORMAT_SHARED_FLAT__)
extern void _dl_app_init_array(void);
extern void _dl_app_fini_array(void);
# ifndef SHARED
/* These magic symbols are provided by the linker.  */
extern void (*__preinit_array_start []) (void) attribute_hidden;
extern void (*__preinit_array_end []) (void) attribute_hidden;
extern void (*__init_array_start []) (void) attribute_hidden;
extern void (*__init_array_end []) (void) attribute_hidden;
extern void (*__fini_array_start []) (void) attribute_hidden;
extern void (*__fini_array_end []) (void) attribute_hidden;
# endif
#endif

#if defined (__LDSO_STANDALONE_SUPPORT__) && defined (SHARED) && defined __sh__
extern unsigned long _dl_skip_args;
#endif

attribute_hidden const char *__uclibc_progname = "";
#ifdef __UCLIBC_HAS_PROGRAM_INVOCATION_NAME__
const char *program_invocation_short_name = "";
const char *program_invocation_name = "";
#endif
#ifdef __UCLIBC_HAS___PROGNAME__
weak_alias (program_invocation_short_name, __progname)
weak_alias (program_invocation_name, __progname_full)
#endif

/*
 * Declare the __environ global variable and create a weak alias environ.
 * This must be initialized; we cannot have a weak alias into bss.
 */
char **__environ = 0;
weak_alias(__environ, environ)

/* TODO: don't export __pagesize; we cant now because libpthread uses it */
size_t __pagesize = 0;

#ifndef O_NOFOLLOW
# define O_NOFOLLOW	0
#endif

#ifndef __ARCH_HAS_NO_LDSO__
static void __check_one_fd(int fd, int mode)
{
    /* Check if the specified fd is already open */
    if (fcntl(fd, F_GETFD) == -1)
    {
	/* The descriptor is probably not open, so try to use /dev/null */
	int nullfd = open(_PATH_DEVNULL, mode);
	/* /dev/null is major=1 minor=3.  Make absolutely certain
	 * that is in fact the device that we have opened and not
	 * some other wierd file... [removed in uclibc] */
	if (nullfd!=fd)
	{
		abort();
	}
    }
}

static int __check_suid(void)
{
    uid_t uid, euid;
    gid_t gid, egid;

    uid  = getuid();
    euid = geteuid();
    if (uid != euid)
	return 1;
    gid  = getgid();
    egid = getegid();
    if (gid != egid)
	return 1;
    return 0; /* we are not suid */
}
#endif

/* __uClibc_init completely initialize uClibc so it is ready to use.
 *
 * On ELF systems (with a dynamic loader) this function must be called
 * from the dynamic loader (see TIS and ELF Specification), so that
 * constructors of shared libraries (which depend on libc) can use all
 * the libc code without restriction.  For this we link the shared
 * version of the uClibc with -init __uClibc_init so DT_INIT for
 * uClibc is the address of __uClibc_init
 *
 * In all other cases we call it from the main stub
 * __uClibc_main.
 */

extern void __uClibc_init(void);
libc_hidden_proto(__uClibc_init)
void __uClibc_init(void)
{
    /* Don't recurse */
    if (__pagesize)
	return;

    /* Setup an initial value.  This may not be perfect, but is
     * better than  malloc using __pagesize=0 for atexit, ctors, etc.  */
    __pagesize = PAGE_SIZE;

#ifdef __UCLIBC_HAS_THREADS__
    /* Before we start initializing uClibc we have to call
     * __pthread_initialize_minimal so we can use pthread_locks
     * whenever they are needed.
     */
#if !defined (__UCLIBC_HAS_THREADS_NATIVE__) || defined (SHARED)
    if (likely(__pthread_initialize_minimal!=NULL))
#endif
	__pthread_initialize_minimal();
#endif

#ifndef SHARED
# ifdef __UCLIBC_HAS_SSP__
    /* Set up the stack checker's canary.  */
    stack_chk_guard = _dl_setup_stack_chk_guard();
#  ifdef THREAD_SET_STACK_GUARD
    THREAD_SET_STACK_GUARD (stack_chk_guard);
#  else
    __stack_chk_guard = stack_chk_guard;
#  endif
#  ifdef __UCLIBC_HAS_SSP_COMPAT__
    __guard = stack_chk_guard;
#  endif
# endif
#endif

#ifdef __UCLIBC_HAS_LOCALE__
    /* Initialize the global locale structure. */
    if (likely(not_null_ptr(_locale_init)))
	_locale_init();
#endif

    /*
     * Initialize stdio here.  In the static library case, this will
     * be bypassed if not needed because of the weak alias above.
     * Thus we get a nice size savings because the stdio functions
     * won't be pulled into the final static binary unless used.
     */
    if (likely(not_null_ptr(_stdio_init)))
	_stdio_init();

}
libc_hidden_def(__uClibc_init)

#ifdef __UCLIBC_CTOR_DTOR__
void attribute_hidden (*__app_fini)(void) = NULL;
#endif

void attribute_hidden (*__rtld_fini)(void) = NULL;

extern void __uClibc_fini(void);
libc_hidden_proto(__uClibc_fini)
void __uClibc_fini(void)
{
#ifdef __UCLIBC_CTOR_DTOR__
    /* If __UCLIBC_FORMAT_SHARED_FLAT__, all array finalisation is handled
     * by __app_fini.  */
# ifdef SHARED
    _dl_app_fini_array();
# elif !defined (__UCLIBC_FORMAT_SHARED_FLAT__)
    size_t i = __fini_array_end - __fini_array_start;
    while (i-- > 0)
	(*__fini_array_start [i]) ();
# endif
    if (__app_fini != NULL)
	(__app_fini)();
#endif
    if (__rtld_fini != NULL)
	(__rtld_fini)();
}
libc_hidden_def(__uClibc_fini)

#ifndef SHARED
extern void __nptl_deallocate_tsd (void) __attribute ((weak));
extern unsigned int __nptl_nthreads __attribute ((weak));
#endif

/* __uClibc_main is the new main stub for uClibc. This function is
 * called from crt1 (version 0.9.28 or newer), after ALL shared libraries
 * are initialized, just before we call the application's main function.
 */
void __uClibc_main(int (*main)(int, char **, char **), int argc,
		    char **argv, void (*app_init)(void), void (*app_fini)(void),
		    void (*rtld_fini)(void),
		    void *stack_end attribute_unused) attribute_noreturn;
void __uClibc_main(int (*main)(int, char **, char **), int argc,
		    char **argv, void (*app_init)(void), void (*app_fini)(void),
		    void (*rtld_fini)(void), void *stack_end attribute_unused)
{
#ifndef __ARCH_HAS_NO_LDSO__
    unsigned long *aux_dat;
    ElfW(auxv_t) auxvt[AT_EGID + 1];
#endif

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	/* Result of the 'main' function.  */
	int result;
#endif

#ifndef SHARED
    __libc_stack_end = stack_end;
#endif

    __rtld_fini = rtld_fini;

#if defined __LDSO_STANDALONE_SUPPORT__ && defined SHARED && defined __sh__
	/*
	 * Skip ld.so and its arguments
	 * Other archs except for SH do this in _dl_start before passing
	 * control to the application.
	 * FIXME: align SH _dl_start to other archs and remove this from here,
	 *        so that we can keep the visibility hidden.
	 */
	argc -= _dl_skip_args;
	argv += _dl_skip_args;
#endif

    /* The environment begins right after argv.  */
    __environ = &argv[argc + 1];

    /* If the first thing after argv is the arguments
     * then the environment is empty. */
    if ((char *) __environ == *argv) {
	/* Make __environ point to the NULL at argv[argc] */
	__environ = &argv[argc];
    }

#ifndef __ARCH_HAS_NO_LDSO__
    /* Pull stuff from the ELF header when possible */
    memset(auxvt, 0x00, sizeof(auxvt));
    aux_dat = (unsigned long*)__environ;
    while (*aux_dat) {
	aux_dat++;
    }
    aux_dat++;
    while (*aux_dat) {
	ElfW(auxv_t) *auxv_entry = (ElfW(auxv_t) *) aux_dat;
	if (auxv_entry->a_type <= AT_EGID) {
	    memcpy(&(auxvt[auxv_entry->a_type]), auxv_entry, sizeof(ElfW(auxv_t)));
	}
	aux_dat += 2;
    }
#ifndef SHARED
    /* Get the program headers (_dl_phdr) from the aux vector
       It will be used into __libc_setup_tls. */

    _dl_aux_init (auxvt);
#endif
#endif

    /* We need to initialize uClibc.  If we are dynamically linked this
     * may have already been completed by the shared lib loader.  We call
     * __uClibc_init() regardless, to be sure the right thing happens. */
    __uClibc_init();

#ifndef __ARCH_HAS_NO_LDSO__
    /* Make certain getpagesize() gives the correct answer */
    __pagesize = (auxvt[AT_PAGESZ].a_un.a_val)? auxvt[AT_PAGESZ].a_un.a_val : PAGE_SIZE;

    /* Prevent starting SUID binaries where the stdin. stdout, and
     * stderr file descriptors are not already opened. */
    if ((auxvt[AT_UID].a_un.a_val == (size_t)-1 && __check_suid()) ||
	    (auxvt[AT_UID].a_un.a_val != (size_t)-1 &&
	    (auxvt[AT_UID].a_un.a_val != auxvt[AT_EUID].a_un.a_val ||
	     auxvt[AT_GID].a_un.a_val != auxvt[AT_EGID].a_un.a_val)))
    {
	__check_one_fd (STDIN_FILENO, O_RDONLY | O_NOFOLLOW);
	__check_one_fd (STDOUT_FILENO, O_RDWR | O_NOFOLLOW);
	__check_one_fd (STDERR_FILENO, O_RDWR | O_NOFOLLOW);
    }
#endif

    __uclibc_progname = *argv;
#ifdef __UCLIBC_HAS_PROGRAM_INVOCATION_NAME__
    if (*argv != NULL) {
	program_invocation_name = *argv;
	program_invocation_short_name = strrchr(*argv, '/');
	if (program_invocation_short_name != NULL)
	    ++program_invocation_short_name;
	else
	    program_invocation_short_name = program_invocation_name;
    }
#endif

#ifdef __UCLIBC_CTOR_DTOR__
    /* Arrange for the application's dtors to run before we exit.  */
    __app_fini = app_fini;

    /* If __UCLIBC_FORMAT_SHARED_FLAT__, all array initialisation is handled
     * by __app_init.  */
# if !defined (SHARED) && !defined (__UCLIBC_FORMAT_SHARED_FLAT__)
    /* For dynamically linked executables the preinit array is executed by
       the dynamic linker (before initializing any shared object).
       For static executables, preinit happens rights before init.  */
    {
	const size_t size = __preinit_array_end - __preinit_array_start;
	size_t i;
	for (i = 0; i < size; i++)
	    (*__preinit_array_start [i]) ();
    }
# endif
    /* Run all the application's ctors now.  */
    if (app_init!=NULL) {
	app_init();
    }
    /* If __UCLIBC_FORMAT_SHARED_FLAT__, all array initialisation is handled
     * by __app_init.  */
# ifdef SHARED
    _dl_app_init_array();
# elif !defined (__UCLIBC_FORMAT_SHARED_FLAT__)
    {
	const size_t size = __init_array_end - __init_array_start;
	size_t i;
	for (i = 0; i < size; i++)
	    (*__init_array_start [i]) ();
    }
# endif
#endif

    /* Note: It is possible that any initialization done above could
     * have resulted in errno being set nonzero, so set it to 0 before
     * we call main.
     */
    if (likely(not_null_ptr(__errno_location)))
	*(__errno_location()) = 0;

    /* Set h_errno to 0 as well */
    if (likely(not_null_ptr(__h_errno_location)))
	*(__h_errno_location()) = 0;

#if defined HAVE_CLEANUP_JMP_BUF && defined __UCLIBC_HAS_THREADS_NATIVE__
	/* Memory for the cancellation buffer.  */
	struct pthread_unwind_buf unwind_buf;

	int not_first_call;
	not_first_call =
		setjmp ((struct __jmp_buf_tag *) unwind_buf.cancel_jmp_buf);
	if (__builtin_expect (! not_first_call, 1))
	{
		struct pthread *self = THREAD_SELF;

		/* Store old info.  */
		unwind_buf.priv.data.prev = THREAD_GETMEM (self, cleanup_jmp_buf);
		unwind_buf.priv.data.cleanup = THREAD_GETMEM (self, cleanup);

		/* Store the new cleanup handler info.  */
		THREAD_SETMEM (self, cleanup_jmp_buf, &unwind_buf);

		/* Run the program.  */
		result = main (argc, argv, __environ);
	}
	else
	{
		/* Remove the thread-local data.  */
# ifdef SHARED
		__libc_pthread_functions.ptr__nptl_deallocate_tsd ();
# else
		__nptl_deallocate_tsd ();
# endif

		/* One less thread.  Decrement the counter.  If it is zero we
		   terminate the entire process.  */
		result = 0;
# ifdef SHARED
		unsigned int *const ptr = __libc_pthread_functions.ptr_nthreads;
# else
		unsigned int *const ptr = &__nptl_nthreads;
# endif

		if (! atomic_decrement_and_test (ptr))
			/* Not much left to do but to exit the thread, not the process.  */
			__exit_thread_inline (0);
	}

	exit (result);
#else
	/*
	 * Finally, invoke application's main and then exit.
	 */
	exit (main (argc, argv, __environ));
#endif
}
