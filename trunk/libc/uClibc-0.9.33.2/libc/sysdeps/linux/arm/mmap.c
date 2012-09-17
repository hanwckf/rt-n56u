/* vi: set sw=4 ts=4: */
/*
 * _mmap() for uClibc
 *
 * Copyright (C) 2000-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/syscall.h>

#if defined (__NR_mmap) || defined (__NR_mmap2)

libc_hidden_proto(mmap)
#if defined (__UCLIBC_MMAP_HAS_6_ARGS__) && defined (__NR_mmap)
#define __NR__mmap __NR_mmap
static __inline__ _syscall6 (__ptr_t, _mmap, __ptr_t, addr, size_t, len,
                         int, prot, int, flags, int, fd, __off_t, offset)
__ptr_t mmap(__ptr_t addr, size_t len, int prot,
             int flags, int fd, __off_t offset)
{
  return (__ptr_t) _mmap (addr, len, prot, flags,
                          fd, offset);
}

#elif defined  (__NR_mmap2)
#define __NR__mmap __NR_mmap2

#ifndef MMAP2_PAGE_SHIFT
# define MMAP2_PAGE_SHIFT 12
#endif

static __inline__ _syscall6 (__ptr_t, _mmap, __ptr_t, addr, size_t, len,
                         int, prot, int, flags, int, fd, __off_t, offset);
__ptr_t mmap(__ptr_t addr, size_t len, int prot,
             int flags, int fd, __off_t offset)
{
  /* check if offset is page aligned */
    if (offset & ((1 << MMAP2_PAGE_SHIFT) - 1))
    {
        __set_errno(EINVAL);
        return MAP_FAILED;
    }
#ifdef __USE_FILE_OFFSET64
    return (__ptr_t) _mmap (addr, len, prot, flags,
                            fd, ((__u_quad_t) offset >> MMAP2_PAGE_SHIFT));
#else
    return (__ptr_t) _mmap (addr, len, prot, flags,
                            fd, ((__u_long) offset >> MMAP2_PAGE_SHIFT));
#endif
}
#elif defined (__NR_mmap)
# define __NR__mmap __NR_mmap
static __inline__ _syscall1(__ptr_t, _mmap, unsigned long *, buffer)
__ptr_t mmap(__ptr_t addr, size_t len, int prot,
             int flags, int fd, __off_t offset)
{
    unsigned long buffer[6];

    buffer[0] = (unsigned long) addr;
    buffer[1] = (unsigned long) len;
    buffer[2] = (unsigned long) prot;
    buffer[3] = (unsigned long) flags;
    buffer[4] = (unsigned long) fd;
    buffer[5] = (unsigned long) offset;
    return (__ptr_t) _mmap(buffer);
}
#endif
libc_hidden_def (mmap)
#else
# error "Your architecture doesn't seem to provide mmap() !?"
#endif
