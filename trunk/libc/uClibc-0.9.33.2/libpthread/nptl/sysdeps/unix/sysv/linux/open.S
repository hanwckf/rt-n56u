#include <sysdep-cancel.h>

/*
extern int __open_nocancel (const char *, int, ...) attribute_hidden;
*/
#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

PSEUDO (__libc_open, open, 3)
ret
PSEUDO_END(__libc_open)

libc_hidden_def (__open_nocancel)
libc_hidden_def (__libc_open)
weak_alias (__libc_open, __open)
libc_hidden_weak (__open)
weak_alias (__libc_open, open)
libc_hidden_weak (open)



#endif
