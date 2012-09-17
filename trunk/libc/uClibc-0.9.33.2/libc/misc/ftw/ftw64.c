/* File tree walker functions.  LFS version.
   Copyright (C) 1996, 1997, 1998, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

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

#define FTW_NAME ftw64
#define NFTW_NAME nftw64
#define NFTW_OLD_NAME __old_nftw64
#define NFTW_NEW_NAME __new_nftw64
#define INO_T ino64_t
#define STAT stat64
#define LXSTAT(V,f,sb) lstat64(f,sb)
#define XSTAT(V,f,sb) stat64(f,sb)
#define FTW_FUNC_T __ftw64_func_t
#define NFTW_FUNC_T __nftw64_func_t

#include "ftw.c"
