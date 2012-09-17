#include <sysdep-cancel.h>

#ifndef __NR_waitpid
#error Makefile error: No NR_waitpid on this arch
#endif

/*
extern pid_t __waitpid_nocancel (pid_t, int *, int) attribute_hidden;
*/
#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt


PSEUDO (__waitpid, waitpid, 3)
ret
PSEUDO_END(__waitpid)

libc_hidden_def (__waitpid)
weak_alias (__waitpid, waitpid)
libc_hidden_weak (waitpid)
weak_alias (__waitpid, __libc_waitpid)
libc_hidden_weak (__libc_waitpid)

#endif
