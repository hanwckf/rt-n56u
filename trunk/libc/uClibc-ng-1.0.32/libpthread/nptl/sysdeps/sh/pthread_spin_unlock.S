/* Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

	.globl	pthread_spin_unlock
	.type	pthread_spin_unlock,@function
	.align	5
pthread_spin_unlock:
	mov	#0,r0
	rts
	 mov.l	r0,@r4
	.size	pthread_spin_unlock,.-pthread_spin_unlock

	/* The implementation of pthread_spin_init is identical.  */
	.globl	pthread_spin_init
pthread_spin_init = pthread_spin_unlock
