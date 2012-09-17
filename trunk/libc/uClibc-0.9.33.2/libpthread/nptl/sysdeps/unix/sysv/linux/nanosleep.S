#include <sysdep-cancel.h>
PSEUDO (__libc_nanosleep, nanosleep, 3)
ret
PSEUDO_END (__libc_nanosleep)
libc_hidden_def (__libc_nanosleep)
weak_alias (__libc_nanosleep, __nanosleep)
libc_hidden_weak (__nanosleep)
weak_alias (__libc_nanosleep, nanosleep)
libc_hidden_weak (nanosleep)
