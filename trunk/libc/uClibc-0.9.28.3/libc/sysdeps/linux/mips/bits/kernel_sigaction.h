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


#define _KERNEL_NSIG	       128
#define _KERNEL_NSIG_BPW       _MIPS_SZLONG
#define _KERNEL_NSIG_WORDS     (_KERNEL_NSIG / _KERNEL_NSIG_BPW)

typedef struct {
	unsigned long sig[_KERNEL_NSIG_WORDS];
} kernel_sigset_t;

/* This is the sigaction structure from the Linux 2.1.68 kernel.  */
struct kernel_sigaction {
	unsigned int	sa_flags;
	__sighandler_t	k_sa_handler;
	kernel_sigset_t	sa_mask;
	void		(*sa_restorer)(void);
	int		s_resv[1]; /* reserved */
};

extern int __syscall_rt_sigaction (int, const struct kernel_sigaction *__unbounded,
	struct kernel_sigaction *__unbounded, size_t);
