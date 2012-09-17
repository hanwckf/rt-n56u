#include <sysdep-cancel.h>
PSEUDO (__libc_write, write, 3)
ret
PSEUDO_END (__libc_write)
libpthread_hidden_def (__libc_write)
weak_alias (__libc_write, __write)
libpthread_hidden_weak (__write)
weak_alias (__libc_write, write)
libpthread_hidden_weak (write)
