#include <unistd.h>
#include <errno.h>
#include <sys/syscall.h>
#include <bfin_sram.h>

_syscall1 (int, sram_free, const void *, addr)
