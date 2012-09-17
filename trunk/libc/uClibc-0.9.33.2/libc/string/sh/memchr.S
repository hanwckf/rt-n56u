/* $Id: memchr.S,v 1.1 2000/04/14 16:49:01 mjd Exp $
 *
 * "memchr" implementation of SuperH
 *
 * Copyright (C) 1999  Niibe Yutaka
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/*
 * void *memchr(const void *s, int c, size_t n);
 */

#include <sysdep.h>

ENTRY(memchr)
	tst	r6,r6
	bt/s	2f
	 exts.b	r5,r5
1:	mov.b	@r4,r1
	cmp/eq	r1,r5
	bt/s	3f
	 dt	r6
	bf/s	1b
	 add	#1,r4
2:	mov	#0,r4
3:	rts
	 mov	r4,r0
END(memchr)
libc_hidden_def (memchr)
