/* Set current rounding direction.
   Copyright (C) 2004, 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Aldy Hernandez <aldyh@redhat.com>, 2004.

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

#include "fenv_libc.h"

int
fesetround (int round)
{
  unsigned long fpescr;

  if ((unsigned int) round > 3)
    return 1;

  fpescr = fegetenv_register ();
  fpescr = (fpescr & ~SPEFSCR_FRMC) | (round & 3);
  fesetenv_register (fpescr);

  return 0;
}
libm_hidden_def (fesetround)
