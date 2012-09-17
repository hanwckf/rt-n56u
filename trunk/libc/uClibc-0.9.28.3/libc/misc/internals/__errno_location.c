#include <errno.h>
#undef errno

int * weak_const_function __errno_location (void)
{
    return &errno;
}

