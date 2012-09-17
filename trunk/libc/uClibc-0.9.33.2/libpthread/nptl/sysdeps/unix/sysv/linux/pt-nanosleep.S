#include <sysdep-cancel.h>
PSEUDO (__libc_nanosleep, nanosleep, 3)
ret
PSEUDO_END (__libc_nanosleep)
libpthread_hidden_def (__libc_nanosleep)
weak_alias (__libc_nanosleep, __nanosleep)
libpthread_hidden_weak (__nanosleep)
weak_alias (__libc_nanosleep, nanosleep)
libpthread_hidden_weak (nanosleep)
