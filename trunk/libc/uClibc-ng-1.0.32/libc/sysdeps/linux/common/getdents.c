/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <bits/wordsize.h>

#if !(defined __NR_getdents64 && __WORDSIZE == 64)

#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <bits/kernel_types.h>
#include <bits/kernel-features.h>

/* If the condition above is met, __getdents is defined as an alias
 * for __getdents64 (see getdents64.c). Otherwise...
 */

/* With newer versions of linux, the getdents syscall returns d_type
 * information after the name field.
 *
 * See __ASSUME_GETDENTS32_D_TYPE in glibc's kernel-features.h for specific
 * version / arch details.
 */

# ifdef __ARCH_HAS_DEPRECATED_SYSCALLS__
struct kernel_dirent
{
	long int d_ino;
	__kernel_off_t d_off;
	unsigned short int d_reclen;
	char d_name[256];
};
# else
#  define kernel_dirent dirent
# endif

# if defined __NR_getdents
#  define __NR___syscall_getdents __NR_getdents
static __always_inline _syscall3(int, __syscall_getdents, int, fd, unsigned char *, kdirp, size_t, count)
# endif

# if defined __ASSUME_GETDENTS32_D_TYPE && defined __NR_getdents

ssize_t __getdents (int fd, char *buf, size_t nbytes)
{
	ssize_t retval;

	retval = __syscall_getdents(fd, (unsigned char *)buf, nbytes);

	/* The kernel added the d_type value after the name.  Change
	this now.  */
	if (retval != -1) {
		union {
			struct kernel_dirent k;
			struct dirent u;
		} *kbuf = (void *) buf;

		while ((char *) kbuf < buf + retval) {
			char d_type = *((char *) kbuf + kbuf->k.d_reclen - 1);
			memmove (kbuf->u.d_name, kbuf->k.d_name,
			strlen (kbuf->k.d_name) + 1);
			kbuf->u.d_type = d_type;

			kbuf = (void *) ((char *) kbuf + kbuf->k.d_reclen);
		}
	}

	return retval;
}

# elif !defined __NR_getdents64

#  include <assert.h>
#  include <stddef.h>
#  include <errno.h>
#  include <unistd.h>
#  include <sys/param.h>
#  include <bits/uClibc_alloc.h>

ssize_t __getdents (int fd, char *buf, size_t nbytes)
{
    struct dirent *dp;
    off_t last_offset = -1;
    ssize_t retval;
    size_t red_nbytes;
    struct kernel_dirent *skdp, *kdp;
    const size_t size_diff = (offsetof (struct dirent, d_name)
	    - offsetof (struct kernel_dirent, d_name));

#  ifndef __ARCH_HAS_DEPRECATED_SYSCALLS__
    red_nbytes = MIN (nbytes - ((nbytes /
		    (offsetof (struct dirent, d_name) + 14)) * size_diff),
	    nbytes - size_diff);

    dp = (struct dirent *) buf;
    skdp = kdp = stack_heap_alloc(red_nbytes);

    retval = __syscall_getdents(fd, (unsigned char *)kdp, red_nbytes);
#  else

    dp = (struct dirent *) buf;
    skdp = kdp = stack_heap_alloc(nbytes);

    retval = INLINE_SYSCALL(getdents64, 3, fd, (unsigned char *)kdp, nbytes);
    if (retval > 0) {
	    /* Did we overflow? */
	    if (kdp->__pad1 || kdp->__pad2) {
		    __set_errno(EINVAL);
		    return -1;
	    }
    }
#  endif

    if (retval == -1) {
	stack_heap_free(skdp);
	return -1;
    }

    while ((char *) kdp < (char *) skdp + retval) {
	const size_t alignment = __alignof__ (struct dirent);
	/* Since kdp->d_reclen is already aligned for the kernel structure
	   this may compute a value that is bigger than necessary.  */
	size_t new_reclen = ((kdp->d_reclen + size_diff + alignment - 1)
		& ~(alignment - 1));
	if ((char *) dp + new_reclen > buf + nbytes) {
	    /* Our heuristic failed.  We read too many entries.  Reset
	       the stream.  */
	    assert (last_offset != -1);
	    lseek(fd, last_offset, SEEK_SET);

	    if ((char *) dp == buf) {
		/* The buffer the user passed in is too small to hold even
		   one entry.  */
		stack_heap_free(skdp);
		__set_errno (EINVAL);
		return -1;
	    }
	    break;
	}

	last_offset = kdp->d_off;
	dp->d_ino = kdp->d_ino;
	dp->d_off = kdp->d_off;
	dp->d_reclen = new_reclen;
	dp->d_type = DT_UNKNOWN;
	memcpy (dp->d_name, kdp->d_name,
		kdp->d_reclen - offsetof (struct kernel_dirent, d_name));
	dp = (struct dirent *) ((char *) dp + new_reclen);
	kdp = (struct kernel_dirent *) (((char *) kdp) + kdp->d_reclen);
    }
    stack_heap_free(skdp);
    return (char *) dp - buf;
}

# elif __WORDSIZE == 32 && !defined __NR_getdents64

#  include <stddef.h>

ssize_t __getdents (int fd, char *buf, size_t nbytes)
{
    struct dirent *dp;
    struct dirent64 *dp64;
    ssize_t ret = __getdents64 (fd, buf, nbytes);

    if (ret <= 0)
	return ret;

    dp64 = (struct dirent64 *) buf;
    buf += ret;
    while ((void *) dp64 < (void *) buf) {
	dp = (struct dirent *) dp64;
	dp->d_ino = dp64->d_ino;
	dp->d_off = dp64->d_off;
	dp->d_reclen = dp64->d_reclen;
	dp->d_type = dp64->d_type;
	memmove (dp->d_name, dp64->d_name, dp->d_reclen - offsetof (struct dirent64, d_name));
	memmove (dp64, dp, dp->d_reclen);
	dp64 = ((void *) dp64) + dp->d_reclen;
    }

    return ret;
}

# endif

# if ! defined __NR_getdents64
strong_alias(__getdents,__getdents64)
# endif

#endif
