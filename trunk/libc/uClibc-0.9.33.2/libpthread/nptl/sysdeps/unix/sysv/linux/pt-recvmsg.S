#include <sysdep-cancel.h>
#ifndef __NR_recvmsg
#error Missing definition of NR_recvmsg needed for cancellation.
#endif
PSEUDO (__libc_recvmsg, recvmsg, 3)
ret
PSEUDO_END(__libc_recvmsg)
libpthread_hidden_def (__libc_recvmsg)
weak_alias (__libc_recvmsg, __recvmsg)
libpthread_hidden_weak (__recvmsg)
weak_alias (__libc_recvmsg, recvmsg)
libpthread_hidden_weak (recvmsg)
