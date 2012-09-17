#include <sysdep-cancel.h>
#ifndef __NR_sendmsg
#error Missing definition of NR_sendmsg needed for cancellation.
#endif
PSEUDO (__libc_sendmsg, sendmsg, 3)
ret
PSEUDO_END(__libc_sendmsg)
libc_hidden_def (__libc_sendmsg)
weak_alias (__libc_sendmsg, __sendmsg)
libc_hidden_weak (__sendmsg)
weak_alias (__libc_sendmsg, sendmsg)
libc_hidden_weak (sendmsg)
