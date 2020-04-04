/*
 * posix_fadvise64() for uClibc
 * http://www.opengroup.org/onlinepubs/009695399/functions/posix_fadvise.html
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <_lfs_64.h>
#include <sys/syscall.h>
#include <bits/wordsize.h>

#ifdef __NR_arm_fadvise64_64
# define __NR_fadvise64_64 __NR_arm_fadvise64_64
#endif

#if defined __NR_fadvise64_64 && __WORDSIZE == 32
# include <fcntl.h>
# include <endian.h>

int posix_fadvise64(int fd, off64_t offset, off64_t len, int advice)
{
	int ret;
	INTERNAL_SYSCALL_DECL (err);
	/* ARM has always been funky. */
#if defined (__arm__) || defined (__nds32__) || defined (__csky__) || \
    (defined(__UCLIBC_SYSCALL_ALIGN_64BIT__) && (defined(__powerpc__) || defined(__xtensa__)))
	/* arch with 64-bit data in even reg alignment #1: [powerpc/xtensa]
	 * custom syscall handler (rearranges @advice to avoid register hole punch) */
	ret = INTERNAL_SYSCALL (fadvise64_64, err, 6, fd, advice,
			OFF64_HI_LO (offset), OFF64_HI_LO (len));
#elif defined(__UCLIBC_SYSCALL_ALIGN_64BIT__)
	/* arch with 64-bit data in even reg alignment #2: [arcv2/others-in-future]
	 * stock syscall handler in kernel (reg hole punched) */
	ret = INTERNAL_SYSCALL (fadvise64_64, err, 7, fd, 0,
			OFF64_HI_LO (offset), OFF64_HI_LO (len),
			advice);
# else
	ret = INTERNAL_SYSCALL (fadvise64_64, err, 6, fd,
			OFF64_HI_LO (offset), OFF64_HI_LO (len),
			advice);
# endif
	if (INTERNAL_SYSCALL_ERROR_P (ret, err))
		return INTERNAL_SYSCALL_ERRNO (ret, err);
	return 0;
}
#endif
