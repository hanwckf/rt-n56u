/* Copyright (C) 2007 Free Software Foundation, Inc.
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

#include <limits.h>
#include <sched.h>


int
__sched_cpucount (size_t setsize, const cpu_set_t *setp)
{
  int s = 0;
  const __cpu_mask *p = setp->__bits;
  const __cpu_mask *end = &setp->__bits[setsize / sizeof (__cpu_mask)];

  while (p < end)
    {
      __cpu_mask l = *p++;

#ifdef POPCNT
      s += POPCNT (l);
#else
      if (l == 0)
	continue;

# if LONG_BIT > 32
      l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
      l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
      l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
      l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
      l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
      l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# else
      l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
      l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
      l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
      l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
      l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif

      s += l;
#endif
    }

  return s;
}
