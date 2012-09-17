#include <sysdep-cancel.h>

/*
extern int __read_nocancel (int, void *, size_t) attribute_hidden;
*/
#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

PSEUDO (__libc_read, read, 3)
ret
PSEUDO_END(__libc_read)

libc_hidden_def (__read_nocancel)
libc_hidden_def (__libc_read)
weak_alias (__libc_read, __read)
libc_hidden_weak (__read)
weak_alias (__libc_read, read)
libc_hidden_weak (read)

#endif
