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
/* In uclibc, userspace struct sigaction is identical to
 * "new" struct kernel_sigaction (one from the Linux 2.1.68 kernel).
 * See sigaction.h
 */

extern int __syscall_sigaction(int, const struct old_kernel_sigaction *,
	struct old_kernel_sigaction *);

#endif


extern int __syscall_rt_sigaction(int, const struct sigaction *,
	struct sigaction *, size_t);

#endif /* _BITS_SIGACTION_STRUCT_H */
