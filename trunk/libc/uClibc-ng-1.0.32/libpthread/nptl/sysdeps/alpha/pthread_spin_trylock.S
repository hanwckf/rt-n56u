/* Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson  <rth@twiddle.net>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */


#define _ERRNO_H 1
#include <bits/errno.h>

	.text
	.align	4

	.globl	pthread_spin_trylock
	.ent	pthread_spin_trylock
pthread_spin_trylock:
	.frame	$sp, 0, $26, 0
	.prologue 0

0:	ldl_l	$1, 0($16)
	lda	$2, 1
	lda	$0, EBUSY
	bne	$1, 1f

	stl_c	$2, 0($16)
	beq	$2, 2f
	mb
	lda	$0, 0

1:	ret
2:	br	0b

	.end	pthread_spin_trylock
