#include <sysdep-cancel.h>
PSEUDO (__libc_creat, creat, 2)
ret
PSEUDO_END(__libc_creat)
libc_hidden_def (__libc_creat)
weak_alias (__libc_creat, creat)
libc_hidden_weak (creat)
