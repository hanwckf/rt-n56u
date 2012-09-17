/* We can't use the real errno in ldso, since it has not yet
 * been dynamicly linked in yet. */
#include "sys/syscall.h"
extern int _dl_errno;
#undef __set_errno
#define __set_errno(X) {(_dl_errno) = (X);}

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

