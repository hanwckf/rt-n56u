/*
 * Copyright (C) 2013 Imagination Technologies Ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <errno.h>
#include <sys/syscall.h>

_syscall2(int,metag_setglobalbit,char *,addr,int,mask)
_syscall1(void,metag_set_fpu_flags,unsigned int,flags)
