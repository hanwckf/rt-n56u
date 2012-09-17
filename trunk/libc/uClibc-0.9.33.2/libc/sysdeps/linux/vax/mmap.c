
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/syscall.h>

_syscall6 (void *, mmap, void *, start, size_t, length, int, prot, int, flags,
		int, fd, off_t, offset)
libc_hidden_def(mmap)

