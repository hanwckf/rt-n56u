/*  This file is lisenced under LGPL
 *  Copyright (C) 2002-2003,    George Thanos <george.thanos@gdt.gr>
 *                              Yannis Mitsos <yannis.mitsos@gdt.gr>
 */

#include <syscall.h>

/* We now need a declaration of the `errno' variable.  */
extern int errno;
#   define __set_errno(val) ((errno) = (val))
_syscall2( int, kprintf, char *, msg, int, len)
