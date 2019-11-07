/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef __SYSERRMSG_H
#define __SYSERRMSG_H 1

#include <bits/uClibc_uintmaxtostr.h>

/**********************************************************************/
/* NOTE: If we ever do internationalized syserr messages, this will
 * have to be changed! */

#if defined(__mips__) || defined(__sparc__)
/* sparce and mips have an extra error entry, as EDEADLK and EDEADLOCK have
 * different meanings on those platforms. */
# define _SYS_NERR			126
#else
# define _SYS_NERR			125
#endif

#ifdef __UCLIBC_HAS_ERRNO_MESSAGES__
# define _SYS_ERRMSG_MAXLEN	 50
#else  /* __UCLIBC_HAS_ERRNO_MESSAGES__ */
# define _SYS_ERRMSG_MAXLEN	 0
#endif /* __UCLIBC_HAS_ERRNO_MESSAGES__ */

#if _SYS_ERRMSG_MAXLEN < __UIM_BUFLEN_INT + 14
# define _STRERROR_BUFSIZE (__UIM_BUFLEN_INT + 14)
#else
# define _STRERROR_BUFSIZE _SYS_ERRMSG_MAXLEN
#endif

#endif
