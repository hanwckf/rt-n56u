#define USE_AS_STPCPY
#define STRCPY stpcpy
#include "strcpy.S"
libc_hidden_def(stpcpy)
