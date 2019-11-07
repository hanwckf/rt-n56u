/* Set floating-point environment exception handling.
   Copyright (C) 1997,99,2000,01,04 Free Software Foundation, Inc.
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

#include "fenv_libc.h"

int
__fesetexceptflag (const fexcept_t *flagp, int excepts)
{
  unsigned long spefscr;
  fexcept_t flag;

  /* Get the current state.  */
  spefscr = fegetenv_register ();

  /* Ignore exceptions not listed in 'excepts'.  */
  flag = *flagp & excepts;

  /* Replace the exception status */
  spefscr = (spefscr & ~FE_ALL_EXCEPT) | flag;

  /* Store the new status word (along with the rest of the environment).
     This may cause floating-point exceptions if the restored state
     requests it.  */
  fesetenv_register (spefscr);
  feraiseexcept (spefscr & FE_ALL_EXCEPT);

  /* Success.  */
  return 0;
}

