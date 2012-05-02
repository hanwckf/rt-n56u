/*
 * Manuel Novoa III           Feb 2001
 * Erik Andersen              2002-2004
 *
 * __uClibc_main is the routine to be called by all the arch-specific
 * versions of crt0.S in uClibc.
 *
 * It is meant to handle any special initialization needed by the library
 * such as setting the global variable(s) __environ (environ) and
 * initializing the stdio package.  Using weak symbols, the latter is
 * avoided in the static library case.
 */

#define	_ERRNO_H
#include <features.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <bits/uClibc_page.h>
#include <paths.h>
#include <unistd.h>
#include <asm/errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#ifdef __UCLIBC_HAS_SSP__
extern void __guard_setup(void);
#endif


/*
 * Prototypes.
 */
void *__libc_stack_end;
extern void weak_function _stdio_init(void);
extern int *weak_const_function __errno_location(void);
extern int *weak_const_function __h_errno_location(void);
#ifdef __UCLIBC_HAS_LOCALE__
extern void weak_function _locale_init(void);
#endif
#ifdef __UCLIBC_HAS_THREADS__
extern void weak_function __pthread_initialize_minimal(void);
#endif





/*
 * Declare the __environ global variable and create a weak alias environ.
 * Note: Apparently we must initialize __environ to ensure that the weak
 * environ symbol is also included.
 */
char **__environ = 0;
weak_alias(__environ, environ);

size_t __pagesize = 0;
const char *__progname = 0;


#ifndef O_NOFOLLOW
# define O_NOFOLLOW	0
#endif

extern int __libc_fcntl(int fd, int cmd, ...);
extern int __libc_open(const char *file, int flags, ...);

static void __check_one_fd(int fd, int mode)
{
    /* Check if the specified fd is already open */
    if (unlikely(__libc_fcntl(fd, F_GETFD)==-1 && *(__errno_location())==EBADF))
    {
	/* The descriptor is probably not open, so try to use /dev/null */
	struct stat st;
	int nullfd = __libc_open(_PATH_DEVNULL, mode);
	/* /dev/null is major=1 minor=3.  Make absolutely certain
	 * that is in fact the device that we have opened and not
	 * some other wierd file... */
	if ( (nullfd!=fd) || fstat(fd, &st) || !S_ISCHR(st.st_mode) ||
		(st.st_rdev != makedev(1, 3)))
	{
	    /* Somebody is trying some trickery here... */
	    while (1) {
		abort();
	    }
	}
    }
}

static int __check_suid(void)
{
    uid_t uid, euid;
    gid_t gid, egid;

    uid  = getuid();
    euid = geteuid();
    gid  = getgid();
    egid = getegid();

    if(uid == euid && gid == egid) {
	return 0;
    }
    return 1;
}


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
 * __uClibc_start_main.
 */

void __uClibc_init(void)
{
    static int been_there_done_that = 0;

    if (been_there_done_that)
	return;
    been_there_done_that++;

    /* Setup an initial value.  This may not be perfect, but is
     * better than  malloc using __pagesize=0 for atexit, ctors, etc.  */
    __pagesize = PAGE_SIZE;

#ifdef __UCLIBC_HAS_THREADS__
    /* Before we start initialzing uClibc we have to call
     * __pthread_initialize_minimal so we can use pthread_locks
     * whenever they are needed.
     */
    if (likely(__pthread_initialize_minimal!=NULL))
	__pthread_initialize_minimal();
#endif

#ifdef __UCLIBC_HAS_LOCALE__
    /* Initialize the global locale structure. */
    if (likely(_locale_init!=NULL))
	_locale_init();
#endif

    /*
     * Initialize stdio here.  In the static library case, this will
     * be bypassed if not needed because of the weak alias above.
     */
    if (likely(_stdio_init != NULL))
	_stdio_init();

}

#ifdef __UCLIBC_CTOR_DTOR__
void attribute_hidden (*__app_fini)(void) = NULL;
#endif

void attribute_hidden (*__rtld_fini)(void) = NULL;

#ifdef _DL_FINI_CRT_COMPAT
void attribute_hidden (*__dl_fini)(void) = NULL;

void _set__dl_fini(void *fini_func)
{
	if (fini_func != NULL)
		__dl_fini = fini_func;
}
#endif

/* __uClibc_start_main is the new main stub for uClibc. This function is
 * called from crt0 (version 0.9.16 or newer), after ALL shared libraries
 * are initialized, just before we call the application's main function.
 */
void __attribute__ ((__noreturn__))
__uClibc_main(int (*main)(int, char **, char **), int argc,
		    char **argv, void (*app_init)(void), void (*app_fini)(void),
		    void (*rtld_fini)(void), void *stack_end)
{
#ifdef __ARCH_HAS_MMU__
    unsigned long *aux_dat;
    Elf32_auxv_t auxvt[AT_EGID + 1];
#endif
    __libc_stack_end = stack_end;
    /* We need to initialize uClibc.  If we are dynamically linked this
     * may have already been completed by the shared lib loader.  We call
     * __uClibc_init() regardless, to be sure the right thing happens. */
    __uClibc_init();

    __rtld_fini = rtld_fini;

    /* The environment begins right after argv.  */
    __environ = &argv[argc + 1];

    /* If the first thing after argv is the arguments
     * the the environment is empty. */
    if ((char *) __environ == *argv) {
	/* Make __environ point to the NULL at argv[argc] */
	__environ = &argv[argc];
    }

    /* Pull stuff from the ELF header when possible */
#ifdef __ARCH_HAS_MMU__
    aux_dat = (unsigned long*)__environ;
    while (*aux_dat) {
	aux_dat++;
    }
    aux_dat++;
    while (*aux_dat) {
	Elf32_auxv_t *auxv_entry = (Elf32_auxv_t *) aux_dat;
	if (auxv_entry->a_type <= AT_EGID) {
	    memcpy(&(auxvt[auxv_entry->a_type]), auxv_entry, sizeof(Elf32_auxv_t));
	}
	aux_dat += 2;
    }

    /* Make certain getpagesize() gives the correct answer */
    __pagesize = (auxvt[AT_PAGESZ].a_un.a_val)? auxvt[AT_PAGESZ].a_un.a_val : PAGE_SIZE;

    /* Prevent starting SUID binaries where the stdin. stdout, and
     * stderr file descriptors are not already opened. */
    if ((auxvt[AT_UID].a_un.a_val==-1 && __check_suid()) ||
	    (auxvt[AT_UID].a_un.a_val != -1 &&
	    (auxvt[AT_UID].a_un.a_val != auxvt[AT_EUID].a_un.a_val ||
	     auxvt[AT_GID].a_un.a_val != auxvt[AT_EGID].a_un.a_val)))
    {
	__check_one_fd (STDIN_FILENO, O_RDONLY | O_NOFOLLOW);
	__check_one_fd (STDOUT_FILENO, O_RDWR | O_NOFOLLOW);
	__check_one_fd (STDERR_FILENO, O_RDWR | O_NOFOLLOW);
    }
#endif

    __progname = *argv;

#ifdef __UCLIBC_CTOR_DTOR__
    /* Arrange for the application's dtors to run before we exit.  */
    __app_fini = app_fini;

    /* Run all the application's ctors now.  */
    if (app_init!=NULL) {
	app_init();
    }
#endif

#ifdef __UCLIBC_HAS_SSP__
    __guard_setup ();
#endif

    /* Note: It is possible that any initialization done above could
     * have resulted in errno being set nonzero, so set it to 0 before
     * we call main.
     */
    if (likely(__errno_location!=NULL))
	*(__errno_location()) = 0;

    /* Set h_errno to 0 as well */
    if (likely(__h_errno_location!=NULL))
	*(__h_errno_location()) = 0;

    /*
     * Finally, invoke application's main and then exit.
     */
    exit(main(argc, argv, __environ));
}

#ifdef _DL_FINI_CRT_COMPAT
extern int weak_function main(int argc, char **argv, char **envp);
void __attribute__ ((__noreturn__))
__uClibc_start_main(int argc, char **argv, char **envp,
		    void (*app_fini)(void), void (*app_init)(void))
{
	__uClibc_main(main, argc, argv, app_init, app_fini, NULL, NULL);
}
#endif
