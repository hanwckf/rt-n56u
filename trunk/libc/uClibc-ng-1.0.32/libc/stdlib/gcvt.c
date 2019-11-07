#include <stdio.h>
#include <stdlib.h>


#ifdef __UCLIBC_HAS_FLOATS__
#define MAX_NDIGIT 17
char *gcvt (double number, int ndigit, char *buf)
{
    sprintf(buf, "%.*g", (ndigit > MAX_NDIGIT)? MAX_NDIGIT : ndigit, number);
    return buf;
}
#endif
