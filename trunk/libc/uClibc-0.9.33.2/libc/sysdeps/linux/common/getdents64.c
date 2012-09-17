/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <features.h>
#include <assert.h>
#include <errno.h>
#include <dirent.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <bits/kernel_types.h>
#include <bits/uClibc_alloc.h>

#if defined __UCLIBC_HAS_LFS__ && defined __NR_getdents64

# ifndef offsetof
#  define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
# endif

struct kernel_dirent64
{
    uint64_t		d_ino;
    int64_t		d_off;
    unsigned short	d_reclen;
    unsigned char	d_type;
    char		d_name[256];
};

# define __NR___syscall_getdents64 __NR_getdents64
static __inline__ _syscall3(int, __syscall_getdents64, int, fd, unsigned char *, dirp, size_t, count)

ssize_t __getdents64 (int fd, char *buf, size_t nbytes) attribute_hidden;
ssize_t __getdents64 (int fd, char *buf, size_t nbytes)
{
    struct dirent64 *dp;
    off64_t last_offset = -1;
    ssize_t retval;
    size_t red_nbytes;
    struct kernel_dirent64 *skdp, *kdp;
    const size_t size_diff = (offsetof (struct dirent64, d_name)
	    - offsetof (struct kernel_dirent64, d_name));

    red_nbytes = MIN (nbytes - ((nbytes /
		    (offsetof (struct dirent64, d_name) + 14)) * size_diff),
	    nbytes - size_diff);

    dp = (struct dirent64 *) buf;
    skdp = kdp = stack_heap_alloc(red_nbytes);

    retval = __syscall_getdents64(fd, (unsigned char *)kdp, red_nbytes);
    if (retval == -1) {
	stack_heap_free(skdp);
	return -1;
    }

    while ((char *) kdp < (char *) skdp + retval) {
	const size_t alignment = __alignof__ (struct dirent64);
	/* Since kdp->d_reclen is already aligned for the kernel structure
	   this may compute a value that is bigger than necessary.  */
	size_t new_reclen = ((kdp->d_reclen + size_diff + alignment - 1)
		& ~(alignment - 1));
	if ((char *) dp + new_reclen > buf + nbytes) {
	    /* Our heuristic failed.  We read too many entries.  Reset
	       the stream.  */
	    assert (last_offset != -1);
	    lseek64(fd, last_offset, SEEK_SET);

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
	dp->d_type = kdp->d_type;
	memcpy (dp->d_name, kdp->d_name,
		kdp->d_reclen - offsetof (struct kernel_dirent64, d_name));
	dp = (struct dirent64 *) ((char *) dp + new_reclen);
	kdp = (struct kernel_dirent64 *) (((char *) kdp) + kdp->d_reclen);
    }
    stack_heap_free(skdp);
    return (char *) dp - buf;
}

#if __WORDSIZE == 64
/* since getdents doesnt give us d_type but getdents64 does, try and
 * use getdents64 as much as possible */
attribute_hidden strong_alias(__getdents64,__getdents)
#endif

#endif
