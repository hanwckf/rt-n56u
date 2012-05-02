#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

#include <features.h>
/* Do something very evil for now.  Until we include our out syscall
 * macros, short circuit bits/syscall.h  and use asm/unistd.h instead */
#include <asm/unistd.h>
#endif /* _BITS_SYSCALLS_H */

