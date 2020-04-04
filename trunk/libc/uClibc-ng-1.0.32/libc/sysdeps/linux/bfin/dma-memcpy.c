#include <unistd.h>
#include <errno.h>
#include <sys/syscall.h>
#include <bfin_sram.h>

_syscall3 (void *, dma_memcpy, void *, dest, const void *, src, size_t, len)
