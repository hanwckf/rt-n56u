/* _flush_cache system call for Linux/MIPS.
   Copyright (C) 2003 Christophe Massiot <cmassiot@freebox.fr>

   The uClibc Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The uClibc Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the uClibc Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sys/syscall.h>

#ifdef __NR_cacheflush
# include <sys/cachectl.h>
_syscall3(int, cacheflush, void *, addr, const int, nbytes, const int, op)
strong_alias_untyped(cacheflush, _flush_cache)
#endif

#ifdef __NR_cachectl
# include <sys/cachectl.h>
_syscall3(int, cachectl, void *, addr, const int, nbytes, const int, op)
#endif
