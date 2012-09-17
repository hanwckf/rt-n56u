/* Determine whether the host has multiple processors.  Linux version.
   Copyright (C) 1996, 2002, 2004, 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* Test whether the machine has more than one processor.  This is not the
   best test but good enough.  More complicated tests would require `malloc'
   which is not available at that time.  */
static inline int
is_smp_system (void)
{
  /* Assume all machines are SMP and/or CMT and/or SMT.  */
  return 1;
}
