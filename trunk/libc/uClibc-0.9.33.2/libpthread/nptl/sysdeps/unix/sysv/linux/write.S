#include <sysdep-cancel.h>

/*
extern int __write_nocancel (int, const void *, size_t) attribute_hidden;
*/
#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

PSEUDO (__libc_write, write, 3)
ret
PSEUDO_END(__libc_write)

libc_hidden_def (__write_nocancel)
libc_hidden_def (__libc_write)
weak_alias (__libc_write, __write)
libc_hidden_weak (__write)
weak_alias (__libc_write, write)
libc_hidden_weak (write)

#endif
