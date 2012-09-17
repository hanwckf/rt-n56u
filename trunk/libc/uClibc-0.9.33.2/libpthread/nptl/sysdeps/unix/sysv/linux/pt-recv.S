#include <sysdep-cancel.h>
#ifndef __NR_recv
#error Missing definition of NR_recv needed for cancellation.
#endif
PSEUDO (__libc_recv, recv, 4)
ret
PSEUDO_END(__libc_recv)
libpthread_hidden_def (__libc_recv)
weak_alias (__libc_recv, __recv)
libpthread_hidden_weak (__recv)
weak_alias (__libc_recv, recv)
libpthread_hidden_weak (recv)
