/* strcpy implementation for SUPERH
 *
 * Copyright (C) 2009 STMicroelectronics Ltd.
 *
 * Author: Giuseppe Cavallaro <peppe.cavallaro@st.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/*
	char *strcpy(char *dest, const char *src);
 */

#include <sysdep.h>

ENTRY(strcpy)
	mov	r4,r2
1:
	mov.b	@r5+,r1
	tst	r1,r1
	mov.b	r1,@r2
	bf/s	1b
	 add	#1,r2

	rts
	 mov	r4,r0
END(strcpy)
libc_hidden_def (strcpy)
