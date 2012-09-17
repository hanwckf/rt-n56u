#include <sysdep-cancel.h>
PSEUDO (__libc_pause, pause, 0)
ret
PSEUDO_END (__libc_pause)
libpthread_hidden_def (__libc_pause)
weak_alias (__libc_pause, pause)
libpthread_hidden_weak (pause)
