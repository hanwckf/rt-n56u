#include <sysdep-cancel.h>
PSEUDO (__libc_lseek, lseek, 3)
ret
PSEUDO_END (__libc_lseek)
libpthread_hidden_def (__libc_lseek)
weak_alias (__libc_lseek, lseek)
libpthread_hidden_weak (lseek)
