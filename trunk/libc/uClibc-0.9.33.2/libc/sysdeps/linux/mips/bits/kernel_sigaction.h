#ifndef _BITS_SIGACTION_STRUCT_H
#define _BITS_SIGACTION_STRUCT_H

/* This is the sigaction structure from the Linux 2.1.24 kernel.  */

#include <sgidefs.h>

#define HAVE_SA_RESTORER

struct old_kernel_sigaction {
	unsigned int	sa_flags;
	__sighandler_t	k_sa_handler;
	unsigned long	sa_mask;
	unsigned int    __pad0[3]; /* reserved, keep size constant */

	/* Abi says here follows reserved int[2] */
	void		(*sa_restorer)(void);
#if (_MIPS_SZPTR < 64)
	/*
	 * For 32 bit code we have to pad struct sigaction to get
	 * constant size for the ABI
	 */
	int		pad1[1]; /* reserved */
#endif
};

/* In uclibc, userspace struct sigaction is identical to
 * "new" struct kernel_sigaction (one from the Linux 2.1.68 kernel).
 * See sigaction.h
 */

extern int __syscall_rt_sigaction (int, const struct sigaction *,
	struct sigaction *, size_t) attribute_hidden;

#endif
