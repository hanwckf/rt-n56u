/* Xtensa configuration settings.
   Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007
   Free Software Foundation, Inc.
   Contributed by Bob Wilson (bwilson@tensilica.com) at Tensilica.

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

#ifndef XTENSA_CONFIG_H
#define XTENSA_CONFIG_H

/* The macros defined here match those with the same names in the Xtensa
   compile-time HAL (Hardware Abstraction Layer).  Please refer to the
   Xtensa System Software Reference Manual for documentation of these
   macros.  */

/* The following macros reflect the default expectations for Xtensa
   processor configurations that can run glibc.  If you want to try
   building glibc for an Xtensa configuration that is missing these
   options, you will at least need to change the values of these
   macros.  */

#undef XCHAL_HAVE_NSA
#define XCHAL_HAVE_NSA			1

#undef XCHAL_HAVE_LOOPS
#define XCHAL_HAVE_LOOPS		1

/* Assume the maximum number of AR registers.  This currently only affects
   the __window_spill function, and it is always safe to flush extra.  */

#undef XCHAL_NUM_AREGS
#define XCHAL_NUM_AREGS			64

#endif /* !XTENSA_CONFIG_H */
