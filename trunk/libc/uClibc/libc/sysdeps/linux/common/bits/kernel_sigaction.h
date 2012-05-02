#ifndef _BITS_STAT_STRUCT_H
#define _BITS_STAT_STRUCT_H

/* This file provides whatever this particular arch's kernel thinks 
 * the sigaction struct should look like... */

#if defined(__alpha__)
#undef HAVE_SA_RESTORER
/* This is the sigaction struction from the Linux 2.1.20 kernel.  */
struct old_kernel_sigaction {
    __sighandler_t k_sa_handler;
    unsigned long sa_mask;
    unsigned int sa_flags;
};
/* This is the sigaction structure from the Linux 2.1.68 kernel.  */
struct kernel_sigaction {
    __sighandler_t k_sa_handler;
    unsigned int sa_flags;
    sigset_t sa_mask;
};
#elif defined(__hppa__)
#undef HAVE_SA_RESTORER
/* This is the sigaction struction from the Linux 2.1.20 kernel.  */
/* Blah.  This is bogus.  We don't ever use it. */
struct old_kernel_sigaction {
    __sighandler_t k_sa_handler;
    unsigned long sa_mask;
    unsigned long sa_flags;
};
/* This is the sigaction structure from the Linux 2.1.68 kernel.  */
struct kernel_sigaction {
    __sighandler_t k_sa_handler;
    unsigned long sa_flags;
    sigset_t sa_mask;
};
#elif defined(__mips__)
#undef HAVE_SA_RESTORER
/* This is the sigaction structure from the Linux 2.1.24 kernel.  */
#include <sgidefs.h>
struct old_kernel_sigaction {
    __sighandler_t  k_sa_handler;
    unsigned int    sa_flags;
    unsigned long   sa_mask;
};
#define _KERNEL_NSIG           128
#define _KERNEL_NSIG_BPW       32
#define _KERNEL_NSIG_WORDS     (_KERNEL_NSIG / _KERNEL_NSIG_BPW)

typedef struct {
    unsigned long sig[_KERNEL_NSIG_WORDS];
} kernel_sigset_t;

/* This is the sigaction structure from the Linux 2.1.68 kernel.  */
struct kernel_sigaction {
    unsigned int    sa_flags;
    __sighandler_t  k_sa_handler;
    kernel_sigset_t sa_mask;
    void            (*sa_restorer)(void);
    int             s_resv[1]; /* reserved */
};
#else
#define HAVE_SA_RESTORER
/* This is the sigaction structure from the Linux 2.1.20 kernel.  */
struct old_kernel_sigaction {
    __sighandler_t k_sa_handler;
    unsigned long sa_mask;
    unsigned long sa_flags;
    void (*sa_restorer) (void);
};
/* This is the sigaction structure from the Linux 2.1.68 kernel.  */
struct kernel_sigaction {
    __sighandler_t k_sa_handler;
    unsigned long sa_flags;
    void (*sa_restorer) (void);
    sigset_t sa_mask;
};
#endif

extern int __syscall_sigaction (int, const struct old_kernel_sigaction *__unbounded,
	struct old_kernel_sigaction *__unbounded);

extern int __syscall_rt_sigaction (int, const struct kernel_sigaction *__unbounded,
	struct kernel_sigaction *__unbounded, size_t);

#endif /* _BITS_STAT_STRUCT_H */
