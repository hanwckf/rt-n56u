#define __FORCE_GLIBC
#include <features.h>
#include <netdb.h>
#undef h_errno

int * weak_const_function __h_errno_location (void)
{
    return &h_errno;
}

