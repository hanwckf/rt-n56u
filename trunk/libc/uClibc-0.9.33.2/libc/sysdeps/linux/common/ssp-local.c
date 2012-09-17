/* Copyright (C) 2005 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/*
 * Peter S. Mazinger ps.m[@]gmx.net
 * copied stack_chk_fail_local.c from glibc and adapted for uClibc
 */

#include <features.h>

extern void __stack_chk_fail (void) attribute_noreturn;

/* On some architectures, this helps needless PIC pointer setup
   that would be needed just for the __stack_chk_fail call.  */

void __stack_chk_fail_local (void) attribute_noreturn attribute_hidden;
void __stack_chk_fail_local (void)
{
  __stack_chk_fail ();
}
