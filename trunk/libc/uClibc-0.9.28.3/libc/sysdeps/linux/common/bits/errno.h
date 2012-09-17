/* Error constants.  Linux specific version.
   Copyright (C) 1996, 1997, 1998, 1999, 2002 Free Software Foundation, Inc.
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

#ifdef _ERRNO_H

# include <bits/errno_values.h>

#ifndef ENOTSUP
# define ENOTSUP EOPNOTSUPP
#endif

#ifndef ECANCELED
# define ECANCELED	125
#endif

# ifndef __ASSEMBLER__

/* We now need a declaration of the `errno' variable.  */
extern int errno;

/* Function to get address of global `errno' variable.  */
extern int *__errno_location (void) __THROW __attribute__ ((__const__));

#  if defined _LIBC
/* We wouldn't need a special macro anymore but it is history.  */
#   define __set_errno(val) ((errno) = (val))
#  endif /* _LIBC */

#  if defined __UCLIBC_HAS_THREADS__
/* When using threads, errno is a per-thread value.  */
#   define errno (*__errno_location ())
#  endif

# endif /* !__ASSEMBLER__ */
#endif /* _ERRNO_H */

