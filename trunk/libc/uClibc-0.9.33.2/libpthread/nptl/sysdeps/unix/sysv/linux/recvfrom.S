#include <sysdep-cancel.h>
#ifndef __NR_recvfrom
#error Missing definition of NR_recvfrom needed for cancellation.
#endif
PSEUDO (__libc_recvfrom, recvfrom, 6)
ret
PSEUDO_END(__libc_recvfrom)
libc_hidden_def (__libc_recvfrom)
weak_alias (__libc_recvfrom, __recvfrom)
libc_hidden_weak (__recvfrom)
weak_alias (__libc_recvfrom, recvfrom)
libc_hidden_weak (recvfrom)
