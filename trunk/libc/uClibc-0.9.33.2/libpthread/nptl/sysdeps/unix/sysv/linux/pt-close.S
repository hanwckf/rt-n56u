#include <sysdep-cancel.h>
PSEUDO (__libc_close, close, 1)
ret
PSEUDO_END (__libc_close)
libpthread_hidden_def (__libc_close)
weak_alias (__libc_close, __close)
libpthread_hidden_weak (__close)
weak_alias (__libc_close, close)
libpthread_hidden_weak (close)
