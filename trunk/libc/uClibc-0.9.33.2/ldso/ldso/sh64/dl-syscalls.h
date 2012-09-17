#undef __syscall_return
#define __syscall_return(type, res)					\
do {									\
	/*								\
	 * Note: when returning from kernel the return value is in r9	\
	 *								\
	 * This prevents conflicts between return value and arg1	\
	 * when dispatching signal handler, in other words makes	\
	 * life easier in the system call epilogue (see entry.S)	\
	 */								\
	register unsigned long __sr2 __asm__ ("r2") = res;		\
	if ((unsigned long)(res) >= (unsigned long)(-125)) {		\
		_dl_errno = -(res);					\
		__sr2 = -1;						\
	}								\
	return (type)(__sr2);						\
} while (0)

