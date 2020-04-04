/* strncpy implementation for SUPERH
 *
 * Copyright (C) 2009 STMicroelectronics Ltd.
 *
 * Author: Giuseppe Cavallaro <peppe.cavallaro@st.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/*
	char *strncpy(char *dest, const char *src, size_t n);
 */

#include <sysdep.h>

ENTRY(strncpy)
	mov	#0,r0
	bra	2f
	 mov	r4,r2
1:
	mov.b	r1,@(r0,r2)
	add	#1,r0
2:
	cmp/hs	r6,r0
	bt	5f
	mov.b	@(r0,r5),r1
	tst	r1,r1
	bf/s	1b
	 cmp/hs	r6,r0
	bra	4f
	 nop
3:
	mov.b	r1,@(r0,r2)
	add	#1,r0
	cmp/hs	r6,r0
4:
	bf/s	3b
	 mov	#0,r1
5:
	rts
	 mov     r2,r0
END(strncpy)
libc_hidden_def(strncpy)
