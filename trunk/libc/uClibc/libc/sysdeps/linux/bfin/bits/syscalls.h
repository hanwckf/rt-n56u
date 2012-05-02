#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

#include <features.h>

/* Do something very evil for now.  Until we create our own syscall
 * macros, short circuit bits/sysnum.h  and use asm/unistd.h instead */
#include <asm/unistd.h>

/* This includes the `__NR_<name>' syscall numbers taken from the Linux kernel
 * header files.  It also defines the traditional `SYS_<name>' macros for older
 * programs.  */
#include <bits/sysnum.h>

#endif /* _BITS_SYSCALLS_H */

