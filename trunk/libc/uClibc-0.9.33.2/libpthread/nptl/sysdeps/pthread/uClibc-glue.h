#ifndef _UCLIBC_GLUE_H
#define _UCLIBC_GLUE_H 1

#include <features.h>
#include <sys/cdefs.h>
#include <bits/uClibc_page.h>

#ifdef IS_IN_libpthread
#include <bits/kernel-features.h>

#ifndef __GLIBC_HAVE_LONG_LONG
# define __GLIBC_HAVE_LONG_LONG
#endif

#define __getpagesize getpagesize
#define __sched_get_priority_max sched_get_priority_max
#define __sched_get_priority_min sched_get_priority_min
#define __sched_getscheduler sched_getscheduler
#define __sched_setscheduler sched_setscheduler
#define __sched_getparam sched_getparam
#define __getpid getpid
#define __gettimeofday gettimeofday
#define __poll poll
#define __sysctl sysctl
#define __open open
#define __read read
#define __close close
#define __on_exit on_exit
#define __libc_current_sigrtmin_private __libc_current_sigrtmin
#define __clone clone

extern void *__libc_stack_end;
extern int __cxa_atexit (void (*func) (void *), void *arg, void *d);

#endif /* IS_IN_libpthread */

#ifdef __UCLIBC_HAS_XLOCALE__
# define __uselocale(x) uselocale(x)
#else
# define __uselocale(x) ((void)0)
#endif

/* Use a funky version in a probably vein attempt at preventing gdb
 * from dlopen()'ing glibc's libthread_db library... */
#define VERSION __stringify(__UCLIBC_MAJOR__) "." __stringify(__UCLIBC_MINOR__) "." __stringify(__UCLIBC_SUBLEVEL__)

#endif
