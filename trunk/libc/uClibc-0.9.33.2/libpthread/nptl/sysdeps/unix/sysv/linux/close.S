#include <sysdep-cancel.h>

/*
extern int __close_nocancel (int) attribute_hidden;
*/
#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt


PSEUDO (__libc_close, close, 1)
ret
PSEUDO_END(__libc_close)

libc_hidden_def (__close_nocancel)
libc_hidden_def (__libc_close)
weak_alias (__libc_close, __close)
libc_hidden_weak (__close)
weak_alias (__libc_close, close)
libc_hidden_weak (close)


#endif
