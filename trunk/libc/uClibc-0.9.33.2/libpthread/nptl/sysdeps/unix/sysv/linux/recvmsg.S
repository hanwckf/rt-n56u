#include <sysdep-cancel.h>
#ifndef __NR_recvmsg
#error Missing definition of NR_recvmsg needed for cancellation.
#endif
PSEUDO (__libc_recvmsg, recvmsg, 3)
ret
PSEUDO_END(__libc_recvmsg)
libc_hidden_def (__libc_recvmsg)
weak_alias (__libc_recvmsg, __recvmsg)
libc_hidden_weak (__recvmsg)
weak_alias (__libc_recvmsg, recvmsg)
libc_hidden_weak (recvmsg)
