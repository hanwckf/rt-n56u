#ifndef _BITS_SIGACTION_STRUCT_H
#define _BITS_SIGACTION_STRUCT_H

/* This file provides whatever this particular arch's kernel thinks
 * the sigaction struct should look like... */


#if defined(__ia64__)

#undef HAVE_SA_RESTORER

#else

#define HAVE_SA_RESTORER
/* This is the sigaction structure from the Linux 2.1.20 kernel.  */
struct old_kernel_sigaction {
	__sighandler_t k_sa_handler;
	unsigned long sa_mask;
	unsigned long sa_flags;
	void (*sa_restorer)(void);
};

/* This is the sigaction structure from the Linux 2.1.68 kernel.  */
struct kernel_sigaction {
	__sighandler_t k_sa_handler;
	unsigned long sa_flags;
	void (*sa_restorer) (void);
	sigset_t sa_mask;
};
#endif

#endif /* _BITS_SIGACTION_STRUCT_H */
