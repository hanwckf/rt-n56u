#include <sysdep-cancel.h>
PSEUDO (__libc_msync, msync, 3)
ret
PSEUDO_END(__libc_msync)
libpthread_hidden_def (__libc_msync)
weak_alias (__libc_msync, msync)
libpthread_hidden_weak (msync)
