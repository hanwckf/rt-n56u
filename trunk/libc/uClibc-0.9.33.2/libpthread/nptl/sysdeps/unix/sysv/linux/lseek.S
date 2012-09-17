#include <sysdep-cancel.h>
PSEUDO (__libc_lseek, lseek, 3)
ret
PSEUDO_END (__libc_lseek)
libc_hidden_def (__libc_lseek)
weak_alias (__libc_lseek, lseek)
libc_hidden_weak (lseek)
