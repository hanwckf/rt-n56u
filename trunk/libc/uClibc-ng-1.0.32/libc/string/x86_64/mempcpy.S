#define memcpy mempcpy
#define __memcpy_chk __mempcpy_chk
#include "memcpy.S"
libc_hidden_def(mempcpy)
