#include <unistd.h>
#include <errno.h>
#include <sys/syscall.h>
#include <bfin_sram.h>

_syscall2 (__ptr_t, sram_alloc, size_t, len, unsigned long, flags)
