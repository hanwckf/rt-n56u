/*
 * system call not available stub
 * based on libc's stubs.c
 *
 * Copyright (C) 2009 Analog Devices Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <errno.h>
#include <sys/syscall.h>

#ifdef __UCLIBC_HAS_STUBS__

static int rt_enosys_stub(void) __attribute_used__;
static int rt_enosys_stub(void)
{
	__set_errno(ENOSYS);
	return -1;
}

#define make_stub(stub) \
	link_warning(stub, #stub ": this function is not implemented") \
	strong_alias(rt_enosys_stub, stub)

#ifndef __NR_mq_timedreceive
make_stub(mq_receive)
# ifdef __UCLIBC_HAS_ADVANCED_REALTIME__
make_stub(mq_timedreceive)
# endif
#endif

#ifndef __NR_mq_timedsend
make_stub(mq_send)
# ifdef __UCLIBC_HAS_ADVANCED_REALTIME__
make_stub(mq_timedsend)
# endif
#endif

#endif
