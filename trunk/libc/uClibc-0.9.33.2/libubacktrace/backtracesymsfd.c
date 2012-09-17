/* Write formatted list with names for addresses in backtrace to a file.
   Copyright (C) 1998, 2000, 2003, 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

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
   02111-1307 USA.

   Based on glibc/sysdeps/generic/elf/backtracesymsfd.c

   Copyright (C) 2010 STMicroelectronics Ltd
   Author(s): Carmelo Amoroso <carmelo.amoroso@st.com>
   * Modified to work with uClibc
     - updated headers inclusion
     - updated formatting and style
     - updated to use dladdr from libdl
     - updated to use snprintf instead of _itoa_word */

#include <execinfo.h>
#include <string.h>
#include <sys/uio.h>
#include <dlfcn.h>
#include <stdio.h>
#include <link.h>	/* required for __ELF_NATIVE_CLASS */

#if __ELF_NATIVE_CLASS == 32
# define WORD_WIDTH 8
#else
/* We assyme 64bits.  */
# define WORD_WIDTH 16
#endif

#define BUF_SIZE (WORD_WIDTH + 1)

void backtrace_symbols_fd (void *const *array, int size, int fd)
{
	struct iovec iov[9];
	int cnt;

	for (cnt = 0; cnt < size; ++cnt) {
		char buf[BUF_SIZE];
		Dl_info info;
		size_t last = 0;
		size_t len = 0;

		memset(buf, 0, sizeof(buf));
		if (dladdr (array[cnt], &info)
			&& info.dli_fname && info.dli_fname[0] != '\0')	{
			/* Name of the file.  */
			iov[0].iov_base = (void *) info.dli_fname;
			iov[0].iov_len = strlen (info.dli_fname);
			last = 1;

			/* Symbol name.  */
			if (info.dli_sname != NULL) {
				char buf2[BUF_SIZE];
				memset(buf2, 0, sizeof(buf2));
				size_t diff;

				iov[1].iov_base = (void *) "(";
				iov[1].iov_len = 1;
				iov[2].iov_base = (void *) info.dli_sname;
				iov[2].iov_len = strlen (info.dli_sname);

				if (array[cnt] >= (void *) info.dli_saddr) {
					iov[3].iov_base = (void *) "+0x";
					diff = array[cnt] - info.dli_saddr;
				} else {
					iov[3].iov_base = (void *) "-0x";
					diff = info.dli_saddr - array[cnt];
				}

				iov[3].iov_len = 3;

				/* convert diff to a string in hex format */
				len = snprintf(buf2, sizeof(buf2), "%lx", (unsigned long) diff);
				iov[4].iov_base = buf2;
				iov[4].iov_len = len;

				iov[5].iov_base = (void *) ")";
				iov[5].iov_len = 1;

				last = 6;
			}
		}

		iov[last].iov_base = (void *) "[0x";
		iov[last].iov_len = 3;
		++last;

		/* convert array[cnt] to a string in hex format */
		len = snprintf(buf, sizeof(buf), "%lx", (unsigned long) array[cnt]);
		iov[last].iov_base = buf;
		iov[last].iov_len = len;

		++last;

		iov[last].iov_base = (void *) "]\n";
		iov[last].iov_len = 2;
		++last;

		writev (fd, iov, last);
	}
}
