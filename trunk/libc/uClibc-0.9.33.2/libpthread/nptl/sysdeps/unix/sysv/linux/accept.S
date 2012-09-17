#include <sysdep-cancel.h>
#ifndef __NR_accept
#error Missing definition of NR_accept needed for cancellation.
#endif
PSEUDO (__libc_accept, accept, 3)
ret
PSEUDO_END(__libc_accept)
libc_hidden_def (__libc_accept)
weak_alias (__libc_accept, __accept)
libc_hidden_weak (__accept)
weak_alias (__libc_accept, accept)
libc_hidden_weak (accept)
