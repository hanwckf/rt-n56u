#include <sysdep-cancel.h>
#ifndef __NR_send
#error Missing definition of NR_send needed for cancellation.
#endif
PSEUDO (__libc_send, send, 4)
ret
PSEUDO_END (__libc_send)
libpthread_hidden_def (__libc_send)
weak_alias (__libc_send, __send)
libpthread_hidden_weak (__send)
weak_alias (__libc_send, send)
libpthread_hidden_weak (send)
