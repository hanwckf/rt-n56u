/* Use new style mmap for microblaze */

#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/syscall.h>

_syscall6 (__ptr_t, mmap, __ptr_t, addr, size_t, len, int, prot,
	   int, flags, int, fd, __off_t, offset);
