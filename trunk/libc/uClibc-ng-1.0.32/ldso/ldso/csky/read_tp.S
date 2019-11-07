/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

#include <sysdep.h>

#ifndef NOT_IN_libc
.global __read_tp
#else
.hidden __read_tp
#endif

ENTRY (__read_tp)
#ifdef __CSKYABIV2__
	mov	a0, tls
#else
	trap 3
#endif
	rts
END (__read_tp)

