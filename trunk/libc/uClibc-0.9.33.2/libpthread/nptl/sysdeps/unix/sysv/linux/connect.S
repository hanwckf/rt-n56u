#include <sysdep-cancel.h>
#ifndef __NR_connect
#error Missing definition of NR_connect needed for cancellation.
#endif
PSEUDO (__libc_connect, connect, 3)
ret
PSEUDO_END(__libc_connect)
libc_hidden_def (__libc_connect)
weak_alias (__libc_connect, __connect)
libc_hidden_weak (__connect)
weak_alias (__libc_connect, connect)
libc_hidden_weak (connect)
