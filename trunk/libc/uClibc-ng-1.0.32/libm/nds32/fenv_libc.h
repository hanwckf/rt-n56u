/*
 * Copyright (C) 2016-2017 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Copyright (C) 2000-2013 Free Software Foundation, Inc.
   Contributed by Alexandre Oliva <aoliva@redhat.com>
   based on the corresponding file in the mips port.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _FENV_LIBC_H
#define _FENV_LIBC_H

/* Mask for enabling exceptions and for the CAUSE bits.  */
#define ENABLE_MASK	0x00F80U

/* Shift for FE_* flags to get up to the ENABLE bits.  */
#define	ENABLE_SHIFT	5

#endif /* _FENV_LIBC_H */
