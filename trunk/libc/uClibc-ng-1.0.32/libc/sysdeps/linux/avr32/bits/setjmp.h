/*
 * Copyright (C) 2004-2005 Atmel Corporation
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.  See the file "COPYING.LIB" in the main directory of this
 * archive for more details.
 */
#ifndef _BITS_SETJMP_H
#define _BITS_SETJMP_H	1

#if !defined _SETJMP_H && !defined _PTHREAD_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

/*
 * The jump buffer contains r0-r7, sr, sp and lr. Other registers are
 * not saved.
 */
typedef int __jmp_buf[11];

#endif /* _BITS_SETJMP_H */
