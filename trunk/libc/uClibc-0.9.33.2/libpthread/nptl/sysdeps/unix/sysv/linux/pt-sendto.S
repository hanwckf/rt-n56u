#include <sysdep-cancel.h>
#ifndef __NR_sendto
#error Missing definition of NR_sendto needed for cancellation.
#endif
PSEUDO (__libc_sendto, sendto, 6)
ret
PSEUDO_END(__libc_sendto)
libpthread_hidden_def (__libc_sendto)
weak_alias (__libc_sendto, __sendto)
libpthread_hidden_weak (__sendto)
weak_alias (__libc_sendto, sendto)
libpthread_hidden_weak (sendto)
