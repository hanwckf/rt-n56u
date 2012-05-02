/* This file is licensed under LGPL.
 * Copyright (C) 2002-2003,    George Thanos <george.thanos@gdt.gr>
 *                             Yannis Mitsos <yannis.mitsos@gdt.gr>
 */
#ifndef _BITS_SYSCALLS_H
#define _BITS_SYSCALLS_H
#ifndef _SYSCALL_H
# error "Never use <bits/syscalls.h> directly; include <sys/syscall.h> instead."
#endif

#include <features.h>

/* This includes the `__NR_<name>' syscall numbers taken from the Linux kernel
 * header files.  It also defines the traditional `SYS_<name>' macros for older
 * programs.  */
#include <bits/sysnum.h>

/* Include the library _syscallx macros */
#include <bits/unistd.h>

#endif /* _BITS_SYSCALLS_H */

