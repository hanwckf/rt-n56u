/* vi: set sw=4 ts=4: */
/* Licensed under the LGPL v2.1, see the file LICENSE in this tarball. */

#include <sys/mman.h>
#include <sys/syscall.h>

#if defined __NR_madvise && defined __USE_XOPEN2K && defined __UCLIBC_HAS_ADVANCED_REALTIME__
int posix_madvise(void *addr, size_t len, int advice)
{
	int result;
	/* We have one problem: the kernel's MADV_DONTNEED does not
	 * correspond to POSIX's POSIX_MADV_DONTNEED.  The former simply
	 * discards changes made to the memory without writing it back to
	 * disk, if this would be necessary.  The POSIX behaviour does not
	 * allow this.  There is no functionality mapping for the POSIX
	 * behaviour so far so we ignore that advice for now. */
	if (advice == POSIX_MADV_DONTNEED)
		return 0;

	/* this part might use madvise function */
	INTERNAL_SYSCALL_DECL (err);
	result = INTERNAL_SYSCALL (madvise, err, 3, addr, len, advice);
	return INTERNAL_SYSCALL_ERRNO (result, err);
}
#endif
