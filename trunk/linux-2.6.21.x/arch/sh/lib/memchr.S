/* $Id: memchr.S,v 1.1.1.1 2007-05-25 06:50:20 bruce Exp $
 *
 * "memchr" implementation of SuperH
 *
 * Copyright (C) 1999  Niibe Yutaka
 *
 */

/*
 * void *memchr(const void *s, int c, size_t n);
 */

#include <linux/linkage.h>
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
