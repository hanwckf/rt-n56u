#include <sysdep-cancel.h>
PSEUDO (__libc_read, read, 3)
ret
PSEUDO_END (__libc_read)
libpthread_hidden_def (__libc_read)
weak_alias (__libc_read, __read)
libpthread_hidden_weak (__read)
weak_alias (__libc_read, read)
libpthread_hidden_weak (read)
