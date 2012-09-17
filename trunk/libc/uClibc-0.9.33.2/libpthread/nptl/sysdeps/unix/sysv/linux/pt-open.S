#include <sysdep-cancel.h>
PSEUDO (__libc_open, open, 3)
ret
PSEUDO_END (__libc_open)
libpthread_hidden_def (__libc_open)
weak_alias (__libc_open, __open)
libpthread_hidden_weak (__open)
weak_alias (__libc_open, open)
libpthread_hidden_weak (open)
